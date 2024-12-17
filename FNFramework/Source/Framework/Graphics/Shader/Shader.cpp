#include "Shader.h"

void Shader::Create(const std::wstring& filePath,
    const RenderingSetting& renderingSetting, const std::vector<RangeType>& rangeTypes)
{
    // ファイルパスからパス名を取得
    utl::str::WideToSJis(m_shaderName, filePath);

    LoadShaderFile(filePath);

    // ルートシグネチャの作成
    m_spRootSignature = std::make_shared<RootSignature>();
    m_spRootSignature->Create(rangeTypes, m_cbvCount);

    // パイプラインステートの 設定 / 作成
    m_spPipeline = std::make_shared<PipeLine>();
    m_spPipeline->SetRenderSetting(m_spRootSignature.get(), renderingSetting.InputLayouts,
        renderingSetting.CullMode, renderingSetting.BlendMode,
        renderingSetting.TopologyType);
    m_spPipeline->Create({ m_pVSBlob, m_pHSBlob, m_pDSBlob, m_pGSBlob, m_pPSBlob }, renderingSetting.Formats,
        renderingSetting.IsDepth, renderingSetting.IsDepthMask, renderingSetting.DSVFormat , renderingSetting.RTVCount,
        renderingSetting.IsWireFrame, renderingSetting.UseInstanceData);
}

void Shader::Begin(float w, float h)
{
    const auto& pCmdList = GraphicsDevice::Instance().GetCmdList();

    pCmdList->SetPipelineState(m_spPipeline->GetPipeline());

    // ルートシグネチャのセット
    pCmdList->SetGraphicsRootSignature(m_spRootSignature->GetRootSignature());

    auto topologyType =
        static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(m_spPipeline->GetTopologyType());

    switch (topologyType)
    {
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT:
        pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        break;
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE:
        pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        break;
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
        pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        //m_pGraphicsDevice->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        break;
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH:
        pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
        break;
    default:
        break;
    }

    m_viewPort = {};
    m_rect = {};

    m_viewPort.Width = w;
    m_viewPort.Height = h;

    //
    m_viewPort.MinDepth = 0.0f;
    m_viewPort.MaxDepth = 1.0f;

    m_rect.right = static_cast<LONG>(w);
    m_rect.bottom = static_cast<LONG>(h);

    GraphicsDevice::Instance().GetCmdList()->RSSetViewports(1, &m_viewPort);
    GraphicsDevice::Instance().GetCmdList()->RSSetScissorRects(1, &m_rect);
}

void Shader::LoadShaderFile(const std::wstring& filePath)
{
    auto include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
    UINT flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    ID3DBlob* pErrorBlob = nullptr;

    // todo : マクロを使ってファイルパスを設定するようにする
    std::wstring currentPath = L"Assets/Data/Shader/" + filePath + L"/";
    std::wstring format = L".hlsl";

    // 頂点シェーダーのコンパイル
    {
        std::wstring fullFilepath = currentPath + filePath + L"_VS" + format;

        auto hr = D3DCompileFromFile(fullFilepath.c_str(), nullptr, include, "main",
            "vs_5_0", flag, 0, &m_pVSBlob, &pErrorBlob);

        if (FAILED(hr))
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
            {
                FNENG_ASSERT_ERROR(m_shaderName + ":頂点シェーダーのファイルパスが間違っています");
                return;
            }
            FNENG_ASSERT_ERROR(m_shaderName + ":頂点シェーダーのコンパイルに失敗しました。");
            FNENG_ASSERT_ERROR_BLOB(pErrorBlob);
            return;
        }
    }

    // ハルシェーダーのコンパイル
    {
        std::wstring fullFilepath = currentPath + filePath + L"_HS" + format;

        D3DCompileFromFile(fullFilepath.c_str(), nullptr, include, "main",
            "hs_5_0", flag, 0, &m_pHSBlob, &pErrorBlob);
    }

    // ドメインシェーダーのコンパイル
    {
        std::wstring fullFilepath = currentPath + filePath + L"_DS" + format;

        D3DCompileFromFile(fullFilepath.c_str(), nullptr, include, "main",
            "ds_5_0", flag, 0, &m_pDSBlob, &pErrorBlob);
    }

    // ジオメトリシェーダーのコンパイル
    {
        std::wstring fullFilepath = currentPath + filePath + L"_GS" + format;

        D3DCompileFromFile(fullFilepath.c_str(), nullptr, include, "main",
            "gs_5_0", flag, 0, &m_pGSBlob, &pErrorBlob);
    }

    // ピクセルシェーダーのコンパイル
    {
        std::wstring fullFilepath = currentPath + filePath + L"_PS" + format;

        auto hr = D3DCompileFromFile(fullFilepath.c_str(), nullptr, include, "main",
            "ps_5_0", flag, 0, &m_pPSBlob, &pErrorBlob);

        if (FAILED(hr))
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
            {
                FNENG_ASSERT_ERROR(m_shaderName + ":ピクセルシェーダーのファイルパスが見つかりませんでした。");
                return;
            }
            FNENG_ASSERT_ERROR(m_shaderName + ":ピクセルシェーダーのコンパイルに失敗しました。");
            FNENG_ASSERT_ERROR_BLOB(pErrorBlob);
        }
    }
}
