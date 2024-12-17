#include "ResultState.h"

#include "LoadState.h"
#include "ResultUIAnimation.h"
#include "Application/Component/Script/GoalScript/GoalScript.h"
#include "Application/Component/Script/StageScript/StageScript.h"
#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"

void SetStarIconData(int _idx, ResultUIAnimation::InitResultUIData::InitResultUI& _initData)
{
    // 〇 = i(0, 1, 2)
    // StarObj -[Child]-> StarIcon〇 -> SpriteComp x 2:オーダー順で設定
    std::string starObjectName = "Star" + std::to_string(_idx + 1) + "Icon";
    const std::shared_ptr<GameObject>& starObject = SceneManager::Instance().GetNowScene()->FindObject(starObjectName);


    if (!starObject)
    {
        FNENG_ASSERT_LOG("クリア条件のオブジェクトが見つかりませんでした", false);
            return;
    }

    std::vector<std::shared_ptr<SpriteComponent>> spriteComps;

    const std::string& SpriteCompName = typeid(SpriteComponent).name();

    // 対象オブジェクトが持っているスプライトコンポ―ネントを取得 -> 最前面と最背面のスプライトコンポーネントを取得
    for (auto comp : starObject->GetComponents())
    {
        if (comp->GetComponentName() != SpriteCompName)
        {
            continue;
        }

        spriteComps.push_back(std::dynamic_pointer_cast<SpriteComponent>(comp));
    }

    // スプライトコンポーネントが少なくとも2つあることを確認
    if (spriteComps.size() < 2)
    {
        FNENG_ASSERT_LOG("スプライトコンポーネントが不足しています", false);
        return;
    }

    // スプライトコンポーネントをオーダー順で並び替え (降順)
    std::sort(spriteComps.begin(), spriteComps.end(), [](const std::shared_ptr<SpriteComponent>& a, const std::shared_ptr<SpriteComponent>& b)
        {
            return a->GetUpdateOrder() > b->GetUpdateOrder(); // オーダーが大きいものを先にする
        });

    _initData.spStarSpriteComponent = spriteComps[0];
    _initData.spBaseStarSpriteComponent = spriteComps[1];
}

void ResultState::Enter(GoalScript* _pOwner)
{
    if (!m_resultUiAnimation)
    {
        m_resultUiAnimation = std::make_shared<ResultUIAnimation>();
    }

    ResultUIAnimation::InitResultUIData initData;

    for (int i = 0; i < static_cast<int>(ResultUIAnimation::ResultUIState::eCount); ++i)
    {
        ResultUIAnimation::InitResultUIData::InitResultUI initResultUI;

        // 〇 = i(0, 1, 2)
        SetStarIconData(i, initResultUI);

        // StarText -[Child]-> Star〇Text
        std::string clearTextObjectName = "Star" + std::to_string(i + 1) + "Text";
        const std::shared_ptr<GameObject>& clearTextObject = SceneManager::Instance().GetNowScene()->FindObject(clearTextObjectName);

        if (!clearTextObject)
        {
            FNENG_ASSERT_LOG("クリア条件のテキストが見つかりませんでした", false)
        }

        initResultUI.spClearTextObject = clearTextObject;

        initData.InitResultUIs[i] = initResultUI;
    }

    m_resultUiAnimation->Init(_pOwner, initData);

    //x---- ゲームUIの非表示 ----x//
    if (const auto& spGameUI = SceneManager::Instance().GetNowScene()->FindObject("GameUI"))
    {
        spGameUI->SetStateContagion(GameObject::State::ePaused);
    }

    // 背景画像を取得 -> 有効ならばアルファ値を取得 -> 設定
    if (const auto& spBackImageObj = SceneManager::Instance().GetNowScene()->FindObject("BackGround"))
    {
        spBackImageObj->SetState(GameObject::State::eActive);

        if (const auto& spSpriteComp = spBackImageObj->GetComponent<SpriteComponent>())
        {
            m_wpBackgroundSpriteComp = spSpriteComp;
        }
    }
}

void ResultState::Update(GoalScript* _pOwner)
{

    // BackGround のアルファが一定値になるまで待機
    if (ResultUIUpdateBackImage(_pOwner))
    {
        return;
    }

    m_resultUiAnimation->Update();

    /**
     * 1. R が押されたらもう一度同じステージを読み込む
     * 2. Enter が押されたら次のステージに進む
     * 3. T が押されたらタイトルに戻る
     */

    if (InputSystem::Instance().IsHold("Enter"))
    {
        m_changeNextStage = true;
    }

    if (m_changeNextStage)
    {
        _pOwner->GetResultController().PopState();
        _pOwner->GetResultController().AddState<LoadState>();
    }
}

void ResultState::Exit(GoalScript* _pOwner)
{
    // リザルトフラグを下げておく
    _pOwner->SetResultFlg(false);
}

bool ResultState::ResultUIUpdateBackImage(GoalScript* _pOwner)
{
    float backImageAlpha = 0.0f;

    if (m_wpBackgroundSpriteComp.expired()) { return false; }

    backImageAlpha = m_wpBackgroundSpriteComp.lock()->GetAlpha();

    if (backImageAlpha >= 1.0f)
    {
        // 背景が一定以上になったらリザルトUIも表示する
        const auto& spResultUIObj = SceneManager::Instance().GetNowScene()->FindObject("ResultUI");

        if (spResultUIObj)
        {
            // 再帰的に子孫すべての状態を設定するラムダ
            std::function<void(const std::shared_ptr<GameObject>&)> setStateRecursively = [&](const std::shared_ptr<GameObject>& obj)
                {
                    obj->SetState(GameObject::State::eActive);

                    for (auto&& spChild : obj->GetChildren())
                    {
                        if (spChild.expired()) { continue; }
                        setStateRecursively(spChild.lock()); // 再帰的に孫以降のオブジェクトにも伝える
                    }
                };

            // リザルトUIオブジェクトとその子孫に状態を伝播
            setStateRecursively(spResultUIObj);
        }


        return false;
    }

    backImageAlpha += 1.0f * SceneManager::Instance().FrameDeltaTime();

    m_wpBackgroundSpriteComp.lock()->SetAlpha(backImageAlpha);

    return true;
}

void ResultState::ImGui(GoalScript* _pOwner)
{
    ImGui::Text("x--- ResultState ---x");

    ImGui::Checkbox(U8_TEXT("次のステージへ"), &m_changeNextStage);
}
