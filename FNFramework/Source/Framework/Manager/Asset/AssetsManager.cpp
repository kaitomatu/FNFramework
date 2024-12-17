#include "AssetsManager.h"

std::shared_ptr<ModelData> AssetManager::LoadData(std::string_view fileName)
{
    auto modelData = std::make_shared<ModelData>();

    if (!modelData->Load(fileName.data()))
    {
        FNENG_ASSERT_LOG("ImportFileName : " + std::string(fileName) + "\nモデルのロードに失敗。パスを確認してください", false);
        return nullptr;
    }

    m_modelDatas[fileName.data()] = modelData;

    return std::move(modelData);
}

std::shared_ptr<ShaderResourceTexture> AssetManager::LoadTexture(std::string_view fileName, bool constantData)
{
    auto texture = std::make_shared<ShaderResourceTexture>();

    if (!texture->Load(fileName.data(), constantData))
    {
        FNENG_ASSERT_LOG("ImportFileName : " + std::string(fileName) + "\nテクスチャのロードに失敗。パスを確認してください", false);
        return nullptr;
    }

    // ファイルパスを設定
    texture->SetFilePath(fileName.data());

    m_textureDatas[fileName.data()] = texture;

    return texture;
}

std::shared_ptr<ModelData> AssetManager::GetModelData(std::string_view fileName)
{
    // データがすでにあるかを検索する
    auto findData = m_modelDatas.find(fileName.data());

    // 読み込み済み
    if (findData != m_modelDatas.end())
    {
        // そのままデータを共有
        return findData->second;
    }

    // 読み込み済みでない
    // 新たにデータをロードする
    const auto& spLoadModelData = LoadData(fileName);

    // もし読み込みに失敗したらデフォルトのモデルを返す
    if (!spLoadModelData)
    {
        const auto& spDefaultModel = GetModelData(RenderingData::Model::DefaultModelName.data());

        if(!spDefaultModel)
        {
            FNENG_ASSERT_ERROR("デフォルトのモデルが読み込めませんでした。")
            return nullptr;
        }

        return spDefaultModel;
    }

    return spLoadModelData;
}

std::shared_ptr<ShaderResourceTexture> AssetManager::GetTexture(std::string_view fileName)
{

    // データがすでにあるかを検索する
    auto findData = m_textureDatas.find(fileName.data());

    // 読み込み済み
    if (findData != m_textureDatas.end())
    {
        // そのままデータを共有
        return findData->second;
    }
    // 読み込み済みでない
    // 新たにデータをロードする
    return LoadTexture(fileName);
}

void AssetManager::ClearData()
{
    //--------------------------------
    // モデルデータの解放 - モデルデータの参照カウントが1のものを解放
    //--------------------------------
    // アプリ上で使用されておらず、ModelManagerクラスが保持しているだけのデータを破棄
    for (auto dataIter = m_modelDatas.begin(); dataIter != m_modelDatas.end();)
    {
        if (dataIter->second.use_count() < 2)
        {
            dataIter = m_modelDatas.erase(dataIter);

            continue;
        }

        ++dataIter;
    }

    //--------------------------------
    // テクスチャデータの解放 - テクスチャデータの参照カウントが1のものを解放
    //--------------------------------
    // アプリ上で使用されておらず、ModelManagerクラスが保持しているだけのデータを破棄
    for (auto dataIter = m_textureDatas.begin(); dataIter != m_textureDatas.end();)
    {
        if (dataIter->second.use_count() < 2)
        {
            dataIter = m_textureDatas.erase(dataIter);

            continue;
        }

        ++dataIter;
    }
}
