#include "inc_ToneMappingShader.hlsli"

cbuffer cbToonMapping : register(b1)
{
    float g_exposure; // 露出
}

Texture2D g_toneMapSrcTex : register(t0);
SamplerState g_ss : register(s0);

// Reinhardトーンマッピング
float3 Reinhard(float3 col)
{
    return col / (col + 1.0);
}

// ACESトーンマッピング
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

float4 main(VSOutput i) : SV_TARGET
{
    // Reinhardトーンマッピング //
    // HDRテクスチャからピクセルの色を取得
    float3 hdrColor = g_toneMapSrcTex.Sample(g_ss, i.uv).rgb;

    // 露出調整
    float3 mappedColor = hdrColor;

    // Reinhardトーンマッピング
    //mappedColor = Reinhard(mappedColor * g_exposure);

    // ACESトーンマッピング
    mappedColor = ACESFilm(mappedColor * g_exposure);

    // ガンマ補正
    mappedColor = pow(mappedColor, 1.0 / 2.2);

    return float4(mappedColor, 1.0);
}
