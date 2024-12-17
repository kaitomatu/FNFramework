#include "LightComponent.h"

#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"
#include "Application/Component/Renderer/ModelComponent/ModelComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"


void LightComponent::Awake()
{
    // デフォルトのライト設定
    m_posOffset = Math::Vector3::Zero;
    m_pointLight.Color = Math::Vector3(1.0f, 1.0f, 1.0f);
    m_pointLight.Intensity = 1.0f;
    m_pointLight.Range = 10.0f;
    m_colorLock = false;
}

void LightComponent::Start()
{
    m_colorLock = false;
}

void LightComponent::Update()
{
    if (!OwnerValid()) { return; }

    // オーナーの TransformComponent から位置を取得
    const auto& spTransform = m_wpOwnerObj.lock()->GetTransformComponent();

    if (spTransform)
    {
        m_pointLight.Position = spTransform->GetWorldPos() + m_posOffset;
    }

    ShaderManager::Instance().GetAmbientManager()->AddLight(m_pointLight);
}

void LightComponent::ImGuiUpdate()
{

    ImGui::DragFloat3("PosOffset", &m_posOffset.x, 0.1f);

    ImGui::Checkbox("ColorLock", &m_colorLock);
    if (!m_colorLock)
    {
        ImGui::ColorEdit3("Color", &m_pointLight.Color.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float);
    }

    ImGui::DragFloat("Intensity", &m_pointLight.Intensity, 0.1f, 0.0f, 3000.0f);

    ImGui::DragFloat("Range", &m_pointLight.Range, 0.1f, 0.0f, 1000.0f);
}

void LightComponent::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::LightComponent::PosOffset.data()] = { m_posOffset.x, m_posOffset.y, m_posOffset.z };
    _json[jsonKey::Comp::LightComponent::Color.data()] = { m_pointLight.Color.x, m_pointLight.Color.y, m_pointLight.Color.z };
    _json[jsonKey::Comp::LightComponent::Intensity.data()] = m_pointLight.Intensity;
    _json[jsonKey::Comp::LightComponent::Range.data()] = m_pointLight.Range;
}

void LightComponent::Deserialize(const Json& _json)
{
    auto posOffset = _json.find(jsonKey::Comp::LightComponent::PosOffset.data());
    if (posOffset != _json.end())
    {
        m_posOffset = Math::Vector3((*posOffset)[0], (*posOffset)[1], (*posOffset)[2]);
    }

    auto color = _json.find(jsonKey::Comp::LightComponent::Color.data());
    if (color != _json.end())
    {
        m_pointLight.Color = Math::Vector3((*color)[0], (*color)[1], (*color)[2]);
    }

    auto intensity = _json.find(jsonKey::Comp::LightComponent::Intensity.data());
    if (intensity != _json.end())
    {
        m_pointLight.Intensity = intensity.value();
    }

    auto range = _json.find(jsonKey::Comp::LightComponent::Range.data());
    if (range != _json.end())
    {
        m_pointLight.Range = range.value();
    }
}

void LightComponent::Release()
{
}
