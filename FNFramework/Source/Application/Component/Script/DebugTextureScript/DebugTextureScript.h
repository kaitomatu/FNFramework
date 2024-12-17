#pragma once

#include "../../BaseComponent.h"

/**
* @class DebugTextureScript
* @brief デバッグ用のテクスチャを描画するスクリプト
*/
class DebugTextureScript
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
    DebugTextureScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    //--------------------------------
    // その他関数
    //--------------------------------
    void Start() override;

private:

};

