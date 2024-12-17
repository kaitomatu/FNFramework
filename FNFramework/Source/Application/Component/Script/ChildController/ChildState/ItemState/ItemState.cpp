#include "ItemState.h"

#include <complex>

#include "../ChildState/ChildState.h"
#include "../../../../Renderer/ModelComponent/ModelComponent.h"
#include "../../../../Script/ChildController/ChildController.h"
#include "../../../../TransformComponent/TransformComponent.h"
#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"
#include "Application/Component/Script/PlayerScript/PlayerScript.h"

void ItemState::Enter(ChildController* _pOwner)
{
    auto spModelComp = _pOwner->GetOwner()->GetComponent<ModelComponent>(false);

    // モデルコンポーネントが存在しない場合はアニメーションコンポーネントの方も検索
    if (!spModelComp)
    {
        spModelComp = _pOwner->GetOwner()->GetComponent<AnimationComponent>();
    }

    m_wpModelComponent = spModelComp;
}

void ItemState::Update(ChildController* _pOwner)
{
    const std::shared_ptr<PlayerScript>& spPlayerScript = _pOwner->GetPlayerScript();

    if (!spPlayerScript)
    {
        // プレイヤーが存在しない場合は何もしない
        return;
    }

    const std::shared_ptr<GameObject>& spPlayerObj = spPlayerScript->GetOwner();
    const std::shared_ptr<TransformComponent>& spPlayerTransform = spPlayerObj->GetTransformComponent();

    if (!spPlayerTransform)
    {
        return;
    }

    const Math::Vector3& playerPos = spPlayerTransform->GetWorldPos();
    const Math::Vector3& childPos = _pOwner->GetOwner()->GetTransformComponent()->GetWorldPos();

    SceneManager::Instance().GetDebugWire()->AddDebugSphere(childPos, Color::Blue, m_contactDistance);

    // プレイヤーの子オブジェクトリストを取得
    const std::list<PlayerScript::ChildData>& childList = spPlayerScript->GetChildList();

    // プレイヤーとその子オブジェクト全体で最も近い距離を探す
    float minDistance = (childPos - playerPos).Length();  // 初期値としてプレイヤーとの距離を設定

    for (const auto& childData : childList)
    {
        // オブジェクトが無効な場合はスキップ
        if (childData.wpChildObj.expired())
        {
            continue;
        }

        const std::shared_ptr<GameObject>& spChild = childData.wpChildObj.lock();

        const std::shared_ptr<TransformComponent>& childTransform = spChild->GetTransformComponent();
        if (childTransform)
        {
            float distance = (childPos - childTransform->GetWorldPos()).Length();
            minDistance = std::min(minDistance, distance);  // 最小距離を更新
        }
    }

    // 最小距離が接触距離以下であればステートを変更
    if (minDistance <= m_contactDistance)
    {
        _pOwner->GetChildController().PopState();
        _pOwner->GetChildController().AddState<ChildState>();
    }
}

void ItemState::Exit(ChildController* /* _pOwner */)
{
}

void ItemState::ImGui(ChildController* /* _pOwner */)
{
    ImGui::DragFloat(U8_TEXT("接触範囲"), &m_contactDistance, 0.5f, 0.0f, 100.0f);
}
