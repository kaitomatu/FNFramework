#pragma once

namespace CBufferData
{
    struct cbDeferredObject;
}

/**
 * @class GBufferPass
 * @brief ディファードレンダリングで利用するバッファを作成するシェーダー
 */
class GBufferPass
    : public Shader
{
public:

    GBufferPass()
    : m_cbObject(1)
    {
        Init();
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    bool Begin() override;
    void End() override;

    void Init();

    const ShaderResourceTexture& GetAlbedoGB() const { return m_spAlbedoGB->GetTexture(); }
    const std::shared_ptr<RenderTarget>& GetAlbedoRT() const { return m_spAlbedoGB; }
    const ShaderResourceTexture& GetNormalGB() const { return m_spNormalGB->GetTexture(); }
    const std::shared_ptr<RenderTarget>& GetNormalRT() const { return m_spNormalGB; }
    const ShaderResourceTexture& GetDepthGB() const { return m_spDepthGB->GetTexture(); }
    const std::shared_ptr<RenderTarget>& GetDepthRT() const { return m_spDepthGB; }

    void DrawModelInstanced(
        const std::shared_ptr<ModelData>& modelData,
        const Renderer::InstancedRenderEntry& instanceDataEntry);

private:
    struct GPUBoneData
    {
        // ボーン行列バッファ関連
        ComPtr<ID3D12Resource> pBoneMatrixBuffer;
        UINT BoneMatrixBufferSize = 0;

        // SRVの登録番号
        int BoneMatrixSRVIndex = -1;
    };

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
     * @fn void SetMaterial(const Material& _material, const Math::Vector4& colRate)
     * @brief マテリアルをセット
     *
     * @param _material - マテリアル情報
     * @param colRate  - 色倍率
     */
    void SetMaterial(const Material& _material);

    void UploadBoneMatrices(
        const std::vector<Math::Matrix>& allBoneMatrices,
        UINT numBonesPerInstance,
        GPUBoneData& _boneData);
    void BindBoneMatricesSRV(UINT _srvIdx);
    
    std::shared_ptr<RenderTarget> m_spAlbedoGB = nullptr;
    std::shared_ptr<RenderTarget> m_spNormalGB = nullptr;
    std::shared_ptr<RenderTarget> m_spDepthGB = nullptr;

    ConstantBuffer<CBufferData::cbDeferredObject> m_cbObject;

    // メッシュ単位でのボーン情報
    std::unordered_map<std::shared_ptr<ModelData>, GPUBoneData> m_umModelDataToBoneData;
};

/**
 * @class LightingPass
 * @brief ディファードレンダリングを行うシェーダー
 */
class LightingPass
    : public Shader
{
public:

    LightingPass()
    {
        Init();
    }

    const std::shared_ptr<RenderTarget>& GetMainRenderTarget() const { return m_spMainRenderTarget; }
    std::shared_ptr<RenderTarget>& WorkMainRenderTarget() { return m_spMainRenderTarget; }
    const ShaderResourceTexture& GetMainRenderTexture() const { return m_spMainRenderTarget->GetTexture(); }

    //--------------------------------
    // その他関数
    //--------------------------------
    void Rendering();

    void Init();

private:
    bool Begin() override;
    void End() override;

    std::shared_ptr<SpriteMesh> m_spSpriteMesh; // 全画面クアッド用のSpriteMesh
    std::shared_ptr<RenderTarget> m_spMainRenderTarget = nullptr;
};
