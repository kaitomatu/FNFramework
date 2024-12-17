#include "LightAnimationScript.h"

void LightAnimationScript::Awake()
{
    LightComponent::Awake();

    m_intensityMinMax = Math::Vector2(0.0f, 1.0f);
}

void LightAnimationScript::Start()
{
    LightComponent::Start();
}

void LightAnimationScript::Update()
{
    float delta = SceneManager::Instance().FrameDeltaTime();

    bool easeFinish = false;

    if (m_lightAnimationType & ePositionXToEase)
    {
        easeFinish = m_easingData.Easing(delta, m_posOffset.x);
    }
    if (m_lightAnimationType & ePositionYToEase)
    {
        easeFinish = m_easingData.Easing(delta, m_posOffset.y);
    }
    if (m_lightAnimationType & ePositionZToEase)
    {
        easeFinish = m_easingData.Easing(delta, m_posOffset.z);
    }
    if (m_lightAnimationType & eIntensityToEase)
    {
        easeFinish = m_easingData.Easing(delta, m_pointLight.Intensity, m_intensityMinMax.x, m_intensityMinMax.y);
    }

    if (easeFinish)
    {
        m_easingData.Reverse = !m_easingData.Reverse;
    }

    LightComponent::Update();
}

void LightAnimationScript::Serialize(Json& _json) const
{
    // Easing データと LightComponent のシリアライズ
    m_easingData.Serialize(_json);
    LightComponent::Serialize(_json);

    // IntensityMinMax と LightAnimationType のシリアライズ
    _json[jsonKey::Comp::LightAnimationScript::IntensityMinMax.data()] =
    { m_intensityMinMax.x, m_intensityMinMax.y };

    _json[jsonKey::Comp::LightAnimationScript::LightAnimationType.data()] =
        m_lightAnimationType;
}

void LightAnimationScript::Deserialize(const Json& _json)
{
    // Easing データと LightComponent のデシリアライズ
    m_easingData.Deserialize(_json);
    LightComponent::Deserialize(_json);

    // IntensityMinMax のデシリアライズ
    auto intensityArray = _json.value(jsonKey::Comp::LightAnimationScript::IntensityMinMax.data(),
        Json::array({ 0.0f, 1.0f }));

    if (intensityArray.size() == 2)
    {
        m_intensityMinMax = Math::Vector2{
            intensityArray[0].get<float>(),
            intensityArray[1].get<float>()
        };
    }
    else
    {
        m_intensityMinMax = Math::Vector2{ 0.0f, 1.0f }; // デフォルト値
    }

    // LightAnimationType のデシリアライズ
    m_lightAnimationType = _json.value(jsonKey::Comp::LightAnimationScript::LightAnimationType.data(), 0);
}

void LightAnimationScript::Release()
{
    LightComponent::Release();
}

void LightAnimationScript::ImGuiUpdate()
{
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Position X", ePositionXToEase, m_lightAnimationType);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Position Y", ePositionYToEase, m_lightAnimationType);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Position Z", ePositionZToEase, m_lightAnimationType);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Intensity", eIntensityToEase, m_lightAnimationType);

    if (m_lightAnimationType & eIntensityToEase)
    {
        ImGui::DragFloat2("IntensityMinMax", &m_intensityMinMax.x, 0.1f);
    }

    ImGui::Separator();

    m_easingData.ImGuiUpdate();

    ImGui::Separator();
    ImGui::Separator();

    LightComponent::ImGuiUpdate();
}
