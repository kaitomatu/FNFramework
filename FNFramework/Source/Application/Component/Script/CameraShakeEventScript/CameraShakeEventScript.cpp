#include "CameraShakeEventScript.h"

#include "Application/Component/Camera/TrackingCameraComponent.h"

void CameraShakeEventScript::Awake()
{

}

void CameraShakeEventScript::Start()
{
    if(!OwnerValid()) { return; }

    // カメラを名前で復元
    if (!m_trackingCameraComponentName.empty())
    {
        auto shakeScriptObj = m_wpOwnerObj.lock()->GetScene()->FindObject(m_trackingCameraComponentName);
        if (shakeScriptObj)
        {
            m_wpTrackingCameraComponent = shakeScriptObj->GetComponent<TrackingCameraComponent>();
        }
    }

    // この時点で設定されていない場合警告を出しておく
    if(m_wpTrackingCameraComponent.expired())
    {
        FNENG_ASSERT_LOG("カメラが設定されていません", false)
    }
}

void CameraShakeEventScript::Update()
{
    if(!m_isShake) { return; }

    const std::shared_ptr<TrackingCameraComponent>& trackingCamera = m_wpTrackingCameraComponent.lock();

    if(!trackingCamera) { FNENG_ASSERT_ERROR("カメラが設定されていません") return; }

    //x-------------------------x
    //  ランダムな揺れを加える
    //x-------------------------x
    Math::Vector2 shakeDir = {
        static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-m_shakePower, m_shakePower)),
        static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-m_shakePower, m_shakePower))
    };

    //x-------------------------x
    //     カメラ揺れの処理
    //x-------------------------x
    // カメラが揺らされ始めてからの時間を取得
    float time = static_cast<float>(m_shakeTime.Elapsed<Timer::Milliseconds>().count())*0.001f;

    Math::Vector3 localPos = trackingCamera->GetLocalCameraPos();

    localPos.x += shakeDir.x;
    localPos.y += shakeDir.y;

    // 初期位置-vibrato ~ 初期位置+vibrato の間に収める
    float ratio = 1.0f - std::clamp(time / m_duration, 0.0f, 1.0f);

    m_vibrato *= ratio; // フェードアウトさせるため、経過時間により揺れの量を減衰

    localPos.x = std::clamp(localPos.x, -m_vibrato, m_vibrato);
    localPos.y = std::clamp(localPos.y, -m_vibrato, m_vibrato);

    trackingCamera->SetLocalCameraPos(localPos);

    // m_duration秒経過したらシェイク終了
    if(time >= m_duration)
    {
        // シェイク終了
        trackingCamera->SetLocalCameraPos(Math::Vector3::Zero);
        m_shakeTime.Stop();
        m_shakeTime.Reset();
        m_isShake = false;
    }
}

void CameraShakeEventScript::UpdateWorldTransform()
{

}

void CameraShakeEventScript::Release()
{

}

void CameraShakeEventScript::Serialize(Json& _json) const
{
    // TrackingCameraComponentのオブジェクト名を保存
    if (auto trackingCamera = m_wpTrackingCameraComponent.lock())
    {
        _json[jsonKey::Comp::CameraShakeEventScript::TrackingCameraComponentObjName.data()] = trackingCamera->GetOwner()->GetName();
    }
}

void CameraShakeEventScript::Deserialize(const Json& _json)
{
    // TrackingCameraComponentのオブジェクト名を復元
    auto cameraCompIt = _json.find(jsonKey::Comp::CameraShakeEventScript::TrackingCameraComponentObjName.data());
    if (cameraCompIt != _json.end())
    {
        m_trackingCameraComponentName = cameraCompIt->get<std::string>();
    }
}

void CameraShakeEventScript::ImGuiUpdate()
{
    ImGui::DragFloat("Duration", &m_duration, 0.1f, 0.0f, 10.0f);
    ImGui::DragFloat("ShakePower", &m_shakePower, 1.0f, 0.0f, 30.0f);
    ImGui::DragFloat("Vibrato", &m_vibrato, 0.1f, 0.0f, 100.0f);

    if(ImGui::Button("Shake"))
    {
        ShakeStart(m_shakePower, m_vibrato, m_duration);
    }

    ImGui::Text("-------TimerInfo-------");
    ImGui::Text(m_shakeTime.GetName().c_str());
    ImGui::Text("IsRunning : %s", m_shakeTime.IsRunning() ? "true" : "false");
    ImGui::Text("Elapsed : %f(ms)", m_shakeTime.Elapsed<Timer::Milliseconds>().count());
}
