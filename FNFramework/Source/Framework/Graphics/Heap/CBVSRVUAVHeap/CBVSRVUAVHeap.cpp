#include "CBVSRVUAVHeap.h"

int CBVSRVUAVHeap::CreateSRV(ID3D12Resource* pBuffer, bool constantData)
{
    if (m_currentHeapData.UseCount.y < m_currentHeapData.NextRegistNumber)
    {
        FNENG_ASSERT_ERROR("確保済みのヒープ領域を超えました");
        return 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_currentHeapData.pHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (static_cast<UINT64>(m_currentHeapData.UseCount.x) + 1) * m_currentHeapData.IncrementSize +
        static_cast<UINT64>(m_currentHeapData.NextRegistNumber) * m_currentHeapData.IncrementSize;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = pBuffer->GetDesc().Format;

    if (pBuffer->GetDesc().Format == DXGI_FORMAT_R32_TYPELESS)
    {
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    }

    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    GraphicsDevice::Instance().GetDevice()->CreateShaderResourceView(pBuffer, &srvDesc, handle);

    const int registNumber = m_currentHeapData.NextRegistNumber++;

    // 定数データである場合は、リセットされないようにする
    if (constantData)
    {
        SetConstantHeapData(registNumber);
    }

    return registNumber;
}

int CBVSRVUAVHeap::CreateStructuredBufferSRV(ID3D12Resource* pBuffer, UINT NumElements, UINT StructureByteStride,
    bool constantData)
{
    if (m_currentHeapData.UseCount.y < m_currentHeapData.NextRegistNumber)
    {
        FNENG_ASSERT_ERROR("確保済みのヒープ領域を超えました");
        return 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_currentHeapData.pHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (static_cast<UINT64>(m_currentHeapData.UseCount.x) + 1) * m_currentHeapData.IncrementSize +
        static_cast<UINT64>(m_currentHeapData.NextRegistNumber) * m_currentHeapData.IncrementSize;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = NumElements;
    srvDesc.Buffer.StructureByteStride = StructureByteStride;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    GraphicsDevice::Instance().GetDevice()->CreateShaderResourceView(pBuffer, &srvDesc, handle);

    const int registNumber = m_currentHeapData.NextRegistNumber++;

    // 定数データである場合は、リセットされないようにする
    if (constantData)
    {
        SetConstantHeapData(registNumber);
    }

    return registNumber;
}

const D3D12_GPU_DESCRIPTOR_HANDLE CBVSRVUAVHeap::GetGPUHandle(int number)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_currentHeapData.pHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(m_currentHeapData.IncrementSize) * (static_cast<UINT64>(m_currentHeapData.UseCount
        .x) + 1);
    handle.ptr += static_cast<UINT64>(m_currentHeapData.IncrementSize) * number;
    return handle;
}

void CBVSRVUAVHeap::SetHeap()
{
    ID3D12DescriptorHeap* ppHeaps[] = {m_currentHeapData.pHeap.Get()};
    GraphicsDevice::Instance().GetCmdList()->SetDescriptorHeaps(1, ppHeaps);
}
