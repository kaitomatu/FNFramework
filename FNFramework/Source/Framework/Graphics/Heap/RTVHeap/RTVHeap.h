#pragma once

class RTVHeap
    : public Heap<int>
{
public:
    RTVHeap()
    {
    }

    ~RTVHeap() override
    {
    }

    /**
    * @brief RTVの作成
    *
    * @param pBuffer - バッファのポインタ
    * @param format - フォーマット
    * @param constantData - 定数データかどうか
    * @result ヒープの紐づけられた登録番号
    */
    int CreateRTV(ID3D12Resource* pBuffer, DXGI_FORMAT format, bool constantData = false);
};
