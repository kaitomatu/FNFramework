#pragma once

// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace SkySphereScript
    {
        constexpr std::string_view TargetName = "TargetName";
    }
}

#include "../../BaseComponent.h"

/**
* @class SkySphereScript
* @brief スカイスフィアを操作するスクリプト
* @details
*   m_wpTargetに設定されたオブジェクトの座標に追従する
*
*/
class SkySphereScript
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
    SkySphereScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    // 追従するオブジェクトの設定
    void SetTarget(const std::shared_ptr<GameObject>& _target) { m_wpTarget = _target; }

    //--------------------------------
    // その他関数
    //--------------------------------

    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;
    void ImGuiUpdate() override;

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    std::string m_targetName;

    // 追従するオブジェクト
    std::weak_ptr<GameObject> m_wpTarget;
};
