#include "inc_BloomShader.hlsli"

Texture2D g_bloomTex : register(t0);
Texture2D g_sceneTex : register(t1);

SamplerState g_ss : register(s0);

float4 main(VSOutput i) : SV_TARGET
{
    // シーンとブルームの色を取得
    float3 sceneColor = g_sceneTex.Sample(g_ss, i.uv).rgb;
    float3 bloomColor = g_bloomTex.Sample(g_ss, i.uv).rgb;

    // 合成（乗算合成）
    float3 finalColor = sceneColor * bloomColor;

    // 出力色
    return float4(finalColor, 1.0);
}
