#pragma once

class SpriteMesh;

class SpriteShader
	:public Shader
{
public:

	//--------------------------------
	// コンストラクタ / デストラクタ
	//--------------------------------
	SpriteShader() { Init(); }

    void SetCBSpriteData(
        const Math::Color& color,
        const Math::Matrix& worldMatrix,
        const Math::Vector2& tilling,
        const Math::Vector2& offset
    )
    {
        auto& cbWork = m_cbSpriteObject.Work();
        cbWork.mWorld = worldMatrix ;
        cbWork.Color = color;
        cbWork.Tiling = tilling;
        cbWork.Offset = offset;

        m_cbSpriteObject.Bind();
    }

    void DrawSprite(
        std::shared_ptr<SpriteMesh>& _vertex,        // 描画用頂点配列
        const std::shared_ptr<ShaderResourceTexture>& _maskTex,
        const Math::Vector4& _pixelPos,             // x, y, w, h座標(ピクセル)
        const std::shared_ptr<Math::Rectangle>& _srcRect = nullptr,  // 元画像のRECT : nullptr で上の x, y, h ,w で指定した範囲が描画される
        const Math::Vector2& _pivot = { 0.5, 0.5f });       // 基準点 0.0～1.0の範囲で指定する : {0.5, 0.5} で中心


    void DrawSprite(
        std::shared_ptr<SpriteMesh>& _vertex,
        const std::shared_ptr<ShaderResourceTexture>& _maskTex,
        int _x, int _y,
        const std::shared_ptr<Math::Rectangle>& _srcRect = nullptr,
        const Math::Vector2& _pivot = { 0.5, 0.5f });

    bool Begin() override;

private:

	//--------------------------------
	// その他関数
	//--------------------------------
	/* @brief 初期化 */
	void Init();

    ConstantBuffer<CBufferData::cbSpriteObject> m_cbSpriteObject = { 1 };
};
