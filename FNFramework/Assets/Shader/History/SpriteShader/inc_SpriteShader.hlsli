
struct VSOutput
{
	float4 pos		: SV_Position;	// 座標
	float2 uv		: TEXCOORD;		// テクスチャ座標
};

cbuffer cbObject : register(b1)
{
	row_major matrix	g_mWorld;
	float4				g_color;	// 色
    float2 g_Tiling;
    float2 g_Offset;
}
