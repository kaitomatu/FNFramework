#pragma once

/*==================================
*	モデル出力用シェーダー
==================================*/
class ModelShader
    : public Shader
{
public:
    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    ModelShader()
        : m_cbObject(2)
        , m_cbMaterial(1)
    { Init(); }
    ~ModelShader() {}

    bool Begin() override;

    /**
     * @fn void DrawModel(ModelWork& _modelData, const Math::Matrix& _mWorld, const Math::Vector4& _colRate)
     * @brief モデルの描画
     *
     * @param _modelData - モデルデータ
     * @param _mWorld    - ワールド行列
     * @param _colRate   - 色倍率
     */
    void DrawModel(ModelWork& _modelData,
        const Math::Matrix& _mWorld,
        const Math::Vector4& _colRate);

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /**
     * @fn void DrawMesh(const Mesh& _mesh, const Math::Matrix& _mWorld, const std::vector<Material>& _materials, const Math::Vector4& _colRate)
     * @brief メッシュの描画
     *
     * @param _mesh      - メッシュ
     * @param _mWorld    - ワールド行列
     * @param _materials - マテリアルの配列
     * @param _colRate   - 色倍率
     */
    void DrawMesh(
        const Mesh& _mesh,
        const Math::Matrix& _mWorld,
        const std::vector<Material>& _materials,
        const Math::Vector4& _colRate);

    /**
     * @fn void SetMaterial(const Material& _material, const Math::Vector4& colRate)
     * @brief マテリアルをセット
     *
     * @param _material - マテリアル情報
     * @param colRate  - 色倍率
     */
    void SetMaterial(const Material& _material, const Math::Vector4& colRate);

    /* @brief 初期化 */
    void Init();

    ConstantBuffer<CBufferData::cbObject> m_cbObject;
    ConstantBuffer<CBufferData::cbMaterial> m_cbMaterial;
};
