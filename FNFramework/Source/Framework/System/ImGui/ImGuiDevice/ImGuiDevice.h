#pragma once

//====================================
// ImGuiヘルパークラス
// ヒープ領域やバッファをもっている
// ※シングルトンではなくメインが一つだけ持ってるように変更する
//====================================
class ImGuiDevice
    : public utl::Singleton<ImGuiDevice>
{
    friend class utl::Singleton<ImGuiDevice>;

public:
    /* @brief 初期化処理 */
    void Init();
    /* @brief 解放処理 */
    void Release();

    /* @brief ImGui用のヒープ領域取得 @result ImGui用のヒープ領域 */
    ComPtr<ID3D12DescriptorHeap> GetImGuiHeap() const { return m_pImGuiHeap; }

    /* @brief 新しいフレームの取得 */
    void NewFrame();
    /* @brief レンダリング処理 */
    void SetHeap();

private:
    ImGuiDevice()
    {
    }

    ~ImGuiDevice() override
    {
    }

    /* @brief ヒープ領域確保 */
    void CreateDescriptorHeapForImgui();

    ComPtr<ID3D12DescriptorHeap> m_pImGuiHeap = nullptr;
};
