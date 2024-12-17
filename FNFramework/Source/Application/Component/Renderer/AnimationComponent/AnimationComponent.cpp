#include "AnimationComponent.h"
#include "../../TransformComponent/TransformComponent.h"

void AnimationComponent::SetAnimation(std::string_view animName, bool isLoop)
{
    // モデルの読み込みがまだの場合はデータを読み込んでから処理を行う
    if (!IsLoadedModelData())
    {
        LoadModelData();
    }

    const auto& animList = m_spModelData->GetModelData()->GetAnimationList();

    for(int i = 0; i <  animList.size(); ++i)
    {
        if (animList[i]->Name == animName)
        {
            m_animIdx = i;
            break;
        }
    }

    SetAnimation(m_spModelData->GetAnimation(animName), isLoop);
}

void AnimationComponent::SetAnimation(int animIdx, bool isLoop)
{
    // モデルの読み込みがまだの場合はデータを読み込んでから処理を行う
    if (!IsLoadedModelData())
    {
        LoadModelData();
    }

    m_animIdx = animIdx;

    SetAnimation(m_spModelData->GetAnimation(animIdx), isLoop);
}

void AnimationComponent::SetAnimation(const std::shared_ptr<AnimationData>& spAnimData, bool isLoop)
{
    // 実体化されていない場合はメモリを確保する
    if (!m_spAnimator)
    {
        m_spAnimator = std::make_shared<Animator>();
    }

    m_spAnimator->SetAnimation(spAnimData, isLoop);
}

void AnimationComponent::Start()
{
    ModelComponent::Start();

    // animationはとりあえず最初のアニメーションを設定
    SetAnimation(0, false);
}

void AnimationComponent::Update()
{
    if (!OwnerValid()) { return; }

    //--------------------------------
    // アニメーションの更新
    //--------------------------------
    // アニメーションが設定されている場合はアニメーションを進める
    if (m_spAnimator)
    {
        m_spAnimator->AdvanceTime(m_spModelData->WorkNodes(), m_animationSpeed);
    }

    ModelComponent::Update();
}

void AnimationComponent::Serialize(Json& _json) const
{
    ModelComponent::Serialize(_json);

    // アニメーション速度を保存
    _json[jsonKey::Comp::AnimationComponent::AnimationSpeed.data()] = m_animationSpeed;

    // ループ設定を保存
    if (m_spAnimator)
    {
        _json[jsonKey::Comp::AnimationComponent::IsLoop.data()] = m_spAnimator->IsLoop();
        _json[jsonKey::Comp::AnimationComponent::ProgressTime.data()] = m_spAnimator->GetProgressTime();
    }
}

void AnimationComponent::Deserialize(const Json& _json)
{
    ModelComponent::Deserialize(_json);

    // アニメーション速度を復元
    m_animationSpeed = _json.at(jsonKey::Comp::AnimationComponent::AnimationSpeed.data()).get<float>();

    // 実体化されていない場合はメモリを確保する
    if(! m_spAnimator)
    {
        m_spAnimator = std::make_shared<Animator>();
    }

    // ループ設定と進行時間を復元
    bool isLoop = _json.at(jsonKey::Comp::AnimationComponent::IsLoop.data()).get<bool>();
    m_spAnimator->SetLoop(isLoop);

    float progressTime = _json.at(jsonKey::Comp::AnimationComponent::ProgressTime.data()).get<float>();
    m_spAnimator->SetProgressTime(progressTime);
}

void AnimationComponent::ImGuiUpdate()
{
    ModelComponent::ImGuiUpdate();

    // アニメーションデータがない場合は何もしない
    if (!m_spAnimator || !m_spModelData) { return; }

    ImGui::Text("--------- AnimationData ----------");

    if (ImGui::TreeNode("AnimationInfo"))
    {
        ImGui::Text(U8_TEXT("Name: %s"), m_spAnimator->GetAnimationName().data());
        ImGui::Text(U8_TEXT("最大アニメーション時間: %.2f"), m_spAnimator->GetMaxFrame());
        ImGui::Text(U8_TEXT("アニメーション進捗度: %.3f"), m_spAnimator->GetProgressTime());
        ImGui::Text(U8_TEXT("アニメーション進捗度(0～1): %.3f"), m_spAnimator->GetNormalizeTime());

        ImGui::TreePop();
    }

    ImGuiChangeAnimData();

    // アニメーションの再生速度
    bool isLoop = m_spAnimator->IsLoop();
    if (ImGui::Checkbox("IsLoop", &isLoop))
    {
        m_spAnimator->SetLoop(isLoop);
    }

    ImGui::DragFloat("AnimationSpeed", &m_animationSpeed, 0.1f, 0.0f, 10.0f);

    // アニメーション進行度
    if (ImGui::Button("RestartAnimation"))
    {
        m_spAnimator->ResetAnimation();
    }

    float progressTime = m_spAnimator->GetProgressTime();
    if (ImGui::DragFloat("Progress", &progressTime, 0.1f, 0.0f, m_spAnimator->GetMaxFrame()))
    {
        m_spAnimator->SetProgressTime(progressTime);
    }
}

void AnimationComponent::ImGuiChangeAnimData()
{
    // モデルに含まれるアニメーションデータを取得
    const auto& animations = m_spModelData->GetModelData()->GetAnimationList();
    if (animations.empty())
    {
        ImGui::Text(U8_TEXT("アニメーションデータが存在しません"));
        return;
    }

    ImGui::DragInt("AnimationIndex", &m_animIdx, 1, 0, static_cast<int>(animations.size()) - 1);

    if (ImGui::Button("ChangeAnimation"))
    {
        if (m_animIdx < 0 || m_animIdx >= static_cast<int>(animations.size()))
        {
            FNENG_ASSERT_LOG("アニメーションインデックスが不正です", false)
            return;
        }

        SetAnimation(m_animIdx, true);
    }

}
