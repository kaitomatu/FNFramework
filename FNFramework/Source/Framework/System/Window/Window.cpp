#include "Window.h"

#include <commdlg.h>
#include <combaseapi.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    // メッセージごとに処理を選択
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0); // OSに対して終了命令を行う
        break;

    default:
        DefWindowProc(hWnd, msg, wParam, lParam); // メッセージの処理を行う
        return TRUE;
    }
    return 0;
}

LRESULT CALLBACK Window::WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* pThis = nullptr;

    if (msg == WM_NCCREATE)
    {
        // CreateWindow()から渡されたCreateStructポインタを取得
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<Window*>(pCreate->lpCreateParams);

        // GWLP_USERDATAにthisポインタを保存
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

        // ウィンドウハンドルをインスタンス変数に保存
        pThis->m_hWnd = hWnd;
    }
    else
    {
        // 以降のメッセージでthisポインタを取得
        pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->ActualWindowProcedure(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::ActualWindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // ImGuiのウィンドウプロシージャを呼ぶ
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return 1; // LRESULT 型の値を返す
    }

    // メッセージごとに処理を選択
    switch (msg)
    {
    case WM_MOUSEWHEEL:
        m_mouseWheelVal = GET_WHEEL_DELTA_WPARAM(wParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0); // OSに対して終了命令を行う
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

bool Window::ProcessMessage()
{
    m_mouseWheelVal = 0;

    // メッセージ取得
    MSG msg;

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        // 終了メッセージがきたら
        if (msg.message == WM_QUIT)
        {
            return false;
        }

        // メッセージを処理
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // マウス情報更新
    m_mouse.Update(m_mouseWheelVal);

    return true;
}

bool Window::Create(int clientWidth, int clientHeight, const std::wstring& titleName,
                    const std::wstring& windowClassName)
{
    // 外部からウィンドウ情報を取得できるようにする
    m_windowInfo = WindowInfo(clientWidth, clientHeight);

    // インスタンスハンドル取得
    HINSTANCE hInst = GetModuleHandle(nullptr);

    //===================================
    // メインウィンドウ作成
    //===================================

    // ウィンドウクラスの定義
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX); // 構造体のサイズ
    wc.lpfnWndProc = Window::WindowProcedure; // ウィンドウプロシージャをクラスの関数に設定
    wc.lpszClassName = windowClassName.c_str(); // ウィンドウクラス名
    wc.hInstance = hInst; // インスタンスハンドル

    // ウィンドウクラスの登録
    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    m_hWnd = CreateWindow(
        windowClassName.c_str(), // ウィンドウクラス名
        titleName.c_str(), // ウィンドウのタイトル
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, // ウィンドウタイプを標準タイプに
        0, // ウィンドウの位置 ( X座標 )
        0, // ウィンドウの位置 ( Y座標 )
        clientWidth, // ウィンドウの幅
        clientHeight, // ウィンドウの高さ
        nullptr, // 親ウィンドウのハンドル
        nullptr, // メニューのハンドル
        hInst, // インスタンスハンドル
        this //
    );

    if (m_hWnd == nullptr)
    {
        return false;
    }

    // クライアントのサイズを設定
    SetClientSize(clientWidth, clientHeight);

    // ウィンドウの表示
    ShowWindow(m_hWnd, SW_SHOW);

    // ウィンドウの更新
    UpdateWindow(m_hWnd);

    // 分解能を上げる
    timeBeginPeriod(1); // 1msに設定

    m_mouse.ShowCursor(false);

    return true;
}

void Window::SetClientSize(int w, int h)
{
    RECT rcWnd, rcCli;

    GetWindowRect(m_hWnd, &rcWnd); // ウィンドウのRECT取得
    GetClientRect(m_hWnd, &rcCli); // クライアント領域のRECT取得

    // ウィンドウの余白を考えて、クライアントのサイズを指定サイズにする。
    MoveWindow(m_hWnd,
               rcWnd.left, // X座標
               rcWnd.top, // Y座標
               w + (rcWnd.right - rcWnd.left) - (rcCli.right - rcCli.left),
               h + (rcWnd.bottom - rcWnd.top) - (rcCli.bottom - rcCli.top),
               TRUE);

    m_windowInfo.ClientHeight = h;
    m_windowInfo.ClientWidth = w;
}

bool Window::OpenFileDialog(std::string& filepath, std::string_view title, const char* filters)
{
    // 現在のカレントディレクトリ保存
    auto current = std::filesystem::current_path();
    // ファイル名のみ
    auto filename = std::filesystem::path(filepath).filename();

    // 結果用
    static char fname[1000];
    strcpy_s(fname, sizeof(fname), filename.string().c_str());

    // デフォルトフォルダ
    std::string dir;
    if (filepath.size() == 0)
    {
        dir = current.string() + "\\";
    }
    else
    {
        auto path = std::filesystem::absolute(filepath);
        dir = path.parent_path().string() + "\\";
    }

    OPENFILENAMEA o;
    ZeroMemory(&o, sizeof(o));

    o.lStructSize = sizeof(o); // 構造体サイズ
    o.hwndOwner = nullptr; // 親ウィンドウのハンドル
    o.lpstrInitialDir = dir.c_str(); // 初期フォルダー
    o.lpstrFile = fname; // 取得したファイル名を保存するバッファ
    o.nMaxFile = sizeof(fname); // 取得したファイル名を保存するバッファサイズ
    o.lpstrFilter = filters; // (例) "TXTファイル(*.TXT)\0*.TXT\0全てのファイル(*.*)\0*.*\0";
    o.lpstrDefExt = "";
    o.lpstrTitle = title.data();
    o.nFilterIndex = 1;
    if (GetOpenFileNameA(&o))
    {
        // カレントディレクトリを元に戻す
        std::filesystem::current_path(current);
        // 相対パスへ変換
        filepath = std::filesystem::relative(fname).string();
        return true;
    }
    std::filesystem::current_path(current); // カレントディレクトリを元に戻す
    return false;
}

bool Window::SaveFileDialog(std::string& filepath, std::string_view title, const char* filters, std::string_view defExt)
{
    // 現在のカレントディレクトリ保存
    auto current = std::filesystem::current_path();
    // ファイル名のみ
    auto filename = std::filesystem::path(filepath).filename();

    // 結果用
    static char fname[1000];
    strcpy_s(fname, sizeof(fname), filename.string().c_str());

    // デフォルトフォルダ
    std::string dir;
    if (filepath.size() == 0)
    {
        dir = current.string() + "\\";
    }
    else
    {
        auto path = std::filesystem::absolute(filepath);
        dir = path.parent_path().string() + "\\";
    }

    OPENFILENAMEA o;
    ZeroMemory(&o, sizeof(o));

    o.lStructSize = sizeof(o); // 構造体サイズ
    o.hwndOwner = nullptr; // 親ウィンドウのハンドル
    o.lpstrInitialDir = dir.c_str(); // 初期フォルダー
    o.lpstrFile = fname; // 取得したファイル名を保存するバッファ
    o.nMaxFile = sizeof(fname); // 取得したファイル名を保存するバッファサイズ
    o.lpstrFilter = filters; // (例) "TXTファイル(*.TXT)\0*.TXT\0全てのファイル(*.*)\0*.*\0";
    o.lpstrDefExt = defExt.data();
    o.lpstrTitle = title.data();
    o.nFilterIndex = 1;
    o.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameA(&o))
    {
        // カレントディレクトリを元に戻す
        std::filesystem::current_path(current);
        // 相対パスへ変換
        filepath = std::filesystem::relative(fname).string();
        return true;
    }
    std::filesystem::current_path(current); // カレントディレクトリを元に戻す
    return false;
}

