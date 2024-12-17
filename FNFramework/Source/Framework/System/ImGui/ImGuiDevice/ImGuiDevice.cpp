#include "ImGuiDevice.h"

#include "Application/Application.h"

void ImGuiDevice::Init()
{
    CreateDescriptorHeapForImgui();

    //=======================
    // ImGui
    //=======================
    // コンテキスト設定
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "SaveData/imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.DisplaySize = ImVec2(Screen::Width, Screen::Height);

    //--------------------
    // フォント関係設定
    //--------------------
    // 日本語フォント追加
    {
        ImFont* font = io.Fonts->AddFontDefault();

        ImFontConfig imFontConfig;
        imFontConfig.MergeMode = true;

#include "ImGui/ja_glyph_ranges.h"
        font = io.Fonts->AddFontFromFileTTF("c:/Windows/Fonts/meiryo.ttc", 16.0f, &imFontConfig, glyphRangesJapanese);

        font->Scale = 1.5f;
    }

    // ウィンドウスタイル
    ImGui::StyleColorsClassic();

    // ImGuiの初期化
    ImGui_ImplWin32_Init(Application::Instance().GetWindowHandle());
    ImGui_ImplDX12_Init(GraphicsDevice::Instance().GetDevice(), 3,
                        DXGI_FORMAT_R8G8B8A8_UNORM, m_pImGuiHeap.Get(),
                        m_pImGuiHeap->GetCPUDescriptorHandleForHeapStart(),
                        m_pImGuiHeap->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiDevice::NewFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDevice::Release()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiDevice::CreateDescriptorHeapForImgui()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto hr = GraphicsDevice::Instance().GetDevice()->CreateDescriptorHeap(
        &desc, IID_PPV_ARGS(m_pImGuiHeap.GetAddressOf()));

    if (FAILED(hr)) { FNENG_ASSERT_ERROR("ImGui用のヒープ領域作成失敗"); }
}

void ImGuiDevice::SetHeap()
{
    ImGui::Render(); // ImGuiへのレンダリング

    // ヒープ設定
    ID3D12DescriptorHeap* heaps[] = {
        /* ImGuiDevice::Instance():: */ GetImGuiHeap().Get()
    };
    GraphicsDevice::Instance().GetCmdList()->SetDescriptorHeaps(_countof(heaps), heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GraphicsDevice::Instance().GetCmdList());
}
