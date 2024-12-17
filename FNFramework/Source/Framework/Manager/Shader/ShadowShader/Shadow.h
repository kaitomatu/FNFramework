#pragma once

class Shadow
    :public Shader
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Shadow()
        : m_cbObject(1)
    { Init(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    const std::shared_ptr<RenderTarget>& GetShadowMap() const { return m_spShadowMap; }
    const Math::Matrix& GetShadowProj() const { return m_mShadowProj; }
    float GetDirLightHeight() const { return m_dirLigHeight; }

    //--------------------------------
    // その他関数
    //--------------------------------
    bool Begin() override;
    void End() override;

    void Init();

    void DrawModelInstanced(
        const std::shared_ptr<ModelData>& modelData,
        const std::vector<InstanceData>& instanceDataList,
        const std::vector<ModelWork*>& modelWorks);

private:

    /* @biref 影生成エリアの設定 */
    void SetCBShadowArea(const Math::Matrix& mProj, float ligHeight);
    // 影描画エリアの定数バッファのセット
    bool SetCBShadowAreaData(std::string_view camName);

    bool CreateShadowMap();

    // 影用レンダーターゲット
    std::shared_ptr<RenderTarget> m_spShadowMap = nullptr;

    // 影用プロジェクション行列
    Math::Matrix m_mShadowProj;
    // 影のビュー行列をどの高さから作成するか
    float m_dirLigHeight = 0.0f;

    // 定数バッファ
    struct GPUBoneData
    {
        // ボーン行列バッファ関連
        ComPtr<ID3D12Resource> pBoneMatrixBuffer;
        UINT BoneMatrixBufferSize = 0;

        // SRVの登録番号
        int BoneMatrixSRVIndex = -1;
    };

    void UploadBoneMatrices(
        const std::vector<Math::Matrix>& allBoneMatrices,
        UINT numBonesPerInstance,
        GPUBoneData& _boneData);
    void BindBoneMatricesSRV(UINT _srvIdx);

    // メッシュ単位でのボーン情報
    std::unordered_map<std::shared_ptr<ModelData>, GPUBoneData> m_umModelDataToBoneData;
    ConstantBuffer<CBufferData::cbDeferredObject> m_cbObject;

};
