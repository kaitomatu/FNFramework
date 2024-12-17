#pragma once

//--------------------------------
// メッシュの面情報
//--------------------------------
struct MeshFace
{
    UINT Idx[3];
};

/**
* @class Vertices
* @brief 頂点クラス
* @brief 頂点情報をもっていて主に描画などに使用する
*/
class Vertices
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Vertices()
    {
    }

    virtual ~Vertices()
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
    * @brief インスタンス数を取得
    * @result インスタンス数
    */
    UINT GetVertexCount() const { return m_vertexCount; }

    /**
     * @brief 頂点バッファ本体を取得
     * @return 頂点バッファ
     */
    ID3D12Resource* GetVertexBuffer() const
    {
        return m_pVBuffer.Get();
    }

    /**
    * @brief 面の配列を取得
    * @result 面の配列
    */
    const std::vector<MeshFace>& GetFaces() const
    {
        return m_faces;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
     * @fn void CreateVertexBuffer(GraphicsDevice* pGraphicsDevice, UINT bufSize)
     * @brief 頂点バッファ生成 - バッファの作成のみ行いたい場合
     * @param[in] _pGraphicsDevice - グラフィックデバイス
     * @param[in] _structSize - 構造体サイズ
     * @param[in] _bufSize - バッファサイズ
    */
    void CreateVertexBuffer(UINT _structSize, UINT _bufSize);

    /**
     * @fn void CreateIndexBuffer()
     * @param _faces - 面情報
     * @brief インデックスバッファ生成と面情報の作成
     */
    void CreateIndexBufferAndFaceData(const std::vector<MeshFace>& _faces);

    /**
    * @brief インスタンス描画
    * @param[in] _vertexCount - 頂点数
    */
    virtual void DrawInstanced(UINT _vertexCount) const;


protected:

    //--------------------------------
    // 頂点バッファ / 頂点情報
    //--------------------------------
    ComPtr<ID3D12Resource> m_pVBuffer = nullptr; // 頂点バッファ
    D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
    UINT m_vertexCount = 0; // インスタンス数


    //--------------------------------
    // インデックスバッファ / 面情報
    //--------------------------------
    ComPtr<ID3D12Resource> m_pIBuffer = nullptr; // インデックスバッファ
    D3D12_INDEX_BUFFER_VIEW m_ibView = {};
    std::vector<MeshFace> m_faces; // 面情報(copy)
};
