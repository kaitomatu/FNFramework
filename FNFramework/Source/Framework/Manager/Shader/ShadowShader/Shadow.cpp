#include "Shadow.h"

bool Shadow::Begin()
{
    // レンダリングターゲットとして利用できるようにバリアを張る
    GraphicsDevice::Instance().SetRenderTargetResourceBarrier(*m_spShadowMap);

    // レンダーターゲットを0番目に設定
    GraphicsDevice::Instance().SetRenderTarget(*m_spShadowMap);

    m_spShadowMap->ClearDSV();
    m_spShadowMap->ClearRTV();

    Shader::Begin(static_cast<float>(m_spShadowMap->GetTexWidth()),
        static_cast<float>(m_spShadowMap->GetTexHeight()));

    //---------------------
    // 定数バッファセット
    //---------------------
    auto ligCam = ShaderManager::Instance().FindCameraData(RenderingData::LightCameraName);
    SetCBShadowArea(ligCam->GetProjMat(), ligCam->GetCBData().Height);

    CBufferData::Camera camDat;
    camDat.mViewProj = ShaderManager::Instance().GetAmbientManager()->GetLightCBData().DirLight_mVP;

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, camDat);

    return true;
}

void Shadow::End()
{

    // レンダリングターゲットの書き込みを待つ
    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarrier(*m_spShadowMap);

    const D3D12_CPU_DESCRIPTOR_HANDLE& rtvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerRTV();
    const D3D12_CPU_DESCRIPTOR_HANDLE& dsvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerDSV();

    // レンダリングターゲットをバックバッファに戻す
    GraphicsDevice::Instance().SetRenderTarget(rtvH, dsvH);
}

void Shadow::Init()
{
	//////////////////////////////
	// シャドウマップの作成
	//////////////////////////////
	if(!CreateShadowMap())
	{
		FNENG_ASSERT_ERROR("シャドウマップの作成に失敗しました。");
		return;
	}

	//////////////////////////////
	// シェーダーの作成
	//////////////////////////////
    // ※ RangeTypeは VS + PS で使用する定数バッファの数を指定する
	std::vector<RangeType> rangeTypes =
	{
		RangeType::CBV, // カメラ
		RangeType::CBV, // オブジェクト
		RangeType::SRV, // ボーン
		RangeType::SRV, // ボーン
	};

	// 描画設定
	RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts =
    {
        InputLayout::POSITION,
        InputLayout::TEXCOORD,
        InputLayout::COLOR,
        InputLayout::NORMAL,
        InputLayout::TANGENT,
        InputLayout::SKININDEX,
        InputLayout::SKINWEIGHT
    };
	renderingSetting.Formats = { DXGI_FORMAT_R32_FLOAT };
	renderingSetting.RTVCount = 1;

    renderingSetting.UseInstanceData = true;

	Shader::Create(L"Shadow", renderingSetting, rangeTypes);
}

void Shadow::DrawModelInstanced(const std::shared_ptr<ModelData>& modelData,
    const std::vector<InstanceData>& instanceDataList, const std::vector<ModelWork*>& modelWorks)
{
    if (!modelData || instanceDataList.empty() || modelWorks.size() != instanceDataList.size())
    {
        return;
    }

    // 全てのメッシュノードを取得
    const auto& dataNodes = modelData->GetNodes();

    // スキンメッシュかどうかの判定
    bool isSkinMesh = modelData->IsSkinMesh();

    // インスタンスごとのボーンデータを収集
    std::vector<Math::Matrix> allBoneMatrices;
    UINT bonesPerInstance = static_cast<UINT>(modelData->GetBoneNodeIdxList().size()); // 1インスタンスあたりのボーン数

    if (isSkinMesh && bonesPerInstance > 0)
    {
        // 各インスタンスのボーンデータを収集
        for (size_t instanceIdx = 0; instanceIdx < instanceDataList.size(); ++instanceIdx)
        {
            ModelWork* pModelWork = modelWorks[instanceIdx];

            if (pModelWork->NeedCalcNodeMatrices())
            {
                pModelWork->CalcNodeMatrices();
            }

            const auto& workNodes = pModelWork->WorkNodes();

            // 各インスタンスのボーンデータを計算
            for (auto&& nodeIdx : modelData->GetBoneNodeIdxList())
            {
                if (nodeIdx >= RenderingData::MaxBoneNum)
                {
                    break;
                }

                const auto& dataNode = pModelWork->GetDataNodes()[nodeIdx];
                const auto& workNode = workNodes[nodeIdx];

                // ボーン行列を計算
                Math::Matrix boneMatrix = dataNode.Bone.OffsetMatrix * workNode.mWorldTransform;

                allBoneMatrices.push_back(boneMatrix);
            }
        }

        // ボーン行列をGPUにアップロード
        UploadBoneMatrices(
            allBoneMatrices,
            bonesPerInstance,
            m_umModelDataToBoneData[modelData]
        );

        // SRVをシェーダーにバインド
        BindBoneMatricesSRV(m_umModelDataToBoneData[modelData].BoneMatrixSRVIndex);

        m_cbObject.Work().IsSkin = true;
        m_cbObject.Work().BonePerInstance = static_cast<float>(bonesPerInstance);
    }
    else
    {
        m_cbObject.Work().BonePerInstance = 0.0f;
        m_cbObject.Work().IsSkin = false;
    }
    m_cbObject.Bind();

    // 描画対象のメッシュノードのインデックスを取得
    const auto& meshNodeIndices = modelData->GetDrawMeshNodeIdxList();

    // メッシュごとに描画
    for (const auto& meshIdx : meshNodeIndices)
    {
        const auto& dataNode = dataNodes[meshIdx];

        if (!dataNode.spMesh) { continue; }

        // メッシュへのポインタを取得
        const auto& mesh = dataNode.spMesh;

        mesh->UpdateInstanceBuffer(instanceDataList);

        // サブセットごとに描画
        for (UINT subi = 0; subi < mesh->GetSubsets().size(); ++subi)
        {
            // インスタンス描画を行う
            mesh->DrawSubsetInstanced(subi, static_cast<UINT>(instanceDataList.size()));
        }
    }
}

