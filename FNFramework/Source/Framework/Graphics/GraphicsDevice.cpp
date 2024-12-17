#include "GraphicsDevice.h"
#include "GDErrorHandler.h"
#include "Application/Application.h"
#include "Buffer/RenderTarget/RenderTarget.h"

bool GraphicsDevice::Init(HWND hWnd, int w, int h)
{
    //-----------------------------
    // デバイス関連の初期化処理
    //-----------------------------

    // ファクトリー作成
    if (!CreateFactory())
    {
        FNENG_ASSERT_ERROR("ファクトリー作成失敗");
        return false;
    }

    // デバッグレイヤ設定
#ifdef _DEBUG
    EnableDebugLayer();
    m_enableDREAD = GDErrorHandler::EnableDREAD(true);
#endif

    // デバイス作成
    if (!CreateDevice())
    {
        FNENG_ASSERT_ERROR("デバイス作成失敗");
        return false;
    }

#ifdef _DEBUG

    // デバッグデバイス作成
    auto hr = m_pGraphicsDevice->QueryInterface(m_pDebugDevice.GetAddressOf());
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("デバッグデバイスの作成が失敗しました。");
    }

    // ブレーク発生
    GDErrorHandler::GPUDebugOnBreak(*this);
#endif

    // コマンドリスト作成
    if (!CreateCommandList())
    {
        FNENG_ASSERT_ERROR("コマンドリスト作成失敗");
        return false;
    }

    // スワップチェイン作成
    if (!CreateSwapChain(hWnd, w, h))
    {
        FNENG_ASSERT_ERROR("スワップチェイン作成失敗");
        return false;
    }

    //---------------------
    // 各種ヒープ作成
    //---------------------
    // RTVHeap設定
    constexpr int RTVHeapUseMaxCount = 50; // この個数以上レンダーターゲットを使おうとするとAssertで落ちるので注意
    m_upRTVHeap = std::make_unique<RTVHeap>();
    if (!m_upRTVHeap->Create(HeapType::RTV, RTVHeapUseMaxCount))
    {
        FNENG_ASSERT_ERROR("RTVヒープの作成失敗");
        return false;
    }

    // CBVSRVUAVHeap
    constexpr Math::Vector3 CBVSRVUAVHeapUseMaxCount = { 5000, 5000, 100 };
    m_upCBVSRVUAVHeap = std::make_unique<CBVSRVUAVHeap>();
    if (!m_upCBVSRVUAVHeap->Create(HeapType::CBVSRVUAV, CBVSRVUAVHeapUseMaxCount))
    {
        FNENG_ASSERT_ERROR("CBVSRVUAVヒープの作成失敗");
        return false;
    }

    // CBufferAllocater
    m_upCBufferAllocater = std::make_unique<CBufferAllocater>();
    m_upCBufferAllocater->Create(m_upCBVSRVUAVHeap.get());

    // DSVHeap
    constexpr int DSVHeapUseMaxCount = 100;
    m_upDSVHeap = std::make_unique<DSVHeap>();
    if (!m_upDSVHeap->Create(HeapType::DSV, DSVHeapUseMaxCount))
    {
        FNENG_ASSERT_ERROR("DSVヒープの作成失敗");
        return false;
    }

    // DepthStencil
    m_upDepthStencil = std::make_unique<DepthStencil>();
    if (!m_upDepthStencil->Create(
        Screen::Width, Screen::Height,
        DXGI_FORMAT_R32_TYPELESS,
        /* constantData =  */ true))
    {
        FNENG_ASSERT_ERROR("DepthStencilの作成失敗");
        return false;
    }

    // SwapChainRTV作成
    if (!CreateSwapChainRTV())
    {
        FNENG_ASSERT_ERROR("スワップチェインのRTVの作成失敗");
        return false;
    }

    // Fence作成
    if (!CreateFence())
    {
        FNENG_ASSERT_ERROR("フェンスの作成失敗");
        return false;
    }

    //-----------------------------
    // デバイス関連以外の初期化処理
    //-----------------------------

    // 白テクスチャ作成
    m_spWhiteTex = std::make_shared<ShaderResourceTexture>();
    if (!CreateWhiteTexture(*m_spWhiteTex))
    {
        FNENG_ASSERT_ERROR("白テクスチャの作成失敗");
        return false;
    }

    return true;
}

