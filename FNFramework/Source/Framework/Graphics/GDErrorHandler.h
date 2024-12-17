#pragma once

class GraphicsDevice;

/*
* @class GDErrorHandler
* @brief グラフィックスデバイスのエラーハンドラー
* @details
*	デバイスロストが起こった際のエラーハンドリングを行うための関数を持つ
*/
namespace GDErrorHandler
{
    /*========================================
    /	@brief 初期化関数
    ==========================================*/

    /* @brief ブレークを発生させる */
    static void GPUDebugOnBreak(const GraphicsDevice& graphicsDevice)
    {
        //-----------------ブレーク処理有効化-----------------//
        ComPtr<ID3D12InfoQueue> pInfoQueue = nullptr;

        ID3D12Device8* device = graphicsDevice.GetDevice();

        if (!device)
        {
            FNENG_ASSERT_ERROR("デバイスが作成されていません");
            return;
        }

        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(pInfoQueue.GetAddressOf()))))
        {
            // Error時にブレークを発生させる
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        }
    }

    /**
    * @brief  DREADを適用
    *
    * @param  enableDread - Dreadを適応させる場合はtrue
    * @result DREAD作成できたらtrue
    */
    static bool EnableDREAD(bool enableDread)
    {
        if (!enableDread) { return false; }

        ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDreadSettings = nullptr;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(pDreadSettings.GetAddressOf()));

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR("Dredの作成失敗");
            return false;
        }

        // 自動パンくずとページフォルトリポートを有効にする
        pDreadSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        pDreadSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

        return true;
    }

    /*========================================
    /	@brief デバイスロストのエラー処理
    ==========================================*/
    /**
    * @brief DREADのデータを取得する
    * @param[in] result - DREADの結果
    * @param[in] isCheck - チェックするかどうか true : チェックする , false : チェックしない
    */
    static void GetDREADData(const GraphicsDevice& graphicsDevice, const HRESULT& result, bool isCheck)
    {
        if (!isCheck) { return; }

        HRESULT hr = {};

        // デバイスがなくなっていれば...
        if (result != DXGI_ERROR_DEVICE_REMOVED) { return; }

        ComPtr<ID3D12DeviceRemovedExtendedData> pDread = nullptr;

        ID3D12Device8* device = graphicsDevice.GetDevice();
        if (!device)
        {
            FNENG_ASSERT_ERROR("デバイスが作成されていません");
            return;
        }

        hr = device->QueryInterface(IID_PPV_ARGS(&pDread));

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR_F("クエリインターフェイスの取得に失敗 - ret = 0x%x", hr);
            return;
        }

        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT autoBreadcrumbsOutput = {};

        hr = pDread->GetAutoBreadcrumbsOutput(&autoBreadcrumbsOutput);

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR_F("パンくずデータの取得に失敗 - ret = 0x%x", hr);
            return;
        }

        // パンくずデータの表示
        auto pNode = autoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;

        while (pNode != nullptr)
        {
            char buffer[512];
            (void)sprintf_s(buffer, sizeof(buffer), "Node : \n"
                      "CommandList Name = %s, Address = 0x%p\n"
                      "CommandQueue Name = %s, Address = 0x%p\n"
                      "Command Count = %d\n"
                      "Completed Commands = %d\n",
                      pNode->pCommandListDebugNameA, pNode->pCommandList,
                      pNode->pCommandQueueDebugNameA, pNode->pCommandQueue,
                      pNode->BreadcrumbCount,
                      *(pNode->pLastBreadcrumbValue));

            FNENG_ASSERT_ERROR(buffer);

            // ノード内の各パンくずコマンドをループ処理
            for (UINT32 i = 0; i < pNode->BreadcrumbCount; ++i)
            {
                D3D12_AUTO_BREADCRUMB_OP cmd = pNode->pCommandHistory[i];

                GraphicsHelper::Msg::BreadCrumbMsgToString(cmd);
            }

            pNode = pNode->pNext;
        }

        //
        D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput;

        hr = pDread->GetPageFaultAllocationOutput(&pageFaultOutput);

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR_F("GetPageFaultAllocationOutputの失敗 - ret = 0x%x", hr);
            return;
        }

        FNENG_ASSERT_ERROR_F("PageFalut VirtualAddress = %lu", pageFaultOutput.PageFaultVA);

        auto pAllocationNode = pageFaultOutput.pHeadExistingAllocationNode;

        while (pAllocationNode != nullptr)
        {
            FNENG_ASSERT_ERROR_F("Existing Allocation Node Name = % s, Type = % d",
                                  pAllocationNode->ObjectNameA, pAllocationNode->AllocationType);

            pAllocationNode = pAllocationNode->pNext;
        }

        pAllocationNode = pageFaultOutput.pHeadRecentFreedAllocationNode;

        while (pAllocationNode != nullptr)
        {
            FNENG_ASSERT_ERROR_F("Recent Freed Allocation Node Name = %s, Type = %s",
                                  pAllocationNode->ObjectNameA,
                                  GraphicsHelper::Msg::DredAllocationTypeToString(pAllocationNode->AllocationType).c_str());

            pAllocationNode = pAllocationNode->pNext;
        }
    }
};
