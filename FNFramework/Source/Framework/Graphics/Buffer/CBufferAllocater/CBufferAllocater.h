#pragma once

class CBufferAllocater
{
public:
    /**
    * @brief 作成
    *
    * @param pHeap			 - CBVSRVUAVHeapのポインタ
    */
    void Create(CBVSRVUAVHeap* pHeap);

    /**
     * @brief 使用しているバッファの番号を初期化
     * */
    void ResetCurrentUseNumber();

    /**
    * @brief 定数バッファにデータのバインドを行う
    *
    * @param descIndex - レジスタ番号
    * @param data	   - バインドデータ
    *
    * ToDo : 各種シェーダーのCBVCountを参照して、描画時に少なければ警告を出すようにする
    */
    template <typename T>
    void BindAttachData(int descIndex, const T& data);

private:
    /* @brief バッファリソースの作成 */
    bool CreateBufferResource();
    /* @brief ヒープのプロパティ設定 */
    void CreateHeapProperties(D3D12_HEAP_PROPERTIES& heapProp);
    /* @brief リソースデスクの作成 */
    void CreateResourceDescription(D3D12_RESOURCE_DESC& resDesc);

    CBVSRVUAVHeap* m_pCbvHeap = nullptr;

    // GPUに転送するためのバッファ
    ComPtr<ID3D12Resource> m_pBuffer = nullptr;

    // CPUで操作するための作業用バッファ
    struct
    {
        char buf[256];
    }* m_pMappedBuffer = nullptr;

    int m_currentUseNumber = 0;
};

/**
*	DirectX12ではConstantsBufferは256アラインメント
*/
template <typename T>
void CBufferAllocater::BindAttachData(int descIndex, const T& data)
{
    if (!m_pCbvHeap)return;

    const auto& pDevice = GraphicsDevice::Instance().GetDevice();

    // dataサイズを256アラインメントして計算
    int sizeAligned = (sizeof(T) + 0xff) & ~0xff;

    // 256byteをいくつ使用するかアラインメントした結果を256で割って計算
    int useValue = sizeAligned / 0x100;

    // 現在使い終わっている番号と今から使う容量がヒープの容量を超えている場合はリターン
    if (m_currentUseNumber + useValue > static_cast<int>(m_pCbvHeap->GetUseCount().x))
    {
        FNENG_ASSERT_ERROR("使用できるヒープ容量を超えました");
        return;
    }

    int top = m_currentUseNumber;

    // 先頭アドレスに使う分のポインタを足してから、メモリをコピー
    std::memcpy(m_pMappedBuffer + top, &data, sizeof(T));

    // ビューを使って値をシェーダーにアタッチ
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc = {};
    cbDesc.BufferLocation = m_pBuffer->GetGPUVirtualAddress() + static_cast<UINT64>(top) * 0x100;
    cbDesc.SizeInBytes = sizeAligned;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pCbvHeap->GetCurrentHeapData().pHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += static_cast<UINT64>(pDevice->GetDescriptorHandleIncrementSize
        (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) * m_currentUseNumber;

    pDevice->CreateConstantBufferView(&cbDesc, cpuHandle);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pCbvHeap->GetCurrentHeapData().pHeap->GetGPUDescriptorHandleForHeapStart();
    gpuHandle.ptr += static_cast<UINT64>(pDevice->GetDescriptorHandleIncrementSize
        (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) * m_currentUseNumber;

    GraphicsDevice::Instance().GetCmdList()->SetGraphicsRootDescriptorTable(descIndex, gpuHandle);

    m_currentUseNumber += useValue;
}
