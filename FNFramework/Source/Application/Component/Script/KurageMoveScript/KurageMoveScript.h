#pragma once

#include "../../BaseComponent.h"

class LightComponent;
class AnimationComponent;

/**
 * @class KurageMoveScript
 * @brief クラゲの移動処理を行うクラス
 * @details
 *   - 一定時間力をためた後に進む
 *   - 進む方向は入力方向によって決まり、力をためる時間が長いほど進む距離が長くなる
 */
class KurageMoveScript
    : public BaseComponent
{
public:

    enum class KurageState
    {
        eDefault,
        eEvent,
        eTangled,
        eKnockBack,
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
     * @brief コンストラクタ
     * @param[in] owner - オーナーオブジェクトのポインタ
     * @param[in] name - コンポーネントの名前
     */
    KurageMoveScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~KurageMoveScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    KurageState GetKurageState() const { return m_kurageState; }
    KurageState GetBeforeKurageState() const { return m_beforeKurageState; }
    void SetKurageState(KurageState state)
    {
        m_beforeKurageState = m_kurageState;
        m_kurageState = state;
    }

    bool IsTargetState(std::string_view _stateName) const
    {
        return m_kurageMoveController.GetNowStateName() == _stateName;
    }

    /**
     * @brief 入力方向の設定 / 取得
     */
    const Math::Vector3& GetInputDirection() const { return m_inputDir; }
    void SetInputDirection(const Math::Vector3& dir)
    {
        m_inputDir = dir;
    }

    // ベースの高さ取得
    float GetBaseHeight() const { return m_baseHeight; }

    // 加速度と減速度の取得
    float GetAcceleration() const { return m_acceleration; }
    float GetDecelerationRate() const { return m_decelerationRate; }
    float GetKnockBackDelFactor() const { return m_knockBackDelFactor; }

    float GetMaxSpeed() const { return m_maxSpeed; }

    // 移動中かどうかの設定 / 取得
    bool IsMove() const { return m_isMove; }
    void SetMoveFlg(bool flg)
    {
        m_isMove = flg;
    }

    // ステートマシンの取得
    utl::StateMachine<KurageMoveScript>& GetKurageMoveController() { return m_kurageMoveController; }

    // アニメーションコンポーネントの取得
    std::shared_ptr<AnimationComponent> GetAnimationComponent() const { return m_wpAnimationComp.lock(); }

    float GetSwimAnimSpeed() const { return m_swimAnimSpeed; }
    float GetChargeAnimSpeed() const { return m_chargeAnimSpeed; }
    float GetFloatAnimSpeed() const { return m_chargeAnimSpeed / 2.0f; }
    void SetChargeAnimSpeed(float _chargeAnimSpeed) { m_chargeAnimSpeed = _chargeAnimSpeed; }

    float GetTiltAngle() const { return m_tiltAngle; }
    void SetTiltAngle(float _tilt) { m_tiltAngle = _tilt; }

    // ライトコンポーネントの取得
    std::shared_ptr<LightComponent> GetLightComponent() const { return m_wpLightComponent.lock(); }
    const Math::Vector3& GetDefaultLigColor() const { return m_defaultLigColor; }
    bool IsLightUpdate() const { return m_isLightUpdate; }
    void SetLightUpdate(bool _flg)
    {
        m_isLightUpdate = _flg;
    }

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

    /* @brief 保存 / 読みこみ */
    void Serialize(Json& _json) const override;

    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn void ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    std::weak_ptr<LightComponent> m_wpLightComponent;
    Math::Vector3 m_defaultLigColor = Math::Vector3{
        Color::White.x,
        Color::White.y,
        Color::White.z
    };
    bool m_isLightUpdate = true;

    bool m_firstUpdate = false; // 初回更新かどうか
    float m_baseHeight = 0.0f;  // 初期の高さ

    // 移動中かどうか
    bool m_isMove = false;

    // --- 移動関係 --- //
    Math::Vector3 m_inputDir = Math::Vector3::Zero; // 入力方向

    float m_decelerationRate = 0.0f; // 反対方向入力されたときの減速度
    float m_acceleration = 0.0f; // 入力があったときの加速度

    float m_maxSpeed = 0.0f; // 最大速度

    float m_knockBackDelFactor = 0.0f; // ノックバック時の減速度

    // --- アニメーション --- //
    float m_swimAnimSpeed = 0.0f;   // 泳ぐアニメーションの速度
    float m_chargeAnimSpeed = 0.0f; // チャージアニメーションの速度

    float m_tiltAngle = 0.0f;

    std::weak_ptr<AnimationComponent> m_wpAnimationComp;

    utl::StateMachine<KurageMoveScript> m_kurageMoveController;

    KurageState m_kurageState = KurageState::eDefault;
    KurageState m_beforeKurageState = KurageState::eDefault;
};

// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace KurageMoveScript
    {
        constexpr std::string_view Deceleration = "Deceleration";
        constexpr std::string_view Acceleration = "Acceleration";
        constexpr std::string_view KnockBackDelFactor = "KnockBackDelFactor";

        constexpr std::string_view MaxSpeed = "MaxSpeed";
        constexpr std::string_view KnockBackPower = "KnockBackPower";
        constexpr std::string_view SwimAnimSpeed = "SwimAnimSpeed";
        constexpr std::string_view ChargeAnimSpeed = "ChargeAnimSpeed";
        constexpr std::string_view RotationX = "RotationX";
    }
}
