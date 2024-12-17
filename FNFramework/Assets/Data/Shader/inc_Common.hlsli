//=====================================================
//
// 全シェーダ共通
//
//=====================================================

#define PI 3.14159265359

//---------------------------------
// カメラ定数バッファ
//---------------------------------
cbuffer cbCamera : register(b0)
{
    row_major matrix g_mViewProj;
    row_major matrix g_mViewProjInv;

    float3 g_eyePos; // 視点座標
}

//---------------------------------
// ライト定数バッファ
//---------------------------------
cbuffer cbDirectionLight : register(b3)
{
    //-----------環境光---------------//
    float4 g_AmbientLight; // 環境光の強さ
    //--------------------------------//

    //-----------平行光---------------//
    float3 g_ligDirection; // ライトの方向
    float3 g_ligColor; // ライトの色
    row_major matrix g_mLigViewProj; // ライトのビュー行列とプロジェクション行列
    //--------------------------------//

    //-----------ポイントライト---------------//
    int4 g_FramePointLightNum; // .x = 現在出ているポイントライトの数

    struct PointLight
    {
        float3 PLPosition; // 座標
        float PLRange; // 影響範囲
        float3 PLColor; // カラー
        float PLIntensity; // 強度
    } g_PointLights[300];

    //----------------------------------------//
}
