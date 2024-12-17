#pragma once

class DSVHeap
    : public Heap<int>
{
public:
    DSVHeap()
    {
    }

    ~DSVHeap() override
    {
    }

    /**
    * @brief DSVの作成
    *
    * @param  pBuffer - バッファのポインタ
    * @param  format  - フォーマット ※DSVHeap.hに詳しい情報書いてます
    * @param constantData - 定数データかどうか
    * @result ヒープの紐づけられた登録番号
    */
    int CreateDSV(ID3D12Resource* pBuffer, DXGI_FORMAT format, bool constantData = false);
};
