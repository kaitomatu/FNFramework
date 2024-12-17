#pragma once

// todo : serviceロケータ的な感じで各要素を管理するのが良いのかも
namespace RenderingData::Model
{
    constexpr std::string_view DefaultModelName = "DefaultQube";
}

/**
* @class AssetManager
* @brief アセットデータ管理クラス
* @details
*   FlyWeightパターンを用いてモデルデータを管理するクラス
*   現在対応しているのは { Model, Texture, Json }
*/
class AssetManager
    : public utl::Singleton<AssetManager>
{
    friend class utl::Singleton<AssetManager>;

public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    AssetManager()
    {
    }

    ~AssetManager() override { ClearData(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    /* @brief モデルデータの取得 */
    std::shared_ptr<ModelData> GetModelData(std::string_view fileName);

    /* @brief テクスチャデータの取得 */
    std::shared_ptr<ShaderResourceTexture> GetTexture(std::string_view fileName);

    /* @brief モデルデータの解放 */
    void ClearData();

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    // モデルデータ
    std::unordered_map<std::string, std::shared_ptr<ModelData>> m_modelDatas;
    std::shared_ptr<ModelData> LoadData(std::string_view fileName);

    // テクスチャデータ
    std::unordered_map<std::string, std::shared_ptr<ShaderResourceTexture>> m_textureDatas;
    std::shared_ptr<ShaderResourceTexture> LoadTexture(std::string_view fileName, bool constantData = false);

};
