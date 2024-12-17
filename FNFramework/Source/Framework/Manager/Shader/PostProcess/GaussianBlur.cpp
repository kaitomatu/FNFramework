#include "GaussianBlur.h"

void GaussianBlur::UpdateWeights(float _power)
{
    m_blurPower = _power;

    auto& cb = m_cbBlur.Work();

    float total = 0.0f;

    for(int i = 0; i < CBufferData::NUM_WEIGHTS; i++)
    {
        float weight = std::expf(-0.5f * static_cast<float>(i * i) / _power);
        cb.m_weights[i] = weight;

        total += 2.0f * weight;
    }

    // 正規化をする
    for (int i = 0; i < CBufferData::NUM_WEIGHTS; i++)
    {
        cb.m_weights[i] /= total;
    }
}

void GaussianBlur::Init(ShaderResourceTexture& _origTex)
{
    m_originalTexture = _origTex;

    UpdateWeights(20.0f);

    // 各種リソースの初期化 //
    // X方向のブラー用のリソースの初期化
    InitRenderingData(
        Math::Vector2{
        static_cast<float>(m_originalTexture.GetWidth()) / 2.0f,
        static_cast<float>(m_originalTexture.GetHeight())
        },
        L"XBlurShader",
        m_xBlurData);

    // Y方向のブラー用のリソースの初期化
    InitRenderingData(
        Math::Vector2{
        static_cast<float>(m_originalTexture.GetWidth()) / 2.0f,
        static_cast<float>(m_originalTexture.GetHeight()) / 2.0f
        },
        L"YBlurShader",
        m_yBlurData);
}

void GaussianBlur::Rendering()
{
    //--------------------------------
    //      X 方向のブラー処理      //
    //--------------------------------
    BeginBlurShader(m_xBlurData.Shader, m_xBlurData.spRenderTarget, m_originalTexture);

    m_cbBlur.Bind();

    // 描画 //
    m_xBlurData.spSpriteMesh->DrawInstanced();

    // レンダリングターゲットの書き込みを待つ
    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarrier(*m_xBlurData.spRenderTarget);

    //--------------------------------
    //      Y 方向のブラー処理      //
    //--------------------------------
    BeginBlurShader(m_yBlurData.Shader, m_yBlurData.spRenderTarget, m_xBlurData.spRenderTarget->GetTexture());

    m_cbBlur.Bind();

    // 描画
    m_yBlurData.spSpriteMesh->DrawInstanced();

    GraphicsDevice::Instance().FinishDrawingToRenderTargetResourceBarrier(*m_yBlurData.spRenderTarget);

}

void GaussianBlur::InitRenderingData(
    const Math::Vector2& _texSize,
    const std::wstring& _shaderName,
    BlurRenderingData& _blurRenderData)
{
    // スプライト用メッシュの初期化 //
    _blurRenderData.spSpriteMesh = std::make_shared<SpriteMesh>();

    _blurRenderData.spSpriteMesh->Create();
    _blurRenderData.spSpriteMesh->SpriteVertexSetting(
        { _texSize.x, _texSize.y },
        { 0.0f, 0.0f, _texSize.x, _texSize.y },
        nullptr,
        { 0.5f, 0.5f });

    // レンダーターゲットの初期化 //
    _blurRenderData.spRenderTarget = std::make_shared<RenderTarget>();

    _blurRenderData.spRenderTarget->Create(_texSize.x, _texSize.y,
        1, 1,
        m_originalTexture.GetFormat(),
        DXGI_FORMAT_UNKNOWN,
        Color::Gray,
        true);

    // シェーダーの初期化 //
    //--------------------------------
    // 描画用パイプラインの準備
    //--------------------------------
    std::vector rangeTypes =
    {
        RangeType::CBV, // 0 : スプライト行列用
        RangeType::CBV, // 1 : ブラー用定数バッファ
        RangeType::SRV, // 0 : ブラー元テクスチャ
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts = { InputLayout::POSITION, InputLayout::TEXCOORD };
    renderingSetting.Formats = { m_originalTexture.GetFormat() };

    // 2Dスプライト用なので深度値は不用
    renderingSetting.IsDepth = false;
    renderingSetting.IsDepthMask = false;

    _blurRenderData.Shader.Create(_shaderName, renderingSetting, rangeTypes);
}

void GaussianBlur::BeginBlurShader(
    Shader _shader,
    const std::shared_ptr<RenderTarget>& _spRT,
    const ShaderResourceTexture& _blurSrcTex
    )
{
    // RT のセット / クリア //
    GraphicsDevice::Instance().SetRenderTargetResourceBarrier(*_spRT);

    GraphicsDevice::Instance().SetRenderTarget(*_spRT);

    _spRT->ClearRTV();

    // シェーダーの開始 //
    _shader.Begin(
        _spRT->GetTexWidth(),
        _spRT->GetTexHeight());

    //---------------------
    // 定数バッファセット
    //---------------------
    // ビューポート情報からプロジェクション行列を作成 //
    const D3D12_VIEWPORT& vp = _shader.GetViewPort();
    Math::Matrix mSpriteProj = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, mSpriteProj);

    // ブラー元画像のセット
    _blurSrcTex.Set(_shader.GetCBVCount());
}