void GraphicsDevice::ResetHeaps()
{
    m_upRTVHeap->Reset();
    m_upCBVSRVUAVHeap->Reset();
    m_upDSVHeap->Reset();
}

void GraphicsDevice::SetRenderTargets(int _numRT, RenderTarget** _renderTarget)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtDSHandleTbl[32];

    for (UINT rtNo = 0; rtNo < _numRT; rtNo++)
    {
        auto rtvHND = GraphicsDevice::Instance().GetRTVHeap()->GetCPUHandle(_renderTarget[rtNo]->GetRTVNumber());
        rtDSHandleTbl[rtNo] = rtvHND;
    }

    if (_renderTarget[0]->GetDepthStencil().IsCreate())
    {
        //深度バッファがある。
        const D3D12_CPU_DESCRIPTOR_HANDLE& dsvHND = GraphicsDevice::Instance().GetDSVHeap()->GetCPUHandle(_renderTarget[0]->GetDSVNumber());
        m_pCmdList->OMSetRenderTargets(_numRT, rtDSHandleTbl, FALSE, &dsvHND);
    }
    else
    {
        //深度バッファがない場合はデフォルトの深度バッファを使用する
        const D3D12_CPU_DESCRIPTOR_HANDLE& dsvHND = GraphicsDevice::Instance().GetCurrentFrameBuffuerDSV();
        m_pCmdList->OMSetRenderTargets(_numRT, rtDSHandleTbl, FALSE, &dsvHND);
    }
}

void GraphicsDevice::SetRenderTarget(const RenderTarget& _renderTarget)
{
    auto rtvHND = Instance().GetRTVHeap()->GetCPUHandle(_renderTarget.GetRTVNumber());

    if (_renderTarget.GetDepthStencil().IsCreate())
    {
        // 深度バッファがある場合
        auto dsvHND = Instance().GetDSVHeap()->GetCPUHandle(_renderTarget.GetDSVNumber());
        SetRenderTarget(rtvHND, dsvHND);
    }
    else
    {
        // 深度バッファがない場合
        m_pCmdList->OMSetRenderTargets(1, &rtvHND, false, nullptr);
    }
}

