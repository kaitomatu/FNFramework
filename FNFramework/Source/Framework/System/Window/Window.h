#pragma once

class Window
{
public:
    // ウィンドウ関係の情報
    struct WindowInfo
    {
        WindowInfo()
        {
        }

        WindowInfo(int clientWidth, int clientHeight)
            : ClientWidth(clientWidth), ClientHeight(clientHeight)
        {
        }

        int ClientWidth = 0; // ウィンドウの横幅
        int ClientHeight = 0; // ウィンドウの縦幅
    };

    //===================================
    // 初期化
    //===================================

    // ウィンドウプロシージャ - ウィンドウメッセージを処理する関数
    static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT ActualWindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /**
    * @brief ウィンドウを作成します。
    *
    * @param clientWidth - Windowの横幅
    * @param clientHeight - Windowの縦幅
    * @param titleName - タイトル名
    * @param windowClassName - ウィンドウクラス名
    * @return ウィンドウ作成が成功した場合はtrue、それ以外はfalse
    */
    bool Create(int clientWidth, int clientHeight, const std::wstring& titleName, const std::wstring& windowClassName);

    //===================================
    // 処理
    //===================================

    /**
    * @brief ウィンドウメッセージ処理
    * @return 終了メッセージが来た場合false
    */
    bool ProcessMessage();

    /**
    * @brief クライアント領域のセット
    * @param[in] w - ウィンドウの横幅
    * @param[in] h - ウィンドウの縦幅
    */
    void SetClientSize(int w, int h);

    /**
     * @fn static bool OpenFileDialog(std::string& filepath, std::string_view title = "ファイルを開く", const char* filters = "全てのファイル\0*.*\0")
     * @brief ファイルを開くダイアログを表示
     * @param filepath : 選択したファイルのパス
     * @param title : タイトル
     * @param filters : フィルター
     * @return 選択したファイルのパス
     */
    static bool OpenFileDialog(std::string& filepath, std::string_view title = "ファイルを開く", const char* filters = "全てのファイル\0*.*\0");

    /**
     * @fn static bool SaveFileDialog(std::string& filepath, std::string_view title = "ファイルを保存", const char* filters = "全てのファイル\0*.*\0", std::string_view defExt = "")
     * @brief ファイルを保存ダイアログを表示
     * @param filepath : 選択したファイルパス
     * @param title : タイトル
     * @param filters : フィルター
     * @param defExt: デフォルトの拡張子
     * @return 選択したファイルのパス
     */
    static bool SaveFileDialog(std::string& filepath, std::string_view title = "ファイルを保存", const char* filters = "全てのファイル\0*.*\0", std::string_view defExt = "");

    /**
     * @fn static void WarpMouseToInScreen()
     * @brief マウスカーソルが画面外に行ったときに画面内に移動する
     * @param hwnd : ウィンドウハンドル
     * @param mouseWarpX : ワープしたときのマウスのX座標の補正値 - $ScreenEdge$ -> 0 = ワープした距離
     * @param mouseWarpY : ワープしたときのマウスのY座標の補正値 - $ScreenEdge$ -> 0 = ワープした距離
     * @return ワープしたらtrue
     */
    static bool WarpMouseToInScreen(const HWND& hwnd, int& mouseWarpX, int& mouseWarpY);

    //===================================
    // 取得・設定
    //===================================
    /**
    * @brief ウィンドウハンドル取得
    * @result ウィンドウハンドル
    */
    const HWND& GetWndHandle() const { return m_hWnd; }
    /**
    * @brief ウィンドウが作成されているか？
    * @result ウィンドウが作成されていればtrue
    */
    bool IsCreated() const { return m_hWnd ? true : false; }
    /**
    * @brief ウィンドウ情報取得
    * @result ウィンドウ情報
    */
    const WindowInfo& GetWindowInfo() const { return m_windowInfo; }

    int Width() const { return m_windowInfo.ClientWidth; }
    int Height() const { return m_windowInfo.ClientHeight; }

    /**
    * @brief マウス情報取得
    * @result マウス情報
    */
    const Mouse& GetMouse() const { return m_mouse; }
    Mouse& WorkMouse() { return m_mouse; }

private:
    WindowInfo m_windowInfo; // ウィンドウ情報

    int m_mouseWheelVal = 0; // マウスホイールの移動量
    Mouse m_mouse; // マウス

    HWND m_hWnd = {}; // ウィンドウハンドル
};
