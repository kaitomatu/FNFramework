#pragma once

#include "../../BaseComponent.h"

/**
* @class TitleScript
* @brief タイトルから
* @details
*
*/
class TitleScript
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
    TitleScript(const std::shared_ptr<GameObject>& _owner, const std::string& _name, bool _enableSerialize)
        : BaseComponent(_owner, _name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~TitleScript() override
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
    void Update() override;

    /* @fn void Release() @brief 終了 */
    void Release() override;

    /* @brief 保存 / 読みこみ */
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn void ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    bool m_isGameStart = false;

    std::string m_firstStageName = "Stage1";
};


// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace TitleScript
    {
        constexpr std::string_view FirstStageName = "StartSceneName";
    }
}
