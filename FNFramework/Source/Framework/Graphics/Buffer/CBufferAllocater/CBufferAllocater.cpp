#include "CBufferAllocater.h"

void CBufferAllocater::Create(CBVSRVUAVHeap* pHeap)
{
    m_pCbvHeap = pHeap;

    if (!CreateBufferResource())
    {
        FNENG_ASSERT_ERROR("CBuffer作成失敗");
        return;
    }

    m_pBuffer->Map(0, nullptr, (void**)&m_pMappedBuffer);
}

void CBufferAllocater::ResetCurrentUseNumber()
{
    m_currentUseNumber = 0;
}

bool CBufferAllocater::CreateBufferResource()
{
    D3D12_HEAP_PROPERTIES heapProp;
    CreateHeapProperties(heapProp);

    D3D12_RESOURCE_DESC resDesc;
    CreateResourceDescription(resDesc);

    HRESULT hr = GraphicsDevice::Instance().GetDevice()->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pBuffer)
    );

    return SUCCEEDED(hr);
}

void CBufferAllocater::CreateHeapProperties(D3D12_HEAP_PROPERTIES& heapProp)
{
    heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
}

void CBufferAllocater::CreateResourceDescription(D3D12_RESOURCE_DESC& resDesc)
{
    resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Width = static_cast<UINT64>((1 + 0xff) & ~0xff) * static_cast<int>(m_pCbvHeap->GetUseCount().x);
}
