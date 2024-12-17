#include "WallEventScript.h"
#include "../../TransformComponent/TransformComponent.h"
#include "../SeaweedRenderingScript/SeaweedRenderingScript.h"
#include "../PlayerScript/PlayerScript.h"
#include "../KurageMoveScript/KurageMoveScript.h"
#include "../KurageMoveScript/KurageMoveState/KurageMoveState.h"
#include "../SeaWeedWallScript/SeaWeedWallScript.h"

void WallEventScript::Awake()
{
    m_eventData.EventLateMovePos = Math::Vector3::Zero;

    m_basePosition = m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldPos();

    // AABBの構築
    ConstructAABB({ 50.0f, 50.0f, 50.0f });

    m_eventData.EventLateMovePos = m_basePosition;
}

void WallEventScript::Start()
{
    m_wpSeaWeedWallScript = GetOwner()->GetComponent<SeaWeedWallScript>();
    m_wpSeaweedRenderingScript = GetOwner()->GetComponent<SeaweedRenderingScript>();

    m_baseAnimSpeed = m_wpSeaweedRenderingScript.lock()->GetAnimationSpeed();

    // オブジェクトがすでに設定されている場合はスキップ
    if (m_eventObjectName.empty()) { return; }

    const auto& spEventObj = SceneManager::Instance().GetNowScene()->FindObject(m_eventObjectName);
    if (!spEventObj) { return; }

    m_wpEventObject = spEventObj;
    m_wpPlayerScript = spEventObj->GetComponent<PlayerScript>();
    m_wpPlayerKurageMoveScript = spEventObj->GetComponent<KurageMoveScript>();
}

void WallEventScript::Update()
{
    if (!OwnerValid() || m_wpEventObject.expired() || m_wpPlayerScript.expired())
    {
        return;
    }

    const auto& spPlayerObj = m_wpPlayerScript.lock()->GetOwner();
    const Math::Vector3& playerPos = spPlayerObj->GetTransformComponent()->GetWorldPos();

    Math::Color areaColor = Color::Blue;

    static bool m_eventActiveFrame = true;

    // もしアニメーションスピードが変更されていたら、元に戻す
    if(const auto& spAnimComp = m_wpSeaweedRenderingScript.lock())
    {
        float nowAnimSpeed = spAnimComp->GetAnimationSpeed();
        if (nowAnimSpeed > m_baseAnimSpeed)
        {
            nowAnimSpeed -= m_animSubsidePow * SceneManager::Instance().FrameDeltaTime();
            spAnimComp->SetAnimationSpeed(nowAnimSpeed);
        }
    }

    if (m_eventData.EventArea.Contains(playerPos))
    {
        // イベントがアクティブになった最初のフレームで、イベントの有効かどうかを判定する。
        if (m_eventActiveFrame)
        {
            // 自分とイベント側のベクトルを算出する //
            const Math::Vector3& eventCenter = m_eventData.EventArea.GetCenter();

            // イベント中心からプレイヤーの位置へのベクトル
            Math::Vector3 toPlayerPosVec = playerPos - eventCenter;
            // イベント中心からイベント終了後の移動座標へのベクトル
            Math::Vector3 toEventLateVec = m_eventData.EventLateMovePos - eventCenter;

            // イベント終了後の移動座標と上で計算したベクトルの内積を見て、このイベントが有効かどうかを判定する

            // 反対方向の場合はイベントは無効なので、イベントを終了する
            if (toEventLateVec.Dot(toPlayerPosVec) >= 0.0f)
            {
                m_eventPhase = EventPhase::Finished;
            }

            m_eventActiveFrame = false;
        }

        m_wpPlayerScript.lock()->SetIsEventActive(true);
        Event();
        areaColor = Color::Red;
    }
    else
    {
        m_eventActiveFrame = true;

        m_eventPhase = EventPhase::MovingToInitialPositions;

        m_wpPlayerScript.lock()->SetIsEventActive(false);
    }

    // デバッグ用の表示
    if (m_eventPhase == EventPhase::Finished)
    {
        areaColor = Color::DeepBlue;
    }

    SceneManager::Instance().GetDebugWire()->AddDebugSphere(m_eventData.EventLateMovePos, Color::Red, .5f);
    SceneManager::Instance().GetDebugWire()->AddDebugBox(m_eventData.EventArea, areaColor);
}

void WallEventScript::ConstructAABB(const Math::Vector3& size)
{
    // AABBを基準座標を中心に構築
    m_eventData.EventArea.SetMin(m_basePosition - size * 0.5f);
    m_eventData.EventArea.SetMax(m_basePosition + size * 0.5f);
}

