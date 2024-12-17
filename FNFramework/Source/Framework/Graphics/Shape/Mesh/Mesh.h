#pragma once

#include "../Vertices/Vertices.h"

//--------------------------------
// メッシュの頂点情報
//--------------------------------
struct MeshVertex
{
    Math::Vector3 Position;     // 座標
    Math::Vector2 UV;           // uv
    // todo : InstanceData と重複するため削除する予定
    UINT Color = 0xFFFFFFFF;    // 色
    Math::Vector3 Normal;       // 法線
    Math::Vector3 Tangent;      // 法線の接ベクトル

    std::array<short, 4>	BoneIDs;		// スキニングIndexリスト
    std::array<float, 4>	BoneWeights;		// スキニングウェイトリスト
};

//==========================================================
// メッシュ用 サブセット情報
//==========================================================
struct MeshSubset
{
    UINT		MaterialNo = 0;		// マテリアルNo

    UINT		FaceStart = 0;		// 面Index　このマテリアルで使用されている最初の面のIndex
    UINT		FaceCount = 0;		// 面数　FaceStartから、何枚の面が使用されているかの
};

// Todo : マテリアルの情報はここで管理するのではなく、一つ上のNodeで管理するようにする
struct Material
{
    void SetTextures(const std::shared_ptr<ShaderResourceTexture>& spBaseColTex,
        const std::shared_ptr<ShaderResourceTexture>& spMtRfColTex, const std::shared_ptr<ShaderResourceTexture>& spEmiColTex,
        const std::shared_ptr<ShaderResourceTexture>& spNmlColTex);

    void SetTextures(const std::string& fileDir, const std::string& baseColName,
        const std::string& mtRfColName, const std::string& emiColName, const std::string& nmlColName);

    std::string Name = ""; // マテリアルの名前
    std::shared_ptr<ShaderResourceTexture> spBaseColorTex = nullptr; // 基本色のテクスチャ
    Math::Vector4 BaseColor = Color::White; // 基本色のスケーリング係数(RGBA)

    std::shared_ptr<ShaderResourceTexture> spMetallicRoughnessTex = nullptr; // B : 金属製 | G : 粗さ
    float Metallic = 0.0f; // 金属のスケーリング係数
    float Roughness = 1.0f; // 粗さのスケーリング係数

    std::shared_ptr<ShaderResourceTexture> spEmissiveTex = nullptr; // 自己発光テクスチャ
    Math::Vector3 Emissive = {0.0f, 0.0f, 0.0f}; // 自己発光のスケーリング係数(RGB)

    std::shared_ptr<ShaderResourceTexture> spNormalTex = nullptr; // 法線テクスチャ
};

struct InstanceData
{
    Math::Matrix mWorld;
    Math::Vector4 TilingOffset = { 1.0f, 1.0f, 0.0f, 0.0f };
    Math::Vector4 Color = Color::White;
};

/**
* @class Mesh
* @brief メッシュクラス
*/
class Mesh
    : public Vertices
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
    * @brief インスタンス数を取得
    * @result インスタンス数
    */
    UINT GetInstanceCount() const
    {
        return m_instanceCount;
    }

    // todo : フレームワークの回収により不要かも。ブレークポイントを貼って確認語通ってないようなら削除
    /**
    * @brief 頂点座標の配列を取得
    * @result 頂点座標の配列
    */
    const std::vector<Math::Vector3>& GetPositions() const
    {
        return m_positions;
    }
    // 座標配列のクリア
    void ClearPositions()
    {
        m_positions.clear();
    }

    /**
    * @brief オブジェクト空間(モデル)のAABBを取得
    * @result オブジェクト空間(モデル)のAABB
    */
    const DirectX::BoundingBox& GetBoundingBox() const
    {
        return m_boundingBox;
    }

    const DirectX::BoundingSphere& GetBoundingSphere()
    {
        return m_boundingSphere;
    }


    const std::vector<MeshSubset>&	GetSubsets() const { return m_subsets; }


    //--------------------------------
    // その他関数
    //--------------------------------
    /**
     * @brief 作成
     *
     * @param vertices       - 頂点情報
     * @param faces          - 面情報
     * @param subsets        - サブセット情報
     * @param isSkinMesh     - スキンメッシュかどうか
     */
    void Create(const std::vector<MeshVertex>& vertices,
        const std::vector<MeshFace>& faces,
        const std::vector<MeshSubset>& subsets,
        bool isSkinMesh);

    /**
     * @brief 頂点バッファ生成
     * @param[in] vertices - 頂点データ
     */
    void CreateVertexBuffers(const std::vector<MeshVertex>& vertices);

    /**
     * @brief バッファの更新
     * @param[in] srcDatas - 更新するデータ
     */
    void UpdateBuffer(const std::vector<MeshVertex>& srcDatas);

    // インスタンスバッファの初期化
    bool InitializeInstanceBuffer(UINT initialInstanceCount = 100);

    // インスタンスバッファの更新（自動リサイズ対応）
    void UpdateInstanceBuffer(const std::vector<InstanceData>& instanceData);

    // インスタンス描画
    void DrawInstanced(UINT instanceCount) const override;

    /**
     * @brief 指定サブセットを描画
     * @param subsetNo - サブセット番号
     */
    void DrawSubset(UINT subsetNo) const;
    void DrawSubsetInstanced(UINT subsetIndex, UINT instanceCount) const;

private:

    // サブセット情報
    std::vector<MeshSubset>	m_subsets;

    std::vector<Math::Vector3> m_positions; // 頂点座標(copy)

    UINT m_instanceCount = 0; // インスタンス数

    // 境界データ
    DirectX::BoundingBox m_boundingBox; // バウンディングボックス
    DirectX::BoundingSphere m_boundingSphere; // バウンディングスフィア

    bool						m_isSkinMesh = false;

    // インスタンスバッファ関連
    ComPtr<ID3D12Resource> m_pInstanceBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_instanceBufferView = {};

    // 現在のバッファ容量
    UINT m_currentInstanceCapacity = 0;

    // インスタンスバッファのリソースを作成
    void CreateInstanceBufferResource(UINT bufferSize);
};
