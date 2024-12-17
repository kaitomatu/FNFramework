#include "inc_ModelShader_Unlit.hlsli"

//------------------------------
// シェーダーリソース
//------------------------------
Texture2D	g_diffuseTex			: register(t0);	// Diffuseテクスチャ

//------------------------------
// サンプラーステート
//------------------------------
SamplerState g_ss : register(s0);

float4 main(VSOutput In) : SV_TARGET
{
	//----------------最終出力色決定----------------//
	
	float4 outputColor = g_diffuseTex.Sample(g_ss, In.uv) * In.color;
	
	//--------------------------------------------//
	return float4(outputColor.xyz, In.color.a);
}