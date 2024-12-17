#include "Vertices.h"

void Vertices::CreateVertexBuffer(UINT _structSize, UINT _bufSize)
{
    // すでにバッファがある場合は解放して、新たに生成する
    if (m_pVBuffer) { m_pVBuffer->Release(); FNENG_ASSERT_LOG("中身のある頂点バッファを上書きします",/* isOutput = */ true) }

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    //--------------
    // 頂点情報設定
    //--------------
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = static_cast<UINT64>(_structSize * _bufSize);
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = {};
    // 頂点バッファ作成
    hr = GraphicsDevice::Instance().GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE,
                                                                 &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                                 IID_PPV_ARGS(&m_pVBuffer));

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("頂点バッファ作成失敗");
        return;
    }

    // バッファの情報をビューに書き込む
    m_vbView.BufferLocation = m_pVBuffer->GetGPUVirtualAddress(); // 頂点バッファの開始アドレスを設定
    m_vbView.SizeInBytes = static_cast<UINT>(resDesc.Width); // バッファのサイズ(byte)を指定する
    m_vbView.StrideInBytes = _structSize; // バッファの1つのデータのサイズ(byte)をしている
}

void Vertices::CreateIndexBufferAndFaceData(const std::vector<MeshFace>& _faces)
{
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    //--------------
    // 面情報設定
    //--------------
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeof(MeshFace) * _faces.size();
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    const auto& device = GraphicsDevice::Instance().GetDevice();

    if(!device) { FNENG_ASSERT_ERROR("デバイスが無効状態です。"); return; }

    HRESULT hr = device->CreateCommittedResource(&heapProp,
                                    /* heapFlg = */D3D12_HEAP_FLAG_NONE,
                                                 &resDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS(&m_pIBuffer));

    if (FAILED(hr)) { FNENG_ASSERT_ERROR("インデックスバッファ作成失敗"); return; }

    // インデックスバッファのデータをビューに書き込む
    m_ibView.BufferLocation = m_pIBuffer->GetGPUVirtualAddress();
    m_ibView.SizeInBytes = static_cast<UINT>(resDesc.Width);
    m_ibView.Format = DXGI_FORMAT_R32_UINT;

    //---------------------------
    // メッシュ情報格納
    //---------------------------

    // インデックスバッファに情報を描き込む
    MeshFace* ibMap = nullptr;
    {
        m_pIBuffer->Map(0, nullptr, (void**)&ibMap);

        if (FAILED(hr)) { FNENG_ASSERT_ERROR("インデックスバッファマップ失敗"); return; }

        std::copy(std::begin(_faces), std::end(_faces), ibMap);
        m_pIBuffer->Unmap(0, nullptr);

        // 面情報コピー
        m_faces = _faces;
    }
}

void Vertices::DrawInstanced(UINT _vertexCount) const
{
    const auto& pCmdList =  GraphicsDevice::Instance().GetCmdList();
    pCmdList->IASetVertexBuffers(0, 1, &m_vbView);
    pCmdList->DrawInstanced(_vertexCount, 1, 0, 0);
}
