#pragma once

/**
 * todo : 各シーンにつき一つのCBVSRVUAVHeapを持つ方式に変更する
 *          これにより
 *  各シーン
 */
class CBVSRVUAVHeap
    : public Heap<Math::Vector3>
{
public:
    CBVSRVUAVHeap()
    {
    }

    ~CBVSRVUAVHeap() override
    {
    }

    /**
    * @brief SRVの作成
    *
    * @param pBuffer - バッファのポインタ
    * @result ヒープの紐づけられた登録番号
    */
    int CreateSRV(ID3D12Resource* pBuffer, bool constantData = false);

    /**
     * @brief StructuredBuffer用SRVの作成
     * 
     * @param pBuffer : バッファのポインタ
     * @param NumElements : 要素数
     * @param StructureByteStride : 構造体のサイズ
     * @param constantData : 定数バッファかどうか
     * @return 
     */
    int CreateStructuredBufferSRV(ID3D12Resource* pBuffer, UINT NumElements, UINT StructureByteStride, bool constantData = false);

    /**
    * @brief SRVのGPUアドレスを返す
    *
    * @param number - 登録番号
    * @result SRVのGPう側のアドレス
    */
    const D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int number) override;

    /* @brief ヒープをセットする */
    void SetHeap();

    /**
    * @brief ヒープの取得関数
    * @result ヒープのポインタ
    */
    ID3D12DescriptorHeap* GetHeap() { return m_currentHeapData.pHeap.Get(); }

    /**
    * @brief 使用数を取得
    * @result 使用数
    */
    const Math::Vector3& GetUseCount() const { return m_currentHeapData.UseCount; }
};
