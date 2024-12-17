#include "inc_XBlurShader.hlsli"

SamplerState g_ss : register(s0);

float4 main(VSOutput i) : SV_TARGET
{
    float4 Color;

    // 基準テクセルからプラス方向に8テクセル、重み付きでサンプリング
    Color  = weight[0].x * g_mainRTTex.Sample(g_ss, i.tex0.xy);
    Color += weight[0].y * g_mainRTTex.Sample(g_ss, i.tex1.xy);
    Color += weight[0].z * g_mainRTTex.Sample(g_ss, i.tex2.xy);
    Color += weight[0].w * g_mainRTTex.Sample(g_ss, i.tex3.xy);
    Color += weight[1].x * g_mainRTTex.Sample(g_ss, i.tex4.xy);
    Color += weight[1].y * g_mainRTTex.Sample(g_ss, i.tex5.xy);
    Color += weight[1].z * g_mainRTTex.Sample(g_ss, i.tex6.xy);
    Color += weight[1].w * g_mainRTTex.Sample(g_ss, i.tex7.xy);

    // 基準テクセルにマイナス方向に8テクセル、重み付きでサンプリング
    Color += weight[0].x * g_mainRTTex.Sample(g_ss, i.tex0.zw);
    Color += weight[0].y * g_mainRTTex.Sample(g_ss, i.tex1.zw);
    Color += weight[0].z * g_mainRTTex.Sample(g_ss, i.tex2.zw);
    Color += weight[0].w * g_mainRTTex.Sample(g_ss, i.tex3.zw);
    Color += weight[1].x * g_mainRTTex.Sample(g_ss, i.tex4.zw);
    Color += weight[1].y * g_mainRTTex.Sample(g_ss, i.tex5.zw);
    Color += weight[1].z * g_mainRTTex.Sample(g_ss, i.tex6.zw);
    Color += weight[1].w * g_mainRTTex.Sample(g_ss, i.tex7.zw);

    return float4(Color.xyz, 1.0f);
}
