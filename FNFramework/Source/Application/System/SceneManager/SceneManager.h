#pragma once

constexpr std::string_view ScenesDirectoryPath = "Assets/Scenes";

constexpr std::string_view SettingFileName = "Assets/Data/Settings/Setting.json";
constexpr std::string_view FirstLoadSceneName = "FirstLoadSceneName";
//constexpr std::string_view TargetFPS = "TargetFPS";
//constexpr std::string_view ScreenSize = "ScreenSize";

// todo : トランジションの処理は仮実装のため後で修正する

class Scene;
class DebugWire;
class Fade;
class GameObject;

/**
* @class SceneManager
* @brief シーン管理クラス : シングルトン
* */
class SceneManager
    : public utl::Singleton<SceneManager>
{
    friend class utl::Singleton<SceneManager>;

public:

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    /* @brief デルタタイムを取得 @return デルタタイム */
    double FrameDeltaTime() const { return m_deltaTime; }

    // シーンリストの取得
    std::unordered_map<std::string, std::shared_ptr<Scene>>& GetSceneList() { return m_umNameToScene; }


    const std::shared_ptr<Fade>& GetFade() const { return m_spFade; }

    /**
    * @brief デバッグワイヤークラス取得
    * @return デバッグワイヤークラス
    */
    const std::unique_ptr<DebugWire>& GetDebugWire() { return m_upDebugWire; }


    void SetTransitionResult(bool result) { m_transitionResult = result; }

    /**
     * @fn void TransitionAndChangeScene()
     *
     * @brief トランジションエフェクトを行い、シーンを変更する
     */
    void TransitionAndChangeScene();

    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 初期化 */
    void Init();

    /* @brief 更新前準備 */
    void PreUpdate();
    /* @brief 更新処理 */
    void Update();
    /* @brief 更新後処理 */
    void PostUpdate();

    template <typename CompType>
    void RegisterComponent(bool _isGui = true)
    {
        const std::string_view compName = typeid(CompType).name();

        if (_isGui)
        {
            m_vCompNames.push_back(compName.data());
        }

        // hack : typeid(CompType).name() : ここは最適化できるよ
        m_umCompNameToAddCompFunc[compName.data()] = [](std::shared_ptr<GameObject> _spObj) -> std::shared_ptr<CompType>
            {
                // ここの関数を通して追加されるコンポーネントはシリアライズを行う
                return _spObj->AddComponent<CompType>(true);
            };

        // typeid(CompType).name() : ここはなんか最適化できるよ
        m_umCompNameToRemoveCompFunc[compName.data()] = [compName](std::shared_ptr<GameObject> _spObj)
            {
                // コンポーネントの検索 + 削除対象
                _spObj->RemoveComponent(compName);
            };
    }

    void AddComponentToObject(std::string_view _compName, std::shared_ptr<GameObject> _spObj);
    void RemoveComponentToObject(std::string_view _compName, std::shared_ptr<GameObject> _spObj);

    std::vector<const char*>& GetCompNames() { return m_vCompNames; }
    std::vector<const char*>& GetSceneNames() { return m_vSceneNames; }

    /// すでに追加されているシーンかどうか
    bool IsLoadedScene(std::string_view _sceneName);

    void AddScene(std::string_view _sceneName);
    
    void ChangeScene(std::string_view _sceneName);
    void RemoveScene(std::string_view _sceneName);
    std::shared_ptr<Scene> GetScene(std::string_view _sceneName);
    std::shared_ptr<Scene> GetNowScene();

    void SceneUpdate();

    bool SaveScene(std::string_view _sceneName);
    bool SaveScene(std::shared_ptr<Scene> _scene);

    //---------------------------
    // ゲッター / セッター
    //---------------------------
    std::string GetNowSceneName() const { return m_nowSceneName; }
private:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    SceneManager()
    {
    }

    ~SceneManager() override { Release(); }

    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 解放処理 */
    void Release();

private:
    void RegisterComponent();
    void ImportSceneFiles();

    std::unordered_map<std::string, std::shared_ptr<Scene>> m_umNameToScene;
public:
    // シーンの名前とシーンデータのパスを紐づけるコンテナ
    std::unordered_map<std::string, std::filesystem::path> m_umSceneNameToPath;

    std::string m_nowSceneName;

    std::vector<const char*> m_vSceneNames;

    std::unordered_map<std::string, std::function<void(std::shared_ptr<GameObject>)>> m_umCompNameToAddCompFunc;
    std::unordered_map<std::string, std::function<void(std::shared_ptr<GameObject>)>> m_umCompNameToRemoveCompFunc;
    std::vector<const char*> m_vCompNames;

private:

    // デルタタイム
    double m_deltaTime = 0.0;

    // デバッグワイヤー
    std::unique_ptr<DebugWire> m_upDebugWire;

    // todo | hack : メンバで持つのではなく SceneChangerComponent を作成し、そちらで管理する
    std::shared_ptr<Fade> m_spFade; // トランジションエフェクトクラス : todo - リスト管理にしてシーン追加時にpush_backする

    bool m_transitionResult = false;
    std::string m_firstLoadSceneName;

    // 入力処理
    void CreateInputButton();
};
