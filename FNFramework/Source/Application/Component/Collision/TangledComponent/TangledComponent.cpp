#include "TangledComponent.h"

#include "Application/Component/Collision/CollisionComponent/CollisionComponent.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveState/KurageMoveState.h"

void TangledComponent::Start()
{
    if (!OwnerValid()) { return; }

    const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

    m_wpCollisionComponent = spOwner->GetComponent<CollisionComponent>();
    m_wpKurageMoveComponent = spOwner->GetComponent<KurageMoveScript>();

    m_isRelease = true;
}

void TangledComponent::Update()
{
    if (!OwnerValid()) { return; }

    if (m_wpCollisionComponent.expired() || m_wpKurageMoveComponent.expired()) { return; }

    const std::shared_ptr<KurageMoveScript>& spKurageMove = m_wpKurageMoveComponent.lock();

    // コライダーからSphereの当たり判定情報を取得
    const CollisionHelper::SphereHitData& sphereHitData = m_wpCollisionComponent.lock()->GetSphereHitData();

    // 当たり判定が発生したか確認
    if (!sphereHitData.IsHit || !(sphereHitData.HitType & KdCollider::TypeObstacle))
    {
        m_isRelease = true;
        return;
    }

    if (!m_isRelease) { return; }

    // 当たったオブジェクトを取得
    const auto& spHitObj = SceneManager::Instance().GetNowScene()->FindObject(sphereHitData.HitObjectName);

    const auto& tangledState = spKurageMove->GetKurageMoveController().AddState<TangledState>(true);
    tangledState->SetTangleDir(sphereHitData.HitDir);

    m_isRelease = false;
}

void TangledComponent::Release()
{
}

void TangledComponent::Serialize(Json& _json) const
{
    
}

void TangledComponent::Deserialize(const Json& _json)
{
    
}

void TangledComponent::ImGuiUpdate()
{
    
}
