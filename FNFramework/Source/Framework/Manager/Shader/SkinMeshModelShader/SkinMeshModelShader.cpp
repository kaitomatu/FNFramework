#include "SkinMeshModelShader.h"

bool SkinMeshModelShader::Begin()
{

    Shader::Begin(Screen::Width, Screen::Height);

    //---------------------
    // SRVをセット
    //---------------------
    //ShaderManager::Instance().GetShadowShader()->GetShadowMap()->SetRootDescriptorTable(*this, 4);

    //---------------------
    // 定数バッファセット
    //---------------------
    if (!ShaderManager::Instance().SetCBCameraData(0, RenderingData::MainCameraName)) { return false; }

    auto ligData = ShaderManager::Instance().GetAmbientManager()->GetLightCBData();
    auto fogData = ShaderManager::Instance().GetAmbientManager()->GetFogCBData();

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(3, ligData);
    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(4, fogData);

    return true;
}

void SkinMeshModelShader::DrawMesh(
    const Mesh& _mesh,
    const Math::Matrix& _mWorld,
    const std::vector<Material>& _materials,
    const Math::Vector4& _colRate)
{
    m_cbObject.Work().mWorld = _mWorld;
    m_cbObject.Bind();

    // 全サブセットを描画
    for (UINT subi = 0; subi < _mesh.GetSubsets().size(); ++subi)
    {
        // 設定されたマテリアルをセット
        SetMaterial(_materials[_mesh.GetSubsets()[subi].MaterialNo], _colRate);

        _mesh.DrawSubset(subi);
    }
}

void SkinMeshModelShader::DrawModel(
    ModelWork& _modelData,
    const Math::Matrix& _mWorld,
    const Math::Vector4& _colRate)
{
    if (!_modelData.IsEnable()) { return; }

    const std::shared_ptr<ModelData>& spOrigModelData = _modelData.GetModelData();

    // モデルが無効な場合はスキップ
    if (!spOrigModelData)
    {
        FNENG_ASSERT_ERROR("モデルデータが不正です");
        return;
    }

    // 行列の更新が必要な場合は、ここで行う
    if (_modelData.NeedCalcNodeMatrices())
    {
        _modelData.CalcNodeMatrices();
    }

    auto& dataNodes = spOrigModelData->GetNodes();
    auto& workNodes = _modelData.WorkNodes();

    //// ノード内からボーン情報を取得
    //for (auto&& nodeIdx : spOrigModelData->GetBoneNodeIdxList())
    //{
    //    // 転送できるボーンの数を超えたら警告
    //    if (nodeIdx >= RenderingData::MaxBoneNum)
    //    {
    //        //FNENG_ASSERT_ERROR(" : 登録できるボーン数を超えました")
    //        break;
    //    }

    //    auto& dataNode = dataNodes[nodeIdx];
    //    auto& workNnode = workNodes[nodeIdx];

    //    m_cbBones.Work().mBone[dataNode.Bone.Index] = dataNode.Bone.OffsetMatrix * workNnode.mWorldTransform;
    //}

    //m_cbBones.Bind();

    // ノード全てを描画
    for (const auto& meshIdx : spOrigModelData->GetDrawMeshNodeIdxList())
    {
        const auto& dataNode = dataNodes[meshIdx];
        const auto& workNnode = workNodes[meshIdx];

        if (!dataNode.spMesh) { continue; }

        DrawMesh(
            *dataNode.spMesh,
            workNnode.mWorldTransform * _mWorld,
            spOrigModelData->GetMaterials(),
            _colRate);
    }
}

void SkinMeshModelShader::SetMaterial(const Material& _material, const Math::Vector4& colRate)
{
    const Material& material = _material;

    // ベースカラーテクスチャが設定されていなければ、白テクスチャをセットする
    material.spBaseColorTex
        ? material.spBaseColorTex->Set(m_cbvCount)
        : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount);

    // 法線用テクスチャが設定されていなければ、白テクスチャをセットする
    material.spNormalTex
        ? material.spNormalTex->Set(m_cbvCount + 1)
        : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 1);

    // 金属反射用テクスチャが設定されていなければ、白テクスチャをセットする
    material.spMetallicRoughnessTex
        ? material.spMetallicRoughnessTex->Set(m_cbvCount + 2)
        : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 2);

    // 発光テクスチャが設定されていなければ、白テクスチャをセットする
    material.spEmissiveTex
        ? material.spEmissiveTex->Set(m_cbvCount + 3)
        : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 3);

    //-------------------------
    // 定数データへの書き込み
    //-------------------------
    CBufferData::cbMaterial& materialData = m_cbMaterial.Work();
    materialData.BaseColor = material.BaseColor * colRate;
    materialData.Emissive = material.Emissive;
    materialData.Metallic = material.Metallic;
    materialData.Roughness = material.Roughness;

    m_cbMaterial.Bind();
}

void SkinMeshModelShader::Init()
{
    std::vector<RangeType> rangeTypes =
    {
        RangeType::CBV, // カメラ
        RangeType::CBV, // マテリアル
        RangeType::CBV, // オブジェクト(行列)
        RangeType::CBV, // ライト
        RangeType::CBV, // ボーン
        RangeType::CBV, // フォグ
        // モデル用SRV
        RangeType::SRV, // アルベド
        RangeType::SRV, // 法線
        RangeType::SRV, // メタリック
        RangeType::SRV, // ラフネス
        // 特殊処理SRV
        RangeType::SRV, // 影
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts =
    {
        InputLayout::POSITION, InputLayout::TEXCOORD,
        InputLayout::COLOR, InputLayout::NORMAL,
        InputLayout::TANGENT,
        InputLayout::SKININDEX, InputLayout::SKINWEIGHT
    };
    renderingSetting.Formats = { DXGI_FORMAT_R8G8B8A8_UNORM };

    Create(L"ModelShader_SkinMesh", renderingSetting, rangeTypes);
}
