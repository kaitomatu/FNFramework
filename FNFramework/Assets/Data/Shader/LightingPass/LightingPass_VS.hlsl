#include "inc_LightingPass.hlsli"

struct VSInput
{
    float4 pos : POSITION; // 座標
    float2 uv : TEXCOORD; // テクスチャ座標
};

VSOutput main(VSInput In)
{
    VSOutput Out;

    Out.pos = In.pos;// mul(In.pos, g_mView);
    Out.pos = mul(Out.pos, g_mSpriteProj);

    Out.uv = In.uv;

    return Out;
}
