#include "SeaWeedWallScript.h"

#include "Application/Component/Script/PlayerScript/PlayerScript.h"
#include "Application/Component/Collision/CollisionComponent/CollisionComponent.h"

void SeaWeedWallScript::Awake()
{
    
}

void SeaWeedWallScript::Start()
{
    m_wpCollisionComponent = GetOwner()->GetComponent<CollisionComponent>();

    const auto& wpPlayerObj= GetOwner()->GetScene()->FindObject("Player");
    m_wpPlayerScript = wpPlayerObj->GetComponent<PlayerScript>();
}

void SeaWeedWallScript::Update()
{ 
    if (m_wpPlayerScript.expired() || m_wpCollisionComponent.expired())
    {
        return;
    }

    const auto& spColComp = m_wpCollisionComponent.lock();

    // 自身を含めた合計人数（自身 + 子どもたちの数）
    int totalPushers = 1 + m_wpPlayerScript.lock()->GetChildCount(); // 自身を1人として加算

    m_canOpen = totalPushers >= m_requiredPusherCount;

    // 壁を開くのに必要な人数を満たしているかつ、トリガーが有効な場合に壁を開く
    if (!m_openTrigger || !m_canOpen)
    {
        // 満たしていない場合は壁を閉じる（当たり判定を追加）
        spColComp->GetCollider()->AddTypeAll(KdCollider::TypeObstacle);
        return;
    }

    // 条件を満たしている場合は壁を開ける（当たり判定を解除）
    spColComp->GetCollider()->RemoveTypeAll(KdCollider::TypeObstacle);
}

void SeaWeedWallScript::Release()
{
    
}

void SeaWeedWallScript::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::SeaWeedWallScript::RequiredPusherCount.data()] = m_requiredPusherCount;
}

void SeaWeedWallScript::Deserialize(const Json& _json)
{
    m_requiredPusherCount = _json.value(jsonKey::Comp::SeaWeedWallScript::RequiredPusherCount.data(), 0);
}

void SeaWeedWallScript::ImGuiUpdate()
{
    ImGui::Text(U8_TEXT("壁を空けるのに必要な子どもの数"));
    ImGui::DragInt("##RequiredCount", &m_requiredPusherCount, 1, 0, 10);

    ImGui::Text(U8_TEXT("条件を満たしているかどうか: %s"), m_canOpen ? "true" : "false");
}
