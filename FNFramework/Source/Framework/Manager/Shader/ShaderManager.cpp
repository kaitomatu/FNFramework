#include "ShaderManager.h"

#include "AmbientManager.h"
#include "Application/Object/Camera/Camera.h"

const std::shared_ptr<Camera> ShaderManager::FindCameraData(std::string_view camName) const
{
    auto it = m_cbCameraList.find(camName.data());
    // カメラデータが見つからない場合はnullptrを返す
    if (it == m_cbCameraList.end()) { return nullptr; }

    return m_cbCameraList.at(camName.data());
}

bool ShaderManager::SetCBCameraData(int idx, std::string_view camName) const
{
    //---------------------
    // 定数バッファセット
    //---------------------
    const std::shared_ptr<Camera> camDat = FindCameraData(camName);

    if (!camDat)
    {
        FNENG_ASSERT_ERROR("カメラ情報が登録されていませんでした");
        return false;
    }
     
    // カメラ情報があればシェーダーにセットする
    const CBufferData::Camera& cbData = camDat->GetCBData();

    GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(idx, cbData);

    return true;
}

void ShaderManager::RegisterCamData(const std::shared_ptr<Camera>& camera)
{
    // 定数バッファとして必要なデータのみを抽出する //
    const std::string& camName = camera->GetCameraName();

    // 名前がからならデフォルト名を設定しカメラにセット
    if (camName.empty())
    {
        std::string defaultName = "NewCamera";
        auto it = m_cbCameraList.find(defaultName);
        if(it != m_cbCameraList.end())
        {
            // 既に登録されている場合は連番を付与
            int num = 1;
            while (true)
            {
                defaultName = "NewCamera" + std::to_string(num);
                if (m_cbCameraList.find(defaultName) == m_cbCameraList.end()) { break; }
                num++;
            }
        }

        camera->SetCameraName(defaultName);
    }

    // マップ内を検索して、未登録の物はメモリ確保を行う
    auto it = m_cbCameraList.find(camName);
    if (it == m_cbCameraList.end())
    {
        // まだ登録されていないならメモリ確保を行う
        m_cbCameraList[camName] = camera;
    }
    else
    {
        // 既に登録されている場合、カメラ情報が更新された時だけコピーを行う
        if (!camera->IsUpdate()) { return; }

        *it->second = *camera;

        // コピーが終わったら更新フラグを下げる
        camera->SetIsUpdate(/* m_isUpdate = */ false);
    }
}

void ShaderManager::Init()
{
    if(!m_upAmbientManager)
    {
        m_upAmbientManager = std::make_unique<AmbientManager>();
        m_upAmbientManager->Init();
    }

    //=============================
    // 各種シェーダー作成 & 初期化
    //=============================
    //------------------
    // 3D描画関連
    //------------------
    // 頂点描画用シェーダー
    m_upGenericShapeShader = std::make_unique<GenericShapeShader>();
    // 影情報書き込みシェーダー
    m_upShadowShader = std::make_unique<Shadow>();

    m_upGBufferPass = std::make_unique<GBufferPass>();
    m_upLightingPass = std::make_unique<LightingPass>();

    m_upBloomShader = std::make_unique<BloomPass>();

    //------------------
    // 2D描画関連
    //------------------
    // スプライト用シェーダー
    m_upSpriteShader = std::make_unique<SpriteShader>();
}

void ShaderManager::Release()
{
    m_cbCameraList.clear();
    m_upAmbientManager->ClearLight();
}
