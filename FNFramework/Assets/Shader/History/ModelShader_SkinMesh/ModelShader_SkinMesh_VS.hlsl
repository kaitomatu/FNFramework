#include "inc_ModelShader.hlsli"
#include "../inc_Common.hlsli"

cbuffer cbBones : register(b4)
{
    row_major float4x4 g_mBones[300];
}

VSOutput main(
    float4 pos : POSITION, // 座標
    float2 uv : TEXCOORD, // テクスチャ座標
    float4 color : COLOR, // 色
    float3 normal : NORMAL, // 法線
    float3 tangent : TANGENT, // 接ベクトル

    // スキンメッシュのボーンインデックス(何番目のボーンに影響しているか?のデータ(最大4つぶん))
    uint4 skinIndex : SKININDEX,
    // ボーンの影響度(そのボーンに、どれだけ影響しているか?のデータ(最大4つぶん))
    float4 skinWeight : SKINWEIGHT
)
{

    row_major float4x4 mBones = 0; // 行列を0埋め
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        mBones += g_mBones[skinIndex[i]] * skinWeight[i];
    }

    pos = mul(pos, mBones);
    normal = mul(normal, (float3x3)mBones);
    tangent = mul(tangent, (float3x3)mBones);

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
