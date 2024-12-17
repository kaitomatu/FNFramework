#pragma once

#include "../ModelComponent/ModelComponent.h"

/**
* @class AnimationComponent
* @brief アニメーションを行うコンポーネント
* @details
* todo : アニメーションの予約リストなども管理しておく
*
*/
class AnimationComponent
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
    AnimationComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : ModelComponent(owner, name, _enableSerialize)
    {
    }

    ~AnimationComponent() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // 必要があればアニメーターのメモリ確保を行い、アニメーションを設定する
    void SetAnimation(std::string_view animName, bool isLoop);
    void SetAnimation(int animName, bool isLoop);
    void SetAnimation(const std::shared_ptr<AnimationData>& spAnimData, bool isLoop);

    // アニメーションスピードの設定 / 取得
    void SetAnimationSpeed(float speed) { m_animationSpeed = speed; }
    float GetAnimationSpeed() const { return m_animationSpeed; }

    // アニメーションの終了？
    bool IsAnimationEnd() const
    {
        if (!m_spAnimator) { return false; }
        return m_spAnimator->IsAnimationEnd();
    }

    // ループ関係
    void SetLoop(bool isLoop)
    {
        if (!m_spAnimator) { return; }
        m_spAnimator->SetLoop(isLoop);
    }
    bool IsLoop() const
    {
        if (!m_spAnimator) { return false; }
        return m_spAnimator->IsLoop();
    }

    // アニメーション進行度関係
    void ResetAnimation()
    {
        if (!m_spAnimator) { return; }
        m_spAnimator->ResetAnimation();
    }
    void SetProgressTime(float time)
    {
        if (!m_spAnimator) { return; }
        m_spAnimator->SetProgressTime(time);
    }
    float GetProgressTime() const
    {
        if (!m_spAnimator) { return 0.0f; }
        return m_spAnimator->GetProgressTime();
    }

    // アニメーションの最大フレーム数
    float GetMaxFrame() const
    {
        if (!m_spAnimator) { return 0.0f; }
        return m_spAnimator->GetMaxFrame();
    }

    // アニメーションの進行度を正規化したもの
    float GetNormalizeTime() const
    {
        if (!m_spAnimator) { return 0.0f; }
        return m_spAnimator->GetNormalizeTime();
    }

    void SetNormalizeTime(float normalizeTime)
    {
        if (!m_spAnimator) { return; }
        m_spAnimator->SetNormalizeTime(normalizeTime);
    }

    // アニメーションデータの名前
    const std::string& GetAnimationName() const
    {
        if (!m_spAnimator) { return ""; }
        return m_spAnimator->GetAnimationName();
    }

    //--------------------------------
    // その他関数
    //--------------------------------
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

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;
    void ImGuiChangeAnimData();

    int m_animIdx = -1;

    // アニメーション関係
    std::shared_ptr<Animator> m_spAnimator = nullptr;
    float m_animationSpeed = 1.0f;
};

// Jsonで利用するキー
namespace jsonKey::Comp
{
    namespace AnimationComponent
    {
        constexpr std::string_view AnimationSpeed = "AnimationSpeed";
        constexpr std::string_view IsLoop = "IsLoop";
        constexpr std::string_view ProgressTime = "ProgressTime";
    }
}
