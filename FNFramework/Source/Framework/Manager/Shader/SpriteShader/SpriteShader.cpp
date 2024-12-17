#include "SpriteShader.h"

#include "Framework/Graphics/Shape/Mesh/SpriteMesh.h"

void SpriteShader::DrawSprite(
    std::shared_ptr<SpriteMesh>& _vertex,
    const std::shared_ptr<ShaderResourceTexture>& _maskTex,
    const Math::Vector4&    _pixelPos,
    const std::shared_ptr<Math::Rectangle>&  _srcRect,
    const Math::Vector2&    _pivot)
{
	if (!_vertex) { FNENG_ASSERT_ERROR("頂点データが不正です"); return; }

    auto spTex = _vertex->GetMainTex();

    if(!spTex)
    {
        // テクスチャが向こうの場合は白テクスチャをセット
        spTex = GraphicsDevice::Instance().GetWhiteTex();
    }

    // メインテクスチャのセット
    spTex->Set(static_cast<int>(m_cbvCount));

    if(_maskTex)
    {
        _maskTex->Set(m_cbvCount + 1);
    }
    else
    {
        GraphicsDevice::Instance().GetWhiteTex()->Set(m_cbvCount + 1);
    }

    // 頂点の設定
    _vertex->SpriteVertexSetting(spTex, _pixelPos, _srcRect, _pivot);

    _vertex->DrawInstanced(_vertex->GetInstanceCount());
}

void SpriteShader::DrawSprite(std::shared_ptr<SpriteMesh>& _vertex,
    const std::shared_ptr<ShaderResourceTexture>& _maskTex,
    int x, int y, const std::shared_ptr<Math::Rectangle>& _srcRect,
    const Math::Vector2& _pivot)
{
    if (_vertex == nullptr)return;

    const auto& spTex = _vertex->GetMainTex();

    // スプライトデータ作成時に必要な情報の用意
    const Math::Vector4& pixelPos =
        {
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(spTex->GetWidth()),
            static_cast<float>(spTex->GetHeight())
        };

    DrawSprite(_vertex, _maskTex, pixelPos, _srcRect, _pivot);
}

bool SpriteShader::Begin()
{
    Shader::Begin(Screen::Width, Screen::Height);

    //---------------------
    // 定数バッファセット
    //---------------------

    // 2D用のプロジェクション行列はビューポート情報を利用するためローカルで作成する
    CBufferData::Camera camDat;

    // ビューポート情報を取得
    const D3D12_VIEWPORT& vp = GetViewPort();
    camDat.mViewProj = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(0, camDat);

    return true;
}

void SpriteShader::Init()
{
    //--------------------------------
    // 描画用パイプラインの準備
    //--------------------------------
    std::vector rangeTypes =
    {
        RangeType::CBV, // 0 : カメラ
        RangeType::CBV, // 1 : マテリアル
        RangeType::SRV,  // 0 : メインのテクスチャ
        RangeType::SRV   // 1 : マスクテクスチャ
    };

    // 描画設定
    RenderingSetting renderingSetting = {};
    renderingSetting.InputLayouts = { InputLayout::POSITION, InputLayout::TEXCOORD };
    renderingSetting.Formats = { DXGI_FORMAT_R8G8B8A8_UNORM };

    // 2Dスプライト用なので深度値は不用
    renderingSetting.IsDepth = false;
    renderingSetting.IsDepthMask = false;

    // 現在はテクスチャ色を参照していないため加算合成できません
    //renderingSetting.BlendMode = BlendMode::Add;

    Shader::Create(L"SpriteShader", renderingSetting, rangeTypes);
}
