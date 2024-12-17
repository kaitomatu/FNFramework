#pragma once

// todo : unique_ptrを利用する場合は、ヘッダファイルで宣言する必要があるので運用方法をもう少し考える
#include "Framework/Manager/Shader/ModelShader/ModelShader.h"
#include "Framework/Manager/Shader/Unlit/ModelShader_Unlit.h"
#include "Framework/Manager/Shader/ShadowShader/Shadow.h"
#include "Framework/Manager/Shader/GenericShapeShader/GenericShapeShader.h"
#include "Framework/Manager/Shader/SpriteShader/SpriteShader.h"
#include "SkinMeshModelShader/SkinMeshModelShader.h"
#include "DeferredRenderingPass/DeferredRenderingPass.h"
#include "PostProcess/PostProcess.h"

#include "Framework/Manager/Shader/AmbientManager.h"

class Camera;

namespace RenderingData
{
    static const std::string& MainCameraName = "MainCamera";
    static const std::string& LightCameraName = "LightCamera";

}

// todo : 各シェーダーのBegin/Endをここでやるのは役割が多すぎるので、各シェーダーが自分でやるようにする
class ShaderManager
    : public utl::Singleton<ShaderManager>
{
    friend class utl::Singleton<ShaderManager>;

public:
    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    const std::unique_ptr<AmbientManager>& GetAmbientManager() const { return m_upAmbientManager; }

    //------------------
    // 3D描画関連
    //------------------
    /* @brief 幾何学図形描画シェーダー取得 @return m_upGenericShapeShader */
    const std::unique_ptr<GenericShapeShader>& GetGenericShapeShader() const { return m_upGenericShapeShader; }
    // 作業可能
    std::unique_ptr<GenericShapeShader>& WorkGenericShapeShader() { return m_upGenericShapeShader; }

    /* @brief 影情報書き込みシェーダー取得 @return m_upShadowShader */
    const std::unique_ptr<Shadow>& GetShadowShader() const { return m_upShadowShader; }
    // 作業可能
    std::unique_ptr<Shadow>& WorkShadowShader() { return m_upShadowShader; }

    const std::unique_ptr<GBufferPass>& GetGBufferPass() const { return m_upGBufferPass; }
    // 作業可能
    std::unique_ptr<GBufferPass>& WorkGBufferPass() { return m_upGBufferPass; }
    const std::unique_ptr<LightingPass>& GetLightingPass() const { return m_upLightingPass; }
    // 作業可能
    std::unique_ptr<LightingPass>& WorkLightingPass() { return m_upLightingPass; }

    /* @brief ポストエフェクト用シェーダー取得 @return m_upBloomShader */
    const std::unique_ptr<BloomPass>& GetBloomShader() const { return m_upBloomShader; }
    // 作業可能
    std::unique_ptr<BloomPass>& WorkBloomShader() { return m_upBloomShader; }

    //------------------
    // 2D描画関連
    //------------------
    /* @brief 2Dスプライト描画シェーダー取得 @return m_upModelShaoe_UnLitShader */
    const std::unique_ptr<SpriteShader>& GetSpriteShader() const { return m_upSpriteShader; }
    // 作業可能
    std::unique_ptr<SpriteShader>& WorkSpriteShader() { return m_upSpriteShader; }

    //------------------
    // 定数バッファ関連
    //------------------
    // カメラ定数バッファ
    /* @brief カメラ定数バッファへの情報セット */
    void RegisterCamData(const std::shared_ptr<Camera>& camera);
    /* @brief カメラ定数バッファ取得 @return カメラ定数バッファ　*/
    const std::shared_ptr<Camera> FindCameraData(std::string_view camName) const;

    void EraseCameraCBData(std::string_view camName) { m_cbCameraList.erase(camName.data()); }


    // Memo - ToDo : カメラとライトはCommon.hlsliにあるので、idxはなしで直地で入れる？
    // カメラ定数バッファのセット
    bool SetCBCameraData(int idx, std::string_view camName) const;

    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 初期化 */
    void Init();
    /* @brief 解放 */
    void Release();

private:
    //========================
    // 各種シェーダー
    //========================
    // 頂点描画シェーダー
    std::unique_ptr<GenericShapeShader> m_upGenericShapeShader = nullptr;
    // 影情報書き込みシェーダー
    std::unique_ptr<Shadow> m_upShadowShader = nullptr;
    // スプライトシェーダー
    std::unique_ptr<SpriteShader> m_upSpriteShader = nullptr;

    // G-Buffer用シェーダー
    std::unique_ptr<GBufferPass> m_upGBufferPass = nullptr;
    std::unique_ptr<LightingPass> m_upLightingPass = nullptr;

    // ポストエフェクト用シェーダー
    std::unique_ptr<BloomPass> m_upBloomShader = nullptr;

    //========================
    // 各種定数
    //========================
    // 定数バッファ用カメラ行列
    std::unordered_map<std::string, std::shared_ptr<Camera>> m_cbCameraList;

    std::unique_ptr<AmbientManager> m_upAmbientManager = nullptr;

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    ShaderManager()
    {
    }

    ~ShaderManager() override { Release(); }
};
