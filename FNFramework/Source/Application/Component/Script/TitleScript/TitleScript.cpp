#include "TitleScript.h"

#include "Application/Component/Script/GoalScript/GoalScript.h"

void TitleScript::Awake()
{
}

void TitleScript::Start()
{
    // ほかのシーンの StartFlgを false にする
    for(auto[name, scene] : SceneManager::Instance().GetSceneList())
    {
        if(name == SceneManager::Instance().GetNowSceneName()) { continue; }

        for(auto&& obj : scene->GetObjectList())
        {
            for(auto&& comp : obj->GetComponents())
            {
                comp->DownStartFlg();
            }
        }
    }
}

void TitleScript::Update()
{
    if (InputSystem::Instance().IsPressed("Enter"))
    {
        m_isGameStart = true;

        for(auto&& obj : SceneManager::Instance().GetNowScene()->GetObjectList())
        {
            for(auto&& comp : obj->GetComponents())
            {
                comp->DownStartFlg();
            }
        }
    }

    // todo : フェード処理を入れるならここで何かしらの処理を挟む

    /**
     * 1. 今のシーンをフェードアウト
     * 1.5 裏で次のシーンの読み込みを行う
     * 2. フェードアウトが終わったら次のシーンに遷移
     * 3. 次のシーンに遷移したらフェードイン
     */

    // ゲームスタート
    if (m_isGameStart)
    {
        m_isGameStart = false;
        SceneManager::Instance().ChangeScene(m_firstStageName);
    }
}

void TitleScript::Release()
{
}

void TitleScript::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::TitleScript::FirstStageName.data()] = m_firstStageName;
}

void TitleScript::Deserialize(const Json& _json)
{
    m_firstStageName = _json.value(jsonKey::Comp::TitleScript::FirstStageName.data(), "Stage");
}

void TitleScript::ImGuiUpdate()
{
    utl::ImGuiHelper::InputTextWithString(U8_TEXT("次のステージの名前"), m_firstStageName);
}
