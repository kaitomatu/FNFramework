#pragma once

namespace CBufferData
{
    enum { NUM_WEIGHTS = 8 };

    struct cbBlur
    {
        float m_weights[NUM_WEIGHTS]; // ガウス関数の重み
    };
}

/**
 * @class GaussianBlur
 * @brief ガウスブラー処理を行うクラス
 */
class GaussianBlur
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    GaussianBlur()
        : m_cbBlur(1)
    {
    }

    ~GaussianBlur()
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
     
    // ブラー画像の取得 //
    const ShaderResourceTexture& GetBlurTexture() const { return m_yBlurData.spRenderTarget->GetTexture(); }
    ShaderResourceTexture& WorkBlurTexture() { return m_yBlurData.spRenderTarget->WorkTexture(); }

    // ブラー用定数バッファの取得 //
    const ConstantBuffer<CBufferData::cbBlur>& GetCBBlur() const { return m_cbBlur; }
    // 作業可能
    ConstantBuffer<CBufferData::cbBlur>& WorkCBBlur() { return m_cbBlur; }

    // ブラーの強度を更新する //
    void UpdateWeights(float _power);
    float GetBlurPower() const { return m_blurPower; }

    //--------------------------------
    // その他関数
    //--------------------------------
    // 初期化 //
    void Init(ShaderResourceTexture& _origTex);

    void Rendering();

private:

    struct BlurRenderingData
    {
        Shader Shader;
        std::shared_ptr<RenderTarget> spRenderTarget;
        std::shared_ptr<SpriteMesh> spSpriteMesh; // 全画面クアッド用のSpriteMesh
    };

    // ブラー処理用のデータの初期化 //
    void InitRenderingData(
        const Math::Vector2& _texSize,
        const std::wstring& _shaderName,
        BlurRenderingData& _blurRenderData);

    // ブラーシェーダの開始処理 //
    void BeginBlurShader(
        Shader _shader,
        const std::shared_ptr<RenderTarget>& _spRT,
        const ShaderResourceTexture& _blurSrcTex);

    ConstantBuffer<CBufferData::cbBlur> m_cbBlur; // ガウス関数の重み用の定数バッファ

    ShaderResourceTexture m_originalTexture; // 加工元のテクスチャ

    BlurRenderingData m_xBlurData; // X方向のブラー処理用データ
    BlurRenderingData m_yBlurData; // Y方向のブラー処理用データ

    float m_blurPower = 1.0f; // ブラーの強度
};
