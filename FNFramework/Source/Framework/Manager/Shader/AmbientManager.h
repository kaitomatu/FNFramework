#pragma once

/**
 * @class AmbientManager
 */

class AmbientManager
{
public:

    void Init();

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const;
    void Deserialize(const Json& _json);

    //x--------- コースティクス関連 ---------x//
    const CBufferData::cbCousticsData& GetCousticsCBData() const { return m_cousticsData.CousticsData; }
    CBufferData::cbCousticsData& WorkCousticsCBData() { return m_cousticsData.CousticsData; }

    const std::shared_ptr<ShaderResourceTexture>& GetCousticsTexture() const { return m_cousticsData.spCousticsTexture; }

    void AddTime(float _time) { m_cousticsData.CousticsData.Time += _time; }
    void ResetTime() { m_cousticsData.CousticsData.Time = 0.0f; }

    //x--------- フォグ関連 ---------x//
    const CBufferData::cbFog& GetFogCBData() const { return m_fogData; }
    CBufferData::cbFog& WorkFogCBData() { return m_fogData; }

    //x--------- ライト関連 ---------x//
    const CBufferData::Light& GetLightCBData() const { return m_lightData; }
    CBufferData::Light& WorkLightCBData() { return m_lightData; }

    void ClearLight() { m_lightData.FramePointLightNum = 0; }

    // ポイントライトの追加
    void AddLight(const PointLight& _light);

private:

    struct CousticsData
    {
        // コースティクスのテクスチャ
        std::shared_ptr<ShaderResourceTexture> spCousticsTexture = nullptr;
        CBufferData::cbCousticsData CousticsData;
    } m_cousticsData;

    CBufferData::Light m_lightData;

    CBufferData::cbFog m_fogData;
};

namespace jsonKey::AmbientController
{
    //x--------- コースティクス関連 ---------x//
    constexpr std::string_view CousticsTexPath = "CousticsTexPath";
    constexpr std::string_view CousticsSpeed = "CousticsSpeed";
    constexpr std::string_view CousticsHeight = "CousticsHeight";
    constexpr std::string_view CousticsScale = "CousticsScale";
    constexpr std::string_view CousticsIntencity = "CousticsIntencity";
    constexpr std::string_view CousticsColor = "CousticsColor";
    
    //x--------- フォグ関連 ---------x//
    constexpr std::string_view FogEnable = "FogEnable";
    constexpr std::string_view FogColor = "FogColor";
    constexpr std::string_view DistanceFogDensity = "DistanceFogDensity";
    constexpr std::string_view DistanceFogStart = "DistanceFogStart";

    //x--------- ライト関連 ---------x//
    constexpr std::string_view AmbientLight = "AmbientLight";
    constexpr std::string_view LigDirection = "LigDirection";
    constexpr std::string_view LigColor = "LigColor";
}
