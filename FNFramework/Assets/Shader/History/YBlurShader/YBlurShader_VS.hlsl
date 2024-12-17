#include "inc_YBlurShader.hlsli"

struct VSInput
{
    float4 pos : POSITION; // 座標
    float2 uv : TEXCOORD; // テクスチャ座標
};

VSOutput main(VSInput In)
{
    VSOutput Out;

    Out.pos = In.pos;
    Out.pos = mul(Out.pos, g_mSpriteProj);
    
    // テクスチャサイズを取得
    float2 texSize;
    float level;
    g_mainRTTex.GetDimensions(0, texSize.x, texSize.y, level);

    float2 uv = In.uv;
    
    // 基準テクセルからV座標を+1テクセルずらすためのオフセットを計算する
    Out.tex0.xy = float2(0.0f, 1.0f / texSize.y);

    // 基準テクセルからV座標を+3テクセルずらすためのオフセットを計算する
    Out.tex1.xy = float2(0.0f, 3.0f / texSize.y);

    // 基準テクセルからV座標を+5テクセルずらすためのオフセットを計算する
    Out.tex2.xy = float2(0.0f, 5.0f / texSize.y);

    // 基準テクセルからV座標を+7テクセルずらすためのオフセットを計算する
    Out.tex3.xy = float2(0.0f, 7.0f / texSize.y);

    // 基準テクセルからV座標を+9テクセルずらすためのオフセットを計算する
    Out.tex4.xy = float2(0.0f, 9.0f / texSize.y);

    // 基準テクセルからV座標を+11テクセルずらすためのオフセットを計算する
    Out.tex5.xy = float2(0.0f, 11.0f / texSize.y);

    // 基準テクセルからV座標を+13テクセルずらすためのオフセットを計算する
    Out.tex6.xy = float2(0.0f, 13.0f / texSize.y);

    // 基準テクセルからV座標を+15テクセルずらすためのオフセットを計算する
    Out.tex7.xy = float2(0.0f, 15.0f / texSize.y);

    // オフセットに-1を掛けてマイナス方向のオフセットも計算する
    Out.tex0.zw = Out.tex0.xy * -1.0f;
    Out.tex1.zw = Out.tex1.xy * -1.0f;
    Out.tex2.zw = Out.tex2.xy * -1.0f;
    Out.tex3.zw = Out.tex3.xy * -1.0f;
    Out.tex4.zw = Out.tex4.xy * -1.0f;
    Out.tex5.zw = Out.tex5.xy * -1.0f;
    Out.tex6.zw = Out.tex6.xy * -1.0f;
    Out.tex7.zw = Out.tex7.xy * -1.0f;

    // オフセットに基準テクセルのUV座標を足し算して、
    // 実際にサンプリングするUV座標に変換する
    Out.tex0 += float4(uv, uv);
    Out.tex1 += float4(uv, uv);
    Out.tex2 += float4(uv, uv);
    Out.tex3 += float4(uv, uv);
    Out.tex4 += float4(uv, uv);
    Out.tex5 += float4(uv, uv);
    Out.tex6 += float4(uv, uv);
    Out.tex7 += float4(uv, uv);

    return Out;
}
