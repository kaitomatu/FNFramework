#include "inc_SpriteShader.hlsli"

Texture2D	g_mainTex				: register(t0);	// メインテクスチャ
Texture2D	g_maskTex				: register(t1);	// メインテクスチャ

// memo: 2Dスプライトにノーマルマップを適応することで、スプライトにも奥行をだせる？？
// Texture2D	g_normalTex				: register(t1);	// 法線テクスチャ

SamplerState g_ss : register(s0);

float4 main(VSOutput In) : SV_TARGET
{
    // todo : r値のみを見ているのはまずい...？
    float mask = g_maskTex.Sample(g_ss, In.uv).r;
    if (mask < 0.1f)
    {
        discard;
    }

	float4 color = g_mainTex.Sample(g_ss, In.uv) * g_color;
    if (color.a < 0.1f)
    {
        discard;
    }
	return color;
}
