#pragma once

#include "../../BaseComponent.h"

/**
* @class StageScript
* @brief
*  - ステージの初期化
*  - 移動オブジェクトの座標設定などを行う
*/
class StageScript
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
    StageScript(const std::shared_ptr<GameObject>& _owner, const std::string& _name, bool _enableSerialize)
        : BaseComponent(_owner, _name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~StageScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // ステージの子クラゲ最大数
    int GetMaxCollectiveCount() const { return m_maxCollectiveCount; }
    // ステージの目標クリアタイム
    int GetClearTargetTime() const { return m_clearTargetSecTime; }
    // 時間の取得
    int GetTimeFromSeconds() const { return m_stageTimer.Elapsed<Timer::Seconds>().count(); }

    // 次のステージの名前
    const std::string& GetNextStageName() const { return m_nextStageName; }

    //--------------------------------
    // その他関数
    //--------------------------------
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
    void Serialize(Json& /*_json*/) const override;

    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    void RegisterStageObjectPosForImGui();

    int m_maxCollectiveCount = 0;

    // ステージのクリアタイムを計測する
    Timer m_stageTimer;
    int m_clearTargetSecTime = 300;

    // 次のステージの名前(これが設定されていない = タイトルへ)
    std::string m_nextStageName;

    struct EditData
    {
        /* 設置するオブジェクトの名前 */
        std::string ObjectName;
        /* 登録する座標 : シーンが初期化されたときにここの座標にオブジェクトを設置する */
        Math::Vector3 Pos;

        /* デバッグ表示するかどうか */
        bool IsDebug = false;
        /* デバッグ表示に利用するオブジェクト */
        std::weak_ptr<GameObject> Object;
    };
    std::map</* ObjectName */std::string, EditData> m_registerStageObjectPos;
    std::string m_stageObjectName;

};

// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace StageScript
    {
        constexpr std::string_view StageObjectList = "StageObjectList";
        constexpr std::string_view ObjectName = "ObjectName";
        constexpr std::string_view Pos = "Pos";

        constexpr std::string_view MaxCollectiveCount = "MaxCollectiveCount";

        constexpr std::string_view NextStageName = "NextStageName";
    }
}
