#include "KurageMoveState.h"
#include "../KurageMoveScript.h"
#include "../../../Renderer/AnimationComponent/AnimationComponent.h"
#include "../../../TransformComponent/TransformComponent.h"
#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/Script/PlayerScript/PlayerScript.h"

/*--------------------------------------------//
 *
 * KurageMoveBaseState
 *
 *--------------------------------------------*/
void KurageMoveBaseState::Enter(KurageMoveScript* _pOwner)
{

    m_colorChangeFactor = 0.0f;

    const auto& spTransform = _pOwner->GetOwner()->GetTransformComponent();

    // 現在の回転を取得
    m_startQuaternion = spTransform->GetQuaternion();

    // 目標の回転クォータニオンを計算
    CalcTargetRotation(_pOwner, spTransform);

    // 補間パラメータを初期化
    t = 0.0f;
}

void KurageMoveBaseState::Update(KurageMoveScript* _pOwner)
{
    float delta = SceneManager::Instance().FrameDeltaTime();

    // 移動処理 //
    const auto& spTransform = _pOwner->GetOwner()->GetTransformComponent();

    if (!spTransform)
    {
        return;
    }

    // 体を傾ける //
    if (t < 1.0f)
    {
        // クォータニオンのSlerp補間
        Math::Quaternion currentQuat = Math::Quaternion::Slerp(m_startQuaternion, m_targetQuaternion, t);

        // 回転を適用
        spTransform->SetQuarternion(currentQuat);

        // 補間パラメータを更新
        t += delta * 4.0f;
    }
    else
    {
        // 補間が完了したら目標の回転を設定
        spTransform->SetQuarternion(m_targetQuaternion);
    }

    // 加速度による速度の更新
    m_velocity += m_acceleration * delta;

    // 速度の制限
    float speed = m_velocity.Length();

    if (speed > _pOwner->GetMaxSpeed())
    {
        Math::Vector3 vel = m_velocity;
        vel.Normalize();
        m_velocity = vel * _pOwner->GetMaxSpeed();
    }

    // 移動処理
    spTransform->SetPosition(spTransform->GetLocalPos() + m_velocity * delta);

    SceneManager::Instance().GetDebugWire()->AddDebugLine(
        spTransform->GetWorldPos(),
        _pOwner->GetInputDirection(), 5, Color::Red);
}

void KurageMoveBaseState::ImGui(KurageMoveScript* _pOwner)
{
    ImGui::Text("Velocity: { %.2f, %.2f, %.2f }", m_velocity.x, m_velocity.y, m_velocity.z);
    ImGui::Text("Speed: %.2f", m_velocity.Length());

    ImGui::Text("Acceleration: { %.2f, %.2f, %.2f }", m_acceleration.x, m_acceleration.y, m_acceleration.z);
    ImGui::Text("Accel: %.2f", m_acceleration.Length());
}

void KurageMoveBaseState::EasingToLigColor(const std::shared_ptr<LightComponent>& _spLigComp)
{
    if (m_colorChangeFactor >= 1.0f)
    {
        m_colorChangeFactor = 1.0f;
    }

    m_colorChangeFactor += SceneManager::Instance().FrameDeltaTime() * m_easeSpeed;

    Math::Vector3 color = MathHelper::Lerp(_spLigComp->GetColor(), m_targetLigColor, m_colorChangeFactor);

    _spLigComp->SetColor(color);
}

