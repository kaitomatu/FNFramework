#pragma once

#include "../../BaseComponent.h"

class KurageMoveScript;
enum class KurageState;

/**
* @class PlayerScript
* @brief プレイヤーオブジェクトが持つスクリプト
* @details
*   - 子どもの管理やイベントの管理を行う
*/
class PlayerScript
    : public BaseComponent
{
public:

    struct ChildData
    {
        // bool IsChild = false;
        std::weak_ptr<GameObject> wpChildObj;
        std::weak_ptr<KurageMoveScript> wpKurageMoveScript;
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    PlayerScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // イベントフラグの設定 / 取得
    bool GetIsEventActive() const { return m_isEventActive; }
    void SetIsEventActive(bool _isEventActive) { m_isEventActive = _isEventActive; }

    // 子どものカウントの設定 / 取得
    void AddChild(const ChildData& childObj);
    void RemoveChild(const std::shared_ptr<GameObject>& childObj);

    int GetChildCount() const { return static_cast<int>(m_childDataList.size()); }

    const std::list<ChildData>& GetChildList() const { return m_childDataList; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @fn void Awake()
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;

    /**
    * @fn void Start()
    * @brief Awakeを経て初期化された後、1度だけ呼びだされる
    * @details
    *	Awakeの後に呼び出される
    *	他のコンポーネントとの依存関係にある初期化処理や
    *	Awakeの段階ではできない初期化を行う
    */
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;

    /* @fn Release() @brief 終了 */
    void Release() override;

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    bool m_isEventActive = false;

    std::list<ChildData> m_childDataList;    // 子どものリスト

    std::weak_ptr<KurageMoveScript> m_wpKurageMoveScript;    // クラゲの移動スクリプト
};

namespace jsonKey::Comp
{
    namespace PlayerScript
    {
    }
}
