#pragma once

template<typename T>
class ConstantBuffer
{
public:
    ConstantBuffer(int index) : m_index(index) {}
    ~ConstantBuffer() = default;

    // データへの参照を取得
    T& Work() { return m_data; }
    const T& Data() const { return m_data; }

    // データをGPUにバインド
    void Bind()
    {
        // バインド処理を実行
        GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(m_index, m_data);
    }
    void Bind(int _idx)
    {
        // バインド処理を実行
        GraphicsDevice::Instance().GetCBufferAllocater()->BindAttachData(_idx, m_data);
    }

private:
    T m_data;      // 定数バッファのデータ
    int m_index;   // バインドに使用するインデックス
};
