#include "ImGuiUpdate.h"

#include "Application/Application.h"
#include "Framework/Manager/Shader/AmbientManager.h"

void ImGuiUpdate::Init()
{
}

void ImGuiUpdate::Update()
{
    if (InputSystem::Instance().IsPressed("ImGui"))
    {
        m_isUpdate = !m_isUpdate;

        Application::Instance().WorkWindowClass().WorkMouse().ShowCursor(m_isUpdate);
    }

    if (!m_isUpdate) { return; }

    if (ImGui::Begin("Application")) // ApplicationWindow : Start
    {
        ImGui::SetWindowPos({ 880, 200 }, ImGuiCond_Once);
        ImGui::SetWindowSize({ 400, 500 }, ImGuiCond_Once);

        bool enableDebugwire = SceneManager::Instance().GetDebugWire()->IsEnable();
        if (ImGui::Checkbox("EnableDebugWire", &enableDebugwire))
        {
            SceneManager::Instance().GetDebugWire()->EnableDebugWire(enableDebugwire);
        }

        if (ImGui::BeginTabBar("ImGuiUpdateTabBar")) // ImGuiUpdateTabBar : Start
        {
            /**
            * アプリケーションの情報を表示するタブ
            */
            if (ImGui::BeginTabItem("ApplicationInfo"))
            {
                // アプリケーションの情報を表示するタブ
                ApplicationInfoGUI();
                ImGui::EndTabItem();
            }

            // 現在のシーン内部の情報を表示するタブ
            /**
            * 現在のシーン内部の情報を表示するタブ
            */
            if (ImGui::BeginTabItem("CurrentScene")) // CurrentSceneTab : Start
            {
                CurrentSceneGUI();
                ImGui::EndTabItem(); // CurrentSceneTab : End
            }

            ImGui::EndTabBar(); // ImGuiUpdateTabBars : End
        }
    }
    ImGui::End(); // ApplicationWindow : End
}

//==================================
// ApplicationTab
//==================================
void ImGuiUpdate::ApplicationInfoGUI()
{
    // FPSInfo
    {
        int flags;

        flags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow;

        if (ImGui::TreeNodeEx(&flags, flags, "FPSController"))
        {
            FPSControllerGUI();
            ImGui::TreePop();
        }
    }

    ImGui::Separator();

    // Window
    {
        int flags;

        flags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow;

        if (ImGui::TreeNodeEx(&flags, flags, "Window"))
        {
            WindowGUI();
            ImGui::TreePop();
        }
    }

    // AmbientController
    {
        int flags;

        flags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow;

        if (ImGui::TreeNodeEx(&flags, flags, "AmbientController"))
        {
            AmbientControllerGUI();
            ImGui::TreePop();
        }
    }
}

void ImGuiUpdate::FPSControllerGUI()
{
    // FPSController の描画コード
    const std::unique_ptr<FPSController>& fpsController =
        Application::Instance().GetFPSController();

    ImGui::Text(U8_TEXT("目標のFPS"));
    float targetFPS = static_cast<float>(fpsController->GetTargetFPS());
    if (ImGui::DragFloat("##TargetFPS", &targetFPS, 1.0f, 1.0f, 300.0f))
    {
        fpsController->SetTargetFPS(static_cast<double>(targetFPS));
    }

    ImGui::Text(U8_TEXT("現在のFPS : %d"), fpsController->GetCurrentFPS());
    ImGui::Text(U8_TEXT("デルタタイム : %f"), SceneManager::Instance().FrameDeltaTime());
}

void ImGuiUpdate::WindowGUI()
{
    const Window& window = Application::Instance().GetWindowClass();

    // ウィンドウ情報表示
    ImGui::Text(U8_TEXT("ウィンドウの情報"));
    ImGui::Text("WindowWidth : %d", window.Width());
    ImGui::Text("WindowHeight : %d", window.Height());

    // マウス情報表示
    Mouse::MouseInfo mouseInfo = window.GetMouse().GetInfo();
    ImGui::Text(U8_TEXT("マウスの情報"));
    ImGui::Text("MousePos[X : %d][Y : %d]", mouseInfo.MousePos.x, mouseInfo.MousePos.y);
    ImGui::Text("IMGuiMousePos[X : %d][Y : %d]", mouseInfo.MousePos.x, mouseInfo.MousePos.y);
    ImGui::Text("MouseWheelVal %d", mouseInfo.MouseWheelVal);
}

