#include "ModelShader_Unlit.h"

void ModelShader_Unlit::Init()
{
	std::vector<RangeType> rangeTypes =
	{
		RangeType::CBV,
		RangeType::CBV,
		RangeType::SRV
	};

	// 描画設定
	RenderingSetting renderingSetting = {};
	renderingSetting.InputLayouts =
	{ InputLayout::POSITION, InputLayout::TEXCOORD, InputLayout::COLOR };
	renderingSetting.Formats = { DXGI_FORMAT_R8G8B8A8_UNORM };

	Shader::Create(L"ModelShader_Unlit", renderingSetting, rangeTypes);
}

bool ModelShader_Unlit::Begin()
{
    Shader::Begin(Screen::Width, Screen::Height);

    //---------------------
    // 定数バッファセット
    //---------------------
    if (!ShaderManager::Instance().SetCBCameraData(0, RenderingData::MainCameraName)) { return false; }

    return true;
}

void ModelShader_Unlit::DrawModel(
    ModelWork& _modelData,
    const Math::Matrix& _mWorld,
    const Math::Vector4& _colRate)
{
    const auto& modelData = _modelData.GetModelData();
    auto modelNode = modelData->GetNodes();

    // ノード全てを描画
    for (const auto& meshIdx : modelData->GetDrawMeshNodeIdxList())
    {
        auto node = modelNode[meshIdx];

        // 複数のマテリアルを持つNodeの場合は、それぞれのマテリアルをセットして描画する
        if (!node.spMesh) { continue; }

        DrawMesh(
            *node.spMesh,
            node.mWorldTransform * _mWorld,
            modelData->GetMaterials(),
            _colRate);
    }
}

void ModelShader_Unlit::DrawMesh(
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
        _mesh.DrawSubset(subi);
    }
}

void ModelShader_Unlit::SetMaterial(const Material& _material)
{
    const Material& material = _material;
    // ベースカラーテクスチャが設定されていなければ、白テクスチャをセットする
    material.spBaseColorTex ?
        material.spBaseColorTex->Set(m_cbvCount) : GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount);
}
