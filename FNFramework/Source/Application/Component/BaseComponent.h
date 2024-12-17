#pragma once

class GameObject;

/**
* @class BaseComponent
* @brief コンポーネントの基底クラス
*/
class BaseComponent
    : public std::enable_shared_from_this<BaseComponent>
{
public:
    // コンポーネントの更新順設定用 : 定義されていないコンポーネントは、eDefaultになる
    enum class ComponentType
    {
        eDefault = 100, // 既定値
        eTranform, // 行列合成を行うので、最後
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをおこなうかどうか
    * @param[in] order - コンポーネントの更新順 : 既定値 = 100 - この値が小さいほど先に更新される
    */
    BaseComponent(
        const std::shared_ptr<GameObject>& owner,
        const std::string& name,
        bool _enableSerialize = false,
        ComponentType order = ComponentType::eDefault)
        : m_wpOwnerObj(owner)
        , m_compName(name)
        , m_enableSerialize(_enableSerialize)
        , m_updateOrder(order)
    {
    }

    virtual ~BaseComponent()
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
    * @brief コンポーネントの名前取得
    * @return コンポーネントの名前
    */
    const std::string& GetComponentName() const { return m_compName; }

    /*
    * @brief オーナーオブジェクトのポインタ取得
    * @return オーナーオブジェクトのポインタ
    */
    const std::shared_ptr<GameObject> GetOwner() const { return m_wpOwnerObj.lock(); }

    /**
    * @brief コンポーネントの更新順取得
    * @return コンポーネントの更新順
    */
    ComponentType GetUpdateOrder() const { return m_updateOrder; }

    /**
    * @brief コンポーネントの名前設定
    * @param[in] name - 設定するコンポーネントの名前
    */
    void SetComponentName(std::string_view name) { m_compName = name; }

    /**
    * @brief コンポーネントの更新順設定
    * @param[in] order - 設定するコンポーネントの更新順
    */
    void SetUpdateOrder(ComponentType order) { m_updateOrder = order; }

    // コンポーネントの有効無効設定
    bool IsEnable() const { return m_isEnable; }
    void SetEnable(bool isEnable) { m_isEnable = isEnable; }

    // シリアライズを行うかどうか
    bool EnableSerialize() const { return m_enableSerialize; }

    // Start()が呼ばれたかどうか
    bool IsStart() const { return m_isStart; }
    void DownStartFlg() { m_isStart = false; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    virtual void Awake() {}

    /**
     * @brief 初回のUpdateでStartを呼び出す
     * @details
     *	Startを呼び出す
     *	オーナーが有効で、Startが呼び出されていない場合に呼び出す
     */
    void StartComponent()
    {
        if (m_isStart) { return; }
        m_isStart = true;
        Start();
    }

    /* @brief ImGui更新 */
    virtual void ImGuiUpdate() {}

    /* @brief 更新 */
    virtual void Update() {}
    virtual void UpdateWorldTransform() {}

    virtual void PreUpdate() {}
    virtual void PostUpdate() {}

    /* @brief 終了 */
    virtual void Release() {}

    // memo: 保存・読み込み機能（派生クラスでオーバーライド）
    /**
     * @fn virtual void Serialize(JsonWrapper& json) const
     * @brief シリアライズ
     * @details
     *  - 派生クラスでオーバーライドして、シリアライズ処理を行う
     *  - シリアライズ処理 : オブジェクトをJson形式に変換する
     * @param _json : シリアライズ用Jsonラッパー
     */
    virtual void Serialize(Json& /* _json */) const {}
    /**
     * @fn virtual void Deserialize(const JsonWrapper& json)
     * @brief デシリアライズ
     * @details
     *  - 派生クラスでオーバーライドして、デシリアライズ処理を行う
     *  - デシリアライズ処理 : Json形式のデータをオブジェクトに変換する
     * @param _json : デシリアライズ用Jsonラッパー
     */
    virtual void Deserialize(const Json& /* _json */) {}

protected:

    /**
     * @brief Awakeを経て初期化された後、1度だけ呼びだされる
     * @details
     *	Awakeの後に呼び出される
     *	他のコンポーネントとの依存関係にある初期化処理や
     *	Awakeの段階ではできない初期化を行う
     */
    virtual void Start()
    {
    }

    /* @brief オーナーが有効か判定を行う @return オーナーが設定されていたら true */
    bool OwnerValid() const
    {
        if (m_wpOwnerObj.expired())
        {
            FNENG_ASSERT_ERROR("オーナーが設定されていません");
            return false;
        }
        return true;
    }

    /**
    * @brief 前提コンポーネントがセットされているか確認する
    * @brief 前提コンポーネントがセットされていない場合は、エラーを出力する
    * @param[in] preComp - 前提コンポーネントのポインタ
    * @return 前提コンポーネントが設定されていたら true
    */
    template <class Comp>
    bool PreClassValid(const std::shared_ptr<Comp>& preComp, bool isAssert = true)
    {
        if (!preComp)
        {
            if (!isAssert) { return false; }

            std::string owner = m_compName + "コンポーネントの\n";
            std::string preCompName = std::string(typeid(Comp).name()) + "が設定されていません\n";

            FNENG_ASSERT_LOG(owner + preCompName, false);
            return false;
        }
        return true;
    }

    // オーナーオブジェクトの弱参照用
    std::weak_ptr<GameObject> m_wpOwnerObj;

private:

    // コンポーネントの更新を行うかどうか
    bool m_isEnable = true;
    // Start()が呼ばれたかどうか
    bool m_isStart = false;
    // シリアライズ / デシリアライズを行うかどうか
    // - コンポーネント内で行われる AddComponent される場合は false にしてシリアライズを行わない
    bool m_enableSerialize = false;

    // コンポーネントの名前
    std::string m_compName = "";
    // コンポーネントの更新順
    ComponentType m_updateOrder = ComponentType::eDefault;
};
