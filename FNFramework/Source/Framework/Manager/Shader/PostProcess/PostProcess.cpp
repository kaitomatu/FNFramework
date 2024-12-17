#include "PostProcess.h"

void BloomPass::Init()
{
    m_spSpriteMesh = std::make_shared<SpriteMesh>();
    m_spSpriteMesh->Create();
    m_spSpriteMesh->SpriteVertexSetting(
        { Screen::Width, Screen::Height },
        { 0.0f, 0.0f,  Screen::Width, Screen::Height },
        nullptr,
        { 0.5f, 0.5f });

    InitLuminanceShader();

    m_gaussianBlur.Init(m_spLuminanceRenderTarget->WorkTexture());

    InitBloomShader();
    InitToneMappingShader();
}

void BloomPass::InitLuminanceShader()
{
    constexpr Math::Vector2 ScreenSize = {
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height) };

    m_spLuminanceRenderTarget = std::make_shared<RenderTarget>();
    m_spLuminanceRenderTarget->Create(ScreenSize.x, ScreenSize.y,
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
        RangeType::CBV, // 1 : ブルーム用
        RangeType::SRV  // 0 : メインレンダーターゲットテクスチャ
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts = { InputLayout::POSITION, InputLayout::TEXCOORD };
    renderingSetting.Formats = { DXGI_FORMAT_R32G32B32A32_FLOAT };

    // 2Dスプライト用なので深度値は不用
    renderingSetting.IsDepth = false;
    renderingSetting.IsDepthMask = false;

    m_luminanceShader.Create(L"LuminanceShader", renderingSetting, rangeTypes);
}

void BloomPass::InitBloomShader()
{
    m_spBloomRenderTarget = std::make_shared<RenderTarget>();
    m_spBloomRenderTarget->Create(Screen::Width, Screen::Height,
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
        RangeType::SRV, // 0 : 輝度ぼかしテクスチャ
        RangeType::SRV  // 1 : メインレンダーターゲットテクスチャ
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts = { InputLayout::POSITION, InputLayout::TEXCOORD };
    renderingSetting.Formats = { DXGI_FORMAT_R32G32B32A32_FLOAT };

    // 2Dスプライト用なので深度値は不用
    renderingSetting.IsDepth = false;
    renderingSetting.IsDepthMask = false;

    m_bloomShader.Create(L"BloomShader", renderingSetting, rangeTypes);
}

void BloomPass::Rendering()
{
    RenderingLuminancePass();

    // 輝度抽出後、ガウシアンブラーを適用する //
    m_gaussianBlur.Rendering();

    // ブルーム処理を行う //
    RenderingBloomPass();

    //---------------------
    // バックバッファに切り替え
    //---------------------
    const D3D12_CPU_DESCRIPTOR_HANDLE& rtvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerRTV();
    const D3D12_CPU_DESCRIPTOR_HANDLE& dsvH = GraphicsDevice::Instance().GetCurrentFrameBuffuerDSV();

    GraphicsDevice::Instance().SetRenderTarget(rtvH, dsvH);

    //x---------------------x//
    //   トーンマッピング    //
    //x---------------------x//
    RenderingToneMappingPass();
}

void BloomPass::RenderingLuminancePass()
{
    // 輝度抽出 //
    GraphicsDevice::Instance().SetRenderTargetResourceBarrier(*m_spLuminanceRenderTarget);

    GraphicsDevice::Instance().SetRenderTarget(*m_spLuminanceRenderTarget);

    m_spLuminanceRenderTarget->ClearRTV();

    m_luminanceShader.Begin(
        m_spLuminanceRenderTarget->GetTexWidth(),
        m_spLuminanceRenderTarget->GetTexHeight());

    // シーンテクスチャのセット
    ShaderManager::Instance().GetLightingPass()->GetMainRenderTexture().Set(m_luminanceShader.GetCBVCount());

    //---------------------
    // 定数バッファセット
    //---------------------

    // ビューポート情報からプロジェクション行列を作成 //
    const D3D12_VIEWPORT& vp = m_luminanceShader.GetViewPort();
    Math::Matrix mSpriteProj = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, mSpriteProj);

    m_cbBloom.Bind();

    // スプライトの描画 //
    m_spSpriteMesh->DrawInstanced(m_spSpriteMesh->GetInstanceCount());

    // レンダリングターゲットの書き込みを待つ
    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarrier(*m_spLuminanceRenderTarget);
}

void BloomPass::RenderingBloomPass()
{
    // ブルーム処理 //
    // RTのセット
    GraphicsDevice::Instance().SetRenderTargetResourceBarrier(*m_spBloomRenderTarget);
    GraphicsDevice::Instance().SetRenderTarget(*m_spBloomRenderTarget);

    m_spBloomRenderTarget->ClearRTV();

    // ブルームシェーダーの設定開始
    m_bloomShader.Begin(
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height));

    // テクスチャのセット（輝度抽出後の結果を使用）
    m_gaussianBlur.GetBlurTexture().Set(m_bloomShader.GetCBVCount());
    // メインレンダーターゲットの取得
    auto& mainTex = ShaderManager::Instance().GetLightingPass()->GetMainRenderTexture();
    mainTex.Set(m_bloomShader.GetCBVCount() + 1);

    //---------------------
    // 定数バッファの設定
    //---------------------
    // ビューポート情報からプロジェクション行列を作成
    const D3D12_VIEWPORT& vp = m_bloomShader.GetViewPort();
    Math::Matrix mSpriteProj = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);
    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, mSpriteProj);

    // スプライトの描画
    m_spSpriteMesh->DrawInstanced(m_spSpriteMesh->GetInstanceCount());

    // レンダリングターゲットの書き込みを待つ
    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarrier(*m_spBloomRenderTarget);
}

void BloomPass::InitToneMappingShader()
{
    //--------------------------------
    // 描画用パイプラインの準備
    //--------------------------------
    std::vector rangeTypes =
    {
        RangeType::CBV, // 0 : スプライト用
        RangeType::CBV, // 1 : トーンマッピング用
        RangeType::SRV, // 0 : トーンマッピング元テクスチャ
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts = { InputLayout::POSITION, InputLayout::TEXCOORD };
    renderingSetting.Formats = { DXGI_FORMAT_R8G8B8A8_UNORM };

    // 2Dスプライト用なので深度値は不用
    renderingSetting.IsDepth = false;
    renderingSetting.IsDepthMask = false;

    m_toneMappingShader.Create(L"ToneMappingShader", renderingSetting, rangeTypes);
}

void BloomPass::RenderingToneMappingPass()
{
    m_toneMappingShader.Begin(
        static_cast<float>(Screen::Width),
        static_cast<float>(Screen::Height));

    m_cbToneMappingParam.Bind();

    // トーンマッピング元テクスチャのセット
    m_spBloomRenderTarget->GetTexture().Set(m_toneMappingShader.GetCBVCount());

    //---------------------
    // 定数バッファセット
    //---------------------

    // ビューポート情報からプロジェクション行列を作成 //
    const D3D12_VIEWPORT& vp = m_toneMappingShader.GetViewPort();
    Math::Matrix mSpriteProj = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, mSpriteProj);

    // スプライトの描画
    m_spSpriteMesh->DrawInstanced(m_spSpriteMesh->GetInstanceCount());
}
