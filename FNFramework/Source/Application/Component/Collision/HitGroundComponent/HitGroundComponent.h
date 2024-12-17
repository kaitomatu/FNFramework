#pragma once

#include "../../BaseComponent.h"

class CollisionComponent;
/**
* @class HitGroundComponent
* @brief なにかしらの地形にヒットした際に押し戻し処理を行うコンポーネント
*/
class HitGroundComponent
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
    HitGroundComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
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

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    std::weak_ptr<CollisionComponent> m_wpCollisionComponent;

    // 内積の値が m_cliffRate 以内なら、崖として処理を行う
    float m_cliffRate = 70.0f;
};

namespace jsonKey::Comp
{
        namespace HitGroundComponent
        {
            constexpr std::string_view CliffRate = "CliffRate";
            constexpr std::string_view CollisionComponentName = "CollisionComponentName";
        }
}
