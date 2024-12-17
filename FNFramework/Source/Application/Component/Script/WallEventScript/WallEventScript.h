#pragma once

#include "../../BaseComponent.h"

class PlayerScript;
class SeaWeedWallScript;
class SeaweedRenderingScript;
class AnimationComponent;
class KurageMoveScript;

/**
 * @class WallEventScript
 * @brief イベント時の制御を行うクラス
 */
class WallEventScript
    : public BaseComponent
{
public:
    enum class EventPhase
    {
        MovingToInitialPositions,
        MovingToLatePosition,
        Finished
    };

    WallEventScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~WallEventScript() override {}

    void Awake() override;
    void Start() override;
    void Update() override;

    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    void ImGuiUpdate() override;

    // イベント系の処理 //
    void Event();
    void HandleInitialPositioning();
    void HandleMovingToLatePosition();
    void CalcTargetPosition();

    bool MoveChildTowards(
        const std::shared_ptr<KurageMoveScript>& _spKurageMoveScript,
        const Math::Vector3& _nowPos,
        const Math::Vector3& _targetPos,
        float _eventActiveArea = 1.0f);

    /**
     * @brief イベントエリアのAABBを構築する
     * @param[in] size AABBのサイズ
     */
    void ConstructAABB(const Math::Vector3& size);

    // イベントデータ構造体
    struct EventData
    {
        AABB<Math::Vector3> EventArea;               // イベント範囲
        Math::Vector3 EventLateMovePos = { 0.0f, 0.0f, 0.0f }; // 移動後の座標
        std::vector<Math::Vector3> TargetPositions; // 目標座標リスト
    };

    EventData m_eventData;
    // イベントエリアの基準点
    Math::Vector3 m_basePosition = { 0.0f, 0.0f, 0.0f };

    // 各イベントのリクエスト時間: これを超えたら強制的にイベントを進行させる
    float m_requestTime = 0.0f;
    void EventRequestTimeCheck(float _requestEndTime, EventPhase _nextPhase);

    // イベント対象のオブジェクト / 操作用コンポーネント
    std::weak_ptr<GameObject> m_wpEventObject;
    std::weak_ptr<PlayerScript> m_wpPlayerScript;
    std::weak_ptr<KurageMoveScript> m_wpPlayerKurageMoveScript;

    float m_beforePlayerChargeAnimSpeed = 0.0f;
    float m_beforePlayerFloatAnimSpeed = 0.0f;

    std::string m_eventObjectName; // イベント対象の名前

    // 壁の当たり判定操作用コンポーネント
    std::weak_ptr<SeaWeedWallScript> m_wpSeaWeedWallScript;

    // アニメーションコンポーネント
    float m_animSubsidePow = 0.03f;
    float m_baseAnimSpeed = 0.0f;
    std::weak_ptr<SeaweedRenderingScript> m_wpSeaweedRenderingScript;

    bool m_isEventStart = false; // イベント開始フラグ
    EventPhase m_eventPhase = EventPhase::MovingToInitialPositions;
};

// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace WallEventScript
    {
        constexpr std::string_view EventObjectName = "EventObjectName";
        constexpr std::string_view EventLateMovePos = "EventLateMovePos";
        constexpr std::string_view EventAreaSize = "EventAreaSize";
        constexpr std::string_view BasePosition = "BasePosition";
    }
}
