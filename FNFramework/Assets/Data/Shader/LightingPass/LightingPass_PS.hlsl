#include "inc_LightingPass.hlsli"

cbuffer CousticsData : register(b4)
{
    // 経過時間 //
    float Time;
    float Speed;
    // 水面までの高さ
    float Height;
    // スケール
    float CousticsScale;

    // コースティクスの色 //
    float3 CousticsColor;
    float Intencity;
};

Texture2D g_albedMap : register(t0);
Texture2D<float3> g_normalMap : register(t1);
Texture2D<float> g_depthMap : register(t2);
Texture2D g_shadowMap : register(t3);

Texture2D<float> g_cousticsTex : register(t4);

SamplerState g_ss : register(s0);
SamplerComparisonState g_ssCmp : register(s10); // 比較機能付き

// シャドウの計算
float CalculateShadow(float3 worldPos)
{    
    float shadow = 1.0f;

    // ワールド空間座標をライト空間座標に変換
    float4 posInLightSpace = mul(float4(worldPos, 1), g_mLigViewProj);
    float3 liPos = posInLightSpace.xyz / posInLightSpace.w;

    // 深度マップの範囲内か確認
    if (abs(liPos.x) <= 1 && abs(liPos.y) <= 1 && liPos.z <= 1)
    {
		// 射影座標 -> UV座標へ変換
        float2 uv = liPos.xy * float2(1, -1) * 0.5 + 0.5;
        float z = liPos.z - 0.0005; // シャドウアクネ対策

        // シャドウマップのテクセルサイズ
        float w, h;
        g_shadowMap.GetDimensions(w, h);
        float tw = 1.0 / w;
        float th = 1.0 / h;

        // PCF (3x3サンプリングでソフトシャドウを計算)
        shadow = 0.0f;
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                float2 offset = float2(x * tw, y * th);
                shadow += g_shadowMap.SampleCmpLevelZero(g_ssCmp, uv + offset, z);
            }
        }

        // 平均化
        shadow /= 9.0f;
    }

    return shadow;
}

//====================//
// コースティクス関係 //
//====================//
float2 CalculateCausticsUV(
float3 worldPos, // ワールド座標
float3 lightDir, // 光の方向
float waterHeight,  // 水面の高さ
float scale)        // スケール
{
    // 光線と水面の交点を計算
    float t = (waterHeight - worldPos.y) / lightDir.y;
    float3 hitPos = worldPos + t * lightDir;

    // UV座標を生成
    float2 uv = hitPos.xz * scale;

    return uv; // アニメーションは別で処理
}

// UVアトラスのオフセットを計算
float2 CalculateCausticsAtlasUV(
    float2 baseUV, // ベースのUV座標
    float time, // 経過時間
    float gridSize, // 横方向のグリッド数
    float totalFrames, // 総フレーム数
    float animationSpeed // アニメーションスピード
)
{
    // フレーム番号を計算
    int currentFrame = (int) fmod(time * animationSpeed, totalFrames);

    // グリッド内のフレーム位置を計算
    float2 frameOffset;
    frameOffset.x = (currentFrame % (int) gridSize) / gridSize; // X方向オフセット
    frameOffset.y = (currentFrame / (int) gridSize) / (totalFrames / gridSize); // Y方向オフセット

    // 各フレームのUVスケール（1フレーム分のサイズ）
    float frameUVScale = 1.0 / gridSize;

    // ベースUVをフレーム内に収める
    float2 adjustedBaseUV = frac(baseUV) * frameUVScale; // UVをグリッド内に制限

    // オフセットを適用したUVを返す
    return adjustedBaseUV + frameOffset;
}

// コースティクスの計算
float3 CalculateCaustics(float3 normal, float3 worldPos)
{
    // 水面までの距離を計算
    float dist = worldPos.y - Height;

    // ベースUV計算
    float2 baseUV = CalculateCausticsUV(
        worldPos,
        g_ligDirection,
        Height,
        CousticsScale);

    // UVアトラスのオフセットを適用
    float2 uv = CalculateCausticsAtlasUV(baseUV, Time, 4.0, 16.0, Speed); // 例: 4x4アトラス

    // テクスチャサンプリング
    float coustics = g_cousticsTex.Sample(g_ss, uv).r;

    // コースティクスの強度を計算
    float intensity = saturate(1.0f - dist / 0.5f) * Intencity;
    float3 color = coustics * intensity * CousticsColor;

    // 法線と光の方向による影響
    float d = saturate(dot(-normal, g_ligDirection));
    
    return color * d;
}

float4 main(VSOutput input) : SV_TARGET
{
    // アルベドと法線マップのサンプリング
    float4 albedo = g_albedMap.Sample(g_ss, input.uv);
    float3 normal = g_normalMap.Sample(g_ss, input.uv);

    // 法線ベクトルの変換と正規化
    normal = normalize((normal * 2.0f) - 1.0f);

    // 深度値からワールド座標の復元 //
    float4 worldPos;
    float depth = g_depthMap.Sample(g_ss, input.uv);
    float3 screenPos;
    screenPos.xy = (input.uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
    screenPos.z = depth;

    worldPos = mul(float4(screenPos, 1.0f), g_mViewProjInv);
    worldPos.xyz /= worldPos.w;

    // ポイントライトの計算
    float3 lambertDiffuse = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < g_FramePointLightNum.x; i++)
    {
        float dist = length(worldPos.xyz - g_PointLights[i].PLPosition);

        if (dist < g_PointLights[i].PLRange)
        {
            float3 ligPointDir = normalize(g_PointLights[i].PLPosition - worldPos.xyz);
            float NdotL = saturate(dot(normal, ligPointDir));
            float affect = pow(saturate(1.0f - (dist / g_PointLights[i].PLRange)), 5.0f);
            lambertDiffuse += g_PointLights[i].PLColor * g_PointLights[i].PLIntensity * NdotL * affect;
        }
    }

    // ディレクショナルライトの計算
    float t = saturate(dot(normal, -g_ligDirection));
    float3 dirLight = g_ligColor * t;
    
    // シャドウ適用
    float shadow = CalculateShadow(worldPos.xyz);

    // 総合的なライティング結果
    float3 ligCol = dirLight * shadow + lambertDiffuse + g_AmbientLight.xyz;

    // 最終的な色の計算
    float4 finalColor = albedo;
    finalColor.xyz *= ligCol + CalculateCaustics(normal.xyz, worldPos.xyz) * (shadow + 0.2);

    // フォグの適用
    if (g_FogEnable)
    {
        // カメラからの距離
        float camDist = length(g_eyePos.xyz - worldPos.xyz);
        float f = saturate(1.0f / exp((camDist - g_DistanceFogStart) * g_DistanceFogDensity));
        finalColor.xyz = lerp(g_FogColor, finalColor.xyz, f);
    }

    return finalColor;
}
