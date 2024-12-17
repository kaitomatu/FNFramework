#include "Shadow.hlsli"

//---------------------------------
// カメラ定数バッファ
//---------------------------------
cbuffer cbCamera : register(b0)
{
    row_major matrix g_mViewProj;
    row_major matrix g_mViewProjInv;

    float3 g_eyePos; // 視点座標
}

//------------------------------
// オブジェクト
//------------------------------
cbuffer cbObject : register(b1)
{
    float g_BonePerInstance;
    
    bool g_IsSkin;
}

struct BoneBuffer
{
    row_major float4x4 BoneMatrix;
};

StructuredBuffer<BoneBuffer> g_BoneBuffer : register(t1);

// 頂点シェーダーの入力構造体
struct VS_Input
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    uint4 skinIndex : SKININDEX;
    float4 skinWeight : SKINWEIGHT;
    row_major float4x4 mInstanceWorld : INSTANCE_WORLD;
    float4 tillingOffset : INSTANCE_TILING_OFFSET;
};

VS_Output main(VS_Input input, uint instanceID : SV_InstanceID)
{
    VS_Output o;

    // スキンメッシュの計算
    float4 pos = input.pos;

    if (g_IsSkin)
    {
        uint bonesPerInstance = g_BonePerInstance; // 1インスタンスあたりのボーン数
        uint boneStartIndex = instanceID * bonesPerInstance;

        row_major float4x4 skinMatrix =
        {
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        };
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            uint boneIndex = boneStartIndex + input.skinIndex[i];
            skinMatrix += g_BoneBuffer[boneIndex].BoneMatrix * input.skinWeight[i];
        }

        pos = mul(pos, skinMatrix);
    }

    // インスタンスワールド行列を適用
    float4 worldPos = mul(pos, input.mInstanceWorld);

    // ビュー・プロジェクション行列を適用
    o.pos = mul(worldPos, g_mViewProj);

    return o;
}
