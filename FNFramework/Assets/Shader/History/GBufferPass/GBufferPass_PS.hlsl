#include "inc_GBufferPass.hlsli"

struct PSOutput
{
    float4 Color : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Depth : SV_Target2;
};

Texture2D<float4> g_albedMap : register(t0);
// Texture2D<float4> g_normalMap : register(t1);

SamplerState g_ss : register(s0);

PSOutput main(VS_Output i)
{
    PSOutput output;

    float4 albedoColor = g_albedMap.Sample(g_ss, i.uv);

    output.Color = albedoColor;
    output.Color.xyz *= i.color.xyz;
    output.Color.w = 1.0f; // 半透明描画は非対応にしておく

    output.Normal = float4(i.normal.xyz, 1.0f);
    
    output.Depth = float4(i.pos.z, 0.0f, 0.f, 1.0f);
    
    return output;
}
