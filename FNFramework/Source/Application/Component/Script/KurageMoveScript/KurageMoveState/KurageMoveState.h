#pragma once
class KurageMoveScript;
class LightComponent;

class KurageMoveBaseState
    : public utl::StateBase<KurageMoveScript>
{
public:
    void SetVelocity(const Math::Vector3& _velocity) { m_velocity = _velocity; }
    const Math::Vector3& GetVelocity() const { return m_velocity; }

    void Enter(KurageMoveScript* _pOwner) override;
    void Update(KurageMoveScript* _pOwner) override;
    void ImGui(KurageMoveScript* _pOwner) override;

protected:
    void EasingToLigColor(const std::shared_ptr<LightComponent>& _spLigComp);

    // ライト関係 //
    Math::Vector3 m_targetLigColor;
    float m_easeSpeed = 0.5f;
    float m_colorChangeFactor = 0.0f;

    // 移動関係 //
    Math::Vector3 m_velocity = Math::Vector3::Zero;      // 現在の速度ベクトル
    Math::Vector3 m_acceleration = Math::Vector3::Zero;  // 加速度ベクトル

private:
    // 回転に関するメンバ変数と関数
    void CalcTargetRotation(
        KurageMoveScript* _pOwner,
        const std::shared_ptr<TransformComponent>& _spTransform);

    Math::Quaternion m_targetQuaternion; // 目標の回転クォータニオン
    Math::Quaternion m_startQuaternion;  // 開始時の回転クォータニオン
    float t = 0.0f;                      // 補間パラメータ
};

class IdleState
    : public KurageMoveBaseState
{
public:

    void Enter(KurageMoveScript* _pOwner) override;

    void Update(KurageMoveScript* _pOwner) override;

    void Exit(KurageMoveScript* _pOwner) override;

    void ImGui(KurageMoveScript* _pOwner) override;

private:
    float m_animLoopDelay = 0.5;
};

class SwimState : public KurageMoveBaseState
{
public:
    void Enter(KurageMoveScript* _pOwner) override;
    void Update(KurageMoveScript* _pOwner) override;
    void Exit(KurageMoveScript* _pOwner) override;
    void ImGui(KurageMoveScript* _pOwner) override;

private:
    enum class Phase
    {
        Charging,
        Swimming
    };

    Phase m_currentPhase = Phase::Charging;

    // フェーズごとの処理関数
    void ChargingEnter(KurageMoveScript* _pOwner);
    void ChargingUpdate(KurageMoveScript* _pOwner);
    void SwimmingEnter(KurageMoveScript* _pOwner);
    void SwimmingUpdate(KurageMoveScript* _pOwner);
};

class KnockBackState
    : public KurageMoveBaseState
{
public:
    void Enter(KurageMoveScript* pOwner) override;
    void Update(KurageMoveScript* pOwner) override;
    void Exit(KurageMoveScript* pOwner) override;
    void ImGui(KurageMoveScript* pOwner) override;

private:

    bool m_isFirstUpdate = true;
    bool m_isLigReverse = false;

    // ノックバックする時間(s) //
    float m_knockBackTimeCounter = 0.0f;

    // ノックバック関係 //
    Math::Vector3 m_knockBackDir = Math::Vector3::Zero;

    Math::Vector3 m_beforeLigColor = Math::Vector3::Zero;
};

class TangledState
    : public KurageMoveBaseState
{
public:
    void SetTangleDir(const Math::Vector3& _dir) { m_tangleDir = _dir; }

    void Enter(KurageMoveScript* _pOwner) override;

    void Update(KurageMoveScript* _pOwner) override;

    void Exit(KurageMoveScript* _pOwner) override;

    void ImGui(KurageMoveScript* _pOwner) override;

private:
    bool m_isLigReverse = false;

    Math::Vector3 m_beforeLigColor = Math::Vector3::Zero;

    Math::Vector3 m_tangleDir = Math::Vector3::Zero;
};
