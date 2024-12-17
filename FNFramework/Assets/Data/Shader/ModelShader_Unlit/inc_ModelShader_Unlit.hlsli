
struct VSOutput
{
	float4 pos		: SV_Position;	// 座標
	float2 uv		: TEXCOORD;		// テクスチャ座標
	float4 color	: COLOR;		// 色
};

cbuffer cbWorld : register(b1)
{
	row_major matrix g_mWorld;
}
