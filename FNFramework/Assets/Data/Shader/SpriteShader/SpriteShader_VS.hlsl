#include "inc_SpriteShader.hlsli"
#include "../inc_Common.hlsli"

struct VSInput
{
	float4 pos		: POSITION;		// 座標
	float2 uv		: TEXCOORD;		// テクスチャ座標
};

VSOutput main(VSInput In)
{
	VSOutput Out;

	Out.pos = mul(In.pos, g_mWorld);
    Out.pos = mul(Out.pos, g_mViewProj);

    Out.uv = In.uv * g_Tiling + g_Offset;

	return Out;
}
