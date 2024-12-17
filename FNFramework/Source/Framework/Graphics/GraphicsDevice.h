#pragma once

class Mesh;

class RTVHeap;
class CBVSRVUAVHeap;
class CBufferAllocater;
class DSVHeap;
class DepthStencil;

class RenderTarget;
class ShaderResourceTexture;

/**
* @class GraphicsDevice
* @brief グラフィックスデバイスクラス
* @details DirectX12のグラフィックスデバイスを管理するクラス : シングルトン
*
* ToDo : デバイス関連の処理が肥大化してきたので、コマンドリスト / ヒープ / デバイスに分割する
*/
class GraphicsDevice
    : public utl::Singleton<GraphicsDevice>
{
    friend class utl::Singleton<GraphicsDevice>;

public:
    /**
    * @brief 初期化
    *
    * @param hWnd   - ウィンドウハンドル
    * @param width  - ウィンドウの横幅
    * @param height - ウィンドウの縦幅
    * @result 初期化完了したらtrue
    */
    bool Init(HWND hWnd, int width, int height);

    /* @brief ヒープのリセット */
    void ResetHeaps();

    /** @brief 描画準備 */
    void Prepare();

    /* @brief 画面(スワップチェイン)の切り替え */
    void ScreenFlip();

    /** @brief コマンドキューの同期待ち	*/
    void WaitForCommandQueue();

    /**
    * @brief  デバイス取得
    * @result デバイスのポインタ
    */
    ID3D12Device8* GetDevice() const
    {
        return m_pGraphicsDevice.Get();
    }

    //--------------------
    // デバッグ
    //--------------------
    /**
     * デバック用デバイスの取得
     * @return デバック用デバイス:_DEBUGが定義されていない場合はNULLが返る
     */
    ID3D12DebugDevice* GetDebugDevice() const
    {
        return m_pDebugDevice.Get();
    }

    void DebugReportLiveDeviceObject() const
    {
        if (!m_pDebugDevice)
        {
            FNENG_ASSERT_LOG("デバッグデバイスが存在しません。", true);
            return;
        }
        m_pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
    }

    /* @brief スワップチェインの取得 */
    IDXGISwapChain4* GetSwapChain() const
    {
        return m_pSwapChain.Get();
    }

    /* @brief デプスステンシルの取得 */
    DepthStencil* GetDepthStencil() const
    {
        return m_upDepthStencil.get();
    }

    /**
    * @brief  コマンドリスト取得
    * @result コマンドリストのポインタ
    */
    ID3D12GraphicsCommandList6* GetCmdList() const
    {
        return m_pCmdList.Get();
    }

    /**
    * @brief  CBVSRVUAVヒープの取得
    * @result CBVSRVUAVヒープのポインタ
    */
    CBVSRVUAVHeap* GetCBVSRVUAVHeap() const
    {
        return m_upCBVSRVUAVHeap.get();
    }

    /**
    * @brief  CBufferAllocaterの取得
    * @result CBufferAllocaterのポインタ
    */
    CBufferAllocater* GetCBufferAllocater() const
    {
        return m_upCBufferAllocater.get();
    }

    /**
    * @brief  DSVヒープの取得
    * @result DSVヒープのポインタ
    */
    DSVHeap* GetDSVHeap() const
    {
        return m_upDSVHeap.get();
    }

    RTVHeap* GetRTVHeap() const
    {
        return m_upRTVHeap.get();
    }

    /*
    * @brief  フレームバッファのRTVハンドルの取得
    * @result フレームバッファのRTVハンドル
    */
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentFrameBuffuerRTV() const
    {
        return m_currentFrameBufferRTVHandle;
    }

    /*
    * @brief フレームバッファへの描画時に使用されているデプスステンシルビューを取得
    * @result フレームバッファへの描画時に使用されているデプスステンシルビュー
    */
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentFrameBuffuerDSV() const
    {
        return m_currentFrameBufferDSVHandle;
    }

    /**
    * @brief  白テクスチャの取得
    * @result 白テクスチャ
    */
    const std::shared_ptr<ShaderResourceTexture>& GetWhiteTex() const
    {
        return m_spWhiteTex;
    }

    std::shared_ptr<ShaderResourceTexture> WorkWhiteTexture() const
    {
        return m_spWhiteTex;
    }

    /**
    * @brief  レンダーターゲットビューを特定色で塗りつぶす
    * @param rtvHandle - レンダーターゲットビューのハンドル
    * @param clearColor - 塗りつぶす色
    */
    void ClearRTV(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const Math::Color& clearColor)
    {
        m_pCmdList->ClearRenderTargetView(rtvHandle, &clearColor.x, 0, nullptr);
    }

    /**
    * @brief  デプスステンシルビューを特定色で塗りつぶす
    * @param dsvHandle - デプスステンシルビューのハンドル
    * @param depth - 塗りつぶす深度
    * @param stencil - 塗りつぶすステンシル
    */
    void ClearDSV(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, float depth, UINT8 stencil = 0.0f)
    {
        m_pCmdList->ClearDepthStencilView(
            dsvHandle,
            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
            depth, stencil, 0, nullptr);
    }

    /**
    * @brief リソースとして引数に渡したバッファの扱いを変更する関数(バリア)
    *
    * @param pResource - 指定バッファ
    * @param before    - 現在の状態
    * @param after     - 新しい状態
    */
    void SetResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
                            UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                            D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

    /**
    * @brief レンダーターゲット用のリソースバリアの設定
    * @param renderTarget - 指定バッファ
    */
    void SetRenderTargetResourceBarriers(int numRT, RenderTarget** renderTarget);
    void SetRenderTargetResourceBarrier(RenderTarget& renderTarget);

    void SetRenderTargetResourceBarrier(ID3D12Resource* pResource)
    {
        SetResourceBarrier(pResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    /**
    * @brief レンダーターゲット用のリソースバリアの設定(書き込み待ち)
    */
    void FinishDrawingToRenderTargetResourceBarriers(int numRT, RenderTarget** renderTarget);
    void FinishDrawingToRenderTargetResourceBarrier(RenderTarget& renderTarget);

    void FinishDrawingToRenderTargetResourceBarrier(ID3D12Resource* pResource)
    {
        SetResourceBarrier(pResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }

    /**
     * @brief レンダーターゲットの設定
     * @param numRT - レンダーターゲットの数
     * @param renderTarget - レンダーターゲット
     */
    void SetRenderTargets(int numRT, RenderTarget** renderTarget);

    /*
    * @brief レンダーターゲットの設定
    * @param renderTarget - レンダーターゲット
    */
    void SetRenderTarget(const RenderTarget& renderTarget);
    /**
    * @brief レンダーターゲットの設定
    * @param rtvHandle - レンダーターゲットビューのハンドル
    * @param dsvHandle - デプスステンシルビューのハンドル
    */
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
    {
        m_pCmdList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
    }

    // スワップチェインリソースの取得 //
    ID3D12Resource* GetSwapChainBuffer(UINT index) const
    {
        if(index >= m_pSwapchainBuffers.size())
        {
            FNENG_ASSERT_LOG("スワップチェインバッファの取得に失敗しました。", true);
            return nullptr;
        }

        return m_pSwapchainBuffers[index].Get();
    }

    void CopyToBackBuffer(const ShaderResourceTexture& srcTexture);

private:
    GraphicsDevice()
        : m_currentFrameBufferRTVHandle({0})
          , m_currentFrameBufferDSVHandle({0})
    {
    }

    ~GraphicsDevice() override { Release(); }

    /* @brief 解放処理 */
    void Release();

    /**
    * @brief  ファクトリーの作成
    * @result 作成できたらtrue
    */
    bool CreateFactory();

    /*
    * @brief  デバイスの作成
    * @result 作成できたらtrue
    */
    bool CreateDevice();

    /**
    * @brief  コマンドリストの作成
    * @result 作成できたらtrue
    */
    bool CreateCommandList();

    /**
    * @brief スワップチェインの作成
    *
    * @param  hWnd   - ウィンドウハンドル
    * @param  width  - ウィンドウの横幅
    * @param  height - ウィンドウの縦幅
    * @result 作成できたらtrue
    */
    bool CreateSwapChain(HWND hWnd, int width, int height);

    /**
    * @brief  スワップチェインRTVの作成
    * @result 作成できたらtrue
    */
    bool CreateSwapChainRTV();

    /**
    * @brief  モニターのリフレッシュレートなどの設定
    *
    * @param  width		  - ウィンドウの横幅
    * @param  height	  - ウィンドウの縦幅
    * @param  refreshRate - リフレッシュレート
    */
    void SetMonitorInfo(int width, int height, UINT refreshRate);

    /**
    * @brief  Fenceの作成
    * @result 作成できたらtrue
    */
    bool CreateFence();

    //--------------------
    // デバッグ
    //--------------------
    /* @brief デバッグレイヤーを適用 */
    void EnableDebugLayer();
    ComPtr<ID3D12DebugDevice> m_pDebugDevice;

    /**
    * @brief 白テクスチャの作成
    *
    * @param[out] texture : 作成したいテクスチャのアドレス
    * @result 作成出来たらtrue
    */
    bool CreateWhiteTexture(ShaderResourceTexture& texture);

    enum class GPUTier
    {
        NVIDIA,
        Amd,
        Intel,
        Arm,
        Qualcomm,
        Kind,
    };

    //--------------------
    // デバイス関連
    //--------------------
    ComPtr<ID3D12Device8> m_pGraphicsDevice = nullptr;
    ComPtr<IDXGIFactory6> m_pDxgiFactory = nullptr;

    //--------------------
    // コマンド関連
    //--------------------
    ComPtr<ID3D12CommandAllocator> m_pCmdAllocator = nullptr;
    ComPtr<ID3D12GraphicsCommandList6> m_pCmdList = nullptr;
    ComPtr<ID3D12CommandQueue> m_pCmdQueue = nullptr;

    ComPtr<ID3D12Fence> m_pFence = nullptr;
    UINT64 m_fenceVal = 0;

    //--------------------
    // スワップチェイン
    //--------------------
    ComPtr<IDXGISwapChain4> m_pSwapChain = nullptr;
    std::array<ComPtr<ID3D12Resource>, 2> m_pSwapchainBuffers;

    //--------------------
    // ヒープ関連
    //--------------------
    std::unique_ptr<RTVHeap> m_upRTVHeap = nullptr;
    std::unique_ptr<CBVSRVUAVHeap> m_upCBVSRVUAVHeap = nullptr;
    std::unique_ptr<CBufferAllocater> m_upCBufferAllocater = nullptr;
    std::unique_ptr<DSVHeap> m_upDSVHeap = nullptr;
    std::unique_ptr<DepthStencil> m_upDepthStencil = nullptr;

private:
    //--------------------
    // その他定数
    //--------------------
    // 最大リフレッシュレートの設定
    static constexpr UINT RefreshRate = 240;
    // DREADフラグ
    bool m_enableDREAD = false;

    // 現在書き込み中のフレームバッファのレンダリングターゲットビューのハンドル
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentFrameBufferRTVHandle = {};
    // 現在書き込み中のフレームバッファの深度ステンシルビューのハンドル
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentFrameBufferDSVHandle = {};

    // 白テクスチャ
    std::shared_ptr<ShaderResourceTexture> m_spWhiteTex = nullptr;
    // バックバッファの色
    Math::Color m_ClearBackBufferColor = Color::Gray;
};