void KurageMoveBaseState::CalcTargetRotation(
    KurageMoveScript* _pOwner,
    const std::shared_ptr<TransformComponent>& _spTransform)
{
    // 傾きの最大角度
    const float tiltAngle = _pOwner->GetTiltAngle();

    // 移動方向を正規化
    Math::Vector3 moveDir = _pOwner->GetInputDirection();
    moveDir.Normalize();

    // キャラクターのYaw回転（ラジアン）を取得
    float yawInRadians = MathHelper::ConvertToRadians(_spTransform->GetRotation().y);

    // Yaw回転のみのクォータニオンを作成
    Math::Quaternion yawQuat = Math::Quaternion::CreateFromAxisAngle(Math::Vector3::Up, yawInRadians);

    // キャラクターのローカル軸を取得
    Math::Matrix mRot = Math::Matrix::CreateFromQuaternion(yawQuat);
    Math::Vector3 localRight = Math::Vector3::TransformNormal(Math::Vector3::Right, mRot);
    Math::Vector3 localForward = Math::Vector3::TransformNormal(Math::Vector3::Forward, mRot);

    // ローカルの傾き角度（ピッチとロール）を計算
    float pitchAngle = -tiltAngle * moveDir.y;  // 上下の傾き（Y軸方向の移動を使用）
    float rollAngle = tiltAngle * moveDir.x;    // 左右の傾き（X軸方向の移動を使用）

    // 傾きの角度を制限
    pitchAngle = std::clamp(pitchAngle, -tiltAngle, tiltAngle);
    rollAngle = std::clamp(rollAngle, -tiltAngle, tiltAngle);

    // ピッチとロールのクォータニオンを作成（ローカル軸を使用）
    Math::Quaternion pitchQuat = Math::Quaternion::CreateFromAxisAngle(localRight, MathHelper::ConvertToRadians(pitchAngle));
    Math::Quaternion rollQuat = Math::Quaternion::CreateFromAxisAngle(localForward, MathHelper::ConvertToRadians(rollAngle));

    // 傾きのクォータニオンを合成（ロール -> ピッチの順）
    Math::Quaternion tiltQuat = rollQuat * pitchQuat;

    // 目標の回転クォータニオンを計算
    m_targetQuaternion = tiltQuat;

    // 正規化
    m_targetQuaternion.Normalize();
}

/*--------------------------------------------//
 *
 * IdleState
 *
 *--------------------------------------------*/
void IdleState::Enter(KurageMoveScript* _pOwner)
{
    // アニメーションを "Charge" に設定
    const std::shared_ptr<AnimationComponent>& spAnimationComp = _pOwner->GetAnimationComponent();

    if (spAnimationComp)
    {
        spAnimationComp->SetAnimation("Float", false);
        spAnimationComp->SetAnimationSpeed(_pOwner->GetFloatAnimSpeed() + _pOwner->GetChargeAnimSpeed());
    }

    m_animLoopDelay = 0.5f;

    KurageMoveBaseState::Enter(_pOwner);

    // 加速度と速度の設定
    if (m_velocity.LengthSquared() > 0.0f)
    {
        Math::Vector3 vel = m_velocity;
        vel.Normalize();
        m_acceleration = -vel * _pOwner->GetDecelerationRate();
    }
    else
    {
        m_acceleration = Math::Vector3::Zero;
    }

    if (auto spLigComp = _pOwner->GetLightComponent())
    {
        m_targetLigColor = _pOwner->GetDefaultLigColor();
        m_easeSpeed = 0.5f;
    }
}

void IdleState::Update(KurageMoveScript* _pOwner)
{
    // ライトの色を徐々に変更する //
    const std::shared_ptr<LightComponent>& spLigComp = _pOwner->GetLightComponent();
    if (_pOwner->IsLightUpdate() && spLigComp)
    {
        EasingToLigColor(spLigComp);
    }

    KurageMoveBaseState::Update(_pOwner);

    // 速度が0になったら停止
    if (m_velocity.LengthSquared() > 0.0f)
    {
        m_velocity = Math::Vector3{};
        m_acceleration = Math::Vector3{};
    }

    if (_pOwner->IsMove())
    {
        auto swimState = _pOwner->GetKurageMoveController().AddState<SwimState>(true);
        swimState->SetVelocity(m_velocity);
        return;
    }

    // アニメーション終了時に入力されていたら移動状態へ
    if (const auto& spAnimationComp = _pOwner->GetAnimationComponent())
    {
        if (!spAnimationComp->IsAnimationEnd()) { return; }

        // アニメーション終了状態かつ入力がない場合は再度アニメーションを再生
        if (!_pOwner->IsMove())
        {

            m_animLoopDelay -= SceneManager::Instance().FrameDeltaTime();

            if (m_animLoopDelay <= 0.0f)
            {
                spAnimationComp->SetAnimation("Float", false);
                spAnimationComp->SetAnimationSpeed(_pOwner->GetFloatAnimSpeed() + _pOwner->GetChargeAnimSpeed());
            }
        }
    }
    // 何らかの状態でアニメーションがなくなっていたらアニメーションなしで移動状態へ
    else
    {
        auto charge = _pOwner->GetKurageMoveController().AddState<SwimState>(true);
    }
}

