#include "../ModelShader/inc_ModelShader.hlsli"
#include "../inc_Common.hlsli"

//------------------------------
// シェーダーリソース
//------------------------------
Texture2D g_diffuseTex : register(t0); // Diffuseテクスチャ
Texture2D g_normalTex : register(t1); // 法線テクスチャ
Texture2D g_RoughnessMetallicTex : register(t2); // RoughnessMetallicテクスチャ
Texture2D g_emissiveTex : register(t3); // Emissiveテクスチャ

// 特殊処理用テクスチャ
Texture2D g_dirShadowMap : register(t4); // 平行光シャドウマップ

//------------------------------
// サンプラーステート
//------------------------------
SamplerState g_ss : register(s0);
SamplerComparisonState g_ssCmp : register(s10); // 比較機能付きサンプラーステート

//------------------------------
// 関数
//------------------------------
float3 CalcLambertDiffuse(float3 lightDir, float3 lightCol, float3 normal);
float CalcDiffuseFromFresnel(float3 normal, float3 lightDir, float3 eyeDir);

float3 CookTorranceSpecular(float3 lightDir, float3 eyeDir, float3 normal, float roughness, float3 specColor, float metallic);

float Beckmann(float m, float t);
float SpcFresnel(float f0, float u);

float3 BlinngPhong(float3 lightDir, float3 lightCol, float3 vCam, float3 normal, float specPower);
float3 CalcPhongSpecular(float3 lightDir, float3 lightCol, float3 worldPos, float3 normal);

float4 main(VSOutput In) : SV_TARGET
{
    // アルベドカラーとスペキュラカラー
    float4 albedoColor = g_diffuseTex.Sample(g_ss, In.uv);
    
    if (albedoColor.a <= 0.0f)
    {
        discard;
    }

    float3 specColor = albedoColor.xyz;

    // ラフネスとメタリックの取得
    float2 metallicRoughnessCol = g_RoughnessMetallicTex.Sample(g_ss, In.uv).ra;
    float metallic = metallicRoughnessCol.r * g_Metallic;
    float roughness = metallicRoughnessCol.g * g_Roughness;

    // 法線ベクトルと視線ベクトルの計算
    float3 wN = In.wN;
    float3 toEye = normalize(g_eyePos - In.wPos);
    float camDist = length(g_eyePos - In.wPos);

    // ディレクショナルライトの計算
    float NdotL = max(dot(wN, -g_ligDirection), 0.0f);
    float3 lambertDiffuse = g_ligColor * NdotL / PI;

    float3 specDLColor = CookTorranceSpecular(-g_ligDirection, toEye, wN, roughness, specColor, metallic) * g_ligColor.xyz;
    float3 specularLig = specDLColor;

	//=======================
	// ポイントライト
	//=======================
    for (int i = 0; i < g_FramePointLightNum.x; i++)
    {
        // 距離による影響率の計算
        float dist = length(In.wPos - g_PointLights[i].PLPosition);

        // 点光の判定以内
        if (dist < g_PointLights[i].PLRange)
        {
            // 入射ベクトル計算
            float3 ligPointDir = In.wPos - g_PointLights[i].PLPosition;
            ligPointDir = normalize(ligPointDir);

            // 正規化Lambert拡散反射を求める
            float NdotL = saturate(dot(wN, -ligPointDir));

            float3 diffPointLig = g_PointLights[i].PLColor * g_PointLights[i].PLIntensity * NdotL / PI;

            // PhongModel
            float3 specPLColor = CookTorranceSpecular(-ligPointDir, toEye, wN, roughness, specColor, metallic) * (g_PointLights[i].PLColor.xyz * g_PointLights[i].PLIntensity);
            specPLColor *= lerp(float3(1.0f, 1.0f, 1.0f), specColor, metallic);

            // 鏡面反射の絞り
            float affect = saturate(1.0f - 1.0f / g_PointLights[i].PLRange * dist);
            affect = pow(affect, 5.0f);

            diffPointLig *= affect;
            specPLColor *= affect;

            lambertDiffuse += diffPointLig;
            specularLig += specPLColor;
        }
    }

	//=======================
	// シャドウマッピング(影判定)
	//=======================
    float shadow = 1.0f; // 影の強さ
    float3 mixShadowColor = 1.0f; // 影の色 - 最終的に出力する色に影の色を混ぜる際に使用

	// ピクセルの3D座標から、DepthMapFromLight空間へ変換
    float3 liPos = In.posInLVP.xyz / In.posInLVP.w;

	// 深度マップの範囲内？
    if (abs(liPos.x) <= 1 && abs(liPos.y) <= 1 && liPos.z <= 1)
    {
		// 射影座標 -> UV座標へ変換
        float2 uv = liPos.xy * float2(1, -1) * 0.5 + 0.5;
		// ライトカメラからの距離
        float z = liPos.z - 0.005; // シャドウアクネ対策

		// 画像のサイズからテクセルサイズを求める
        float w, h;
        g_dirShadowMap.GetDimensions(w, h);
        float tw = 1.0 / w;
        float th = 1.0 / h;

		// uvの周辺3x3も判定し、平均値を求める
        shadow = 0;
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                shadow += g_dirShadowMap.SampleCmpLevelZero(g_ssCmp, uv + float2(x * tw, y * th), z);
            }
        }

		// shadow / ループ回数で、１ピクセル当たりの平均値を求める
        shadow /= 9.0f;
    }

	//----------------最終出力色決定----------------//
    float4 outputColor = albedoColor * In.color * g_BaseColor;

    outputColor.xyz *= (lambertDiffuse.xyz * (1.0f - roughness) + specularLig.xyz) + g_AmbientLight.xyz;

    float3 shadowColor = outputColor.xyz * shadow;
	// 影の色を混ぜる
    outputColor.xyz = lerp(shadowColor, outputColor.xyz, 0.8f);
	//--------------------------------------------//
    
	//=======================
	// フォグ関係
	//=======================
    if (g_FogEnable)
    {
		// フォグ 1(近い)～0(遠い)
        float f = saturate(1.0 / exp((camDist - g_DistanceFogStart) * g_DistanceFogDensity));
        
		// 適用
        outputColor.xyz = lerp(g_FogColor, outputColor.xyz, f);
    }

    outputColor.a = g_BaseColor.a * g_AmbientLight.a;

    return outputColor;
}

