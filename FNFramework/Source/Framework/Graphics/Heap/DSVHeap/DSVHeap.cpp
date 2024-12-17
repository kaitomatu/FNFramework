﻿#include "DSVHeap.h"

int DSVHeap::CreateDSV(ID3D12Resource* pBuffer, DXGI_FORMAT format, bool constantData)
{
    if (m_currentHeapData.UseCount < m_currentHeapData.NextRegistNumber)
    {
        FNENG_ASSERT_ERROR("確保済みのヒープ領域を超えました");
        return 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_currentHeapData.pHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(m_currentHeapData.NextRegistNumber) * m_currentHeapData.IncrementSize;

    D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
    viewDesc.Format = format;
    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2Dテクスチャ

    GraphicsDevice::Instance().GetDevice()->CreateDepthStencilView(pBuffer, &viewDesc, handle);

    const int registNumber = m_currentHeapData.NextRegistNumber++;

    // 定数データである場合は、リセットされないようにする
    if (constantData)
    {
        SetConstantHeapData(registNumber);
    }

    return registNumber;
}
