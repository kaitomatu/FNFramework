#pragma once

#include "../BaseComponent.h"

/**
 * @brief LightComponent
 * @details
 *  ゲームオブジェクトにポイントライトを追加するコンポーネント。
 *  ライトの色、強度、範囲などを設定できる。
 */
class LightComponent
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
    LightComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    // 色変更のロック / ロック解除 //
    void ColorLock() { m_colorLock = true; }
    void DisableColorLock () { m_colorLock = false; }

    /* @brief ライトの色を取得 */
    const Math::Vector3& GetColor() const { return m_pointLight.Color; }
    /* @brief ライトの色を設定 */
    void SetColor(const Math::Vector3& color)
    {
        if (m_colorLock)
        {
            // FNENG_ASSERT_LOG("色の変更がLockされています", false);
            return;
        }

        m_pointLight.Color = color;
    }

    /* @brief ライトの強度を取得 */
    float GetIntensity() const { return m_pointLight.Intensity; }
    /* @brief ライトの強度を設定 */
    void SetIntensity(float intensity) { m_pointLight.Intensity = intensity; }

    /* @brief ライトの範囲を取得 */
    float GetRange() const { return m_pointLight.Range; }
    /* @brief ライトの範囲を設定 */
    void SetRange(float range) { m_pointLight.Range = range; }

    /* @brief ポイントライトのデータを取得 */
    const PointLight& GetPointLight() const { return m_pointLight; }
    void SetPointLight(const PointLight& pointLight) { m_pointLight = pointLight; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
     * @brief 生成時やシーンの初めに、1度だけ呼びだされる
     * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
     */
    void Awake() override;
    void Start() override;

    /* @brief 更新 */
    void Update() override;

    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

    void Release() override;

protected:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief ImGui 更新 */
    void ImGuiUpdate() override;

    //--------------------------------
    // メンバ変数
    //--------------------------------
    // ライトの位置オフセット
    Math::Vector3 m_posOffset = Math::Vector3::Zero;

    inline static float m_intensity = 1.0f; // ライトの強度

    PointLight m_pointLight; // ポイントライトのデータ

    // 色の変更を制限する
    bool m_colorLock = false;
};

namespace jsonKey::Comp
{
    namespace LightComponent
    {
        constexpr std::string_view PosOffset = "PosOffset";
        constexpr std::string_view Color = "Color";
        constexpr std::string_view Intensity = "Intensity";
        constexpr std::string_view Range = "Range";
    }
}