void GraphicsDevice::CopyToBackBuffer(const ShaderResourceTexture& srcTexture)
{
    // RTの状態をコピー可能にする
    SetResourceBarrier(
        srcTexture.WorkBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_COPY_SOURCE);

    // バックバッファの状態をコピー対象にする
    UINT idx = m_pSwapChain->GetCurrentBackBufferIndex();
    ID3D12Resource* pBBResource = m_pSwapchainBuffers[idx].Get();

    GraphicsDevice::Instance().SetResourceBarrier(
        pBBResource,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_COPY_DEST);

    // RTの内容をBackBufferにコピー
    GraphicsDevice::Instance().GetCmdList()->CopyResource(pBBResource, srcTexture.WorkBuffer());

    // RTの状態を戻す
    GraphicsDevice::Instance().SetResourceBarrier(
        srcTexture.WorkBuffer(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    // バックバッファの状態を戻す
    GraphicsDevice::Instance().SetResourceBarrier(
        pBBResource,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PRESENT);
}

void GraphicsDevice::Release()
{
    WaitForCommandQueue();
}

void GraphicsDevice::Prepare()
{
    // 現在のバックバッファのインデックス取得
    auto bbIdx = m_pSwapChain->GetCurrentBackBufferIndex();
    SetRenderTargetResourceBarrier(m_pSwapchainBuffers[bbIdx].Get());

    m_currentFrameBufferRTVHandle = m_upRTVHeap->GetCPUHandle(bbIdx);

    m_currentFrameBufferDSVHandle = m_upDSVHeap->GetCPUHandle(m_upDepthStencil->GetDSVNumber());
    m_pCmdList->OMSetRenderTargets(1, &m_currentFrameBufferRTVHandle,
        false, &m_currentFrameBufferDSVHandle);

    // 各ビューのクリア
    ClearRTV(m_currentFrameBufferRTVHandle, m_ClearBackBufferColor);
    ClearDSV(m_currentFrameBufferDSVHandle, 1.0f);
}

void GraphicsDevice::ScreenFlip()
{
    // 通常リソースの場合はSwapchainのバリアはRT->Presentに変更する必要がある
    // todo : 今後はここを統合しても問題なさそう
    auto bbIdx = m_pSwapChain->GetCurrentBackBufferIndex(); // 現在のスワップチェインの番号が返ってくる

    // リソースバリアのステートをプレゼントに戻す
    FinishDrawingToRenderTargetResourceBarrier(m_pSwapchainBuffers[bbIdx].Get());

    // コマンドリストを閉じて実行する
    // ※コマンドリストを閉じていないと描画できません
    m_pCmdList->Close();
    ID3D12CommandList* cmdLists[] = { m_pCmdList.Get() };
    m_pCmdQueue->ExecuteCommandLists(1, cmdLists);

    // コマンドリストの同期を待つ
    WaitForCommandQueue();

    // コマンドアロケーターとコマンドリストを初期化
    m_pCmdAllocator->Reset(); // コマンドアロケーターの初期化
    m_pCmdList->Reset(m_pCmdAllocator.Get(), nullptr); // コマンドリストの初期化

#ifdef _DEBUG
    HRESULT hr =
        // スワップチェインに送る
        m_pSwapChain->Present(0, 0);	// 垂直同期 : OFF
    //m_pSwapChain->Present(TRUE, 0);	// 垂直同期 : ON

    // デバイスロストが発生した場合のエラー箇所特定処理
    GDErrorHandler::GetDREADData(*this, hr, m_enableDREAD);
#else
    // スワップチェインに送る
    //m_pSwapChain->Present(0, 0);	// 垂直同期 : OFF
    m_pSwapChain->Present(TRUE, 0); // 垂直同期 : ON
#endif
}

void GraphicsDevice::WaitForCommandQueue()
{
    m_pCmdQueue->Signal(m_pFence.Get(), ++m_fenceVal);

    if (m_pFence->GetCompletedValue() != m_fenceVal)
    {
        auto event = CreateEvent(nullptr, false, false, nullptr); // イベントハンドルの取得
        if (!event)
        {
            FNENG_ASSERT_ERROR("イベントエラー、アプリケーションを終了します");
            return;
        }
        m_pFence->SetEventOnCompletion(m_fenceVal, event);

        WaitForSingleObject(event, INFINITE); // イベントが発生するまで待ち続ける
        CloseHandle(event); // イベントハンドルを閉じる
    }
}

bool GraphicsDevice::CreateFactory()
{
    UINT flagsDXGI = 0;
    flagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
    auto result = CreateDXGIFactory2(flagsDXGI, IID_PPV_ARGS(m_pDxgiFactory.GetAddressOf()));

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool GraphicsDevice::CreateDevice()
{
    ComPtr<IDXGIAdapter> pSelectAdapter = nullptr;
    std::vector<ComPtr<IDXGIAdapter>> pAdapters;
    std::vector<DXGI_ADAPTER_DESC> descs;

    // 使用中PCにあるGPUドライバを検索する
    for (UINT index = 0; true; ++index)
    {
        // GPUドライバが見つかれば格納する
        pAdapters.push_back(nullptr);
        HRESULT ret = m_pDxgiFactory->EnumAdapters(index, &pAdapters[index]);

        if (ret == DXGI_ERROR_NOT_FOUND) { break; }

        descs.push_back({});
        pAdapters[index]->GetDesc(&descs[index]);
    }

    auto gpuTier = GPUTier::Kind;

    // 優先度の高いGPUドライバを使用する
    for (int i = 0; i < descs.size(); ++i)
    {
        if (std::wstring(descs[i].Description).find(L"NVIDIA") != std::wstring::npos)
        {
            pSelectAdapter = pAdapters[i];
            break;
        }
        if (std::wstring(descs[i].Description).find(L"Amd") != std::wstring::npos)
        {
            if (gpuTier > GPUTier::Amd)
            {
                pSelectAdapter = pAdapters[i];
                gpuTier = GPUTier::Amd;
            }
        }
        else if (std::wstring(descs[i].Description).find(L"Intel") != std::wstring::npos)
        {
            if (gpuTier > GPUTier::Intel)
            {
                pSelectAdapter = pAdapters[i];
                gpuTier = GPUTier::Intel;
            }
        }
        else if (std::wstring(descs[i].Description).find(L"Arm") != std::wstring::npos)
        {
            if (gpuTier > GPUTier::Arm)
            {
                pSelectAdapter = pAdapters[i];
                gpuTier = GPUTier::Arm;
            }
        }
        else if (std::wstring(descs[i].Description).find(L"Qualcomm") != std::wstring::npos)
        {
            if (gpuTier > GPUTier::Qualcomm)
            {
                pSelectAdapter = pAdapters[i];
                gpuTier = GPUTier::Qualcomm;
            }
        }
    }

    // フューチャーレベル設定
    D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    // Direct3Dデバイスの初期化
    D3D_FEATURE_LEVEL featureLevel;
    for (auto lv : levels)
    {
        if (D3D12CreateDevice(pSelectAdapter.Get(), lv, IID_PPV_ARGS(m_pGraphicsDevice.GetAddressOf())) == S_OK)
        {
            featureLevel = lv;
            break; // 生成可能なバージョンが見つかったらループ打ち切り
        }
    }

    return true;
}

bool GraphicsDevice::CreateCommandList()
{
    // コマンドアロケーター作成
    auto hr = m_pGraphicsDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_pCmdAllocator.GetAddressOf()));
    if (FAILED(hr))
    {
        return false;
    }

    // コマンドリスト作成
    hr = m_pGraphicsDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCmdAllocator.Get(), nullptr,
        IID_PPV_ARGS(m_pCmdList.GetAddressOf()));
    if (FAILED(hr))
    {
        return false;
    }

    // コマンドキュー作成
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // タイムアウトなし
    cmdQueueDesc.NodeMask = 0; // アダプタを1つしか使わないときは0でよい
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // プライオリティは指定なし
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // コマンドリストと同じにする

    hr = m_pGraphicsDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(m_pCmdQueue.GetAddressOf()));
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool GraphicsDevice::CreateSwapChain(HWND hWnd, int width, int height)
{
    // スワップチェイン作成
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = width;
    swapchainDesc.Height = height;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後(画面切り替え後)は破棄
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ウィンドウとフルスクリーン切り替え可

    auto hr = m_pDxgiFactory->CreateSwapChainForHwnd(m_pCmdQueue.Get(), hWnd, &swapchainDesc, nullptr, nullptr,
        (IDXGISwapChain1**)m_pSwapChain.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    SetMonitorInfo(width, height, RefreshRate);

    return true;
}

bool GraphicsDevice::CreateSwapChainRTV()
{
    for (int i = 0; i < static_cast<int>(m_pSwapchainBuffers.size()); ++i)
    {
        auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pSwapchainBuffers[i].GetAddressOf()));

        if (FAILED(hr))
        {
            return false;
        }

        const D3D12_RESOURCE_DESC& desc = m_pSwapchainBuffers[i]->GetDesc();

        m_upRTVHeap->CreateRTV(m_pSwapchainBuffers[i].Get(), desc.Format, /* constantData = */ true);
    }

    return true;
}

