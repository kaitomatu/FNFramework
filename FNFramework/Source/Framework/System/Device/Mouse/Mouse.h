#pragma once

/**
* @class Mouse
* @brief マウスクラス
*/
class Mouse
{
    friend class Window;

public:
    // マウス関係の情報
    struct MouseInfo
    {
        POINT MousePos = {0, 0}; // マウス座標
        int MouseWheelVal = 0; // マウスホイールの回転量
        bool ShowFlg = true; // マウスカーソル表示フラグ
    };

    //---------------------------------
    // ゲッター / セッター
    //---------------------------------
    // マウスの情報取得
    const MouseInfo& GetInfo() const { return m_mouseInfo; }

    const POINT& GetPos() const { return m_mouseInfo.MousePos; }
    const int& GetWheelVal() const { return m_mouseInfo.MouseWheelVal; }
    const bool& GetShowFlg() const { return m_mouseInfo.ShowFlg; }

    /**
     * @brief マウスカーソルの表示・非表示
     */
    void ShowCursor(bool showFlg)
    {
        // 現在の状態と同じならなにもしない
        if (m_mouseInfo.ShowFlg == showFlg)
        {
            m_showcursorCount = 0;
            return;
        }

        m_mouseInfo.ShowFlg = showFlg;

        ::ShowCursor(showFlg);

        m_showcursorCount++;
    }

private:
    //---------------------------------
    // その他の関数
    //---------------------------------
    /* @brief 更新処理 @param[in] mouseWheelVal - マウスホイールの移動量 */
    void Update(int mouseWheelVal)
    {
        auto& mi = m_mouseInfo;
        mi.MouseWheelVal = mouseWheelVal;
        GetCursorPos(&mi.MousePos);
    }

    // マウス情報
    MouseInfo m_mouseInfo;

    int m_showcursorCount = 0;
};
