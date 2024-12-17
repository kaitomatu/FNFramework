#include "KnockBackScript.h"
#include "Application/Component/Collision/CollisionComponent/CollisionComponent.h"

#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveState/KurageMoveState.h"

#include "Application/Component/TransformComponent/TransformComponent.h"

void KnockBackScript::Start()
{
    if (!OwnerValid()) { return; }

    const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

    m_wpCollisionComponent = spOwner->GetComponent<CollisionComponent>();
    m_wpKurageMoveComponent = spOwner->GetComponent<KurageMoveScript>();
}

void KnockBackScript::Update()
{
    if (!OwnerValid()) { return; }

    if (m_wpCollisionComponent.expired() || m_wpKurageMoveComponent.expired()) { return; }

    // コライダーからSphereの当たり判定情報を取得
    const CollisionHelper::SphereHitData& sphereHitData = m_wpCollisionComponent.lock()->GetSphereHitData();

    // 当たり判定が発生したか確認
    if (!sphereHitData.IsHit)
    {
        m_isKnockBack = false;
        return;
    }

    if (!(sphereHitData.HitType & KdCollider::TypeKnockBack)) { return; }

    // ノックバック中は処理を行わない
    if (m_isKnockBack) { return; }

    const std::shared_ptr<KurageMoveScript>& spKurageMove = m_wpKurageMoveComponent.lock();

    // 最初に入力方向を変更しておく
    Math::Vector3 pushBackDir = sphereHitData.HitDir;

    pushBackDir.y = 0.0f;
    pushBackDir.Normalize();
    spKurageMove->SetInputDirection(pushBackDir);

    const auto& nowState = spKurageMove->GetKurageMoveController().GetNowState<KurageMoveBaseState>();

    Math::Vector3 nowVel = Math::Vector3::Zero;

    if (nowState)
    {
        nowVel = nowState->GetVelocity();
    }

    auto knockBackState = spKurageMove->GetKurageMoveController().AddState<KnockBackState>(true);
    knockBackState->SetVelocity(nowVel);

    m_wpKnockBackState = knockBackState;

    m_isKnockBack = true;
}

void KnockBackScript::Serialize(Json& _json) const
{
}

void KnockBackScript::Deserialize(const Json& _json)
{
}

void KnockBackScript::ImGuiUpdate()
{
}