void ImGuiUpdate::AmbientControllerGUI()
{

    //-----------------------
    // ポストエフェクト関連
    //-----------------------
    float blurPower = ShaderManager::Instance().GetBloomShader()->GetGaussianBlur().GetBlurPower();
    ImGui::Text(U8_TEXT("ブラーの強度"));
    if (ImGui::DragFloat("##BlurPower", &blurPower, 0.01f, 0.0f, 100.0f))
    {
        ShaderManager::Instance().WorkBloomShader()->WorkGaussianBlur().UpdateWeights(blurPower);
    }
    float bloomPower = ShaderManager::Instance().GetBloomShader()->GetBloomIntensity();
    ImGui::Text(U8_TEXT("ブルームの強度"));
    if (ImGui::DragFloat("##BloomPower", &bloomPower, 0.01f, 0.0f, 100.0f))
    {
        ShaderManager::Instance().GetBloomShader()->SetBloomIntensity(bloomPower);
    }

    // 露出度の調整
    float exposure = ShaderManager::Instance().GetBloomShader()->GetExposure();
    ImGui::Text(U8_TEXT("露出度"));
    if (ImGui::DragFloat("##Exposure", &exposure, 0.01f, 0.0f, 100.0f))
    {
        ShaderManager::Instance().GetBloomShader()->SetExposure(exposure);
    }

    //-----------------------
    // フォグ関連
    //-----------------------
    auto& cbFog = ShaderManager::Instance().GetAmbientManager()->WorkFogCBData();

    ImGui::Text(U8_TEXT("フォグ"));

    bool enableFog = cbFog.FogEnable;
    if (ImGui::Checkbox(U8_TEXT("フォグを有効にするかどうか"), &enableFog))
    {
        cbFog.FogEnable = enableFog;
    }

    if (enableFog)
    {
        ImGui::Text(U8_TEXT("距離フォグの色"));
        ImGui::ColorEdit4("##FogColor", &cbFog.FogColor.x);

        ImGui::Text(U8_TEXT("距離フォグの減衰率"));
        ImGui::DragFloat("##FogDensity", &cbFog.DistanceFogDensity, 0.00001f, 0.0f, 1.0f);

        ImGui::Text(U8_TEXT("距離フォグの開始地点"));
        ImGui::DragFloat("##FogStart", &cbFog.DistanceFogStart, 0.01f, 0.0f, 100.0f);

    }

    ImGui::Separator();
    //-----------------------
    // ライト関連
    //-----------------------
    ImGui::Text(U8_TEXT("ライト"));
    const auto& lightData = ShaderManager::Instance().GetAmbientManager()->GetLightCBData();
    ImGui::Text(U8_TEXT("現在有効なポイントライトの数: %d"), lightData.FramePointLightNum);

    ImGui::Separator();

    //-----------------------
    // 環境光
    //-----------------------
    auto& cbLight = ShaderManager::Instance().GetAmbientManager()->WorkLightCBData();

    ImGui::Text(U8_TEXT("環境光"));
    ImGui::Text(U8_TEXT("環境光の色"));
    ImGui::ColorEdit4("##AmbientColor", &cbLight.AmbientLight.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float);

    ImGui::Separator();

    //-----------------------
    // 平行光
    //-----------------------
    ImGui::Text(U8_TEXT("平行光"));

    ImGui::Text(U8_TEXT("平行光の方向"));
    if (ImGui::DragFloat3("##DirLigDirection", &cbLight.LigDirection.x, 0.1f, -1, 1))
    {
        cbLight.LigDirection.Normalize();
    }

    ImGui::Text(U8_TEXT("平行光の色"));
    ImGui::ColorEdit3("##DirCol", &cbLight.LigColor.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float);

    ImGui::Separator();

    //-----------------------
    // コースティクス関連
    //-----------------------
    auto& cbCoustics = ShaderManager::Instance().GetAmbientManager()->WorkCousticsCBData();
    ImGui::Text(U8_TEXT("経過時間: %f"), cbCoustics.Time);
    ImGui::SameLine();
    if (ImGui::Button(U8_TEXT("リセット")))
    {
        ShaderManager::Instance().GetAmbientManager()->ResetTime();
    }

    ImGui::Text(U8_TEXT("スピード: %f"), cbCoustics.Speed);
    ImGui::DragFloat("##Speed", &cbCoustics.Speed, 0.005f, 0.0f, 100.0f);

    ImGui::Text(U8_TEXT("水面までの高さ"));
    ImGui::DragFloat("##Height", &cbCoustics.Height, 0.005f, 0.0f, 100.0f);

    ImGui::Text(U8_TEXT("コースティクスのスケール"));
    ImGui::DragFloat("##Scale", &cbCoustics.CousticsScale, 0.01f, 0.0f, 100.0f);

    ImGui::Text(U8_TEXT("コースティクスの光の強さ"));
    ImGui::DragFloat("##Intencity", &cbCoustics.Intencity, 0.01f, 0.0f, 100.0f);

    ImGui::Text(U8_TEXT("コースティクスの色"));
    ImGui::ColorEdit3("##CousticsColor", &cbCoustics.CousticsColor.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float);
}

