#include "DeferredRenderingPass.h"

void GBufferPass::Init()
{
    m_spAlbedoGB = std::make_shared<RenderTarget>();
    m_spNormalGB = std::make_shared<RenderTarget>();
    m_spDepthGB = std::make_shared<RenderTarget>();

    // シャドウマップのサイズ
    constexpr Math::Vector2 ScreenSize = {
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height) };
    m_spAlbedoGB->Create(ScreenSize.x, ScreenSize.y,
        1, 1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        Color::Gray,
        true);
    
    m_spNormalGB->Create(ScreenSize.x, ScreenSize.y,
        1, 1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        Color::Gray,
        true);

    m_spDepthGB->Create(ScreenSize.x, ScreenSize.y,
        1, 1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN,
        Color::Gray,
        true);

    //////////////////////////////
    // シェーダーの作成
    //////////////////////////////
    // ※ RangeTypeは VS + PS で使用する定数バッファの数を指定する
    std::vector<RangeType> rangeTypes =
    {
        RangeType::CBV, // カメラ
        RangeType::CBV, // オブジェクト

        RangeType::SRV,
        RangeType::SRV
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

    renderingSetting.Formats =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R32_FLOAT
    };
    renderingSetting.RTVCount = 3;

    renderingSetting.UseInstanceData  = true;

    Shader::Create(L"GBufferPass", renderingSetting, rangeTypes);
}

bool GBufferPass::Begin()
{
    RenderTarget* renderTargets[] =
    {
        m_spAlbedoGB.get(),
        m_spNormalGB.get(),
        m_spDepthGB.get()
    };

    GraphicsDevice::Instance().SetRenderTargetResourceBarriers(3, renderTargets);

    GraphicsDevice::Instance().SetRenderTargets(3, renderTargets);

    m_spAlbedoGB->ClearRTV();
    m_spNormalGB->ClearRTV();
    m_spDepthGB->ClearRTV();

    Shader::Begin(
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height));

    //---------------------
    // 定数バッファセット
    //---------------------
    if (!ShaderManager::Instance().SetCBCameraData(0, RenderingData::MainCameraName)) { return false; }

    return true;
}

void GBufferPass::End()
{
    RenderTarget* renderTargets[] =
    {
        m_spAlbedoGB.get(),
        m_spNormalGB.get(),
        m_spDepthGB.get()
    };

    // レンダリングターゲットの書き込みを待つ
    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarriers(3, renderTargets);

    const D3D12_CPU_DESCRIPTOR_HANDLE& rtvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerRTV();
    const D3D12_CPU_DESCRIPTOR_HANDLE& dsvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerDSV();

    // レンダリングターゲットをバックバッファに戻す
    GraphicsDevice::Instance().SetRenderTarget(rtvH, dsvH);
}

