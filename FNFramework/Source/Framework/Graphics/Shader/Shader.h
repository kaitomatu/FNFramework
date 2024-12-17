#pragma once

#include "PipeLine/PipeLine.h"
#include "RootSignature/RootSignature.h"

struct RenderingSetting
{
    std::vector<InputLayout> InputLayouts; // レイアウトの配列
    std::vector<DXGI_FORMAT> Formats; // フォーマット
    CullMode CullMode = CullMode::Back; // カリングモード
    BlendMode BlendMode = BlendMode::Alpha; // ブレンドモード
    PrimitiveTopologyType TopologyType = PrimitiveTopologyType::Triangle; // トポロジータイプ
    bool IsDepth = true; // 深度値の更新を行うか
    bool IsDepthMask = true; // 深度テクスチャへの描き込みを行うか
    DXGI_FORMAT DSVFormat = DXGI_FORMAT_D32_FLOAT; // DepthStencilViewのフォーマット
    int RTVCount = 1; // RenderTargetViewの数
    bool IsWireFrame = false; // ワイヤーフレーム表示するか

    bool UseInstanceData = false; // インスタンスデータを使用するか
};

class Shader
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Shader()
    {
    }

    virtual ~Shader()
    {
    }


    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // このシェーダーで登録されている定数バッファの数を取得
    UINT GetCBVCount() const { return m_cbvCount; }

    // このシェーダーのViewPortを取得
    const D3D12_VIEWPORT& GetViewPort() const { return m_viewPort; }
    // このシェーダーのScissorRectを取得
    const D3D12_RECT& GetScissorRect() const { return m_rect; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief 作成
    *
    * @param filePath			- ファイルパス
    * @param renderingSetting	- 描画設定
    * @param rangeTypes			- レンジタイプ
    */
    void Create(const std::wstring& filePath, const RenderingSetting& renderingSetting,
                const std::vector<RangeType>& rangeTypes);

    virtual bool Begin() { return false; };
    virtual void End() {}

    /**
    * @brief 描画開始
    *
    * @param w - 横幅
    * @param h - 縦幅
    */
    void Begin(float w, float h);

protected:

    /**
    * @brief シェーダーファイルのロード
    *
    * @param filePath - ファイルパス
    */
    void LoadShaderFile(const std::wstring& filePath);

    std::shared_ptr<PipeLine> m_spPipeline = nullptr;
    std::shared_ptr<RootSignature> m_spRootSignature = nullptr;

    ID3DBlob* m_pVSBlob = nullptr; // 頂点シェーダー
    ID3DBlob* m_pHSBlob = nullptr; // ハルシェーダー
    ID3DBlob* m_pDSBlob = nullptr; // ドメインシェーダー
    ID3DBlob* m_pGSBlob = nullptr; // ジオメトリシェーダー
    ID3DBlob* m_pPSBlob = nullptr; // ピクセルシェーダー

    UINT m_cbvCount = 0; // 現在設定されているCBVの数

    // ビューポート / シザリング矩形
    D3D12_VIEWPORT m_viewPort = {};
    D3D12_RECT m_rect = {};

    std::string m_shaderName;
};
