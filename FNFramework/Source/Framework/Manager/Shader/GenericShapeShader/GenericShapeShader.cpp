#include "GenericShapeShader.h"

void GenericShapeShader::Init()
{
    std::vector<RangeType> rangeTypes = {RangeType::CBV};

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts =
        {InputLayout::POSITION, InputLayout::TEXCOORD, InputLayout::COLOR};
    renderingSetting.Formats = { DXGI_FORMAT_R8G8B8A8_UNORM };
    renderingSetting.CullMode = CullMode::None;
    renderingSetting.TopologyType = PrimitiveTopologyType::Line;
    renderingSetting.IsWireFrame = false;

    Create(L"GenericShapeShader", renderingSetting, rangeTypes);
}


bool GenericShapeShader::Begin()
{
    Shader::Begin(Screen::Width, Screen::Height);

    //---------------------
    // 定数バッファセット
    //---------------------
    if (!ShaderManager::Instance().SetCBCameraData(0, RenderingData::MainCameraName)) { return false; }

    return true;
}