void IdleState::Exit(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::Exit(_pOwner);
}

void IdleState::ImGui(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::ImGui(_pOwner);
}

/*--------------------------------------------//
 *
 * SwimState
 *
 *--------------------------------------------*/
void SwimState::Enter(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::Enter(_pOwner);

    ChargingEnter(_pOwner);
}

void SwimState::Update(KurageMoveScript* _pOwner)
{
    switch (m_currentPhase)
    {
    case Phase::Charging:
        ChargingUpdate(_pOwner);
        break;
    case Phase::Swimming:
        SwimmingUpdate(_pOwner);
        break;
    default:
        // 何かしらのエラーが発生した場合は IdleState に遷移
        _pOwner->GetKurageMoveController().AddState<IdleState>(true);
        FNENG_ASSERT_ERROR(_pOwner->GetOwner()->GetName() + "[SwimState]");
        break;
    }
}

void SwimState::Exit(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::Exit(_pOwner);
}

void SwimState::ImGui(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::ImGui(_pOwner);

    ImGui::Text("Current Phase: %s", m_currentPhase == Phase::Charging ? "Charging" : "Swimming");
}

void SwimState::ChargingEnter(KurageMoveScript* _pOwner)
{
    // アニメーションを "Charge" に設定
    if (const auto& spAnimationComp = _pOwner->GetAnimationComponent())
    {
        spAnimationComp->SetAnimation("Charge", false);
        spAnimationComp->SetAnimationSpeed(_pOwner->GetChargeAnimSpeed());
    }

    // ライトの設定
    m_targetLigColor = _pOwner->GetDefaultLigColor();
    m_easeSpeed = 0.5f;

    m_currentPhase = Phase::Charging;
}

void SwimState::ChargingUpdate(KurageMoveScript* _pOwner)
{
    // 加速度の設定
    m_acceleration = Math::Vector3{}; // Charging フェーズでは加速度はゼロ

    // ベースクラスの Update を呼び出す
    KurageMoveBaseState::Update(_pOwner);

    // ライトの色を徐々に変更
    const std::shared_ptr<LightComponent>& spLigComp = _pOwner->GetLightComponent();
    if (_pOwner->IsLightUpdate() && spLigComp)
    {
        EasingToLigColor(spLigComp);
    }

    // アニメーションの終了を待つ
    const auto& spAnimationComp = _pOwner->GetAnimationComponent();
    if (!spAnimationComp || !spAnimationComp->IsAnimationEnd())
    {
        return;
    }
    // フェーズを Swimming に変更
    m_currentPhase = Phase::Swimming;

    SwimmingEnter(_pOwner);
}

void SwimState::SwimmingEnter(KurageMoveScript* _pOwner)
{
    // アニメーションを "Swim" に設定
    if (const auto& spAnimationComp = _pOwner->GetAnimationComponent())
    {
        spAnimationComp->SetAnimation("Swim", false);
        spAnimationComp->SetAnimationSpeed(_pOwner->GetSwimAnimSpeed());
    }

    // ライトの設定
    m_targetLigColor = _pOwner->GetDefaultLigColor();
    m_easeSpeed = 0.5f;
}

