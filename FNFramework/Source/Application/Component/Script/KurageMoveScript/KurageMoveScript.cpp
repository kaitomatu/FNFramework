#include "KurageMoveScript.h"

#include "Application/Component/LightComponent/LightAnimationScript.h"
#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"

#include "KurageMoveState/KurageMoveState.h"

void KurageMoveScript::Awake()
{
    m_baseHeight = 0.0f;
    m_knockBackDelFactor = 0.7f;
    m_isMove = true;
    m_decelerationRate = 50.0f;
    m_inputDir = Math::Vector3::Zero;
    m_swimAnimSpeed = 1.8f;
    m_chargeAnimSpeed = 1.0f;
    m_tiltAngle = 10.0f;
}

void KurageMoveScript::Start()
{
    if (!OwnerValid()) { return; }

    m_firstUpdate = true;

    m_wpAnimationComp = m_wpOwnerObj.lock()->GetComponent<AnimationComponent>();
    m_wpLightComponent = m_wpOwnerObj.lock()->GetComponent<LightComponent>();

    if(m_wpLightComponent.expired())
    {
        m_wpLightComponent = m_wpOwnerObj.lock()->GetComponent<LightAnimationScript>();
    }

    if(!m_wpLightComponent.expired())
    {
        m_defaultLigColor = m_wpLightComponent.lock()->GetColor();
    }

    // 状態マシンをセットアップ
    m_kurageMoveController.SetUp(this);
    m_kurageMoveController.AddState<IdleState>();
}

void KurageMoveScript::Update()
{
    if(m_firstUpdate)
    {
        m_baseHeight = m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldPos().y;

        m_firstUpdate = false;
    }

    m_kurageMoveController.Update();
}

void KurageMoveScript::Release()
{
    m_kurageMoveController.Clean();
}

void KurageMoveScript::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::KurageMoveScript::Deceleration.data()] = m_decelerationRate;
    _json[jsonKey::Comp::KurageMoveScript::Acceleration.data()] = m_acceleration;
    _json[jsonKey::Comp::KurageMoveScript::KnockBackDelFactor.data()] = m_knockBackDelFactor;

    _json[jsonKey::Comp::KurageMoveScript::MaxSpeed.data()] = m_maxSpeed;

    _json[jsonKey::Comp::KurageMoveScript::ChargeAnimSpeed.data()] = m_chargeAnimSpeed;
    _json[jsonKey::Comp::KurageMoveScript::SwimAnimSpeed.data()] = m_swimAnimSpeed;

    _json[jsonKey::Comp::KurageMoveScript::RotationX.data()] = m_tiltAngle;
}

void KurageMoveScript::Deserialize(const Json& _json)
{
    m_decelerationRate = _json.value(jsonKey::Comp::KurageMoveScript::Deceleration.data(), 1.0f);
    m_decelerationRate = std::clamp(m_decelerationRate, 0.0f, 1.0f);

    m_acceleration = _json.value(jsonKey::Comp::KurageMoveScript::Acceleration.data(), 1.0f);
    m_knockBackDelFactor = _json.value(jsonKey::Comp::KurageMoveScript::KnockBackDelFactor.data(), 0.7f);

    m_maxSpeed = _json.value(jsonKey::Comp::KurageMoveScript::MaxSpeed.data(), 5.0f);

    // アニメーション速度のデシリアライズ
    m_swimAnimSpeed = _json.value(jsonKey::Comp::KurageMoveScript::SwimAnimSpeed.data(), 0.0f);
    m_chargeAnimSpeed = _json.value(jsonKey::Comp::KurageMoveScript::ChargeAnimSpeed.data(), 0.0f);

    // 回転のデシリアライズ
    m_tiltAngle = _json.value(jsonKey::Comp::KurageMoveScript::RotationX.data(), 0.0f);
}

void KurageMoveScript::ImGuiUpdate()
{
    ImGui::Text(U8_TEXT("現在のステート: %s"), m_kurageMoveController.GetNowStateName().data());

    ImGui::Text(U8_TEXT("ステートでのライトの更新: %s"), m_isLightUpdate ? "true" : "false");

    ImGui::Text(U8_TEXT("クラゲの状態: [ %s ]"), utl::str::EnumToString(m_kurageState).data());

    m_kurageMoveController.ImGui();

    ImGui::Text("IsMove: %s", m_isMove ? "true" : "false");
    ImGui::Separator();

    ImGui::Text(U8_TEXT("入力なし状態のときの減速率"));
    ImGui::DragFloat("###Deceleration", &m_decelerationRate, 0.01f, 0.0f,1.0f);

    ImGui::Text(U8_TEXT("移動の加速度"));
    ImGui::DragFloat("###Acceleration", &m_acceleration, 0.01f, 0.0f, 3.0f);

    ImGui::Text(U8_TEXT("最大速度"));
    ImGui::DragFloat("###MaxSpeed", &m_maxSpeed, 0.1f, 0.1f, 50.0f);

    ImGui::Text(U8_TEXT("泳ぎアニメーションの速度"));
    ImGui::DragFloat("###SwimAnimSpeed", &m_swimAnimSpeed, 0.1f, 0.1f, 10.0f);

    ImGui::Text(U8_TEXT("チャージアニメーションの速度"));
    ImGui::DragFloat("###ChargeAnimSpeed", &m_chargeAnimSpeed, 0.1f, 0.1f, 10.0f);

    ImGui::Separator();

    // 回転速度
    ImGui::Text("Input Direction: %.2f, %.2f, %.2f", m_inputDir.x, m_inputDir.y, m_inputDir.z);

    ImGui::Text(U8_TEXT("X軸の回転"));
    ImGui::DragFloat("###RotationX", &m_tiltAngle, 0.1f, -360.0f, 360.0f);

    ImGui::Separator();

    ImGui::Text(U8_TEXT("ノックバックのときの減速率"));
    ImGui::DragFloat("###KnockBackDelFactor", &m_knockBackDelFactor, 0.01f, 0.0f, 1.0f);
}
