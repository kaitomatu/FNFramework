#pragma once

#include "../../BaseComponent.h"

/**
* @class SpriteComponent
* @brief
* @details
*/
class SpriteComponent
    : public BaseComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    SpriteComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // ピクセル座標の設定 / 取得
    void SetPos(const Math::Vector2& pos)
    {
        m_spriteRenderingData.PixelPos.x = pos.x;
        m_spriteRenderingData.PixelPos.y = pos.y;
    }

    Math::Vector2 GetPos() const { return Math::Vector2{ m_spriteRenderingData.PixelPos.x, m_spriteRenderingData.PixelPos.y }; }

    void SetSize(const Math::Vector2& size)
    {
        m_spriteRenderingData.PixelPos.z = size.x;
        m_spriteRenderingData.PixelPos.w = size.y;
    }

    void SetSize(float size)
    {
        SetSize(Math::Vector2{ size, size });
    }

    Math::Vector2 GetSize() const { return Math::Vector2{ m_spriteRenderingData.PixelPos.z, m_spriteRenderingData.PixelPos.w }; }

    void SetPixelPos(const Math::Vector4& pos) { m_spriteRenderingData.PixelPos = pos; }
    const Math::Vector4& GetPixelPos() const { return m_spriteRenderingData.PixelPos; }

    // スプライト用のメッシュの設定 / 取得
    const std::shared_ptr<SpriteMesh>& GetSpriteMesh() const { return m_spriteRenderingData.Vertex; }
    void SetSpriteMesh(const std::shared_ptr<SpriteMesh>& mesh) { m_spriteRenderingData.Vertex = mesh; }

    // 矩形情報の設定 / 取得
    void SetSrcRect(const std::shared_ptr<Math::Rectangle>& rect) { m_spriteRenderingData.SrcRect = rect; }
    const std::shared_ptr<Math::Rectangle>& GetSrcRect() const { return m_spriteRenderingData.SrcRect; }

    // タイリング / オフセットの設定 / 取得
    void SetTilling(const Math::Vector2& tilling) { m_spriteRenderingData.Tilling = tilling; }
    const Math::Vector2& GetTilling() const { return m_spriteRenderingData.Tilling; }
    void SetOffset(const Math::Vector2& offset) { m_spriteRenderingData.Offset = offset; }
    const Math::Vector2& GetOffset() const { return m_spriteRenderingData.Offset; }

    // 色情報の設定 / 取得
    void SetColor(const Math::Color& color) { m_spriteRenderingData.Color = color; }
    const Math::Color& GetColor() const { return m_spriteRenderingData.Color; }
    void SetAlpha(float alpha) { m_spriteRenderingData.Color.w = alpha; }
    float GetAlpha() const { return m_spriteRenderingData.Color.w; }

    // ワールド行列の設定 / 取得
    void SetWorldMatrix(const Math::Matrix& matrix) { m_spriteRenderingData.WorldMatrix = matrix; }
    const Math::Matrix& GetWorldMatrix() const { return m_spriteRenderingData.WorldMatrix; }

    // ピボットの設定 / 取得
    void SetPivot(const Math::Vector2& pivot) { m_spriteRenderingData.Pivot = pivot; }
    const Math::Vector2& GetPivot() const { return m_spriteRenderingData.Pivot; }

    // テクスチャの設定
    void SetMainTexture(std::string_view _filePath)
    {
        SetMainTexture(AssetManager::Instance().GetTexture(_filePath.data()));
    }

    void SetMainTexture(const std::shared_ptr<ShaderResourceTexture>& _tex)
    {
        if (!m_spriteRenderingData.Vertex)
        {
            FNENG_ASSERT_LOG("メッシュの実体が作成されていません", false);
            m_spriteRenderingData.Vertex = std::make_shared<SpriteMesh>();
            m_spriteRenderingData.Vertex->Create();
        }

        m_spriteRenderingData.Vertex->SetMainTex(_tex);
    }

    // マスクテクスチャの設定 / 取得
    void SetMaskTexture(std::string_view _filePath)
    {
        SetMaskTexture(AssetManager::Instance().GetTexture(_filePath.data()));
    }

    void SetMaskTexture(const std::shared_ptr<ShaderResourceTexture>& tex)
    {
        m_spriteRenderingData.MaskTex = tex;
    }

    const std::shared_ptr<ShaderResourceTexture>& GetMaskTexture() const { return m_spriteRenderingData.MaskTex; }

    // イージングデータの設定 / 取得
    void SetEasingData(const MathHelper::Easing::EasingData& data) { m_easingData = data; }
    const MathHelper::Easing::EasingData& GetEasingData() const { return m_easingData; }

    // オーダー順の設定 / 取得
    void SetUpdateOrder(int order) { m_spriteRenderingData.Order = order; }
    int GetUpdateOrder() const { return m_spriteRenderingData.Order; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @fn void Awake()
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;

    void Release() override;

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    RenderingData::Sprite::Sprite m_spriteRenderingData;

    // イージングアニメーションデータ
    MathHelper::Easing::EasingData m_easingData;
};

namespace jsonKey::Comp
{
    namespace SpriteComponent
    {
        // テクスチャ関連
        constexpr std::string_view MainTexPath = "MainTexPath";
        constexpr std::string_view MaskTexPath = "MaskTexPath";

        // オーダー順
        constexpr std::string_view UpdateOrder = "UpdateOrder";

        // ピクセル座標
        constexpr std::string_view PixelPos = "PixelPos";
        constexpr std::string_view Size = "Size";
        constexpr std::string_view Pivot = "Pivot";

        // タイリング / オフセット
        constexpr std::string_view Tilling = "Tiling";
        constexpr std::string_view Offset = "Offset";

        // 色情報
        constexpr std::string_view Color = "Color";

    }
}