void SwimState::SwimmingUpdate(KurageMoveScript* _pOwner)
{
    // 入力がある場合はふわっと移動
    if (_pOwner->IsMove())
    {
        // 前のフレームの入力都の差分が大きかったら移動速度を倍にする * _pOwner->GetAcceleration()
        // 差分の大きさ: 反対方向 -> _pOwner->GetAcceleration() * 2倍, 同じ方向 -> _pOwner->GetAcceleration() * 1倍, 反対と同じ方向の間 -> _pOwner->GetAcceleration() * 1.5倍

        // 前のフレームと現在の入力方向を取得し、正規化します
        Math::Vector3 currDir = _pOwner->GetInputDirection();
        Math::Vector3 moveDir = m_velocity;
        moveDir.Normalize();
        currDir.Normalize();

        // 前の方向と現在の方向の内積を計算します
        float dot = moveDir.Dot(currDir);

        // 加速度のスケーリング係数を決定します
        float scalingFactor;

        if (dot <= -0.4f) // 反対方向
        {
            scalingFactor = 3.0f;
        }
        else if (dot >= 0.5f) // 同じ方向
        {
            scalingFactor = 1.0f;
        }
        else // 反対と同じ方向の間
        {
            scalingFactor = 2.f;
        }

        // 加速度を計算します
        m_acceleration = currDir * (_pOwner->GetAcceleration() * scalingFactor);
    }
    // 入力がない場合は徐々に減速していく
    else
    {
        // 減速
        m_velocity *= _pOwner->GetDecelerationRate();
    }

    // ベースクラスの Update を呼び出す
    KurageMoveBaseState::Update(_pOwner);

    // アニメーションの終了に合わせて、ステートの更新
    const auto& spAnimationComp = _pOwner->GetAnimationComponent();

    if (!spAnimationComp || !spAnimationComp->IsAnimationEnd())
    {
        return;
    }

    // 移動速度がアイドルステートに遷移するための閾値
    constexpr float IdleTransitionThreshold = 0.05f;

    // 移動速度が 0.0f に近づいたらアイドルステートに遷移
    if (m_velocity.LengthSquared() <= IdleTransitionThreshold)
    {
        _pOwner->GetKurageMoveController().AddState<IdleState>(true);
        return;
    }

    // フェーズを Charging に変更し、Charging の処理を開始
    m_currentPhase = Phase::Charging;
    ChargingEnter(_pOwner);

}

/*--------------------------------------------//
 *
 *              KnockBackState
 *
 *--------------------------------------------*/
void KnockBackState::Enter(KurageMoveScript* _pOwner)
{
    _pOwner->SetKurageState(KurageMoveScript::KurageState::eKnockBack);

    // ライトの設定
    if (auto spLigComp = _pOwner->GetLightComponent())
    {
        m_targetLigColor = Math::Vector3{ Color::Magenta.x, Color::Magenta.y, Color::Magenta.z };
        m_easeSpeed = 0.5f;

        m_beforeLigColor = spLigComp->GetColor();
        m_isLigReverse = false;
    }

    // この時点でのノックバック方向を設定
    m_knockBackDir = _pOwner->GetInputDirection();

    // ノックバックの初期設定
    m_knockBackDir.Normalize();

    KurageMoveBaseState::Enter(_pOwner);
}

