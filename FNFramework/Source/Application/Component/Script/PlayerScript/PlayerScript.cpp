#include "PlayerScript.h"

#include "Application/Application.h"
#include "Application/Component/LightComponent/LightAnimationScript.h"
#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"

void EventStateChange(
    KurageMoveScript::KurageState _state,
    std::shared_ptr<KurageMoveScript> _spKurageMoveScript,
    std::list<PlayerScript::ChildData> _childData)
{

    if (!_spKurageMoveScript) { return; }

    _spKurageMoveScript->SetKurageState(_state);

    for (auto&& child : _childData)
    {
        if (child.wpChildObj.expired()) { continue; }

        const std::shared_ptr<KurageMoveScript>& spChildKurageMoveScript = child.wpKurageMoveScript.lock();

        if (!spChildKurageMoveScript) { continue; }

        spChildKurageMoveScript->SetKurageState(_state);
    }
}

void PlayerScript::AddChild(const ChildData& childData)
{
    m_childDataList.push_back(childData);
}

void PlayerScript::RemoveChild(const std::shared_ptr<GameObject>& childObj)
{
    m_childDataList.remove_if([&](const ChildData& childData)
        {
            // weak_ptr を shared_ptr に変換して比較
            if (auto spChild = childData.wpChildObj.lock())
            {
                return spChild == childObj;  // 一致したら削除
            }
            return false;  // 無効な weak_ptr は削除しない
        });
}

void PlayerScript::Awake()
{
}

void PlayerScript::Start()
{
    if (!OwnerValid()) { return; }

    m_wpKurageMoveScript = m_wpOwnerObj.lock()->GetComponent<KurageMoveScript>();

    auto spLigComponent = m_wpOwnerObj.lock()->GetComponent<LightComponent>();

    if (!spLigComponent)
    {
        spLigComponent = m_wpOwnerObj.lock()->GetComponent<LightAnimationScript>();
    }

    spLigComponent->SetColor(Math::Vector3{ Color::DeepBlue.x, Color::DeepBlue.y, Color::DeepBlue.z });
}

void PlayerScript::Update()
{
    // メインカメラを取得 : このカメラの向きによって移動方向を決める
    const auto& spCamera = ShaderManager::Instance().FindCameraData(RenderingData::MainCameraName);

    if (!spCamera || m_wpKurageMoveScript.expired()) { return; }

    const std::shared_ptr<KurageMoveScript>& spKurageMoveScript = m_wpKurageMoveScript.lock();

    const Math::Matrix& camMat = spCamera->GetViewMat().Invert();

    Math::Vector3 inputDir = spKurageMoveScript->GetInputDirection();

    //x--- 入力方向の計算 ---x//

    bool isMove = false;

    if (InputSystem::Instance().IsHold("Forward"))
    {
        inputDir += camMat.Backward();
        isMove = true;
    }
    else if (InputSystem::Instance().IsHold("Backward"))
    {
        inputDir -= camMat.Backward();
        isMove = true;
    }

    if (InputSystem::Instance().IsHold("Right"))
    {
        inputDir += camMat.Right();
        isMove = true;
    }
    else if (InputSystem::Instance().IsHold("Left"))
    {
        inputDir -= camMat.Right();
        isMove = true;
    }

    // この入力で行うのは x, z 軸の移動なので、y 軸の移動は無視
    inputDir.y = 0.0f;

    inputDir.Normalize();

    // イベントの処理 //
    if (!m_isEventActive)
    {
        // デフォルトモードに移行
        EventStateChange(
            KurageMoveScript::KurageState::eDefault,
            spKurageMoveScript,
            m_childDataList);

        // 非イベントモードのときにのみ、入力を受け付ける。
        spKurageMoveScript->SetInputDirection(inputDir);
        spKurageMoveScript->SetMoveFlg(isMove);

        return;
    }

    // イベントモードに移行
    EventStateChange(
        KurageMoveScript::KurageState::eEvent,
        spKurageMoveScript,
        m_childDataList);
}

void PlayerScript::Release()
{
    m_childDataList.clear();
}

void PlayerScript::Serialize(Json& _json) const
{
}

void PlayerScript::Deserialize(const Json& _json)
{
}

void PlayerScript::ImGuiUpdate()
{
    ImGui::Text(U8_TEXT("子どもの数: %d"), m_childDataList.size());
}
