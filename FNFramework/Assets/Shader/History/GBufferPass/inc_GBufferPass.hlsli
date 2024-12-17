
struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

//------------------------------
// オブジェクト
//------------------------------
cbuffer cbObject : register(b1)
{
    float g_BonePerInstance;
    
    bool g_IsSkin;
}