void WallEventScript::ImGuiUpdate()
{

    ImGui::Text(U8_TEXT("現在のイベントの状態: [%s]"), utl::str::EnumToString(m_eventPhase).c_str());

    ImGui::Text(U8_TEXT("イベント後の移動座標"));
    ImGui::DragFloat3("##EventLateMovePos", &m_eventData.EventLateMovePos.x, 0.1f);

    Math::Vector3 size = m_eventData.EventArea.GetSize();

    // イベント範囲の基準点
    ImGui::Text(U8_TEXT("イベント範囲の基準点"));
    if (ImGui::DragFloat3("##BasePosition", &m_basePosition.x, 0.1f))
    {
        ConstructAABB(size);
    }

    // イベント範囲のサイズ
    ImGui::Text(U8_TEXT("イベント範囲のサイズ"));
    if (ImGui::DragFloat3("##Area Size", &size.x, 0.1f))
    {
        ConstructAABB(size);
    }

    // イベント対象オブジェクトのアタッチ //
    utl::ImGuiHelper::InputTextWithString(U8_TEXT("イベント対象の名前"), m_eventObjectName);

    if (ImGui::Button(U8_TEXT("イベントオブジェクトのアタッチ")))
    {
        const std::shared_ptr<GameObject>& spEventObj = m_wpOwnerObj.lock()->GetScene()->FindObject(m_eventObjectName);
        if (!spEventObj)
        {
            return;
        }

        m_wpEventObject = spEventObj;
        m_wpPlayerScript = spEventObj->GetComponent<PlayerScript>();
        m_wpPlayerKurageMoveScript = spEventObj->GetComponent<KurageMoveScript>();
    }
}

void WallEventScript::EventRequestTimeCheck(float _requestEndTime, EventPhase _nextPhase)
{
    m_requestTime += SceneManager::Instance().FrameDeltaTime();
    if (m_requestTime < _requestEndTime) { return; }

    m_requestTime = 0.0f;

    // イベントの終了
    m_eventPhase = _nextPhase;
}

void WallEventScript::Event()
{
    // ターゲットポジションの計算
    CalcTargetPosition();

    if (m_eventData.TargetPositions.empty())
    {
        return;
    }

    // イベントフェーズに応じた処理を呼び出す
    switch (m_eventPhase)
    {
    case EventPhase::MovingToInitialPositions:
    {
        HandleInitialPositioning();

        // イベント受付時間を超えるとイベント終了 //
        constexpr float MovingToInitialPhaseRequestEndTime = 25.0f;
        EventRequestTimeCheck(MovingToInitialPhaseRequestEndTime, EventPhase::MovingToLatePosition);
    }
    break;

    case EventPhase::MovingToLatePosition:
    {
        HandleMovingToLatePosition();

        // イベント受付時間を超えるとイベント終了 //
        constexpr float MovingToLatePhaseRequestEndTime = 8.0f;
        EventRequestTimeCheck(MovingToLatePhaseRequestEndTime, EventPhase::Finished);
    }
    break;

    case EventPhase::Finished:
    {
        // イベントが終了したらプレイヤーのアニメーションスピードを元に戻す
        m_wpPlayerKurageMoveScript.lock()->SetChargeAnimSpeed(m_beforePlayerChargeAnimSpeed);
        m_wpPlayerScript.lock()->SetIsEventActive(false);
        m_requestTime = 0.0f;
    }
    break;

    default:
        break;
    }
}

