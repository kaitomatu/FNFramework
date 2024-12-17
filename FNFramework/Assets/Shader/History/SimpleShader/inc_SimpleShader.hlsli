
struct VSOutput
{
    float4 pos : SV_Position; // 座標
    float2 uv : TEXCOORD; // テクスチャ座標
};
//---------------------------------
// カメラ定数バッファ
//---------------------------------
cbuffer cbSpriteMat : register(b0)
{
    row_major matrix g_mSpriteProj; // プロジェクション行列
}
