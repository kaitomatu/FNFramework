#pragma once

#include "../../BaseComponent.h"

class TransformComponent;

/**
* @class ModelComponent
* @brief Rendererへ送信する描画情報を保持するコンポーネント
* @details
*	初期化段階で設定されたモデルのデータをマイフレームRendererへ送信する
*	モデルのデータはアセットマネージャーから取得する
*
*	todo : スキンメッシュ用のデータを持ったコンポーネントと現在のModelComponentoを分けて作成する。このときに共通処理をまとめたRendererComponentを継承させる
*/
class ModelComponent
    : public BaseComponent
{
public:

    enum class CullingType
    {
        eNotCulling, // カリングなし
        eFrustum, // 試錐台カリング
        eIgnoreShadowCulling// 影は常時除外する
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    ModelComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
     * @fn const std::shared_ptr<ModelData>& GetModelData() const
     * @return モデルデータ
    */
    const std::shared_ptr<ModelData> GetOriginalModelData() const
    {
        if(!m_spModelData) { return nullptr; }
        return m_spModelData->GetModelData();
    }
    const std::shared_ptr<ModelWork>& GetModelData() const { return m_spModelData; }
    // モデルの読み込みが終了しているかどうか
    bool IsLoadedModelData() const { return m_spModelData != nullptr; }

    /**
     * @fn const AABB<Math::Vector3>& GetModelAABB() const
     * @return モデル全体を取り囲むAABB
     */
    const AABB<Math::Vector3>& GetColMeshAABB() const { return m_colMeshBox.AABB; }
    const AABB<Math::Vector3>& GetSrcColMeshAABB() const { return m_colMeshBox.SrcAABB; }
    const AABB<Math::Vector3>& GetDrawMeshAABB() const { return m_drawMeshBox.AABB; }
    const AABB<Math::Vector3>& GetSrcDrawMeshAABB() const { return m_drawMeshBox.SrcAABB; }

    /**
     * @fn void SetModelName(std::string_view modelName)
     * @param[in] modelName - ファイルパス
     */
    void SetModelName(std::string_view modelName) { m_modelName = modelName; }

    void SetColor(const Math::Vector4& color) { m_color = color; }
    void SetAlpha(float alpha) { m_color.w = alpha; }
    const Math::Vector4& GetColor() const { return m_color; }
    float GetAlpha() const { return m_color.w; }

    // カリングされているかどうか
    bool IsInsideFrustum() const { return m_insideFrustum; }
    CullingType GetCullingType() const { return m_cullingType; }

    /**
    * @fn void SetRenderType(RenderingData::Model::RenderType renderType)
    * @param[in] renderType - 描画タイプ
    */
    void SetRenderType(RenderingData::Model::RenderType renderType) { m_renderType = static_cast<UINT>(renderType); }
    /**
    * @fn void SetRenderType(RenderingData::Model::RenderType renderType)
    * @param[in] renderType - 描画タイプ
    */
    void AddRenderType(RenderingData::Model::RenderType renderType) { m_renderType |= static_cast<UINT>(renderType); }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;
    void Start() override;

    // モデルデータの読み込み
    void LoadModelData();

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

    void Release() override;

protected:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief ImGui更新 */
    void ImGuiUpdate() override;

    /* @brief 更新 */
    void Update() override;
    void UpdateWorldTransform() override;

    //-----------
    // カリング関係
    //-----------
    /**
     * @fn UINT CullingCheck()
     * @brief カリングチェックを行い、描画タイプを返す
     * @return 描画タイプ
     */
    UINT CullingCheck();
    /* 試錐台カリングのチェック */
    bool CheckFrustumCulling();
    void UpdateModelAABB();

    //--------------------------------
    // 変数
    //--------------------------------
    /* Rendererへのデータ */
    // アニメーションなどで利用されるモデルのデータ
    std::shared_ptr<ModelWork> m_spModelData = nullptr;

    // モデルの色
    Math::Vector4 m_color = {1.0f, 1.0f, 1.0f, 1.0f};

    Math::Vector2 m_tilling = { 1.0f, 1.0f };
    Math::Vector2 m_offset = { 0.0f, 0.0f };

    // モデルのファイルパス
    std::string m_modelName = "";
    // 描画タイプ
    UINT m_renderType = static_cast<UINT>(RenderingData::Model::RenderType::eLit);

    struct ModelBoundingData
    {
        AABB<Math::Vector3> SrcAABB;
        AABB<Math::Vector3> AABB;
    };

    ModelBoundingData m_drawMeshBox;
    ModelBoundingData m_colMeshBox;

    bool m_insideFrustum = true;
    CullingType m_cullingType = CullingType::eFrustum;

private:
    //-----------
    // ImGui用
    //-----------
    /* RenderTypeをImGuiから変更した際の挙動フラグ */
    enum GUIRenderTypeMode
    {
        eNone = 1,
        eAdd = eNone << 1,
        eDel = eNone << 2,
        eSet = eNone << 3,
    };

    UINT m_guiAddRenderType = eNone;
    UINT m_previewRenderType = m_renderType;

    void ImGuiShowNodeData();
    void ChangeRenderType();

    void ComputeAABBFromNodes(const std::vector<int>& nodeIndices, AABB<Math::Vector3>& outAABB);
    void TransformAABB(const AABB<Math::Vector3>& srcAABB, AABB<Math::Vector3>& _dstAABB);
};

namespace jsonKey::Comp
{
    namespace ModelComponent
    {
        constexpr std::string_view ModelName = "ModelName";
        constexpr std::string_view Color = "Color";

        constexpr std::string_view Tilling = "Tilling";
        constexpr std::string_view Offset = "Offset";

        constexpr std::string_view RenderType = "RenderType";
        constexpr std::string_view CullingType = "CullingType";
    }
}