//========================
// 関数
//========================

//--------------Lambert拡散反射を計算する--------------
// LambertModel
// ・lightDir … ライトの方向
// ・lightCol … ライトの色
// ・normal   … 法線
float3 CalcLambertDiffuse(float3 lightDir, float3 lightCol, float3 normal)
{
    float t = saturate(dot(normal, lightDir));
    return lightCol * t;
}

//--------------フレネル反射を考慮した拡散反射を計算する--------------
// ・normal	  … 法線
// ・lightDir … ライトの方向
// ・eyeDir   … 視点ベクトル
float CalcDiffuseFromFresnel(float3 normal, float3 lightDir, float3 eyeDir)
{
	// 法線と光源に向かうベクトル
    float dotNL = saturate(dot(normal, lightDir));
	// 法線と視線ベクトル
    float dotNV = saturate(dot(normal, eyeDir));

    return (dotNL * dotNV);
}

//--------------Phong鏡面反射を計算する--------------
// Phong鏡面反射
// ・lightDir … ライトの方向
// ・lightCol … ライトの色
// ・worldPos … 対象の座標
// ・normal   … 法線
float3 CalcPhongSpecular(float3 lightDir, float3 lightCol, float3 worldPos, float3 normal)
{
    // 反射ベクトルを求める
    float3 refVec = reflect(lightDir, normal);

    // 光が当たったサーフェイスから視点に伸びるベクトルを求める
    float3 toEye = g_eyePos - worldPos;
    toEye = normalize(toEye);

    // 鏡面反射の強さを求める
    float t = saturate(dot(refVec, toEye)); //  0～1に切り詰める

    // 鏡面反射の強さを絞る
    t = pow(t, 5.0f);

    // 鏡面反射光を求める
    return lightCol * t;
}

// BlinngPhongModel
// ・lightDir … ライトの方向
// ・lightCol … ライトの色
// ・vCam     … ピクセルからカメラの方向
// ・normal   … 法線
// ・specPower… 反射の鋭さ
float3 BlinngPhong(float3 lightDir, float3 lightCol, float3 vCam, float3 normal, float specPower)
{
    float3 H = normalize(-lightDir + vCam);
    float spec = saturate(dot(normal, H));
    spec = saturate(spec); //  マイナス値なら0～1に切り詰める
    spec = pow(spec, specPower); //  つるつる度合いを考慮

    return lightCol * spec * ((specPower + 2) / (2 * PI));
}

// ベックマン分布を計算する
float Beckmann(float m, float t)
{
    t = max(t, 0.05f); // tがゼロに近づかないようにする
    float t2 = t * t;
    float t4 = t2 * t2;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4 + 0.001f);
    D *= exp((-1.0f / m2) * (1.0f - t2) / (t2 + 0.001f));
    return D;
}

// フレネルを計算。Schlick近似を使用
float3 SpcFresnel(float3 f0, float u)
{
    return f0 + (1 - f0) * pow(1 - u, 5);
}

// Cook-TrranceModel
// ・lightDir … ライトの方向
// ・eyeDir	  … 視点ベクトル
// ・normal   … 法線
// ・metallic … 金属度
float3 CookTorranceSpecular(float3 lightDir, float3 eyeDir, float3 normal, float roughness, float3 specColor, float metallic)
{
    float3 H = normalize(lightDir + eyeDir);

    float NdotH = max(dot(normal, H), 0.0f);
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float NdotV = max(dot(normal, eyeDir), 0.0f);
    float VdotH = max(dot(eyeDir, H), 0.0f);

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), specColor, metallic);
    float3 F = SpcFresnel(F0, VdotH);

    float D = Beckmann(roughness, NdotH);

    float k = (roughness + 1) * (roughness + 1) / 8.0f;
    float G_V = NdotV / (NdotV * (1.0f - k) + k);
    float G_L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G_V * G_L;

    float denominator = 4.0f * NdotV * NdotL + 0.001f; // ゼロ除算を防ぐために小さな値を追加

    return (F * D * G) / denominator;
}
