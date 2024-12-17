#include "BillboardScript.h"

#include "Application/Component/Camera/TrackingCameraComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"

void BillboardScript::Awake()
{

}

void BillboardScript::Start()
{
}

void BillboardScript::Update()
{
    if (!OwnerValid()) { return; }

    const auto& mainCamera = ShaderManager::Instance().FindCameraData(RenderingData::MainCameraName);

    if (!mainCamera) { return; }

    const std::shared_ptr<TransformComponent> spTransform = m_wpOwnerObj.lock()->GetComponent<TransformComponent>();

    const Math::Vector3& objPos = spTransform->GetWorldPos();

    const Math::Vector3& cameraPos = mainCamera->GetCBData().CamPos;

    // カメラからオブジェクトへの方向ベクトルを計算
    Math::Vector3 direction = cameraPos - objPos;
    direction.Normalize();

    // オイラー角でY軸の回転を求める
    float yaw = std::atan2(direction.x, direction.z);  // Yaw角を計算

    spTransform->SetRotationY(MathHelper::ConvertToDegrees(yaw));
}

void BillboardScript::Release()
{

}

void BillboardScript::Serialize(Json& _json) const
{
}

void BillboardScript::Deserialize(const Json& _json)
{
}

void BillboardScript::ImGuiUpdate()
{
}
