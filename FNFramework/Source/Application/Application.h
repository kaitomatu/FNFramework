#pragma once

class Application
    : public utl::Singleton<Application>
{
    friend class utl::Singleton<Application>;

public:
    /* @brief アプリケーション終了 */
    void End() { m_endFlg = true; }

    /* @brief アプリケーション実行 */
    void Execute();

    /* @brief ウィンドウハンドル取得 @result ウィンドウハンドル */
    const HWND& GetWindowHandle() const { return m_window.GetWndHandle(); }

    /* @brief ウィンドクラス取得 @result ウィンドウクラス */
    const Window& GetWindowClass() const { return m_window; }
    Window& WorkWindowClass() { return m_window; }

    /* @brief FPSコントローラー取得 @result FPSコントローラー */
    const std::unique_ptr<FPSController>& GetFPSController() const { return m_fpsController; }

private:
    Application()
    {
    }

    ~Application() override
    {
    }

    Window m_window; // ウィンドウ本体
    std::unique_ptr<FPSController> m_fpsController = nullptr; // FPSコントローラー

    bool m_endFlg = false; // ウィンドウ破棄フラグ

    /* .dllのディレクトリのセットとロードを行う */
    void SetDirectoryAndLoadDll();

    /* @brief アプリケーション初期化  @result 初期化成功したらtrue */
    bool Initialize();

    /* @brief 更新前準備 */
    void PreUpdate();
    /* @brief 更新処理 */
    void Update();
    /* @brief 更新後処理 */
    void PostUpdate();

    /* @brief 描画処理 */
    void Draw();
    /* @brief 描画前処理 */
    void PostDraw();

    /* @brief アプリケーション解放 */
    void Release();
};
