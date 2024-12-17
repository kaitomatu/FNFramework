#include "InputMoveComponent.h"

#include "Application/Application.h"
#include "Application/Component/TransformComponent/TransformComponent.h"

void InputMoveComponent::Awake()
{
    m_slideSpeed = 1000.0f;
}

void InputMoveComponent::Serialize(Json& _json) const
{
    // 滑走速度を保存
    _json[jsonKey::Comp::InputMoveComponent::SlideSpeed.data()] = m_slideSpeed;
}

void InputMoveComponent::Deserialize(const Json& _json)
{
    // 滑走速度を復元
    m_slideSpeed = _json.at(jsonKey::Comp::InputMoveComponent::SlideSpeed.data()).get<float>();
}

void InputMoveComponent::ImGuiUpdate()
{
    ImGui::DragFloat("SlideSpeed", &m_slideSpeed, 1.0f, 0.0f, 10000.0f);
}

void InputMoveComponent::Update()
{
    if (!OwnerValid()) { return; }

    const std::shared_ptr<TransformComponent>& trans = m_wpOwnerObj.lock()->GetTransformComponent();
    const auto& spCamera = ShaderManager::Instance().FindCameraData(RenderingData::MainCameraName);

    if (!spCamera) { return; }

    const Math::Matrix& camMat = spCamera->GetViewMat().Invert();

    Math::Vector3 moveDirection = Math::Vector3::Zero;

    if (InputSystem::Instance().IsHold("Forward"))
    {
        moveDirection += camMat.Backward();
        moveDirection.y = 0.0f;
    }
    if (InputSystem::Instance().IsHold("Backward"))
    {
        moveDirection -= camMat.Backward();
        moveDirection.y = 0.0f;
    }
    if (InputSystem::Instance().IsHold("Right"))
    {
        moveDirection += camMat.Right();
        moveDirection.y = 0.0f;
    }
    if (InputSystem::Instance().IsHold("Left"))
    {
        moveDirection -= camMat.Right();
        moveDirection.y = 0.0f;
    }

    // 上下移動 : 右クリック + スペースキー or シフトキー
    if(InputSystem::Instance().IsHold("Click"))
    {
        if (InputSystem::Instance().IsHold("Space"))
        {
            moveDirection += Math::Vector3::Up;
        }
        if (InputSystem::Instance().IsHold("Shift"))
        {
            moveDirection -= Math::Vector3::Up;
        }
    }

    // 移動処理の計算
    if (moveDirection != Math::Vector3::Zero)
    {
        float speed = m_slideSpeed * static_cast<float>(SceneManager::Instance().FrameDeltaTime());
        moveDirection.Normalize();
        const Math::Vector3& beforePos = trans->GetWorldPos();
        trans->SetPosition(beforePos + moveDirection * speed);
    }
}