void WallEventScript::HandleInitialPositioning()
{
    // すべての子どもがアイドルかどうか
    bool isAllIdleState = true;
    int index = 1;

    float childChargeAnimSpeed = 0.0f;

    // 子どもたちの移動処理 //
    for (const auto& childData : m_wpPlayerScript.lock()->GetChildList())
    {
        if (childData.wpChildObj.expired() || childData.wpKurageMoveScript.expired())
        {
            ++index;
            continue;
        }

        const std::shared_ptr<GameObject> spChildObj = childData.wpChildObj.lock();
        const std::shared_ptr < KurageMoveScript> spChildKurageMoveScript = childData.wpKurageMoveScript.lock();
        const Math::Vector3& nowPos = spChildObj->GetTransformComponent()->GetWorldPos();
        const Math::Vector3& targetPos = m_eventData.TargetPositions[index];

        // デバッグ表示
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(targetPos, Color::White, 0.5f);
        SceneManager::Instance().GetDebugWire()->AddDebugLine(nowPos, targetPos, Color::White);

        // 移動処理
        if (!MoveChildTowards(spChildKurageMoveScript, nowPos, targetPos))
        {
            isAllIdleState = false;
        }

        childChargeAnimSpeed = spChildKurageMoveScript->GetChargeAnimSpeed();

        ++index;
    }

    // 0 番目はプレイヤー自身: スクリプトが有効な場合にのみ 
    if (!m_wpEventObject.expired() && !m_wpPlayerKurageMoveScript.expired())
    {
        index = 0;// 0 番目はプレイヤー自身

        const std::shared_ptr<GameObject>& spChildObj = m_wpEventObject.lock();
        const std::shared_ptr<KurageMoveScript>& spPlayerKurageMoveScript = m_wpPlayerKurageMoveScript.lock();
        const Math::Vector3& nowPos = spChildObj->GetTransformComponent()->GetWorldPos();
        const Math::Vector3& targetPos = m_eventData.TargetPositions[index];

        // デバッグ表示
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(targetPos, Color::White, 0.5f);
        SceneManager::Instance().GetDebugWire()->AddDebugLine(nowPos, targetPos, Color::White);

        // 移動処理
        if (!MoveChildTowards(spPlayerKurageMoveScript, nowPos, targetPos))
        {
            isAllIdleState = false;
        }

        m_beforePlayerChargeAnimSpeed = spPlayerKurageMoveScript->GetChargeAnimSpeed();

        // 子どものアニメーションと同期する
        if (!m_wpPlayerScript.lock()->GetChildList().empty())
        {
            spPlayerKurageMoveScript->SetChargeAnimSpeed(childChargeAnimSpeed);
        }
    }

    if (isAllIdleState)
    {
        m_eventPhase = EventPhase::MovingToLatePosition;
        m_requestTime = 0.0f;
    }
}

void WallEventScript::HandleMovingToLatePosition()
{
    bool allChildrenAtLatePos = true;

    constexpr float eventActiveArea = 3.0f;

    // 子どもの移動処理
    for (const auto& childData : m_wpPlayerScript.lock()->GetChildList())
    {
        if (childData.wpChildObj.expired() || childData.wpKurageMoveScript.expired())
        {
            continue;
        }

        const std::shared_ptr<GameObject> spChildObj = childData.wpChildObj.lock();
        const std::shared_ptr<KurageMoveScript> spChildKurageMoveScript = childData.wpKurageMoveScript.lock();
        const Math::Vector3& nowPos = spChildObj->GetTransformComponent()->GetWorldPos();
        Math::Vector3 targetPos = m_eventData.EventLateMovePos;
        targetPos.y = nowPos.y;

        // デバッグ表示
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(targetPos, Color::Green, eventActiveArea);
        SceneManager::Instance().GetDebugWire()->AddDebugLine(nowPos, targetPos, Color::Green);

        // 移動処理
        if (!MoveChildTowards(spChildKurageMoveScript, nowPos, targetPos, eventActiveArea))
        {
            allChildrenAtLatePos = false;
        }
    }

    // プレイヤーの移動処理
    if (!m_wpEventObject.expired() && !m_wpPlayerKurageMoveScript.expired())
    {
        const std::shared_ptr<GameObject> spEventObj = m_wpEventObject.lock();
        const std::shared_ptr<KurageMoveScript> spPlayerKurageMoveScript = m_wpPlayerKurageMoveScript.lock();
        const Math::Vector3& nowPos = spEventObj->GetTransformComponent()->GetWorldPos();
        Math::Vector3 targetPos = m_eventData.EventLateMovePos;
        targetPos.y = nowPos.y;

        // デバッグ表示
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(targetPos, Color::Green, 0.5f);
        SceneManager::Instance().GetDebugWire()->AddDebugLine(nowPos, targetPos, Color::Green);

        // 移動処理
        if (!MoveChildTowards(spPlayerKurageMoveScript, nowPos, targetPos, eventActiveArea))
        {
            allChildrenAtLatePos = false;
        }
    }

    // 全員が目標地点に到達したらイベント終了
    if (allChildrenAtLatePos)
    {
        m_eventPhase = EventPhase::Finished;
        m_requestTime = 0.0f;
        return;
    }

    // 海藻のアニメーションなどの再生 //

    if (m_wpSeaWeedWallScript.expired())
    {
        return;
    }

    const auto& spSeaWeedWallScript = m_wpSeaWeedWallScript.lock();

    bool eventSuccess = spSeaWeedWallScript->CanOpen();

    if (eventSuccess)
    {
        // 足りていたら大きく揺れる
        m_wpSeaweedRenderingScript.lock()->SetAnimationSpeed(2.0f);
        spSeaWeedWallScript->SetOpenTrigger(true);
    }
    else
    {
        // 足りていなかったら小さく揺れる
        m_wpSeaweedRenderingScript.lock()->SetAnimationSpeed(0.5f);
        spSeaWeedWallScript->SetOpenTrigger(false);
    }
}

