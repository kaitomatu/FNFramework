#include "inc_GBufferPass.hlsli"
#include "../inc_Common.hlsli"

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
    float4 iColor : INSTANCE_COLOR;
};

VS_Output main(VS_Input input, uint instanceID : SV_InstanceID)
{
    VS_Output o;

    // スキンメッシュの計算
    float4 pos = input.pos;
    float3 normal = input.normal;

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
        normal = mul(normal, (float3x3) skinMatrix);
    }

    // インスタンスワールド行列を適用
    float4 worldPos = mul(pos, input.mInstanceWorld);

    // ビュー・プロジェクション行列を適用
    o.pos = mul(worldPos, g_mViewProj);

    // 法線の変換
    o.normal = normalize(mul(normal, (float3x3) input.mInstanceWorld));

    // その他の出力
    o.color = input.color.rgb * input.iColor.rgb;

    // 値は流れてきているが、uvのオフセットが何故か反映されない
    o.uv = input.uv * input.tillingOffset.xy + input.tillingOffset.zw;
    
    return o;
}
