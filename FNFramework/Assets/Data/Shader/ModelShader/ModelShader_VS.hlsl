#include "inc_ModelShader.hlsli"
#include "../inc_Common.hlsli"

VSOutput main(
    float4 pos : POSITION, // 座標
    float2 uv : TEXCOORD, // テクスチャ座標
    float4 color : COLOR, // 色
    float3 normal : NORMAL, // 法線
    float3 tangent : TANGENT // 接ベクトル
)
{
    VSOutput Out;

    // モデル
    float4 worldPos = mul(pos, g_mWorld);
    Out.wPos = worldPos.xyz;

    Out.pos = mul(worldPos, g_mViewProj);

    Out.posInLVP = mul(worldPos, g_mLigViewProj);

    Out.uv = uv;
    Out.color = color;
    //ここのg_mWorldはクオータニオン管理なのが問題なのかも
    Out.wN = normalize(mul(normal, (float3x3)g_mWorld));

    Out.wT = normalize(mul(tangent, (float3x3)g_mWorld));

    Out.wB = normalize(cross(Out.wN, Out.wT)); // normalとtangentの外積値を用いて接線を出す

    return Out;
}
