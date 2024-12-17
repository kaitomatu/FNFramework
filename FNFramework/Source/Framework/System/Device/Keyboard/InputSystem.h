#pragma once

#include "InputButton.h"

/**
* @class InputSystem
* @brief 入力管理クラス
* @details
*	フレームの一番初めにUpdate()を呼びだす必要がある
*/
class InputSystem
    : public utl::Singleton<InputSystem>
{
    friend class utl::Singleton<InputSystem>;

public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    ~InputSystem() override { Release(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    InputButton::ButtonState GetButtonState(std::string_view key);

    // 通常追加
    void AddButton(std::string_view key, InputButton button);
    // 上書き追加
    void AddButton(std::string_view key, std::shared_ptr<InputButton> button);

    //--------------------------------
    // その他関数
    //--------------------------------
    bool IsPressed(std::string_view key);
    bool IsFree(std::string_view key);
    bool IsHold(std::string_view key);
    bool IsRelease(std::string_view key);

    void Release();

    // 入力処理なのでフレームの1番最初に呼びだす必要がある
    void Update();

private:
    // std::unordered_map< ButtonName, Button >
    std::unordered_map<std::string, std::shared_ptr<InputButton>> m_inputButton;
};
