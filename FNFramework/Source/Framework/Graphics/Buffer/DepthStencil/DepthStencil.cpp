#include "DepthStencil.h"

bool DepthStencil::Create(int w, int h, DXGI_FORMAT format, bool constantData)
{
    DXGI_FORMAT formatType = format;
    switch (format)
    {
    case DXGI_FORMAT_R16_TYPELESS:
        formatType = DXGI_FORMAT_D16_UNORM;
        break;
    case DXGI_FORMAT_R32_TYPELESS:
        formatType = DXGI_FORMAT_D32_FLOAT;
        break;
    case DXGI_FORMAT_R24G8_TYPELESS:
        formatType = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    default:
        FNENG_ASSERT_ERROR("DepthStencilのフォーマットが不正です");
        break;
    }

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    //heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    //heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Alignment = 0;
    resDesc.Width = static_cast<UINT>(w);
    resDesc.Height = static_cast<UINT>(h);
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = formatType;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = formatType;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    auto hr = GraphicsDevice::Instance().GetDevice()->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&m_pBuffer));

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("DepthStencilBufferの作成が失敗しました");
        return false;
    }

    // DSV作成
    m_dsvNumber = GraphicsDevice::Instance().GetDSVHeap()->CreateDSV(m_pBuffer.Get(), clearValue.Format, constantData);

    return true;
}

void DepthStencil::ClearDSV()
{
    const D3D12_CPU_DESCRIPTOR_HANDLE& dsvH = GraphicsDevice::Instance().GetDSVHeap()->GetCPUHandle(m_dsvNumber);
    GraphicsDevice::Instance().ClearDSV(dsvH, 1.0f);
}
