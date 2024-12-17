#pragma once
#pragma once

#include "../../BaseComponent.h"

/**
* @class SeaweedManageScript
* @brief 海藻モデルの配置などを制御するコンポーネント
* - 外壁の当たり判定も行う
*
*/
class SeaweedManageScript
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
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    SeaweedManageScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize)
    {
    }

    ~SeaweedManageScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    //--------------------------------
    // その他関数
    //--------------------------------
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

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

    void Release() override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;
};

// Jsonで利用するキー
namespace jsonKey::Comp
{
    namespace SeaweedManageScript
    {
    }
}
