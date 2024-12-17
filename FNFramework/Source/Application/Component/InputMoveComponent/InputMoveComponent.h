#pragma once

#include "../BaseComponent.h"

class TransformComponent;

/**
* @class InputMoveComponent
* @brief 入力による移動処理を行うコンポーネント
* todo : カメラが設定されている場合は視点方向に移動する
*/
class InputMoveComponent
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
    InputMoveComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
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

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief ImGui更新 */
    void ImGuiUpdate() override;
    /* @brief 更新 */
    void Update() override;

    //--------------------------------
    // 変数
    //--------------------------------
    float m_slideSpeed = 0.0f; // 滑走速度

    /* 各コンポーネントアクセス用 */
};

// シリアライズで使用するキー
namespace jsonKey::Comp
{
    namespace InputMoveComponent
    {
        constexpr std::string_view SlideSpeed = "SlideSpeed";
    }
}
