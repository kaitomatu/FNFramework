#pragma once

enum class HeapType
{
    CBVSRVUAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    SAMPLER = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
    RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
};

// todo : m_constantHeapDataIdxを使うことで、管理は楽になったが、定数 -> 通常 -> 定数 といったヒープの使い方ができなくなったので注意
template <typename T>
class Heap
{
public:
    static_assert(std::is_same_v<T, Math::Vector3> || std::is_same_v<T, int>, "TはintかVector3のみ使用可能です");

    struct HeapData
    {
        HeapData()
        {
        }

        ~HeapData()
        {
            pHeap.Reset();
        }

        ComPtr<ID3D12DescriptorHeap> pHeap = nullptr;
        T UseCount = {};
        int IncrementSize = 0;
        int NextRegistNumber = 0;
    };

    //---------------------------------
    // コンストラクタ / デストラクタ
    //---------------------------------
    Heap()
    {
    }

    virtual ~Heap()
    {
    }

    /**
    * @brief リセット
    * @details ヒープの登録番号をリセットするだけなので、実際のGPUヒープはリセットされないことには注意
    */
    void Reset()
    {
        // m_constantHeapDataIdx:3(定数 | 通常 | 定数)この場合上書きは4つめから行われる
        // ヒープ作成時の定数フラグが設定されているリソース以外を上書きする
        m_currentHeapData.NextRegistNumber = m_constantHeapDataIdx + 1;
    }

    //---------------------------------
    // ゲッター / セッター
    //---------------------------------
    /**
    * @brief 現在のヒープデータを返す
    * @return 現在のヒープデータ
    */
    const HeapData& GetCurrentHeapData() const
    {
        return m_currentHeapData;
    }

    /**
    * @brief CPU側のアドレスを返す
    *
    * @param number - 登録番号
    * @result CPU側のアドレス
    */
    virtual const D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int number)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_currentHeapData.pHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<UINT64>(m_currentHeapData.IncrementSize) * number;
        return handle;
    }

    /**
    * @brief GPU側のアドレスを返す関数
    *
    * @param number - 登録番号
    * @result GPU側のアドレス
    */
    virtual const D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int number)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = m_currentHeapData.pHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<UINT64>(m_currentHeapData.IncrementSize) * number;
        return handle;
    }

    /**
     * @brief 定数ヒープデータのインデックスの設定
     * @return 現在の定数ヒープデータのインデックス
     */
    int SetConstantHeapData(int idx)
    {
        // ヒープ領域は1列で管理されないといけないので、インデックスが不正な場合はエラーを出す
        if (m_currentHeapData.NextRegistNumber <= idx)
        {
            FNENG_ASSERT_ERROR("登録番号が不正です。定数データは初期化段階で設定してください。");
            return -1;
        }

        m_constantHeapDataIdx = idx;
        return idx;
    }

    void ResetConstantHeapDataIdx()
    {
        m_constantHeapDataIdx = 0;
    }

    //---------------------------------
    // その他関数
    //---------------------------------
    /**
    * @brief ヒープ作成
    *
    * @param heapType - ヒープのタイプ
    * @param useCount - 使用個数
    * @result 作成できたらtrue
    */
    bool Create(HeapType heapType, T useCount)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

        heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(heapType);
        heapDesc.NodeMask = 0;
        heapDesc.NumDescriptors = ComputeUseCount(useCount);
        heapDesc.Flags = heapType == HeapType::CBVSRVUAV
                             ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                             : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        const auto& pDevice = GraphicsDevice::Instance().GetDevice();

        auto hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_currentHeapData.pHeap));

        if (FAILED(hr)) { return false; }

        m_currentHeapData.UseCount = useCount;
        m_currentHeapData.IncrementSize = pDevice->GetDescriptorHandleIncrementSize(
            static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(heapType));


        return true;
    }

protected:

    HeapData m_currentHeapData;

    // constantHeapDataNum + 1 の要素からヒープに登録される
    int m_constantHeapDataIdx = 0;

private:
    /* カウント数をUINT型で返す */
    UINT ComputeUseCount(UINT useCount)
    {
        return useCount;
    }

    /* カウント数をUINT型で返す */
    UINT ComputeUseCount(Math::Vector3 useCount)
    {
        return static_cast<UINT>(useCount.x + useCount.y + useCount.z);
    }
};