void Shadow::SetCBShadowArea(const Math::Matrix& mProj, float ligHeight)
{
    // メインカメラの情報を取得
    const std::shared_ptr<Camera>& mainCamera = ShaderManager::Instance().FindCameraData(RenderingData::MainCameraName);

    if (!mainCamera)
    {
        FNENG_ASSERT_ERROR("カメラ情報が登録されていませんでした");
        return;
    }

    // 影生成の開始位置はメインカメラ(今見ている視点)の位置
    Math::Vector3 ligPos = mainCamera->GetCBData().CamPos;

    auto& cbLidhtData = ShaderManager::Instance().GetAmbientManager()->WorkLightCBData();

    // 影の向きは平行光の向き
    Math::Vector3 ligDir = cbLidhtData.LigDirection;
    Math::Vector3 up = (ligDir == Math::Vector3::Up) ? Math::Vector3::Right : Math::Vector3::Up;

    // ライトの位置をカメラの位置から算出
    cbLidhtData.DirLight_mVP = XMMatrixLookAtLH(ligPos - ligDir * ligHeight, ligPos, up);
    cbLidhtData.DirLight_mVP *= mProj;
}

bool Shadow::SetCBShadowAreaData(std::string_view camName)
{
    //---------------------
    // 定数バッファセット
    //---------------------
    // カメラ情報があればシェーダーにセットする
    const std::shared_ptr<Camera>& camDat = ShaderManager::Instance().FindCameraData(camName);

    if (!camDat)
    {
        FNENG_ASSERT_ERROR("カメラ情報が登録されていませんでした");
        return false;
    }

    auto& cbLightData = ShaderManager::Instance().GetAmbientManager()->WorkLightCBData();
    cbLightData.DirLight_mVP = camDat->GetCBData().mViewProj;

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, cbLightData);

    return true;
}

bool Shadow::CreateShadowMap()
{
	m_spShadowMap = std::make_shared<RenderTarget>();

    // シャドウマップのサイズ
	constexpr std::pair shadowMapSize = { 4096, 4096 };
	m_spShadowMap->Create(shadowMapSize.first, shadowMapSize.second,
	    1, 1,
	    DXGI_FORMAT_R32_FLOAT,
	    DXGI_FORMAT_R32_TYPELESS,
	    Color::Red,
	    true);

	return true;
}

void Shadow::UploadBoneMatrices(
    const std::vector<Math::Matrix>& allBoneMatrices,
    UINT numBonesPerInstance,
    GPUBoneData& _boneData)
{
    // バッファのサイズを計算
    UINT bufferSize = static_cast<UINT>(allBoneMatrices.size() * sizeof(Math::Matrix));

    // バッファサイズが変更された場合のみ再作成
    if (bufferSize > _boneData.BoneMatrixBufferSize)
    {
        // 古いバッファを解放
        _boneData.pBoneMatrixBuffer.Reset();

        // ボーン行列用のバッファリソースを作成
        auto device = GraphicsDevice::Instance().GetDevice();
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        HRESULT hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&_boneData.pBoneMatrixBuffer));

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR("ボーン行列バッファの作成に失敗しました");
            return;
        }

        _boneData.BoneMatrixBufferSize = bufferSize;

        // ヒープに SRV を作成
        UINT numElements = static_cast<UINT>(allBoneMatrices.size());
        UINT structureByteStride = sizeof(Math::Matrix);
        _boneData.BoneMatrixSRVIndex = GraphicsDevice::Instance().GetCBVSRVUAVHeap()->CreateStructuredBufferSRV(
            _boneData.pBoneMatrixBuffer.Get(),
            numElements,
            structureByteStride,
            true);
    }

    // データをコピー
    void* pData = nullptr;
    HRESULT hr = _boneData.pBoneMatrixBuffer->Map(0, nullptr, &pData);
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("ボーン行列バッファのマッピングに失敗しました");
        return;
    }
    memcpy(pData, allBoneMatrices.data(), bufferSize);
    _boneData.pBoneMatrixBuffer->Unmap(0, nullptr);
}

void Shadow::BindBoneMatricesSRV(UINT _srvIdx)
{
    // SRVのGPUハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = GraphicsDevice::Instance().GetCBVSRVUAVHeap()->GetGPUHandle(_srvIdx);

    // コマンドリストにSRVをセット
    auto pCmdList = GraphicsDevice::Instance().GetCmdList();

    UINT rootParameterIndex = m_cbvCount + 1;

    pCmdList->SetGraphicsRootDescriptorTable(rootParameterIndex,
        GraphicsDevice::Instance().GetCBVSRVUAVHeap()->GetGPUHandle(_srvIdx));
}