void WallEventScript::CalcTargetPosition()
{
    // プレイヤースクリプトから子どものリストを取得
    const std::list<PlayerScript::ChildData>& childList = m_wpPlayerScript.lock()->GetChildList();
    UINT eventObjectCount = childList.size();

    eventObjectCount += 1; // プレイヤーも含める

    // ターゲット座標のリストを子どもの数に合わせてリサイズ
    m_eventData.TargetPositions.resize(eventObjectCount);

    // イベントエリアの中心とサイズを取得
    const Math::Vector3& eventCenter = m_eventData.EventArea.GetCenter();
    const Math::Vector3& eventSize = m_eventData.EventArea.GetSize();

    // 各軸のサイズを取得
    float sizeX = eventSize.x;
    float sizeZ = eventSize.z;

    // 一番長い軸を判定
    char aligningAxis = 'x'; // デフォルトはx軸
    float maxSize = sizeX;

    if (sizeZ > maxSize)
    {
        aligningAxis = 'z';
        maxSize = sizeZ;
    }

    // 子どもを等間隔に配置するための計算 //
    float totalLength = maxSize;
    float spacing = totalLength / static_cast<float>(eventObjectCount + 1); // 両端にスペースを持たせる
    float startOffset = -totalLength / 2.0f + spacing;

    // 各子どものターゲット位置を計算
    for (UINT i = 0; i < eventObjectCount; ++i)
    {
        // 基本となる位置はイベントエリアの中心
        Math::Vector3 position = eventCenter;

        // 一番長い軸に沿って位置を調整
        float offset = startOffset + spacing * i;
        switch (aligningAxis)
        {
        case 'x':
            position.x += offset;
            break;
        case 'z':
            position.z += offset;
            break;
        default:
            // 想定外の軸の場合は何もしない
            break;
        }

        position.y = m_wpPlayerScript.lock()->GetOwner()->GetTransformComponent()->GetWorldPos().y;

        // 計算した位置をターゲット座標リストに設定
        m_eventData.TargetPositions[i] = position;
    }
}

bool WallEventScript::MoveChildTowards(
    const std::shared_ptr<KurageMoveScript>& _spKurageMoveScript,
    const Math::Vector3& _nowPos,
    const Math::Vector3& _targetPos,
    float _eventActiveArea)
{
    // ターゲットへ近づいたらIdleStateに移行
    if (Math::Vector3::Distance(_nowPos, _targetPos) < _eventActiveArea)
    {
        _spKurageMoveScript->SetMoveFlg(false);
        _spKurageMoveScript->GetKurageMoveController().AddState<IdleState>(true);
        return true;
    }
    else
    {
        Math::Vector3 direction = _targetPos - _nowPos;
        direction.Normalize();
        direction.y = 0.0f;

        _spKurageMoveScript->SetMoveFlg(true);
        _spKurageMoveScript->SetInputDirection(direction);
        return false;
    }
}

void WallEventScript::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::WallEventScript::EventLateMovePos.data()] = {
        m_eventData.EventLateMovePos.x,
        m_eventData.EventLateMovePos.y,
        m_eventData.EventLateMovePos.z
    };

    _json[jsonKey::Comp::WallEventScript::EventAreaSize.data()] = {
        m_eventData.EventArea.GetSize().x,
        m_eventData.EventArea.GetSize().y,
        m_eventData.EventArea.GetSize().z
    };

    _json[jsonKey::Comp::WallEventScript::BasePosition.data()] = {
        m_basePosition.x,
        m_basePosition.y,
        m_basePosition.z
    };

    if (!m_wpEventObject.expired())
    {
        _json[jsonKey::Comp::WallEventScript::EventObjectName.data()] = m_wpEventObject.lock()->GetName();
    }
}

void WallEventScript::Deserialize(const Json& _json)
{
    m_eventObjectName = _json.value(jsonKey::Comp::WallEventScript::EventObjectName.data(), "");

    auto eventMoveDir = _json.value(jsonKey::Comp::WallEventScript::EventLateMovePos.data(), Json::array({ 0.0f, 0.0f, 0.0f }));

    m_eventData.EventLateMovePos = Math::Vector3{
        eventMoveDir[0],
        eventMoveDir[1],
        eventMoveDir[2]
    };

    auto basePos = _json.value(jsonKey::Comp::WallEventScript::BasePosition.data(), Json::array({ 0.0f, 0.0f, 0.0f }));
    m_basePosition = Math::Vector3{
        basePos[0],
        basePos[1],
        basePos[2]
    };

    auto eventSize = _json.value(jsonKey::Comp::WallEventScript::EventAreaSize.data(), Json::array({ 0.0f, 0.0f, 0.0f }));
    ConstructAABB(
        Math::Vector3{
                eventSize[0],
                eventSize[1],
                eventSize[2]
        });

}
