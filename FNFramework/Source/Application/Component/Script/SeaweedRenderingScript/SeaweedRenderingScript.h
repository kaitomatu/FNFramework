#pragma once

#include "../../../Component/Renderer/ModelComponent/ModelComponent.h"

/**
* @class SeaweedRenderingScript
* @brief 海藻モデルのアニメーションを制御するコンポーネント
* @details 特定の範囲内に海藻モデルを敷き詰めて描画する
*
*/
class SeaweedRenderingScript
    : public ModelComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    SeaweedRenderingScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : ModelComponent(owner, name, _enableSerialize)
    {
    }

    ~SeaweedRenderingScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // 必要があればアニメーターのメモリ確保を行い、アニメーションを設定する
    void SetAnimation(
        std::shared_ptr<Animator>& spAnimator,
        std::string_view animName, bool isLoop);
    void SetAnimation(
        std::shared_ptr<Animator>& spAnimator, int animName, bool isLoop);
    void SetAnimation(
        std::shared_ptr<Animator>& spAnimator,
        const std::shared_ptr<AnimationData>& spAnimData,
        bool isLoop);

    // アニメーションスピードの設定 / 取得
    void SetAnimationSpeed(float speed) { m_animationSpeed = speed; }
    float GetAnimationSpeed() const { return m_animationSpeed; }

    //--------------------------------
    // その他関数
    //--------------------------------
    void Awake() override;
    /**
    * @fn void Start()
    * @brief Awakeを経て初期化された後、1度だけ呼びだされる
    * @details
    *	Awakeの後に呼び出される
    *	他のコンポーネントとの依存関係にある初期化処理や
    *	Awakeの段階ではできない初期化を行う
    */
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

    void Release() override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;
    void ImGuiChangeAnimData();

    void CullingCheck(const Math::Vector3& worldPos, UINT& renderType);
    bool CheckFrustumCulling(const Math::Vector3& pos);

    void CalcRenderingPos();

    // 範囲の設定 //
    OBB m_area;

    // 海藻の配置位置のオフセット用 //
    enum PlacementPosition
    {
        eDefault = 0,// オフセットなし
        eTop,        // 上面に配置
        eBottom      // 底面に配置
    };
    int m_placementPosition = PlacementPosition::eDefault;

    // アニメーション関係 //
    // - グループ単位で不規則なな動きを再現するために、複数のアニメーションを持つ //
    struct AnimationInfo
    {
        std::shared_ptr<Animator> spAnimator;
        std::shared_ptr<ModelWork> DummyModelData;
    };
    std::vector<AnimationInfo> m_spAnimationInfo;
    float m_animationSpeed = 1.0f;
    int m_animIdx = -1;

    // 海藻の描画情報 //
    struct SeaweedInstanceData
    {
        Math::Vector4 PosAndRotZ;
        int UseAnimatorIdx = 0;
    };
    std::vector<SeaweedInstanceData> m_seaweedInstanceRenderData;
    int m_currentSeaweedIdx = 0;
};

// Jsonで利用するキー
namespace jsonKey::Comp
{
    namespace SeaweedRenderingScript
    {
        constexpr std::string_view AnimationSpeed = "AnimationSpeed";

        constexpr std::string_view PlacementPosition = "PlacementPosition";
        constexpr std::string_view OBBExtents = "OBBExtents";

        constexpr std::string_view CurrentSeaweedIdx = "CurrentSeaweedIdx";
    }
}
