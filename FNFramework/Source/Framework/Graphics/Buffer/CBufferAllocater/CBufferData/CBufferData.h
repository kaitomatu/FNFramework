#pragma once

struct PointLight
{
    PointLight()
    {
    }

    PointLight(const Math::Vector3& pos, float radius, const Math::Vector3& color)
        : Position(pos), Range(radius), Color(color)
    {
    }

    Math::Vector3 Position = {0.0f, 9.0f, 0.0f}; // 座標
    float Range = 30.0f; // 影響範囲
    Math::Vector3 Color = {1.0f, 1.0f, 1.0f}; // カラー
    float Intensity = 0.0f;
};

// CBufferData - 定数バッファアクセス用
// ここはあくまで汎用的な物だけを置いておく。
// 特定のシェーダーファイルのみでしか使わないやつは分けるべき...？
namespace CBufferData
{
    // カメラ用定数バッファ
    struct Camera
    {
        Math::Matrix mViewProj; // ビュー行列
        Math::Matrix mViewProjInv; // 射影行列

        Math::Vector3 CamPos; // カメラのワールド座標
        float Height = 0.0f; // カメラの高さ - シャドウマップで利用する
    };

    // マテリアル用定数バッファ
    struct cbMaterial
    {
        Math::Vector4 BaseColor = {1.0f, 1.0f, 1.0f, 1.0f}; // 基本色のスケーリング係数(RGBA)
        float Metallic = 0.0f; // 金属のスケーリング係数
        float Roughness = 1.0f; // 粗さのスケーリング係数
        float pad[2] = {};
        Math::Vector3 Emissive = {0.0f, 0.0f, 0.0f}; // 自己発光のスケーリング係数(RGB)
        float pad1 = 0.0f;
    };

    struct cbObject
    {
        cbObject()
        {
        }

        cbObject(const Math::Matrix& _mWorld, bool _isSkin)
            : mWorld(_mWorld), IsSkin(_isSkin)
        {
        }

        Math::Matrix mWorld; // ワールド行列

        bool IsSkin = false;
        float pad[3] = {};
    };
    struct cbDeferredObject
    {
        cbDeferredObject()
        {
        }

        cbDeferredObject(UINT _bonePerInstance, bool _isSkin)
            : BonePerInstance(_bonePerInstance), IsSkin(_isSkin)
        {
        }

        float BonePerInstance = 0.0f;

        bool IsSkin = false;
        float pad[2] = {};
    };
    struct cbSpriteObject
    {
        cbSpriteObject()
        {
        }

        cbSpriteObject(const Math::Matrix& world, const Math::Vector4& color = Color::White)
            : mWorld(world), Color(color)
        {
        }

        Math::Matrix mWorld; // ワールド行列
        Math::Vector4 Color; // 色

        Math::Vector2 Tiling = { 1.0f, 1.0f }; // テクスチャの繰り返し数
        Math::Vector2 Offset = { 0.0f, 0.0f }; // テクスチャのオフセット
    };

    struct cbFog
    {
        //x--- 距離フォグ ---x//
        int FogEnable   = 0;    // フォグを有効にするかどうか
        Math::Vector3 FogColor = {0.0f, 0.0f, 0.0f}; // フォグのカラー

        float DistanceFogDensity = 0.001f;	// 距離フォグ減衰率
        float DistanceFogStart = 0.0f;		// 距離フォグ開始距離
        float _pad[2] = {};
    };

    struct cbCousticsData
    {
        // 経過時間
        float Time = 0.0f;
        // スピード
        float Speed = 0.0f;

        // 水面までの高さ
        float Height = 0.0f;

        // スケール / 光の強さ
        float CousticsScale = 0.0f;

        Math::Vector3 CousticsColor = { 1.0f, 1.0f, 1.0f };
        float Intencity = 1.0f;
    };

    // 基本的なライト用定数バッファ
    struct Light
    {
        static constexpr int MaxPointLightNum = 300;

        //-----------環境光---------------//
        Math::Vector4 AmbientLight = { 0.4f, 0.4f, 0.4f, 1.0f };
        //--------------------------------//

        //-----------平行光---------------//
        Math::Vector3 LigDirection = {1.0f, -1.0f, 1.0f}; // ライトの方向
        float pad = 0.0f;
        Math::Vector3 LigColor = {1.0f, 1.0f, 1.0f}; // ライトのカラー
        float pad1 = 0.0f;

        Math::Matrix DirLight_mVP; // ビュー行列と正射影行列の合成行列
        //--------------------------------//

        //-----------ポイントライト---------------//
        int FramePointLightNum = 0; // 今アクティブなポイントライトの数
        float pad2[3] = {};

        std::array<PointLight, MaxPointLightNum> PointLights;
        //----------------------------------------//
    };
}
