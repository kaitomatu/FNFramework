#include "inc_ModelShader_Unlit.hlsli"
#include "../inc_Common.hlsli"

VSOutput main(
	float4 pos		: POSITION, // 座標
	float2 uv		: TEXCOORD, // テクスチャ座標
	float4 color	: COLOR		// 色
)
{
	VSOutput Out;
	
	// モデル
	Out.pos = mul(pos ,  g_mWorld);
	Out.pos = mul(Out.pos, g_mViewProj);

	Out.uv  = uv;
	Out.color = color;
	
	return Out;
}
