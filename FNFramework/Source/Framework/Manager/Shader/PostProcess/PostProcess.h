#pragma once

namespace CBufferData
{
    struct cbBloom
    {
        float BloomIntensity = 0.2f;
        float padding[3];
    };

    struct cbToneMappingParam
    {
        // 露出度
        float Exposure = 0.350f;
        float padding[3];
    };
}

//x--------------------x//
//   ブルーム処理
// - ブルーム処理を行うクラス
// - 輝度抽出 -> ぼかし -> 合成を行う
// ※ 複数のシェーダーをまたぐため、このクラス自体はShaderを継承しないでメンバに保持する
//x--------------------x//
class BloomPass
{
public:

    BloomPass()
        : m_cbBloom(1)
        , m_cbToneMappingParam(1)
    {
        Init();
    }

    const std::shared_ptr<RenderTarget>& GetLuminanceRenderTarget() const { return m_spLuminanceRenderTarget; }
    const ShaderResourceTexture& GetLuminanceRenderTexture() const { return m_spLuminanceRenderTarget->GetTexture(); }

    const std::shared_ptr<RenderTarget>& GetBloomRenderTarget() const { return m_spBloomRenderTarget; }
    const ShaderResourceTexture& GetBloomRenderTexture() const { return m_spBloomRenderTarget->GetTexture(); }

    const GaussianBlur& GetGaussianBlur() const { return m_gaussianBlur; }
    GaussianBlur& WorkGaussianBlur() { return m_gaussianBlur; }

    void UpdateWeights(float _power)
    {
        m_gaussianBlur.UpdateWeights(_power);
    }

    // ブルームの強度を設定
    void SetBloomIntensity(float _intensity)
    {
        m_cbBloom.Work().BloomIntensity = _intensity;
    }
    float GetBloomIntensity() const
    {
        return m_cbBloom.Data().BloomIntensity;
    }

    // 露出度の調整
    void SetExposure(float _exposure)
    {
        m_cbToneMappingParam.Work().Exposure = _exposure;
    }
    float GetExposure() const
    {
        return m_cbToneMappingParam.Data().Exposure;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    void Rendering();

    void Init();

private:

    std::shared_ptr<SpriteMesh> m_spSpriteMesh; // 全画面クアッド用のSpriteMesh

    // 輝度抽出用のリソース //
    void InitLuminanceShader();
    void RenderingLuminancePass();
    Shader m_luminanceShader;
    std::shared_ptr<RenderTarget> m_spLuminanceRenderTarget = nullptr;

    // ガウシアンブラー //
    GaussianBlur m_gaussianBlur;

    // ブルーム処理用のリソース //
    void InitBloomShader();
    void RenderingBloomPass();
    Shader m_bloomShader;
    ConstantBuffer<CBufferData::cbBloom> m_cbBloom;

    std::shared_ptr<RenderTarget> m_spBloomRenderTarget = nullptr;

    // トーンマッピング用のリソース //
    void InitToneMappingShader();
    void RenderingToneMappingPass();
    Shader m_toneMappingShader;
    ConstantBuffer<CBufferData::cbToneMappingParam> m_cbToneMappingParam;
};