void ImGuiUpdate::SceneManagerGUI()
{
    //x----- シーンの変更 / 削除 -----x//
    {
        const char** items = SceneManager::Instance().GetSceneNames().data();

        if (ImGui::Button(U8_TEXT("シーンの変更")))
        {
            SceneManager::Instance().ChangeScene(SceneManager::Instance().GetSceneNames()[m_selectItemCurrent]);
        }
        ImGui::SameLine();
        if (ImGui::Button(U8_TEXT("シーンの削除")))
        {
            SceneManager::Instance().RemoveScene(SceneManager::Instance().GetSceneNames()[m_selectItemCurrent]);
        }

        std::string comboName = "###Object";
        ImGui::Combo(comboName.data(), &m_selectItemCurrent, items,
            static_cast<int>(SceneManager::Instance().GetSceneNames().size()));
    }

    //x----- シーンの追加 -----x//
    {
        if (ImGui::Button(U8_TEXT("シーンの追加")))
        {
            if (m_generateSceneName.empty())
            {
                // デフォルトのシーン名を設定
                m_generateSceneName = "NewScene";
            }

            SceneManager::Instance().AddScene(m_generateSceneName);

            m_generateSceneName.clear();
        }
        ImGui::SameLine();
        utl::ImGuiHelper::InputTextWithString("###SceneName", m_generateSceneName);
    }
}

void ImGuiUpdate::CurrentSceneGUI()
{
    bool pushRButton = InputSystem::Instance().IsHold("LeftClick");

    // クリック中 & マウスがウィンドウ外に出た時の処理
    if (pushRButton)
    {
        const HWND& hwnd = Application::Instance().GetWindowClass().GetWndHandle();
        // アクティブなウィンドウのときのみ設定
        if (hwnd == GetForegroundWindow())
        {
            // このモードの時はマウスがウィンドウ外に出ないようにする
            int mouseWarpX = 0, mouseWarpY = 0;
            if (Window::WarpMouseToInScreen(hwnd, mouseWarpX, mouseWarpY))
            {
                // ウィンドウハンドルをチェック
                if (!hwnd) { return; }

                // ImGui のマウスドラッグ量を更新
                ImGuiIO& io = ImGui::GetIO();

                // マウスのドラッグ量（移動量）を調整
                io.MouseDelta.x += static_cast<float>(-mouseWarpX);
                io.MouseDelta.y += static_cast<float>(-mouseWarpY);
            }
        }
    }

    //x----- シーンマネージャーのGUI -----x//

    SceneManagerGUI();

    ImGui::Separator();
    ImGui::Separator();

    //x----- 現在のシーンの情報を表示 -----x//

    ImGui::Text(U8_TEXT("シーン名 : %s"), SceneManager::Instance().GetNowSceneName().data());

    ImGui::Separator();
    ImGui::Separator();

    const std::shared_ptr<Scene>& spNowScene = SceneManager::Instance().GetScene(
        SceneManager::Instance().GetNowSceneName());

    if (!spNowScene) { return; }

    // シーンの保存 //
    if (ImGui::Button(U8_TEXT("シーンの保存")))
    {
        SceneManager::Instance().SaveScene(spNowScene);
    }

    //x------ オブジェクトのImGuiを更新 ------x//
    ImGui::Text(U8_TEXT("オブジェクトの数 : %d"), spNowScene->GetObjectList().size());

    // 現在のシーンにオブジェクトを追加する処理
    spNowScene->AddObjectImGui();

    int flags =
        //ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow;

    bool bTreeOpen;

    for (auto&& obj : spNowScene->GetObjectList())
    {
        // 親オブジェクトのみ表示
        if (obj->GetParent()) { continue; }

        ImGui::PushID(&obj);

        // オブジェクト全体の有効フラグ更新
        bool isActiveState = obj->GetState() == GameObject::State::eActive;
        ImGui::Checkbox("##Enable", &isActiveState);
        if (isActiveState)
        {
            obj->SetState(GameObject::State::eActive);

            // 子どももポーズ状態にする
            for (auto&& spChild : obj->GetChildren())
            {
                if (spChild.expired()) { continue; }
                spChild.lock()->SetState(GameObject::State::eActive);
            }
        }
        else
        {
            obj->SetState(GameObject::State::ePaused);

            // 子どももポーズ状態にする
            for (auto&& spChild : obj->GetChildren())
            {
                if (spChild.expired()) { continue; }
                spChild.lock()->SetState(GameObject::State::ePaused);
            }
        }
        ImGui::SameLine();

        // ツリーを開く //
        bTreeOpen =
            ImGui::TreeNodeEx(&obj, flags, obj->GetName().data());

        if (bTreeOpen)
        {
            obj->ImGuiUpdate();
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}
