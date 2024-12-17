#include "ShaderResourceTexture.h"

bool ShaderResourceTexture::Load(const std::string& filePath, bool constantData)
{
    wchar_t wFilePath[128];
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, filePath.c_str(), -1, wFilePath, _countof(wFilePath));
    // ファイルパスが長すぎたら
    if (requiredSize > _countof(wFilePath))
    {
        FNENG_ASSERT_ERROR("ファイルパスが長すぎます");
        return false;
    }

    DirectX::TexMetadata metadata = {};
    DirectX::ScratchImage srcratchImage = {};
    const DirectX::Image* pImage = nullptr;

    auto hr = LoadFromWICFile(wFilePath, DirectX::WIC_FLAGS_NONE, &metadata, srcratchImage);

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("テクスチャの読み込み失敗");
        return false;
    }

    pImage = srcratchImage.GetImage(0, 0, 0);

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
    resDesc.Format = metadata.format;
    resDesc.Width = metadata.width;
    resDesc.Height = static_cast<UINT>(metadata.height);
    resDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
    resDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
    resDesc.SampleDesc.Count = 1;
    // バッファを作成
    hr = GraphicsDevice::Instance().GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
                                                                 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                                 IID_PPV_ARGS(&m_pBuffer));

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("テクスチャバッファ作成失敗");
        return false;
    }
    // データ描き込み
    hr = m_pBuffer->WriteToSubresource(0, nullptr, pImage->pixels, static_cast<UINT>(pImage->rowPitch),
                                       static_cast<UINT>(pImage->slicePitch));
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("バッファにテクスチャデータの書き込み失敗");
        return false;
    }

    // バッファを作成
    m_srvNumber = GraphicsDevice::Instance().GetCBVSRVUAVHeap()->CreateSRV(m_pBuffer.Get(), constantData);

    m_bufferDesc = m_pBuffer->GetDesc();
    return true;
}

void ShaderResourceTexture::Set(int index) const
{
    // ヒープ上に登録されていない(m_srvNumberが無効な)場合エラーを出す
    if (m_srvNumber < 0)
    {
        FNENG_ASSERT_ERROR("m_srvNumberが無効な値です");
        return;
    }

    GraphicsDevice::Instance().GetCmdList()->SetGraphicsRootDescriptorTable
        (index, GraphicsDevice::Instance().GetCBVSRVUAVHeap()->GetGPUHandle(m_srvNumber));
}

void ShaderResourceTexture::InitFromD3DResource(ID3D12Resource* pBuffer, int srvNumber)
{
    if (!pBuffer)
    {
        FNENG_ASSERT_ERROR("バッファがnullptrです");
        m_pBuffer.Reset();
        return;
    }

    m_pBuffer = pBuffer;
    m_bufferDesc = pBuffer->GetDesc();

    m_srvNumber = srvNumber;
}