void GraphicsDevice::SetMonitorInfo(int width, int height, UINT refreshRate)
{
    ComPtr<IDXGIAdapter> pAdapter;
    ComPtr<IDXGIOutput> pOutput;

    DXGI_MODE_DESC desiredMode;
    DXGI_MODE_DESC closestMatchingMode;

    UINT adapterIndex = 0;
    UINT outputIndex = 0;
    while (m_pDxgiFactory->EnumAdapters(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        outputIndex = 0;
        while (pAdapter->EnumOutputs(outputIndex, &pOutput) != DXGI_ERROR_NOT_FOUND)
        {
            // 各出力に対する設定を行う
            desiredMode = {};

            desiredMode.Width = width;
            desiredMode.Height = height;
            desiredMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desiredMode.RefreshRate.Numerator = refreshRate; // 分子
            desiredMode.RefreshRate.Denominator = 1; // 分母

            if (SUCCEEDED(pOutput->FindClosestMatchingMode(&desiredMode, &closestMatchingMode, nullptr)))
            {
                // ToDo : ここでclosestMatchingModeを使用してスワップチェインやデバイスの設定を変更することができます
            }

            pOutput = nullptr;
            outputIndex++;
        }

        pAdapter = nullptr;
        adapterIndex++;
    }
}

bool GraphicsDevice::CreateFence()
{
    auto result = m_pGraphicsDevice->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(m_pFence.GetAddressOf()));
    if (FAILED(result))
    {
        FNENG_ASSERT_ERROR("フェンス作成失敗");
        return false;
    }

    return true;
}

