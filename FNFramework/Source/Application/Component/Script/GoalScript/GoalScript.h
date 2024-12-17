#pragma once

#include "../../BaseComponent.h"

class StageScript;
class ResultState;
/**
* @class GoalScript
* @brief ステージの切り替えやステージの読み込みを行うクラス
* @details
*  - json ファイルに保存された StageList の n 番目を読み込む
*/
class GoalScript
    : public BaseComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] _owner - オーナーオブジェクトのポインタ
    * @param[in] _name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズするかどうか
    */
    GoalScript(const std::shared_ptr<GameObject>& _owner, const std::string& _name, bool _enableSerialize)
        : BaseComponent(_owner, _name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~GoalScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // ステージのコントローラーの取得
    utl::StateMachine<GoalScript>& GetResultController() { return m_resultController; }

    // リザルト状態かどうか
    bool IsResult() const { return m_isResult; }
    void SetResultFlg(bool _isResult) { m_isResult = _isResult; }

    // 追跡するオブジェクトの名前
    const std::string& GetTargetObjectName() const { return m_targetObjectName; }
    void SetTargetObjectName(const std::string& _name) { m_targetObjectName = _name; }

    const std::weak_ptr<StageScript>& GetStageScript() const { return m_wpStageScript; }

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

    /* @brief 保存 / 読みこみ */
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    // ゴールに到達したかどうかを判定する
    void CheckGoalTouch();

    // プレイヤーのインスタンス : ゴールに到達したかどうかを判定する
    std::weak_ptr<GameObject> m_wpPlayer;
    std::string m_targetObjectName = "Player";

    // todo : ↓
    /**'*' は後々実装するかも
     x--- ResultState ---x
     * 0. 現在のステージのの進捗度保存
     * 1. リザルト画面の表示
     * 2. 次のステージの読み込み & シーンへの追加
     *
     x--- LoadState ---x
     * 0. 遷移処理開始 *
     * 1. ステージの読み込み
     * 2. ステージの初期化
     * 3. ステージの切り替え
     * 4. 遷移処理終了 *
     */
    utl::StateMachine<GoalScript> m_resultController;

    // リザルト状態かどうか
    bool m_isResult = false;

    // ゴールが有効になる距離
    float m_goalActivationDistance = 10.0f;

    // 子どもの数カウント用
    std::weak_ptr<StageScript> m_wpStageScript;
};

namespace jsonKey::Comp
{
    namespace GoalScript
    {
        constexpr std::string_view TargetObjectName = "TargetObjectName";
        constexpr std::string_view GoalActivationDistance = "GoalActivationDistance";
    }
}
