#pragma once

#include "../../BaseComponent.h"

class StageScript;
class PlayerScript;
class SpriteComponent;

/**
* @class GameUIScript
* @brief ゲームUIの制御を行うクラス
*/
class GameUIScript
    : public BaseComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    */
    GameUIScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~GameUIScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

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
    void PreUpdate() override;
    void Update() override;

    /* @fn Release() @brief 終了 */
    void Release() override;

    /* @brief 保存 / 読みこみ */
    void Serialize(Json& _json) const override;

    void Deserialize(const Json& _json) override;

    //--------------------------------
    // その他関数
    //--------------------------------

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    bool m_isFirstUpdate = true;    // 最初の更新かどうか
    /* @fn void ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    // 0 - 9 の数字テクスチャ
    std::array<std::shared_ptr<ShaderResourceTexture>, 10> m_spNumberTextures;

    std::string m_stageScriptObjectName = "";       // ステージスクリプト名
    std::weak_ptr<StageScript> m_wpStageScript;     // ステージスクリプト

    std::string m_playerScriptObjectName = "";      // プレイヤースクリプト名
    std::weak_ptr<PlayerScript> m_wpPlayerScript;   // プレイヤースクリプト

    std::string m_timeUIObjectName = "";        // 時間UIオブジェクト名
    // 時間(〇 : 〇〇)を操作するためのコンポーネント
    std::vector<std::weak_ptr<SpriteComponent>> m_wpTimeUIObjHasSpriteComps;

    std::string m_scoreUIObjectName = "";       // スコアUIオブジェクト名
    // スコア(〇/〇)を操作するためのコンポーネント
    std::weak_ptr<SpriteComponent> m_wpScoreUIHasSpriteComp;
};

// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace GameUIScript
    {
        constexpr std::string_view StageScriptObjectName = "StageScriptObjectName";
        constexpr std::string_view PlayerScriptObjectName = "PlayerScriptObjectName";
        constexpr std::string_view TimeUIObjectName = "TimeUIObjectName";
        constexpr std::string_view ScoreUIObjectName = "ScoreUIObjectName";
    }
}
