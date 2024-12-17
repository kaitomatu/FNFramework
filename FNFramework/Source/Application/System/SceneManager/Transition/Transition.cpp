#include "Transition.h"

void BaseTransitionEffect::Update()
{
    // 更新が停止している場合は何も行わない
    if (m_state == State::eStop) { return; }

    // 各フェーズごとの処理を行う
    switch (m_phase)
    {
    case BaseTransitionEffect::Phase::eStart:
        StartUpdate();
        break;

    case BaseTransitionEffect::Phase::eBlackOut:
        BlackOutUpdate();
        break;

    case BaseTransitionEffect::Phase::eComplete:
        CompleteUpdate();
        break;

    default:
        // 画面遷移状態じゃない場合は何もしない
            //Assert::ErrorAssert("画面遷移状態が設定されていません");
                break;
    }
}

void TransitionController::Release()
{
    if (m_spTransitionEffectList.empty()) { return; }

    for (auto& transition : m_spTransitionEffectList)
    {
        if (!transition) { continue; }

        transition.reset();
    }

    m_spTransitionEffectList.clear();
}

void TransitionController::DeleteTransitionEffect()
{
    // リストが空なら何もしない
    if (m_spTransitionEffectList.empty()) return;

    for (auto it = m_spTransitionEffectList.begin(); it != m_spTransitionEffectList.end();)
    {
        // 更新中に削除するとエラーになりかねないためチェック
        if ((*it)->IsPlay())
        {
            ++it;
            continue;
        }

        // 画面遷移状態がPhase::eCompleteのものを削除する
        if (!(*it)->IsFinish())
        {
            ++it;
            continue;
        }

        it = m_spTransitionEffectList.erase(it);
    }
}