bool Window::WarpMouseToInScreen(const HWND& hwnd, int& mouseWarpX, int& mouseWarpY)
{
    bool isMouseWrapped = false;

    if (!hwnd) { return isMouseWrapped; }

    // ウィンドウのクライアント領域を取得
    RECT rect;
    GetClientRect(hwnd, &rect);

    // クライアント領域をスクリーン座標に変換
    POINT topLeft = {rect.left, rect.top};
    POINT bottomRight = {rect.right, rect.bottom};
    ClientToScreen(hwnd, &topLeft);
    ClientToScreen(hwnd, &bottomRight);

    // 現在のマウスの座標を取得
    POINT pt;
    GetCursorPos(&pt);

    // マウスがウィンドウ外に出たら反対側に座標をリセット
    if (pt.x > bottomRight.x)
    {
        isMouseWrapped = true;
        mouseWarpX = topLeft.x - pt.x;  // 右端から左端へワープ
        pt.x = topLeft.x;
    }
    else if (pt.x < topLeft.x)
    {
        isMouseWrapped = true;
        mouseWarpX = bottomRight.x - pt.x;  // 左端から右端へワープ
        pt.x = bottomRight.x;
    }

    if (pt.y > bottomRight.y)
    {
        isMouseWrapped = true;
        mouseWarpY = topLeft.y - pt.y;  // 下端から上端へワープ
        pt.y = topLeft.y;
    }
    else if (pt.y < topLeft.y)
    {
        isMouseWrapped = true;
        mouseWarpY = bottomRight.y - pt.y;  // 上端から下端へワープ
        pt.y = bottomRight.y;
    }

    // マウスの座標を設定
    SetCursorPos(pt.x, pt.y);   // 新しい座標にマウスを移動

    return isMouseWrapped;
}
