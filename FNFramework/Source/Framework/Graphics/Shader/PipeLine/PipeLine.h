#pragma once

// カリングの方法設定
enum class CullMode
{
    None = D3D12_CULL_MODE_NONE, // なにもしない
    Front = D3D12_CULL_MODE_FRONT, // 前面カリング
    Back = D3D12_CULL_MODE_BACK, // 背面カリング
};

// 色合成の方法
enum class BlendMode
{
    Add, // 加算合成
    Alpha, // 不透明度合成
};

//  頂点レイアウト
enum class InputLayout
{
    POSITION, // 頂点座標
    TEXCOORD, // テクスチャ座標
    NORMAL, // 法線ベクトル
    TANGENT, // 法線ベクトルに垂直な接線ベクトル
    COLOR, // 頂点の色
    SKININDEX, // スキニング用のボーンインデックス
    SKINWEIGHT, // スキニング用のボーンの重み
};

// プリミティブトポロジー
enum class PrimitiveTopologyType
{
    Undefined = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, // 未定義のトポロジータイプ
    Point = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, // 単一の点 - 頂点毎に単一の点として描画されます。
    Line = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, // 線分     - 2つの頂点が1つの線分として描画されます。
    Triangle = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // 三角形   - 3つの頂点が1つの三角形として描画されます。
    Patch = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, // パッチ   - テッセレーションされる頂点の集合。DirectXのテッセレーションに必要。
};

class RootSignature;

class PipeLine
{
public:
    /**
    * @brief 描画設定のセット
    *
    * @param[in] pRootSignature     - ルートシグネチャのポインタ
    * @param[in] inputLayouts       - 頂点レイアウト
    * @param[in] cullMode           - カリングモード
    * @param[in] blendMode          - ブレンドモード
    * @param[in] topologyType       - プリミティブトポロジー
    */
    void SetRenderSetting(RootSignature* pRootSignature,
                          const std::vector<InputLayout>& inputLayouts,
                          CullMode cullMode, BlendMode blendMode, PrimitiveTopologyType topologyType);

    /**
    * @brief 作成
    *
    * @param[in] pBlobs         - シェーダーデータリスト
    * @param[in] formats        - フォーマットリスト
    * @param[in] isDepth        - 深度テスト
    * @param[in] isDepthMask    - 深度書き込み
    * @param[in] dsvFormat      - DSVフォーマット
    * @param[in] rtvCount       - RTV数
    * @param[in] isWireFrame    - ワイヤーフレームかどうか
    */
    void Create(
        std::vector<ID3DBlob*> pBlobs,
        std::vector<DXGI_FORMAT> formats,
        bool isDepth,
        bool isDepthMask,
        DXGI_FORMAT dsvFormat, 
        int rtvCount,
        bool isWireFrame, bool useInstanceData);

    /**
    * @brief パイプラインの取得
    * @result パイプラインのポインタ
    */
    ID3D12PipelineState* GetPipeline()
    {
        return m_pPipelineState.Get();
    }

    /**
    * @brief トポロジータイプの取得
    * @result 現在のトポロジータイプ
    */
    PrimitiveTopologyType GetTopologyType()
    {
        return m_topologyType;
    }

private:
    /**
    * @brief インプットレイアウトの設定
    *
    * @param[out] inputElements      - 頂点のフォーマットとメモリレイアウトを定義する構造体のリスト
    * @param[in] inputLayouts       - 使用されるインプットレイアウトのリスト
    */
    void SetInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& inputeElements,
                        const std::vector<InputLayout>& inputLayouts, bool useInstanceData);

    /**
    * @brief ブレンドモードのセット
    *
    * @param[out] blendDesc      - レンダーターゲットのブレンド情報
    * @param[in] blendMode      - セットしたいブレンドモード
    */
    void SetBlendMode(D3D12_RENDER_TARGET_BLEND_DESC& blendDesc, BlendMode blendMode);

    RootSignature* m_pRootSignature = nullptr; //  ルートシグネチャ

    std::vector<InputLayout> m_inputLayouts; //  頂点レイアウトの配列
    CullMode m_cullMode; //  カリングモード
    BlendMode m_blendMode; //  ブレンドモード
    PrimitiveTopologyType m_topologyType; //  プリミティブトポロジータイプ

    ComPtr<ID3D12PipelineState> m_pPipelineState = nullptr; //  パイプラインステート
};
