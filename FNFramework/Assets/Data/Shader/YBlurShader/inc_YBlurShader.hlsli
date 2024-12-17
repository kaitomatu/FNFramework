struct VSOutput
{
    // 座標
    float4 pos : SV_Position;
    // テクスチャ座標 //
    float4 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float4 tex2 : TEXCOORD2;
    float4 tex3 : TEXCOORD3;
    float4 tex4 : TEXCOORD4;
    float4 tex5 : TEXCOORD5;
    float4 tex6 : TEXCOORD6;
    float4 tex7 : TEXCOORD7;
};

//---------------------------------
// カメラ定数バッファ
//---------------------------------
cbuffer cbSpriteMat : register(b0)
{
    row_major matrix g_mSpriteProj; // プロジェクション行列
}

cbuffer cbBlur : register(b1)
{
    float4 weight[2]; // 重みテーブル
}

Texture2D g_mainRTTex : register(t0);
