#pragma once

using Json = nlohmann::json;

namespace utl::file
{
    /**
        * @brief  ファイルパスから親ディレクトリまでのパスを取得
        *
        * @param  path - ファイルパス
        * @result ディレクトリパス
        */
    inline std::string GetDirFromPath(const std::string& path)
    {
        const std::string::size_type pos = path.find_last_of('/');
        return (pos == std::string::npos) ? std::string() : path.substr(0, pos + 1);
    }


    template <typename T>
    inline void SaveSet(const Json& _json, std::string_view _key, T& _target)
    {
        auto itr = _json.find(_key);

        // ファイルが存在したらデータをセットする
        if (itr != _json.end())
        {
            _target = itr.value().get<T>();
        }
    }


    /**
     * @fn bool SaveToFile(std::string_view _filePath)
     * @brief ファイルからJsonを読み込む
     * @param _filePath 開くJsonのファイルパス
     * @return ファイルの読み込み成功かどうか
     */
    inline bool LoadFromFile(Json& _json, std::string_view _filePath)
    {
        std::ifstream file(_filePath.data());

        if (file.fail())
        {
            // ファイルが存在しない場合は、新しいJsonオブジェクトを作成
            _json = Json::object();
            return false;
        }

        // ----------- ここより下は時点でファイルは開けている ----------- //
        try
        {
            file >> _json; // ファイルデータからJsonファイルの読み込み
            file.close();
            return true;
        }
        catch (const Json::parse_error& e)
        {
            FNENG_ASSERT_ERROR(e.what())
            file.close();
            return false;
        }
    }

    /**
        * @fn bool SaveToFile(std::string_view _filePath)
        * @brief Jsonをファイルに保存する
        * @param _filePath 保存するJsonのファイルパス
        * @return ファイルの保存成功かどうか
        */
    inline bool SaveToFile(const Json& _json, std::string_view _filePath)
    {
        // ファイルパスからディレクトリ部分を取得
        std::string dir = utl::file::GetDirFromPath(_filePath.data());

        // ディレクトリが存在しない場合は作成（既に存在する場合は何もしない）
        if (!std::filesystem::create_directories(dir) && !std::filesystem::exists(dir))
        {
            FNENG_ASSERT_ERROR("ディレクトリの作成に失敗しました。パスを確認してください");
            return false;
        }

        // 既存のファイルが存在しない場合は作成
        std::ofstream file(_filePath.data(), std::ios::out | std::ios::trunc);

        if (file.fail())
        {
            // ファイルの作成に失敗した場合のエラーハンドリング
            FNENG_ASSERT_ERROR("Jsonファイルの保存に失敗しました。ファイルパスを確認してください");
            return false;
        }

        // ----------- ここより下は時点でファイルは開けている ----------- //
        try
        {
            file << _json.dump(4); // Jsonファイルの書き込み(インデント付き)
            file.close();
            return true;
        }
        catch (const Json::exception& e)
        {
            FNENG_ASSERT_ERROR(e.what())
            file.close();
            return false;
        }
    }
}


/**
 * @class JsonWrapper
 * @brief Jsonファイルの読み込み / 書き込みの補助を行うクラス
 */
class JsonWrapper
{
public:

    JsonWrapper()
        : m_jsonObject(Json::object()) {}

    //x=================================x
    //         読み込み関数
    //x=================================x
    /**
     * @fn bool JsonWrapper(std::string_view filePath)
     * @brief 文字列からのJson読み込み
     * @param _jsonStr Jsonにしたい文字列
     * @return 読み込み成功かどうか
     */
    bool FromString(std::string_view _jsonStr)
    {
        try
        {
            m_jsonObject = Json::parse(_jsonStr);
            return true;
        }
        catch (const Json::parse_error& e)
        {
            FNENG_ASSERT_ERROR(e.what())
            return false;
        }
    }

