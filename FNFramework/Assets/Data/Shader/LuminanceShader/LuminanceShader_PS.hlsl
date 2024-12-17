#include "inc_LuminanceShader.hlsli"

Texture2D g_mainRTTex : register(t0);

SamplerState g_ss : register(s0);

cbuffer cbBloom : register(b1)
{
    float g_bloomPower; // ブルームの強度
}

float4 main(VSOutput i) : SV_TARGET
{
    float4 color = g_mainRTTex.Sample(g_ss, i.uv);

    // 輝度を抽出する //
    float t = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f));

    clip(t - 1.0f);

    // ブルームの強度を調整する //
    color += g_bloomPower;

    return color;
}
