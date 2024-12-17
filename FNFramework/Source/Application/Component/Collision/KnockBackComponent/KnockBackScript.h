#pragma once

#include "../../BaseComponent.h"

class KnockBackState;
class KurageMoveScript;
class CollisionComponent;

/**
* @class KnockBackScript
* @brief
* @details
*
*/
class KnockBackScript
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
    KnockBackScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~KnockBackScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    
    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @fn void Awake()
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    */
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:

    void ImGuiUpdate() override;

    bool m_isKnockBack = false;

    std::weak_ptr<CollisionComponent> m_wpCollisionComponent;

    std::weak_ptr<KurageMoveScript> m_wpKurageMoveComponent;

    std::weak_ptr<KnockBackState> m_wpKnockBackState;
};

namespace jsonKey::Comp
{
    namespace KnockBackScript
    {
    }
}
