#include "ChildController.h"

#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"
#include "Application/Component/Script/PlayerScript/PlayerScript.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "ChildState/ItemState/ChildKurageItemState.h"

void ChildController::Awake()
{
}

void ChildController::Start()
{
    if (!OwnerValid()) { return; }

    const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

    m_wpKurageMoveScript = spOwner->GetComponent<KurageMoveScript>();
    m_wpLightComponent = spOwner->GetComponent<LightComponent>();
    ;
    // プレイヤーオブジェクトを名前で復元
    if (!m_playerName.empty())
    {
        auto playerObj = spOwner->GetScene()->FindObject(m_playerName);
        if (playerObj)
        {
            m_wpPlayerScript = playerObj->GetComponent<PlayerScript>();
        }
    }

    m_childController.SetUp(this);
    m_childController.AddState<ChildKurageItemState>();
}

void ChildController::PreUpdate()
{
    // 初回更新時に初期座標を保存
    if (!m_firstUpdate)
    {
        const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

        m_basePosition = spOwner->GetTransformComponent()->GetLocalPos();

        m_firstUpdate = true;
    }
}

void ChildController::Update()
{
    // イベント状態のときはイベント側に操作を任せる
    if(m_wpKurageMoveScript.lock()->GetKurageState() == KurageMoveScript::KurageState::eEvent)
    {
        return;
    }

    SceneManager::Instance().GetDebugWire()->AddDebugSphere(m_basePosition, Color::White, 0.3f);
    SceneManager::Instance().GetDebugWire()->AddDebugLine(m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldPos(), m_basePosition, Color::White);

    m_childController.Update();
}

void ChildController::Release()
{
    m_childController.Clean();
}

void ChildController::Serialize(Json& _json) const
{
    // 初期座標を保存
    _json[jsonKey::Comp::ChildController::BasePosition.data()] =
    {
        m_basePosition.x,
        m_basePosition.y,
        m_basePosition.z
    };

    // プレイヤーオブジェクトの名前を保存
    if (auto playerScript = m_wpPlayerScript.lock())
    {
        _json[jsonKey::Comp::ChildController::PlayerObject.data()] = playerScript->GetOwner()->GetName();
    }
}

void ChildController::Deserialize(const Json& _json)
{
    // 初期座標の復元
    if (auto basePos = _json.find(jsonKey::Comp::ChildController::BasePosition.data());
        basePos != _json.end() && basePos->is_array() && basePos->size() >= 3)
    {
        m_basePosition = Math::Vector3{ (*basePos)[0].get<float>(),
                                        (*basePos)[1].get<float>(),
                                        (*basePos)[2].get<float>() };
    }

    // プレイヤーオブジェクト名の復元
    m_playerName = _json.value(jsonKey::Comp::ChildController::PlayerObject.data(), std::string{});
}

void ChildController::ImGuiUpdate()
{
    ImGui::DragFloat3(U8_TEXT("基準座標"), &m_basePosition.x, 0.01f);

    ImGui::Text(U8_TEXT("ベース座標の方向の影響度"));
    ImGui::DragFloat3("##PosEffective", &ChildControllerSetting::PosEffective.x, 0.01f, 0.0f, 1.0f);
    ImGui::Text(U8_TEXT("ランダム成分の影響度"));
    ImGui::DragFloat3("##RandomEffective", &ChildControllerSetting::RandomEffective.x, 0.01f, 0.0f, 1.0f);

    ImGui::Separator();

    ImGui::Text(U8_TEXT("現在のステート: %s"), m_childController.GetNowStateName().data());
    m_childController.ImGui();
}
