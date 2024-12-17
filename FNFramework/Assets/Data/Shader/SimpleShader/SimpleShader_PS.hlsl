#include "inc_SimpleShader.hlsli"

Texture2D g_tex : register(t0);
SamplerState g_ss : register(s0);

float4 main(VSOutput i) : SV_TARGET
{
    return g_tex.Sample(g_ss, i.uv);
}