    /**
     * @fn bool SaveToFile(std::string_view _filePath)
     * @brief ファイルからJsonを読み込む
     * @param _filePath 開くJsonのファイルパス
     * @return ファイルの読み込み成功かどうか
     */
    bool LoadFromFile(std::string_view _filePath)
    {
        std::ifstream file(_filePath.data());

        if (file.fail())
        {
            // ファイルが存在しない場合は、新しいJsonオブジェクトを作成
            m_jsonObject = Json::object();
            return false;
        }

        // ----------- ここより下は時点でファイルは開けている ----------- //
        try
        {
            file >> m_jsonObject; // ファイルデータからJsonファイルの読み込み
            file.close();
            return true;
        }
        catch (const Json::parse_error& e)
        {
            FNENG_ASSERT_ERROR(e.what())
            file.close();
            return false;
        }
    }


    /**
     * @fn void FromJson(const Json& json)
     * @briief 既存のJsonオブジェクトから初期化する
     * @param json Jsonオブジェクト
     */
    void FromJson(const Json& json)
    {
        m_jsonObject = json;
    }

    //x=================================x
    //         書き込み関数
    //x=================================x
    /**
     * @fn bool SaveToFile(std::string_view _filePath)
     * @brief Jsonをファイルに保存する
     * @param _filePath 保存するJsonのファイルパス
     * @return ファイルの保存成功かどうか
     */
    bool SaveToFile(std::string_view _filePath)
    {
        // ファイルパスからディレクトリ部分を取得
        std::string dir = utl::file::GetDirFromPath(_filePath.data());

        // ディレクトリが存在しない場合は作成（既に存在する場合は何もしない）
        if (!std::filesystem::create_directories(dir) && !std::filesystem::exists(dir))
        {
            FNENG_ASSERT_ERROR("ディレクトリの作成に失敗しました。パスを確認してください");
            return false;
        }

        // 既存のファイルが存在しない場合は作成
        std::ofstream file(_filePath.data(), std::ios::out | std::ios::trunc);

        if (file.fail())
        {
            // ファイルの作成に失敗した場合のエラーハンドリング
            FNENG_ASSERT_ERROR("Jsonファイルの保存に失敗しました。ファイルパスを確認してください");
            return false;
        }

        // ----------- ここより下は時点でファイルは開けている ----------- //
        try
        {
            file << m_jsonObject.dump(4); // Jsonファイルの書き込み(インデント付き)
            file.close();
            return true;
        }
        catch (const Json::exception& e)
        {
            FNENG_ASSERT_ERROR(e.what())
            file.close();
            return false;
        }
    }


    //x=================================x
    //         Json操作関数
    //x=================================x
    /**
     * @fn void AddValue(std::string_view key, const T& value)
     * @brief Jsonにあるキーに入っている値を取得
     * @tparam T ゲットしたい型
     * @param _key Jsonのキー
     * @param _defaultValue キーが存在しない場合のデフォルト値
     * @return キーに対応する値
     */
    template<typename T>
    T GetValue(std::string_view _key, const T& _defaultValue = T()) const
    {
        try
        {
            return m_jsonObject.at(_key.data()).get<T>();
        }
        catch (const Json::exception& e)
        {
            FNENG_ASSERT_ERROR(e.what())
            return _defaultValue;
        }
    }

    /**
     * @fn void SetValue(std::string_view key, const T& value)
     * @brief Jsonに値をセット
     * @tparam T セットしたい型
     * @param _key Jsonのキー
     * @param _value セットしたい値
     */
    template<typename T>
    void SetValue(std::string_view _key, const T& _value)
    {
        m_jsonObject[_key.data()] = _value;
    }

    /**
     * @fn void ContainsKey(std::string_view key)
     * @brief 指定したキーがJSONオブジェクト内に存在するかを確認
     * @param key 存在を確認したいキー
     * @return キーが存在するかどうか
     */
    bool ContainsKey(std::string_view key) const
    {
        return  m_jsonObject.count(key.data()) > 0;
    }

    // Jsonオブジェクトを取得
    Json GetJson() const
    {
        return m_jsonObject;
    }

    // Jsonオブジェクトを文字列に変換
    std::string ToString() const
    {
        return m_jsonObject.dump();
    }

private:
    // 内部で保持するJSONオブジェクト
    Json m_jsonObject;
};
