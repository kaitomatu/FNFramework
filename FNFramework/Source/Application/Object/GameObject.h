#pragma once

class Scene;
class TransformComponent;
class ModelComponent;

#include "../Component/BaseComponent.h"

/**
* @class GameObject
* @brief ゲームオブジェクトの基底クラス
*/
class GameObject final
    : public std::enable_shared_from_this<GameObject>
{
public:
    // 状態管理用
    enum class State
    {
        eActive, // アクティブ
        ePaused, // ポーズ
        eDead, // 死亡

        // ImGuiで変更する用
        eCount
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /*
    * @param[in] Sceneのポインタ
    * @param[in] オブジェクトの名前
    * @param[in] オブジェクトの状態
    * @details Object内で自身を追加できる
    */
    GameObject(const std::shared_ptr<Scene>& baseScene, State eState)
        : m_state(eState)
        , m_wpScene(baseScene)
    {
    }

    ~GameObject()
    {
        Release();
    }

    // memo: 保存・読み込み機能（派生クラスでオーバーライド）
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

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // オブジェクト側の
    State GetState() const { return m_state; }
    void SetState(State state) { m_state = state; }

    void SetStateContagion(bool _childOnly, State _state, const std::shared_ptr<GameObject>& _spParent = nullptr)
    {
        if (_childOnly)
        {
            // 自分自身の状態を更新
            SetState(_state);
        }

        // 再帰的に子オブジェクトに対して状態を伝播
        for (auto&& wpChild : m_wpChildren)
        {
            const std::shared_ptr<GameObject>& spChild = wpChild.lock();

            if (!spChild) { continue; }

            // 有効な子オブジェクトのポインタを取得
            spChild->SetStateContagion(_state, spChild);
        }
        
    }
    void SetStateContagion(State _state, const std::shared_ptr<GameObject>& _spParent = nullptr)
    {
        // 自分自身の状態を更新
        SetState(_state);

        // 再帰的に子オブジェクトに対して状態を伝播
        for (auto&& wpChild : m_wpChildren)
        {
            const std::shared_ptr<GameObject>& spChild = wpChild.lock();

            if (!spChild) { continue; }

            // 有効な子オブジェクトのポインタを取得
           spChild->SetStateContagion(_state, spChild);
        }
    }

    // オブジェクトの名前の設定 / 取得
    void SetName(const std::string& name) { m_objName = name; }
    std::string GetName() const { return m_objName; }

    // 親オブジェクトの設定 / 取得
    const std::string& GetParentName() { return m_parentName; }
    void SetParentName(const std::string& name) { m_parentName = name; }

    std::shared_ptr<Scene> GetScene() const { return m_wpScene.lock(); }

    /**
    * @brief コンポーネントのコンテナを取得する
    * @return コンポーネントのコンテナ
    */
    const std::vector<std::shared_ptr<BaseComponent>>& GetComponents() const { return m_spComponents; }

    /**
    * @brief Transformコンポーネントを取得する
    * @return Transformコンポーネント
    */
    std::shared_ptr<TransformComponent> GetTransformComponent() const { return m_spTransformComponent.lock(); }

    //--------------------------------
    // 親子関係
    //--------------------------------
    // 親オブジェクトの設定 / 取得
    std::shared_ptr<GameObject> GetParent() const { return m_wpParent.lock(); }

    void ClearChildren() { m_wpChildren.clear(); }
    // 子オブジェクトの追加 / 取得
    const std::list<std::weak_ptr<GameObject>>& GetChildren() const { return m_wpChildren; }

    /**
     * @fn void AddChild(const std::shared_ptr<BaseObject>& child)
     * @brief 子オブジェクトの追加
     * @param child : 子オブジェクト
     */
    void AddChild(const std::shared_ptr<GameObject>& child);

    /**
     * @fn void RemoveChild(const std::shared_ptr<BaseObject>& child)
     * @brief 親子関係を解消する関数
     * @details
     *  - 親側からでも子側からでも呼び出せる
     *      - 親側 : 子供のリストの中から for で回して、自分を見つけたら削除する
     *      - 子側 : 親の子供リストから削除し、ポインタをリセットする
     * @param initiator : 親子関係を解除するオブジェクト
     * @return 削除した子オブジェクトの次のイテレータ
     */
    std::list<std::weak_ptr<GameObject>>::iterator  RemoveParentChildRelation(const std::shared_ptr<GameObject>& initiator);

    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 初期化 */
    void Init();

    /* @brief 開始 */
    void Start();

    /* @brief 更新 */
    void Update();

    /* @brief ImGui更新 */
    void ImGuiUpdate();


    //--------------------------------
    // コンポーネント管理
    //--------------------------------
    /**
    * @brief コンポーネントの追加
    * @return 追加したコンポーネントのポインタ
    */
    template <typename CompType>
    std::shared_ptr<CompType> AddComponent(bool _enableSelia = false)
    {
        // コンポーネントの生成
        std::shared_ptr<CompType> spComp =
            std::make_shared<CompType>(shared_from_this(), typeid(CompType).name(), _enableSelia);

        // 初期化をしておく
        spComp->Awake();

        // 以前に追加されたコンポーネントの更新順が早い場合のソートを行う
        auto it = m_spComponents.begin();
        UINT myOrder = static_cast<UINT>(spComp->GetUpdateOrder());
        for (; it != m_spComponents.end(); ++it)
        {
            if (myOrder < static_cast<UINT>((*it)->GetUpdateOrder())) { break; }
        }

        m_spComponents.insert(it, spComp);

        return spComp;
    }

    /**
    * @brief コンポーネントの取得
    * @return 取得したいコンポーネントのポインタ
    *
    * todo | fixme : 同じオブジェクトに複数の同一コンポーネントがついている場合に初めに追加された方しか取得できないので修正を行う
    */
    template <typename CompType>
    std::shared_ptr<CompType> GetComponent(bool assertLog = true, std::source_location _locate = std::source_location::current())
    {
        std::shared_ptr<BaseComponent> spBaseComp = nullptr;

        const std::string& compName = typeid(CompType).name();

        for (auto&& comp : m_spComponents)
        {
            // 型名が一致したらダウンキャスト用に値を保存する
            if (comp->GetComponentName() == compName)
            {
                spBaseComp = comp;
                break;
            }
        }

        // 見つからなければ警告
        if (!spBaseComp)
        {
            if (assertLog)
            {
                Assert::WarningLog("指定したコンポーネント(" + compName + ")は存在しません", false, _locate);
            }
            return nullptr;
        }

        // ダウンキャストして返す
        std::shared_ptr<CompType> spComp = std::dynamic_pointer_cast<CompType>(spBaseComp);

        return spComp;
    }

    template <typename CompType>
    bool HasComponent()
    {
        const std::string& compName = typeid(CompType).name();

        for (auto&& comp : m_spComponents)
        {
            // 探しているコンポーネントの名前が一致 = 見つかったのでtrueを返す
            if (comp->GetComponentName() == compName)
            {
                return true;
            }
        }

        // 見つからなかった
        return false;
    }

    /**
     * @fn std::shared_ptr<BaseComponent> GetComponentByName(std::string_view compName)
     * @brief IDからコンポーネントの取得 : デシリアライズなどで利用
     * @param componentID - コンポーネントのID
     * @return コンポーネントのポインタ
     */
    std::shared_ptr<BaseComponent> GetComponentByID(int componentID)
    {
        // 追加されているコンポーネントを超えないように 0 > componentID < m_spComponents.size() の場合にのみ返す
        if (componentID >= 0 && componentID < static_cast<int>(m_spComponents.size()))
        {
            return m_spComponents[componentID];
        }
        return nullptr;
    }

    /**
    * @brief コンポーネントの削除
    * @param[in] removeCompName - 削除したいコンポーネントのポインタ
    */
    void RemoveComponent(std::string_view removeCompName)
    {
        auto it = m_spComponents.begin();
        while (it != m_spComponents.end())
        {
            if ((*it)->GetComponentName() == removeCompName)
            {
                (*it)->Release();
                it = m_spComponents.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

protected:
    /* @brief 終了 */
    void Release();

    // オブジェクトの状態
    State m_state;

    std::string m_objName;

    // 各コンポーネントのポインタを格納するコンテナ
    std::vector<std::shared_ptr<BaseComponent>> m_spComponents;

    std::weak_ptr<TransformComponent> m_spTransformComponent;

    // シーンのポインタ
    std::weak_ptr<Scene> m_wpScene;

    // 親子関係
    std::weak_ptr<GameObject> m_wpParent;
    std::string m_parentName;
    std::list<std::weak_ptr<GameObject>> m_wpChildren;

    // ImGuiで利用する子ども追加用の名前
    std::string m_imguiSerchChildName;

    int m_selectItemCurrent = 0;

private:
    //----------------
    // ImGui
    //----------------
    // アクティブモード変更
    void ChangeActiveMode();
    // コンポーネントImGui制御
    void ComponentImGuiUpdate();

    //----------------
    // 更新
    //----------------
    // ワールド行列の更新
    void UpdateWorldTransform();
};

namespace jsonKey::Object
{
    constexpr std::string_view Key_Name = "Name";
    constexpr std::string_view Key_ParentName = "ParentName";
    constexpr std::string_view Key_Components = "Components";
    constexpr std::string_view Key_ComponentsName = "ComponentName";
    constexpr std::string_view Key_ComponentsID = "ComponentID";
    constexpr std::string_view Key_ComponentEnable = "Enable";
}
