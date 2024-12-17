#include "AmbientManager.h"

void AmbientManager::Init()
{
    m_lightData.FramePointLightNum = 0;

    if (!m_cousticsData.spCousticsTexture)
    {
        m_cousticsData.spCousticsTexture = AssetManager::Instance().GetTexture("Assets/Texture/Noise/Coustics/000.png");
    }
}

void AmbientManager::Serialize(Json& _json) const
{
    //x--------- コースティクス関連 ---------x//
    _json[jsonKey::AmbientController::CousticsTexPath.data()] = m_cousticsData.spCousticsTexture->GetFilePath();
    _json[jsonKey::AmbientController::CousticsSpeed.data()] = m_cousticsData.CousticsData.Speed;
    _json[jsonKey::AmbientController::CousticsHeight.data()] = m_cousticsData.CousticsData.Height;
    _json[jsonKey::AmbientController::CousticsScale.data()] = m_cousticsData.CousticsData.CousticsScale;
    _json[jsonKey::AmbientController::CousticsIntencity.data()] = m_cousticsData.CousticsData.Intencity;
    _json[jsonKey::AmbientController::CousticsColor.data()] =
    {
        m_cousticsData.CousticsData.CousticsColor.x,
        m_cousticsData.CousticsData.CousticsColor.y,
        m_cousticsData.CousticsData.CousticsColor.z
    };

    //x--------- フォグ関連 ---------x//
    _json[jsonKey::AmbientController::FogEnable.data()] = m_fogData.FogEnable;
    _json[jsonKey::AmbientController::FogColor.data()] =
    {
        m_fogData.FogColor.x,
        m_fogData.FogColor.y,
        m_fogData.FogColor.z
    };
    _json[jsonKey::AmbientController::DistanceFogDensity.data()] = m_fogData.DistanceFogDensity;
    _json[jsonKey::AmbientController::DistanceFogStart.data()] = m_fogData.DistanceFogStart;

    //x--------- ライト関連 ---------x//
    _json[jsonKey::AmbientController::AmbientLight.data()] =
    {
        m_lightData.AmbientLight.x,
        m_lightData.AmbientLight.y,
        m_lightData.AmbientLight.z,
        m_lightData.AmbientLight.w
    };
    _json[jsonKey::AmbientController::LigDirection.data()] =
    {
        m_lightData.LigDirection.x,
        m_lightData.LigDirection.y,
        m_lightData.LigDirection.z
    };
    _json[jsonKey::AmbientController::LigColor.data()] =
    {
        m_lightData.LigColor.x,
        m_lightData.LigColor.y,
        m_lightData.LigColor.z
    };
}

void AmbientManager::Deserialize(const Json& _json)
{
    //x--------- コースティクス関連 ---------x//
    std::string path = _json.value(jsonKey::AmbientController::CousticsTexPath.data(), "Assets/Texture/Noise/Coustics/000.png");
    m_cousticsData.spCousticsTexture = AssetManager::Instance().GetTexture(path);
    m_cousticsData.CousticsData.Speed = _json.value(jsonKey::AmbientController::CousticsSpeed.data(), 0.0f);
    m_cousticsData.CousticsData.Height = _json.value(jsonKey::AmbientController::CousticsHeight.data(), 0.0f);

    m_cousticsData.CousticsData.CousticsScale = _json.value(jsonKey::AmbientController::CousticsScale.data(), 1.0f);
    m_cousticsData.CousticsData.Intencity = _json.value(jsonKey::AmbientController::CousticsIntencity.data(), 1.0f);
    auto cousticsColor = _json.value(jsonKey::AmbientController::CousticsColor.data(), Json::array({ 1.0f, 1.0f, 1.0f }));
    m_cousticsData.CousticsData.CousticsColor = Math::Vector3{ cousticsColor[0], cousticsColor[1], cousticsColor[2] };

    //x--------- フォグ関連 ---------x//
    m_fogData.FogEnable = _json.value(jsonKey::AmbientController::FogEnable.data(), 0);

    auto fogColor = _json.value(jsonKey::AmbientController::FogColor.data(), Json::array({ 1.0f, 1.0f, 1.0f }));
    m_fogData.FogColor = Math::Vector3{ fogColor[0], fogColor[1], fogColor[2] };

    m_fogData.DistanceFogDensity = _json.value(jsonKey::AmbientController::DistanceFogDensity.data(), 1.0f);

    m_fogData.DistanceFogStart = _json.value(jsonKey::AmbientController::DistanceFogStart.data(), 0.0f);

    //x--------- ライト関連 ---------x//
    auto ambientColor = _json.value(jsonKey::AmbientController::AmbientLight.data(), Json::array({ 1.0f, 1.0f, 1.0f}));
    m_lightData.AmbientLight = Math::Vector4{ ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[3]};

    auto ligDirection = _json.value(jsonKey::AmbientController::LigDirection.data(),Json::array({1.0f, 1.0f, 1.0f}));
    m_lightData.LigDirection = Math::Vector3{ ligDirection[0], ligDirection[1], ligDirection[2] };

    auto ligColor = _json.value(jsonKey::AmbientController::LigColor.data(), Json::array({1.0f, 1.0f, 1.0f}));
    m_lightData.LigColor = Math::Vector3{ ligColor[0], ligColor[1], ligColor[2] };
}

//x--------- ライト関連 ---------x//
void AmbientManager::AddLight(const PointLight& _light)
{
    // 最大数に達していないか確認
    if (m_lightData.FramePointLightNum >= CBufferData::Light::MaxPointLightNum)
    {
        return;
    }

    // ポイントライトを追加
    m_lightData.PointLights[m_lightData.FramePointLightNum] = _light;

    // カウンタを更新
    ++m_lightData.FramePointLightNum;
}
