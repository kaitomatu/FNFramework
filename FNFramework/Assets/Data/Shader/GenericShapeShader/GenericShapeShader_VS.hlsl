#include "inc_GenericShapeShader.hlsli"
#include "../inc_Common.hlsli"

VSOutput main(
    float4 pos : POSITION,
    float2 uv : TEXCOORD,
    float4 col : COLOR
)
{
    VSOutput o;
    o.col = col;

    o.uv = uv;

    o.pos = mul(pos, g_mViewProj);

    return o;
}
