#pragma once

/*
* SRV(Texture), RTV(Texture), DSV(DepthStencil)を管理するクラス
* ToDo : 現状必ず深度バッファを作成する方法しかないが、これは無駄が多いので
*/

class RenderTarget
{
public:
    ////////////////////////
    // テクスチャ情報
    ////////////////////////
    int GetTexWidth() const
    {
        return m_width;
    }

    int GetTexHeight() const
    {
        return m_height;
    }

    const Math::Color& GetClearColor() const
    {
        return m_rtvClearColor;
    }

    void SetClearColor(const Math::Color& col)
    {
        m_rtvClearColor = col;
    }

    ////////////////////////
    // ヒープ関連
    ////////////////////////
    UINT GetRTVNumber() const
    {
        return m_rtvNumber;
    }

    UINT GetSRVNumber() const
    {
        return m_renderTargetTexture.GetSRVNumber();
    }

    UINT GetDSVNumber() const
    {
        return m_depthStencil.GetDSVNumber();
    }

    ////////////////////////
    // バッファ
    ////////////////////////
    const ShaderResourceTexture& GetTexture() const
    {
        return m_renderTargetTexture;
    }
    ShaderResourceTexture& WorkTexture()
    {
        return m_renderTargetTexture;
    }

    void SetRootDescriptorTable(const class Shader& shader, int index);

    const DepthStencil& GetDepthStencil() const
    {
        return m_depthStencil;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief レンダリングターゲットの作成
    * @param[in] w - レンダリングターゲットの幅
    * @param[in] h - レンダリングターゲットの高さ
    * @param[in] mipLevel - レンダリングターゲットのミップマップレベル
    * @param[in] arraySize - レンダリングターゲットの配列サイズ
    * @param[in] colorFormat - レンダリングターゲットのフォーマット
    * @param[in] depthFormat - 深度ステンシルバッファのフォーマット ※DXGI_FORMAT_UNKNOWNでDSを作成しない
    * @param[in] clearCol - レンダリングターゲットのクリアカラー
    * @param[in] constantData - 定数データを作成するかどうか
    * @return 作成に成功したらtrue
    */
    bool Create(int w, int h,
        int mipLevel, int arraySize,
        DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT depthFormat = DXGI_FORMAT_R32_TYPELESS,
        const Math::Color& clearCol = Color::Red,
        bool constantData = false);

    void ClearDSV()
    {
        if (!m_depthStencil.IsCreate()) { return; }

        m_depthStencil.ClearDSV();
    }

    /**
    * @brief レンダリングターゲットのクリア
    * @brief 設定された初期化カラーでクリアする
    */
    void ClearRTV();

private:
    /**
    * @brief レンダリングターゲットの作成
    * @param[in] w - レンダリングターゲットの幅
    * @param[in] h - レンダリングターゲットの高さ
    * @param[in] mipLevel - レンダリングターゲットのミップマップレベル
    * @param[in] arraySize - レンダリングターゲットの配列サイズ
    * @param[in] format - レンダリングターゲットのフォーマット
    * @param[in] clearCol - レンダリングターゲットのクリアカラー
    * @return 作成に成功したらtrue
    */
    bool CreateRTTex(int w, int h,
        int mipLevel, int arraySize,
        DXGI_FORMAT format,
        const Math::Vector4& clearCol,
        bool constantData = false);


    ShaderResourceTexture m_renderTargetTexture;

    DepthStencil m_depthStencil; // 深度ステンシルバッファ

    ComPtr<ID3D12Resource> m_pRenderTargetResource = nullptr; // レンダリングターゲットリソース
    UINT m_rtvNumber = 0; // レンダリングターゲットビューの登録番号

    int m_width = 0; // レンダリングターゲットの幅
    int m_height = 0; // レンダリングターゲットの高さ
    Math::Color m_rtvClearColor = Color::Red; // レンダリングターゲットのクリアカラー
};