void GBufferPass::DrawModelInstanced(
    const std::shared_ptr<ModelData>& modelData,
    const Renderer::InstancedRenderEntry& instanceDataEntry
    )
{
    const auto& instanceData = instanceDataEntry.InstanceDataList;
    const auto& modelWorkData = instanceDataEntry.ModelWorkList;

    if (!modelData || instanceData.empty() || modelWorkData.size() != instanceData.size())
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
        for (size_t instanceIdx = 0; instanceIdx < instanceData.size(); ++instanceIdx)
        {
            ModelWork* pModelWork = modelWorkData[instanceIdx];

            if(pModelWork->NeedCalcNodeMatrices())
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
    // const auto& meshNodeIndices = modelData->GetMeshNodeIdxList();

    // メッシュごとに描画
    for (const auto& meshIdx : meshNodeIndices)
    {
        const auto& dataNode = dataNodes[meshIdx];

        if (!dataNode.spMesh) { continue; }

        // メッシュへのポインタを取得
        const auto& mesh = dataNode.spMesh;

        // インスタンスバッファを更新
        //auto instanceList = InstanceDataList;
        //for(auto& data : instanceList)
        //{
        //    data.mWorld = dataNode.mWorldTransform * data.mWorld;
        //}

        mesh->UpdateInstanceBuffer(instanceData);

        // マテリアルの設定
        const auto& materials = modelData->GetMaterials();

        // サブセットごとに描画
        for (UINT subi = 0; subi < mesh->GetSubsets().size(); ++subi)
        {
            // マテリアルの設定
            SetMaterial(materials[mesh->GetSubsets()[subi].MaterialNo]);

            // インスタンス描画を行う
            mesh->DrawSubsetInstanced(subi, static_cast<UINT>(instanceData.size()));
        }
    }
}

void GBufferPass::UploadBoneMatrices(
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

void GBufferPass::BindBoneMatricesSRV(UINT _srvIdx)
{

    // SRVのGPUハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = GraphicsDevice::Instance().GetCBVSRVUAVHeap()->GetGPUHandle(_srvIdx);

    // コマンドリストにSRVをセット
    auto pCmdList = GraphicsDevice::Instance().GetCmdList();

    UINT rootParameterIndex = m_cbvCount + 1;

    pCmdList->SetGraphicsRootDescriptorTable(rootParameterIndex,
        GraphicsDevice::Instance().GetCBVSRVUAVHeap()->GetGPUHandle(_srvIdx));

}

void GBufferPass::SetMaterial(const Material& _material)
{
    const Material& material = _material;

    // ベースカラーテクスチャが設定されていなければ、白テクスチャをセットする
    material.spBaseColorTex
        ? material.spBaseColorTex->Set(m_cbvCount)
        : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount);

    // 法線用テクスチャが設定されていなければ、白テクスチャをセットする
    //material.spNormalTex
    //    ? material.spNormalTex->Set(m_cbvCount + 1)
    //    : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 1);

    //// 金属反射用テクスチャが設定されていなければ、白テクスチャをセットする
    //material.spMetallicRoughnessTex
    //    ? material.spMetallicRoughnessTex->Set(m_cbvCount + 2)
    //    : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 2);

    //// 発光テクスチャが設定されていなければ、白テクスチャをセットする
    //material.spEmissiveTex
    //    ? material.spEmissiveTex->Set(m_cbvCount + 3)
    //    : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 3);

    //-------------------------
    // 定数データへの書き込み
    //-------------------------
    //CBufferData::cbMaterial& materialData = m_cbMaterial.Work();
    //materialData.BaseColor = material.BaseColor;
    //materialData.Emissive = material.Emissive;
    //materialData.Metallic = material.Metallic;
    //materialData.Roughness = material.Roughness;

    //m_cbMaterial.Bind();
}

//x-------------------------------x//
//|       LightingPass   |//
//x-------------------------------x//

void LightingPass::Init()
{
    // シャドウマップのサイズ
    constexpr Math::Vector2 ScreenSize = {
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height) };

    m_spMainRenderTarget = std::make_shared<RenderTarget>();
    m_spMainRenderTarget->Create(ScreenSize.x, ScreenSize.y,
        1, 1,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_UNKNOWN,
        Color::Gray,
        true);
    
    //--------------------------------
    // 描画用パイプラインの準備
    //--------------------------------
    std::vector rangeTypes =
    {
        RangeType::CBV, // 0 : スプライト用
        RangeType::CBV, // 1 : カメラ
        RangeType::CBV, // 2 : ライト
        RangeType::CBV, // 3 : フォグ
        RangeType::CBV, // 4 : コースティクス
        RangeType::SRV, // 0 : アルベドテクスチャ
        RangeType::SRV, // 1 : ノーマルテクスチャ
        RangeType::SRV, // 2 : 深度テクスチャ
        RangeType::SRV, // 3 : シャドウマップ
        RangeType::SRV  // 4 : コースティクステクスチャ
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts = { InputLayout::POSITION, InputLayout::TEXCOORD };
    renderingSetting.Formats = { DXGI_FORMAT_R32G32B32A32_FLOAT };

    // 2Dスプライト用なので深度値は不用
    renderingSetting.IsDepth = false;
    renderingSetting.IsDepthMask = false;

    Shader::Create(L"LightingPass", renderingSetting, rangeTypes);

    m_spSpriteMesh = std::make_shared<SpriteMesh>();
    m_spSpriteMesh->Create();
    m_spSpriteMesh->SpriteVertexSetting(
        { Screen::Width, Screen::Height },
        { 0.0f, 0.0f,  Screen::Width, Screen::Height },
        nullptr,
        { 0.5f, 0.5f });
}

void LightingPass::Rendering()
{
    Begin();

    m_spSpriteMesh->DrawInstanced(m_spSpriteMesh->GetInstanceCount());

    End();
}

bool LightingPass::Begin()
{

    GraphicsDevice::Instance().SetRenderTargetResourceBarrier(*m_spMainRenderTarget);

    GraphicsDevice::Instance().SetRenderTarget(*m_spMainRenderTarget);

    m_spMainRenderTarget->ClearRTV();

    Shader::Begin(
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height));

    // GBufferのセット
    ShaderManager::Instance().GetGBufferPass()->GetAlbedoGB().Set(m_cbvCount);
    ShaderManager::Instance().GetGBufferPass()->GetNormalGB().Set(m_cbvCount + 1);
    ShaderManager::Instance().GetGBufferPass()->GetDepthGB().Set(m_cbvCount + 2);
    ShaderManager::Instance().GetShadowShader()->GetShadowMap()->GetTexture().Set(m_cbvCount + 3);

    // コースティクステクスチャのセット
    ShaderManager::Instance().GetAmbientManager()->GetCousticsTexture()->Set(m_cbvCount + 4);

    //---------------------
    // 定数バッファセット
    //---------------------

    // ビューポート情報からプロジェクション行列を作成 //
    const D3D12_VIEWPORT& vp = GetViewPort();
    Math::Matrix mSpriteProj = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, mSpriteProj);

    // カメラ情報のセット //
    if(!ShaderManager::Instance().SetCBCameraData(1, RenderingData::MainCameraName.data()))
    {
        return false;
    }

    // ライティングで利用する情報のセット //
    auto ligData = ShaderManager::Instance().GetAmbientManager()->GetLightCBData();
    auto fogData = ShaderManager::Instance().GetAmbientManager()->GetFogCBData();
    auto cousticsData = ShaderManager::Instance().GetAmbientManager()->GetCousticsCBData();

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(2, ligData);
    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(3, fogData);
    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(4, cousticsData);

    return true;
}

void LightingPass::End()
{
    // レンダリングターゲットの書き込みを待つ
    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarrier(*m_spMainRenderTarget);

    //// フレームバッファにRTの内容をコピーする //
    // ※ 現在は Format の設定が R32G32B32A32_FLOAT で、バックバッファと違うためコピーできないです。
    //const ShaderResourceTexture& mainTex = m_spMainRenderTarget->GetTexture();

    //GraphicsDevice::Instance().CopyToBackBuffer(mainTex);

    // レンダリングターゲットをバックバッファに戻す
    const D3D12_CPU_DESCRIPTOR_HANDLE& rtvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerRTV();
    const D3D12_CPU_DESCRIPTOR_HANDLE& dsvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerDSV();

    GraphicsDevice::Instance().SetRenderTarget(rtvH, dsvH);
}
