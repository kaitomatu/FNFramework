#pragma once

#include "LightComponent.h"

/**
 * @brief LightAnimationScript
 * @details
 *  ゲームオブジェクトにポイントライトを追加するコンポーネント。
 *  ライトの色、強度、範囲などを設定できる。
 */
class LightAnimationScript
    : public LightComponent
{
public:

    // ライトで変更するアニメーションの種類
    enum LightAnimationType
    {
        ePositionXToEase = 1 << 0, // 位置のイージング
        ePositionYToEase = 1 << 1, // 位置のイージング
        ePositionZToEase = 1 << 2, // 位置のイージング
        eIntensityToEase = 1 << 3, // 強度のイージング
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
     * @brief コンストラクタ
     * @param[in] owner - オーナーオブジェクトのポインタ
     * @param[in] name - コンポーネントの名前
     * @param[in] _enableSerialize - シリアライズをするかどうか
     */
    LightAnimationScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : LightComponent(owner, name, _enableSerialize)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

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

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief ImGui 更新 */
    void ImGuiUpdate() override;

    //--------------------------------
    // メンバ変数
    //--------------------------------
    Math::Vector2 m_intensityMinMax;

    UINT m_lightAnimationType; // ライトのアニメーションの種類
    MathHelper::Easing::EasingData m_easingData; // イージングデータ
};

namespace jsonKey::Comp
{
    namespace LightAnimationScript
    {
        constexpr std::string_view LightAnimationType = "LightAnimationType";
        constexpr std::string_view IntensityMinMax = "IntensityMinMax";
    }
}
