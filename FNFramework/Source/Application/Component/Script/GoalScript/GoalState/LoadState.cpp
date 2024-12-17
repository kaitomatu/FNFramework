#include "LoadState.h"

#include "ResultState.h"
#include "Application/Component/Script/GoalScript/GoalScript.h"
#include "Application/Component/Script/StageScript/StageScript.h"

void LoadState::Enter(GoalScript* _pOwner)
{
    if(_pOwner->GetStageScript().expired())
    {
        FNENG_ASSERT_LOG("ステージスクリプトが取得できませんでした", false);
        return;
    }

    const std::shared_ptr<StageScript>& spStageScript = _pOwner->GetStageScript().lock();

    std::string stageName = spStageScript->GetNextStageName();

    // ステージの名前がない場合はタイトルへ移動する
    if (!SceneManager::Instance().IsLoadedScene(stageName))
    {
        stageName = "Title";
    }

    // ここではシーンの追加のみしておいて、 Update() で演出を含めたシーンの遷移を行う
    m_sceneName = stageName;
}

void LoadState::Update(GoalScript* _pOwner)
{
    // todo: シーンの遷移アニメーションを追加
    SceneManager::Instance().ChangeScene(m_sceneName);

    _pOwner->GetResultController().PopState();
}

void LoadState::Exit(GoalScript* _pOwner)
{
}

void LoadState::ImGui(GoalScript* _pOwner)
{
    ImGui::Text("x--- LoadState ---x");
}