void KnockBackState::Update(KurageMoveScript* _pOwner)
{
    if (m_isFirstUpdate)
    {
        // 加速度を設定（減速方向）
        float speed = m_velocity.Length() * _pOwner->GetKnockBackDelFactor();
        m_velocity = m_knockBackDir * speed;

        m_isFirstUpdate = false;
    }

    // ライトの色を徐々に変更する
    const std::shared_ptr<LightComponent>& spLigComp = _pOwner->GetLightComponent();
    if (spLigComp)
    {
        EasingToLigColor(spLigComp);
    }

    KurageMoveBaseState::Update(_pOwner);

    m_knockBackTimeCounter += SceneManager::Instance().FrameDeltaTime();

    constexpr float KnockBackStateTime = 1.f;
    if (!m_isLigReverse && m_knockBackTimeCounter > KnockBackStateTime)
    {
        // 一定時間経過後、ライトの色を戻す //
        m_colorChangeFactor = 0.0f;
        m_targetLigColor = m_beforeLigColor;
        m_isLigReverse = true;
    }

    //ライトの色が完全に戻ったら SwimState に遷移
    if (m_isLigReverse && m_colorChangeFactor >= 1.0f)
    {
        _pOwner->SetInputDirection(m_knockBackDir);

        Math::Vector3 vel = m_velocity;

        // 次の状態に遷移
        auto swimState = _pOwner->GetKurageMoveController().AddState<SwimState>(true);

        swimState->SetVelocity(vel);
    }
}

void KnockBackState::Exit(KurageMoveScript* _pOwner)
{
    _pOwner->SetKurageState(_pOwner->GetBeforeKurageState());

    // ベースクラスの Exit を呼び出す
    KurageMoveBaseState::Exit(_pOwner);

    // ライトの色を元に戻す
    if (auto spLigComp = _pOwner->GetLightComponent())
    {
        spLigComp->SetColor(m_beforeLigColor);
    }
}

void KnockBackState::ImGui(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::ImGui(_pOwner);
}

/*--------------------------------------------//
 *
 *              TangledState
 *
 *--------------------------------------------*/
void TangledState::Enter(KurageMoveScript* _pOwner)
{
    _pOwner->SetKurageState(KurageMoveScript::KurageState::eTangled);

    if (auto spLigComp = _pOwner->GetLightComponent())
    {
        m_beforeLigColor = spLigComp->GetColor();

        m_targetLigColor = Math::Vector3{ Color::DeepGreen.x, Color::DeepGreen.y, Color::DeepGreen.z };
        m_easeSpeed = 0.5f;
    }
}

void TangledState::Update(KurageMoveScript* _pOwner)
{
    const std::shared_ptr<LightComponent>& spLigComp = _pOwner->GetLightComponent();
    if (spLigComp)
    {
        EasingToLigColor(spLigComp);
    }

    // 自分から絡まったオブジェクトへのベクトルを算出
    Math::Vector3 tangleDir = m_tangleDir * -1.0f;
    tangleDir.Normalize();

    // プレイヤーの入力方向を取得し、正規化
    Math::Vector3 inputDir = _pOwner->GetInputDirection();
    inputDir.Normalize();

    // 内積で進行方向と絡まった方向の類似度を計算
    float dot = tangleDir.Dot(inputDir); // -1.0 - 1.0

    // デバッグ表示
    const Math::Vector3& nowPos =
        _pOwner->GetOwner()->GetTransformComponent()->GetWorldPos();

    // todo : マジックナンバー撲滅
    // 進行方向が絡まった方向と逆向きかチェック
    if (!m_isLigReverse && dot < -0.2f)
    {
        // 一定時間経過後、ライトの色を戻す //
        m_colorChangeFactor = 0.0f;
        m_targetLigColor = m_beforeLigColor;
        m_isLigReverse = true;
    }

    //ライトの色が完全に戻ったら SwimState に遷移 //
    if (m_isLigReverse && m_colorChangeFactor >= 1.0f)
    {
        _pOwner->SetInputDirection(m_tangleDir);
        // SwimmingState に遷移
        _pOwner->GetKurageMoveController().AddState<SwimState>(true);
    }
}

void TangledState::Exit(KurageMoveScript* _pOwner)
{
    _pOwner->SetKurageState(_pOwner->GetBeforeKurageState());
}

void TangledState::ImGui(KurageMoveScript* _pOwner)
{
    KurageMoveBaseState::ImGui(_pOwner);
}