void GraphicsDevice::SetRenderTargetResourceBarriers(int numRT, RenderTarget** renderTarget)
{
    for (int i = 0; i < numRT; ++i)
    {
        SetRenderTargetResourceBarrier(*renderTarget[i]);
    }
}

void GraphicsDevice::SetRenderTargetResourceBarrier(RenderTarget& renderTarget)
{
    SetRenderTargetResourceBarrier(renderTarget.GetTexture().WorkBuffer());
}

void GraphicsDevice::FinishDrawingToRenderTargetResourceBarriers(int numRT, RenderTarget** renderTarget)
{
    for (int i = 0; i < numRT; ++i)
    {
        FinishDrawingToRenderTargetResourceBarrier(*renderTarget[i]);
    }
}

void GraphicsDevice::FinishDrawingToRenderTargetResourceBarrier(RenderTarget& renderTarget)
{
    FinishDrawingToRenderTargetResourceBarrier(renderTarget.GetTexture().WorkBuffer());
}

void GraphicsDevice::SetResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after, UINT subresource,
    D3D12_RESOURCE_BARRIER_FLAGS flags)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = flags;

    barrier.Transition.pResource = pResource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = subresource;

    m_pCmdList->ResourceBarrier(1, &barrier);
}

void GraphicsDevice::EnableDebugLayer()
{
    //-----------------デバッグレイヤー有効化-----------------//
    ComPtr<ID3D12Debug> pDebug = nullptr;
    ComPtr<ID3D12Debug1> pDebug1 = nullptr;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(pDebug.GetAddressOf()))))
    {
        pDebug->EnableDebugLayer(); // デバッグレイヤを有効にする

        if (SUCCEEDED(pDebug->QueryInterface(pDebug1.GetAddressOf())))
        {
            pDebug1->SetEnableGPUBasedValidation(true); // GPUベースの検証機能有効化
            pDebug1->Release();
        }
        pDebug->Release();
    }
}

bool GraphicsDevice::CreateWhiteTexture(ShaderResourceTexture& texture)
{
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    heapProp.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = 4; // 幅
    resDesc.Height = 4; // 高さ
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr;
    ID3D12Resource* whiteBuff = nullptr;

    // バッファを作成
    // ピクセルシェーダーのリソースとして作成
    hr = m_pGraphicsDevice->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
        IID_PPV_ARGS(&whiteBuff));
    //hr = m_pGraphicsDevice->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
    // D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&whiteBuff));

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("テクスチャバッファ作成失敗");
        return false;
    }

    // データ描き込み
    std::vector<unsigned char> data(4 * 4 * 4); // テクスチャのサイズは、最低値(4 x 4)でよい
    std::ranges::fill(data, static_cast<unsigned char>(0xff)); // 全要素255(白)で埋める

    /* todo : ほかの色を表現したい場合はここを変更する
    * ☆一要素づつ足し込む場合はこっち
         for (size_t i = 0; i < data.size(); i += 4) {
        data[i] = 0xFF;     // R
        data[i + 1] = 0xFF; // G
        data[i + 2] = 0xFF; // B
        data[i + 3] = 0xFF; // A
    }
    */

    hr = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("バッファにテクスチャデータの書き込み失敗");
        return false;
    }

    int srvNumber = -1;
    // バッファをヒープ領域に登録
    srvNumber = m_upCBVSRVUAVHeap->CreateSRV(whiteBuff, /* constantData = */true);

    texture.InitFromD3DResource(whiteBuff, srvNumber);

    whiteBuff->Release();

    return true;
}
