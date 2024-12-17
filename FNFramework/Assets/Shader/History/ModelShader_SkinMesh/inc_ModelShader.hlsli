#define PI 3.14159265358979

struct VSOutput
{
    float4 pos : SV_Position; // 座標
    float3 wPos : POSITION0; // オブジェクトのワールド座標
    float4 posInLVP : POSITION1; // ライトビュースクリーン空間でのピクセルの座標
    float2 uv : TEXCOORD; // テクスチャ座標
    float4 color : COLOR; // 色
    float3 wN : NORMAL; // 法線	- normal
    float3 wT : TANGENT0; // 接線	- tangent
    float3 wB : TANGENT1; // 従法線	- binormal
};

//------------------------------
// マテリアル
//------------------------------
cbuffer cbMaterial : register(b1)
{
    float4 g_BaseColor; // ベース色
    float g_Metallic; // 金属度
    float g_Roughness; // 粗さ
    float3 g_Emissive; // 自己発光色
};

//------------------------------
// オブジェクト
//------------------------------
cbuffer cbWorld : register(b2)
{
    row_major matrix g_mWorld;
}

//------------------------------
// フォグ
//------------------------------
cbuffer cbFog : register(b4)
{
    //x--- 距離フォグ ---x//
    int g_FogEnable; // フォグを有効にするかどうか
    float3 g_FogColor; // フォグのカラー

    float g_DistanceFogDensity; // 距離フォグ減衰率
}
