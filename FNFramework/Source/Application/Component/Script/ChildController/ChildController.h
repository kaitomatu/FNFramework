#pragma once

#include "../../BaseComponent.h"

class LightComponent;
class PlayerScript;
class KurageMoveScript;

/**
* @class ChildController
* @brief 親クラゲに追従するクラゲのコントローラークラス
* @details
*   ステートパターンを使用して、子クラゲの挙動を制御する
*/
class ChildController
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
    ChildController(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~ChildController() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // プレイヤオブジェクトの設定 / 取得(State側でアクセスされることを想定)
    std::shared_ptr<PlayerScript> GetPlayerScript() const { return m_wpPlayerScript.lock(); }

    // ステートマシンの取得
    utl::StateMachine<ChildController>& GetChildController() { return m_childController; }

    // 子クラゲの初期座標の設定 / 取得
    void SetBasePosition(const Math::Vector3& pos) { m_basePosition = pos; }
    const Math::Vector3& GetBasePosition() const { return m_basePosition; }

    // クラゲの移動スクリプトの取得
    std::shared_ptr<KurageMoveScript> GetKurageMoveScript() const { return m_wpKurageMoveScript.lock(); }
    std::shared_ptr<LightComponent> GetLightComponent() const { return m_wpLightComponent.lock(); }

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
    void PreUpdate() override;
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

    // 子クラゲの初期座標
    Math::Vector3 m_basePosition = Math::Vector3::Zero;

    utl::StateMachine<ChildController> m_childController;

    std::weak_ptr<PlayerScript> m_wpPlayerScript;
    std::string m_playerName;

    std::weak_ptr<KurageMoveScript> m_wpKurageMoveScript;

    std::weak_ptr<LightComponent> m_wpLightComponent;

    bool m_firstUpdate = false;
};

namespace jsonKey::Comp
{
    namespace ChildController
    {
        constexpr std::string_view BasePosition = "BasePosition";
        constexpr std::string_view PlayerObject = "PlayerObject";
    }
}

namespace ChildControllerSetting
{
    // memo : アイテム個々での設定ではなく全体として設定したいため static で設定

    //x----- アイテム状態のぷかぷか挙動方向の設定 -----x//
    // 基準位置との距離の影響度 :
    inline static Math::Vector3 PosEffective = { 0.8f, 1.0f, 1.0f };

    // 乱数による影響度 : y 軸はあまり下に行ってほしくないのでランダムの影響度を下げる
    inline static Math::Vector3 RandomEffective = { 1.0f, 0.5f, 1.0f };

    inline static Math::Vector3 CalcNewDir(ChildController* _pOwner, const Math::Vector3& _position)
    {
        // 目標位置と現在位置の差を計算
        Math::Vector3 diff = _pOwner->GetBasePosition() - _position;

        // ランダムな方向を基に、基準位置との距離を考慮して方向ベクトルを計算
        Math::Vector3 randomDir = Math::Vector3{
            static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-1.0, 1.0)),
            static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-1.0, 1.0)),
            static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-0.5, 0.5))
        };

        // 各成分にランダム成分と調整成分を加えた方向ベクトルを算出
        Math::Vector3 newDir = Math::Vector3{
            randomDir.x * ChildControllerSetting::RandomEffective.x + diff.x * ChildControllerSetting::PosEffective.x,
            randomDir.y * ChildControllerSetting::RandomEffective.y + diff.y * ChildControllerSetting::PosEffective.y,
            randomDir.z * ChildControllerSetting::RandomEffective.z + diff.z * ChildControllerSetting::PosEffective.z
        };
        newDir.Normalize();

        return newDir;
    }

}
