#pragma once

class ShaderResourceTexture
{
public:
    ShaderResourceTexture()
    {
    }

    ShaderResourceTexture(const std::string& filePath)
    {
        m_filePath = filePath;
        Load(filePath);
    }

    /**
    * @brief テクスチャのロード
    *
    * @param filePath		 - ファイルパス
    * @result ロードが成功したらtrue
    */
    bool Load(const std::string& filePath, bool constantData = false);

    /* @brief ディスクリプタヒープにシェーダーリソースとしてセット */
    void Set(int index) const;

    void SetSRVNumber(int number) { m_srvNumber = number; }
    int GetSRVNumber() const { return m_srvNumber; }

    void InitFromD3DResource(ID3D12Resource* buffer, int srvNumber);

    /**
    * @brief  作業不可能なバッファの取得
    * @result 作業不可能なバッファのポインタ
    */
    const ID3D12Resource* GetBuffer() const { return m_pBuffer.Get(); }

    /**
    * @brief  作業可能なバッファの取得
    * @result 作業可能なバッファのポインタ
    */
    ID3D12Resource* WorkBuffer() const { return m_pBuffer.Get(); }

    const D3D12_RESOURCE_DESC& GetDesc() const { return m_bufferDesc; }
    int GetWidth() const { return static_cast<int>(m_bufferDesc.Width); }
    int GetHeight() const { return static_cast<int>(m_bufferDesc.Height); }
    DXGI_FORMAT GetFormat() const { return m_bufferDesc.Format; }

    const std::string& GetFilePath() const { return m_filePath; }
    void SetFilePath(const std::string& filePath) { m_filePath = filePath; }

private:
    std::string m_filePath = "";
    ComPtr<ID3D12Resource> m_pBuffer = nullptr;
    D3D12_RESOURCE_DESC m_bufferDesc = {};
    int m_srvNumber = -1;
};
