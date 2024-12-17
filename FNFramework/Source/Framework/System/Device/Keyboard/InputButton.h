#pragma once


class InputButton
{
public:
    // キーの状態
    enum class ButtonState
    {
        eNone, // 何もしていない
        ePressed, // 押されたフレーム
        eHold, // 押し続けられている
        eReleased, // 離されたフレーム
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    InputButton() { Init(); }
    ~InputButton() { Release(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    ButtonState GetState() const { return m_state; }

    /**
    * @brief 現在設定されているキーコードを引数に格納する
    * @param[out] keyCodeList キーコードのリスト
    */
    void GetKeyCode(std::list<int>& retKeyCode);

    void ClearKeyCode() { m_keyCodeList.clear(); }

    void AddKeyCode(int _keyCode);
    void AddKeyCode(const std::initializer_list<int>& _keyCode);
    void AddKeyCode(const std::vector<int>& _keyCode);

    //--------------------------------
    // その他関数
    //--------------------------------
    void Init();
    void Release();

    void Update();

private:
    ButtonState m_state = ButtonState::eNone;
    std::list<int> m_keyCodeList;
};
