#pragma once

#include "../../BaseComponent.h"

class CollisionComponent;
class PlayerScript;
class TrackingCameraComponent;
/**
* @class SeaWeedWallScript
*/
class SeaWeedWallScript
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
    SeaWeedWallScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    bool CanOpen() const { return m_canOpen; }
    void SetOpenTrigger(bool _trigger) { m_openTrigger = _trigger; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @fn void Awake()
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;

    /**
    * @fn void Start()
    * @brief Awakeを経て初期化された後、1度だけ呼びだされる
    * @details
    *	Awakeの後に呼び出される
    *	他のコンポーネントとの依存関係にある初期化処理や
    *	Awakeの段階ではできない初期化を行う
    */
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;

    /* @fn Release() @brief 終了 */
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

    int m_requiredPusherCount = 0;
    std::weak_ptr<PlayerScript> m_wpPlayerScript;
    std::weak_ptr<CollisionComponent> m_wpCollisionComponent;

    bool m_openTrigger = false; 
    bool m_canOpen = false;
};

namespace jsonKey::Comp
{
    namespace SeaWeedWallScript
    {
        static const std::string RequiredPusherCount = "RequiredPusherCount";
    }
}
