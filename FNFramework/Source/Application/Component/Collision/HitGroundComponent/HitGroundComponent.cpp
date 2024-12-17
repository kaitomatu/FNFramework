#include "HitGroundComponent.h"

#include "Application/Component/Collision/CollisionComponent/CollisionComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"

void HitGroundComponent::Awake()
{

}

void HitGroundComponent::Start()
{
    if (!OwnerValid()) { return; }

    const auto& spScene = m_wpOwnerObj.lock()->GetScene();

    // 衝突コンポーネントを名前で復元
    m_wpCollisionComponent = m_wpOwnerObj.lock()->GetComponent<CollisionComponent>();
}

void HitGroundComponent::Update()
{
    if (!OwnerValid()) { FNENG_ASSERT_ERROR("HittableObjectComponentのオーナーが設定されていません"); return; }

    const std::shared_ptr<TransformComponent>& spTransform = m_wpOwnerObj.lock()->GetTransformComponent();

    if (m_wpCollisionComponent.expired()) { return; }

    // 地面と当たっている場合、押し出し処理を行う
    const CollisionHelper::SphereHitData& SphereHitData = m_wpCollisionComponent.lock()->GetSphereHitData();

    if (!SphereHitData.IsHit || !(SphereHitData.HitType & KdCollider::TypeBump)) { return; }

    const Math::Vector3& nowPos = spTransform->GetWorldPos();
    const Math::Vector3& hitPos = SphereHitData.HitDir * SphereHitData.MaxOverLap;

    // 当たった場所の向きを取得
    Math::Vector3 objNorm = Math::Vector3{ Math::Vector3::Zero };

    SphereHitData.HitDir.Normalize(objNorm);

    Math::Vector3 downNorm = Math::Vector3::Down;
    downNorm.Normalize();

    float dot = downNorm.Dot(objNorm * -1.0f);
    float rad = std::acos(dot); // acosはラジアンを返す

    // 崖の判定
    if (rad < MathHelper::ConvertToRadians(m_cliffRate))
    {
    }

    Math::Vector3 newPos = nowPos + hitPos;

    // y 軸の押し出しは行わない
    newPos.y = nowPos.y;

    spTransform->SetPosition(newPos);
}

void HitGroundComponent::Serialize(Json& _json) const
{
    // 崖の判定用の内積角度（CliffRate）を保存
    _json[jsonKey::Comp::HitGroundComponent::CliffRate.data()] = m_cliffRate;

    // 衝突コンポーネントのオブジェクト名を保存
    if (auto collisionComp = m_wpCollisionComponent.lock())
    {
        _json[jsonKey::Comp::HitGroundComponent::CollisionComponentName.data()] = collisionComp->GetOwner()->GetName();
    }
}

void HitGroundComponent::Deserialize(const Json& _json)
{
    // 崖の判定用の内積角度（CliffRate）を復元
    m_cliffRate = _json.at(jsonKey::Comp::HitGroundComponent::CliffRate.data()).get<float>();
}

void HitGroundComponent::ImGuiUpdate()
{
    if (m_wpCollisionComponent.expired()) { return; }

    ImGui::DragFloat(U8_TEXT("崖を登れる角度"), &m_cliffRate, 0.1f, 0.0f, 180.0f);

    /* スフィアの当たり判定とレイ判定の要素を出しておく */
    {
        const CollisionHelper::SphereHitData& SphereHitData = m_wpCollisionComponent.lock()->GetSphereHitData();
        ImGui::Text("---------- SphereHitData ----------");
        ImGui::Text("IsHit : %s", SphereHitData.IsHit ? "true" : "false");
        ImGui::Text("MaxOverLap : %f", SphereHitData.MaxOverLap);
        ImGui::Text("HitDir : %.3f, %.3f, %.3f", SphereHitData.HitDir.x, SphereHitData.HitDir.y, SphereHitData.HitDir.z);
        ImGui::Text("HitObjectName : %s", SphereHitData.HitObjectName.c_str());
    }

    {
        const CollisionHelper::RayHitData& RayHitData = m_wpCollisionComponent.lock()->GetRayHitData();
        ImGui::Text("---------- SphereHitData ----------");
        ImGui::Text("IsHit : %s", RayHitData.IsHit ? "true" : "false");
        ImGui::Text("HitDir : %.3f, %.3f, %.3f", RayHitData.HitPosition.x, RayHitData.HitPosition.y, RayHitData.HitPosition.z);
        ImGui::Text("HitObjectName : %s", RayHitData.HitObjectName.c_str());
    }
}
