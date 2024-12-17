#pragma once

namespace jsonKey
{
    constexpr std::string_view Key_Objects = "Objects";
}

class MainCamera;
class Camera;

#include "Framework/System/Device/Keyboard/InputSystem.h"

#include "Framework/System/ImGui/ImGuiUpdate/ImGuiUpdate.h"

/**
* @class Scene
* @brief シーン基底クラス
*/
class Scene final
    : public std::enable_shared_from_this<Scene>
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Scene(std::string_view sceneName)
        : m_sceneName(sceneName)
    {
        // ImGuiWindowに描画するものの初期化
        m_spImGuiUpdate = std::make_unique<class ImGuiUpdate>();
        m_spImGuiUpdate->Init();
    }

    ~Scene() = default;

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /* @brief シーンの名前取得 @return シーンの名前 */
    const std::string& GetSceneName() const
    {
        return m_sceneName;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 更新前準備 */
    void PreUpdate();
    /* @brief 更新処理 */
    void Update();


    /* @brief 初期化 */
    void Init();
    /* @brief 解放 */
    void Release();

    /**
     * @fn virtual void Serialize(JsonWrapper& json) const
     * @brief シリアライズ
     * @details
     *  - 派生クラスでオーバーライドして、シリアライズ処理を行う
     *  - シリアライズ処理 : オブジェクトをJson形式に変換する
     * @param json : シリアライズ用Jsonラッパー
     */
    void Serialize(Json& json) const;
    /**
     * @fn virtual void Deserialize(const JsonWrapper& json)
     * @brief デシリアライズ
     * @details
     *  - 派生クラスでオーバーライドして、デシリアライズ処理を行う
     *  - デシリアライズ処理 : Json形式のデータをオブジェクトに変換する
     * @param json : デシリアライズ用Jsonラッパー
     */
    void Deserialize(const Json& json);


    std::list<std::shared_ptr<GameObject>>& GetObjectList() { return m_spObjectList; }

    /**
    * @brief オブジェクトの追加
    * @param[in] eState - 追加するオブジェクトの状態
    * @param[in] name - 追加するオブジェクトの名前
    * @return 追加したオブジェクトのポインタ
    */
    std::shared_ptr<GameObject> AddObject(GameObject::State eState = GameObject::State::eActive,
                                          std::string_view name = "BaseObj");

    void ClearObjList()
    {
        m_spObjectList.clear();
        m_spPendingObjectList.clear();
    }

    void AddObjectImGui();

    /**
    * @brief オブジェクトの取得
    * @param[in] name - 取得したいオブジェクトの名前
    * @return 取得したいオブジェクトのポインタ
    */
    std::shared_ptr<GameObject> FindObject(std::string_view name);

    /**
      * @brief コンテナ内にすでにある名前と被らない名前を生成する
      * @param[in] container - 名前が被らないか確認したいコンテナ
      * @param[in] baseName  - 基本となる名前
      * @return 生成した名前
      */
    static std::string GenerateUniqueName(const std::list<std::shared_ptr<GameObject>>& container, std::string_view baseName);

    /**
      * @brief コンテナ内のオブジェクトの名前を変更する
      * @brief ※新しくメモリ確保するので、更新頻度が高い場合は注意が必要
      * @param[in] container  - 名前を変更したいコンテナ
      * @param[in] oldName    - 変更前の名前
      * @param[in] newName    - 変更後の名前
      * @return 名前の変更に成功したらtrue
      *
      */
    static bool ChangeObjectName(
        std::list<std::shared_ptr<GameObject>>& container,
        std::string_view oldName,
        std::string_view newName,
        bool isRename);

private:

    // シーンの名前
    std::string m_sceneName;

    //------------------
    // オブジェクト管理
    //------------------
    // 実際にメインフレームで処理を行うオブジェクトリスト
    // key : オブジェクトの名前 - value : オブジェクトのポインタ
    std::list<std::shared_ptr<GameObject>> m_spObjectList;
    // 変更などがあった場合の変更を保存するためのオブジェクトリスト
    // key : オブジェクトの名前 - value : オブジェクトのポインタ
    std::list<std::shared_ptr<GameObject>> m_spPendingObjectList;
    // 更新中かどうか
    bool m_isUpdatingObject = false;

    std::string m_generateObjectName;

    //---------------//テスト用なので後々消す//---------------//
    std::unique_ptr<class ImGuiUpdate> m_spImGuiUpdate;
};
