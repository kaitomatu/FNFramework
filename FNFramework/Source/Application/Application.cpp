#include "Application.h"
#include "Framework/System/Device/Keyboard/InputSystem.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // メモリリーク検知
    _CrtSetBreakAlloc(0x00000032F36FE210);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED); // COM初期化

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("COM初期化失敗");
        return -1;
    }

    //====================================
    // アプリケーション更新処理
    //====================================
    Application::Instance().Execute();

    CoUninitialize(); // COM解放

    return 0;
}

bool Application::Initialize()
{
    //------------------
    // ウィンドウ作成
    //------------------
    if (!m_window.Create(Screen::Width, Screen::Height, L"FrameworkDX12", L"Window"))
    {
        FNENG_ASSERT_ERROR("ウィンドウ作成失敗。");
        return false;
    }

    //------------------
    // Direct3D12デバイス作成
    //------------------
    if (!GraphicsDevice::Instance().Init(m_window.GetWndHandle(), Screen::Width, Screen::Height))
    {
        FNENG_ASSERT_ERROR("グラフィックスデバイス初期化失敗");
        return false;
    }

    //------------------
    // AudioDevice作成
    //------------------
    if (!AudioDevice::Instance().Create())
    {
        FNENG_ASSERT_ERROR("AudioDevice作成失敗");
        return false;
    }

    //======================================
    // Applicationで利用するデータの初期化
    //======================================
    //------------------
    // ImGui初期化
    //------------------
    ImGuiDevice::Instance().Init();

    //------------------
    // ShaderManager
    //------------------
    ShaderManager::Instance().Init();

    //------------------
    // SceneManager
    //------------------
    SceneManager::Instance().Init();

    //------------------
    // FPSController
    //------------------
    if (!m_fpsController)
    {
        m_fpsController = std::make_unique<FPSController>();
    }

    return true;
}

void Application::Release()
{
    // ImGui解放
    ImGuiDevice::Instance().Release();
    
}

void Application::Execute()
{
    //SetDirectoryAndLoadDll();

    //------------------
    // アプリケーション初期化
    //------------------
    if (!/* Application::Instance(). */Initialize())
    {
        FNENG_ASSERT_ERROR("アプリケーション初期化失敗");
        return;
    }

    // FPS表示用
    std::string fpsStr = "FPS : ";

    //*================================*//
    // 	  MainLoooooooooop			    //
    //*================================*//
    while (true)
    {
        // FPSをウィンドウタイトルバーに表示
        fpsStr = "FPS : " + std::to_string(m_fpsController->GetCurrentFPS());
        SetWindowTextA(m_window.GetWndHandle(), std::string("Pikurage   " + fpsStr).c_str());

        if (GetAsyncKeyState(VK_ESCAPE)) { End(); }

        // ! ウィンドウ終了時のここで例外スロー発生中: 
        if (!m_window.ProcessMessage()
            || m_endFlg)
        {
            break;
        }

        // ウィンドウが有効状態じゃなかったらオブジェクトの更新を行わない
        //if(GetForegroundWindow() == GetWindowHandle())
        {
            // 更新処理
            PreUpdate();
            Update();
            PostUpdate();

            Draw();
            PostDraw();
        }

        //===============================================
        // FPSController
        //===============================================
        m_fpsController->UpdateFPS();
    }

    //=====================//
    // 解放処理
    //=====================//
    Release();
}

void Application::SetDirectoryAndLoadDll()
{
//#ifdef _DEBUG
//	SetDllDirectoryA("Library/assimp/build/lib/Debug");
//	LoadLibraryExA("assimp-vc143-mtd.dll", NULL, NULL);
//#else
//    SetDllDirectoryA("Library/assimp/build/lib/Release");
//    LoadLibraryExA("assimp-vc143-mt.dll", nullptr, NULL);
//#endif // _DEBUG
}

/*================================*/
// 更新前準備
/*================================*/
void Application::PreUpdate()
{
    // ImGui
    ImGuiDevice::Instance().NewFrame();

    // Scene
    SceneManager::Instance().PreUpdate();
}

/*================================*/
// 更新
/*================================*/
void Application::Update()
{
    // Input
    InputSystem::Instance().Update();

    // Scene
    SceneManager::Instance().Update();
}

/*================================*/
// 更新後処理
/*================================*/
void Application::PostUpdate()
{
    // Scene
    SceneManager::Instance().PostUpdate();
}

/*================================*/
// 描画
/*================================*/
void Application::Draw()
{
    GraphicsDevice::Instance().Prepare();
    GraphicsDevice::Instance().GetCBVSRVUAVHeap()->SetHeap();

    // フレームのはじめは強制的に定数バッファの使用数0にする
    GraphicsDevice::Instance().GetCBufferAllocater()->ResetCurrentUseNumber();

    // Scene
    Renderer::Instance().Render();
}

/*================================*/
// 描画後処理準備
/*================================*/
void Application::PostDraw()
{

    ImGuiDevice::Instance().SetHeap();

    GraphicsDevice::Instance().ScreenFlip();
}
