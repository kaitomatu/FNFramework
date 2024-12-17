#include "GoalScript.h"

#include "Application/Component/TransformComponent/TransformComponent.h"
#include "GoalState/ResultState.h"
#include "../StageScript/StageScript.h"

void GoalScript::Awake()
{

}

void GoalScript::Start()
{
    m_isResult = false;

    m_wpPlayer = SceneManager::Instance().GetNowScene()->FindObject(m_targetObjectName);

    if (const auto& spStageObj = SceneManager::Instance().GetNowScene()->FindObject("StageScript"))
    {
        m_wpStageScript = spStageObj->GetComponent<StageScript>();
    }
    m_resultController.SetUp(this);
}

void GoalScript::Update()
{

    // すでに ResultState が存在している場合は何もしない
    if (m_resultController.GetNowStateName() != typeid(ResultState).name())
    {
        if (m_isResult)
        {
            // 次のシーンに行くため State を Pop してからシーンの読み込みを行う
            m_resultController.AddState<ResultState>(/* _isPop = */true);
        }
    }

    CheckGoalTouch();

    m_resultController.Update();
}

void GoalScript::CheckGoalTouch()
{
    if (!OwnerValid() || m_wpPlayer.expired()) { return; }

    const Math::Vector3& playerPos = m_wpPlayer.lock()->GetTransformComponent()->GetWorldPos();
    const Math::Vector3& goalPos = m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldPos();

    // ゴールとプレイヤー間のベクトルを計算
    Math::Vector3 goalVec = goalPos - playerPos;

    // ゴールに近づいたかを判定 (しきい値は1.0fで設定)
    Math::Vector4 color = Color::White;
    if (goalVec.Length() < m_goalActivationDistance)
    {
        m_isResult = true;
        color = Color::Red;
    }

    SceneManager::Instance().GetDebugWire()->AddDebugSphere(goalPos, color, m_goalActivationDistance);
}

void GoalScript::Release()
{

}

void GoalScript::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::GoalScript::GoalActivationDistance.data()] = m_goalActivationDistance;

    _json[jsonKey::Comp::GoalScript::TargetObjectName.data()] = m_targetObjectName;
}

void GoalScript::Deserialize(const Json& _json)
{
    m_goalActivationDistance = _json[jsonKey::Comp::GoalScript::GoalActivationDistance.data()];

    m_targetObjectName = _json[jsonKey::Comp::GoalScript::TargetObjectName.data()];
}

void GoalScript::ImGuiUpdate()
{
    ImGui::Checkbox(U8_TEXT("リザルトシーンへ移行"), &m_isResult);

    ImGui::DragFloat(U8_TEXT("ゴール有効距離"), &m_goalActivationDistance, 0.1f, 0.0f, 200.0f);

    m_resultController.ImGui();
}
