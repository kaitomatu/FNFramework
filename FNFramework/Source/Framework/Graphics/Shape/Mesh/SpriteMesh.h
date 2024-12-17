#pragma once

#include "../Vertices/Vertices.h"

struct SpriteMeshVertex
{
    Math::Vector3 Position = { 0.0f, 0.0f, -1.0f }; // 座標
    Math::Vector2 UV; // uv
};

/**
* @class SpriteMesh
* @brief スプライト描画用のメッシュクラス
* todo : 処理の更新順の依存関係が複雑になってしまったので、後で修正する
*   【現在の想定】
*       1. Create()でバッファの作成
*       2. SpriteVertexSetting()で描画情報の設定
*       3. UpdateBuffer()でバッファの更新
*/
class SpriteMesh
    : public Vertices
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
     * @fn GetInstanceCount()
     * @result インスタンス数
    */
    UINT GetInstanceCount() const
    {
        return m_instanceCount;
    }

    // メインテクスチャの取得 / 設定
    const std::shared_ptr<ShaderResourceTexture>& GetMainTex() const
    {
        return m_spMainTex;
    }
    void SetMainTex(const std::shared_ptr<ShaderResourceTexture>& _tex)
    {
        m_spMainTex = _tex;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
     * @fn DrawInstanced(UINT vertexCount) const
     * @brief インスタンス描画
     *
     * @param _vertexCount - 頂点数
     */
    void DrawInstanced(UINT _vertexCount) const override;
    void DrawInstanced() const;

    /**
     * todo : 結局描画時にRendererでデータを設定するので引数で持つのではなくメンバに持っていてもいいんじゃないのだろうか
     *
     * @fn UVSettings(float texWidth, float texHeight, const Math::Vector4& pixelPos, Math::Rectangle* srcRect)
     * @brief スプライト描画に必要な頂点情報の作成を行う
     * @param _tex       : メインテクスチャ
     * @param _pixelPos  : スクリーン空間のピクセル座標
     * @param _srcRect   : 矩形情報
     * @param _pivot     : 基準点
     * @return スプライト描画に必要な頂点情報
     */
    void SpriteVertexSetting(
        const std::shared_ptr<ShaderResourceTexture>& _tex,
        const Math::Vector4& _pixelPos,
        const std::shared_ptr<Math::Rectangle>& _srcRect,
        const Math::Vector2& _pivot);
    void SpriteVertexSetting(
        const Math::Vector2& _spriteSize,
        const Math::Vector4& _pixelPos,
        const std::shared_ptr<Math::Rectangle>& _srcRect,
        const Math::Vector2& _pivot);

    //--------------------------------
    // バッファの作成 / 更新
    //--------------------------------
    /**
     * @fn Create(GraphicsDevice* pGraphicsDevice, const std::vector<MeshVertex>& vertices,
     * @brief バッファの作成
     */
    void Create();

    /**
     * @fn UpdateBuffer(const std::vector<MeshVertex>& srcDatas)
     * @brief バッファの更新
     */
    void UpdateBuffer();

    void Release();

private:
    std::shared_ptr<ShaderResourceTexture> m_spMainTex = nullptr;          // 描画するテクスチャ(Texture)

    std::array<SpriteMeshVertex, 4> m_vertices; // 頂点情報

    UINT m_instanceCount = 0; // インスタンス数
};
