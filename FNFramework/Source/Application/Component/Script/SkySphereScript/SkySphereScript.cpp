#include "SkySphereScript.h"

#include "Application/Component/TransformComponent/TransformComponent.h"

void SkySphereScript::Start()
{
    if(!OwnerValid()) { return; }

    // ターゲットが無効な場合は、名前からオブジェクトを取得
    if(m_wpTarget.expired() && !m_targetName.empty())
    {
        m_wpTarget = m_wpOwnerObj.lock()->GetScene()->FindObject(m_targetName);
    }
}

void SkySphereScript::Update()
{
    if(!OwnerValid()) { return; }

    // 座標設定用の自身のTransformComponent
    const std::shared_ptr<TransformComponent>& spTrans = m_wpOwnerObj.lock()->GetTransformComponent();

    // ターゲットが設定されている場合は、座標設定用のターゲットのTransformComponentを取得
    const std::shared_ptr<GameObject>& target = m_wpTarget.lock();

    if(!target) { return; }

    const std::shared_ptr<TransformComponent>& spTargetTrans = target->GetTransformComponent();

    if(!PreClassValid(spTrans) || !PreClassValid(spTargetTrans)) { return; }

    spTrans->SetPosition(spTargetTrans->GetWorldPos());
}

void SkySphereScript::ImGuiUpdate()
{
    if(!OwnerValid()) { return; }

    // ターゲット名を入力して、ボタンを押すとターゲットを変更
    utl::ImGuiHelper::InputTextWithString("TargetName", m_targetName);

    if(ImGui::Button("ChangeTarget"))
    {
        const std::shared_ptr<GameObject>& spTargetObj = m_wpOwnerObj.lock()->GetScene()->FindObject(m_targetName);

        // 失敗したら名前を Failed にする
        if(!spTargetObj)
        {
            m_targetName = "Failed";
            return;
        }

        SetTarget(spTargetObj);
    }

    if(m_wpTarget.expired()) { return; }

    ImGui::Text("NowTargetName : %s", m_wpTarget.lock()->GetName());
}

void SkySphereScript::Serialize(Json& _json) const
{
    // ターゲットオブジェクトの名前を保存
    _json[jsonKey::Comp::SkySphereScript::TargetName.data()] = m_wpTarget.lock()->GetName();
}

void SkySphereScript::Deserialize(const Json& _json)
{
    // ターゲットオブジェクトの名前を読み込み
    m_targetName = _json.value(jsonKey::Comp::SkySphereScript::TargetName.data(), "");
}
