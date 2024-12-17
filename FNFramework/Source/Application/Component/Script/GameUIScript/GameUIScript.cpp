#include "GameUIScript.h"

#include "../PlayerScript/PlayerScript.h"
#include "../StageScript/StageScript.h"

#include "../../Renderer/SpriteComponent/SpriteComponent.h"

// x座標に基づいてソートされた SpriteComponent リストを取得する関数
inline std::vector<std::shared_ptr<SpriteComponent>> GetSpriteComponentsSortedByX(const std::shared_ptr<GameObject>& spObj)
{
    std::vector<std::shared_ptr<SpriteComponent>> spriteComps;

    const std::string& SpriteCompName = typeid(SpriteComponent).name();

    for (auto&& spComp : spObj->GetComponents())
    {
        if (spComp->GetComponentName() != SpriteCompName)
        {
            continue;
        }

        const std::shared_ptr<SpriteComponent>& spSpriteComp = std::dynamic_pointer_cast<SpriteComponent>(spComp);

        auto it = std::lower_bound(spriteComps.begin(), spriteComps.end(), spSpriteComp,
            [](const std::shared_ptr<SpriteComponent>& a, const std::shared_ptr<SpriteComponent>& b) {
                return a->GetPixelPos().x < b->GetPixelPos().x;
            });

        spriteComps.emplace(it, spSpriteComp);
    }

    return spriteComps;
}

// 名前でオブジェクトを検索し、検索したいコンポーネントを返す //
template <typename T>
std::weak_ptr<T> FindObjectByNameAndGetComponent(const std::string& objectName)
{
    if (objectName.empty())
    {
        return std::weak_ptr<T>(); // 空の名前なら何もしない
    }

    auto obj = SceneManager::Instance().GetNowScene()->FindObject(objectName);
    if (obj)
    {
        return obj->GetComponent<T>();
    }

    // 見つからなかった場合
    return std::weak_ptr<T>();
}

// 再帰的に名前で子オブジェクトを検索する
std::weak_ptr<GameObject> FindChildByNameRecursive(const std::string& objectName, const std::shared_ptr<GameObject>& parent)
{
    if (objectName.empty())
    {
        return std::weak_ptr<GameObject>(); // 空の名前なら何もしない
    }

    // 現在の親オブジェクトの子を探索
    for (const auto& child : parent->GetChildren())
    {
        if (child.expired()) { continue; }

        auto childPtr = child.lock();
        if (childPtr->GetName() == objectName)
        {
            return child; // 名前が一致するオブジェクトを発見
        }

        // 子オブジェクトに対して再帰的に探索
        auto result = FindChildByNameRecursive(objectName, childPtr);
        if (!result.expired())
        {
            return result; // 一致するオブジェクトが見つかった場合
        }
    }

    // 一致するオブジェクトが見つからなかった場合
    return std::weak_ptr<GameObject>();
}

void GameUIScript::Awake()
{
    m_isFirstUpdate = true;

    const std::string_view numberTexturePath = "Assets/Texture/UI/Numeric/";

    // 0 - 9の数字テクスチャを読み込む
    for (UINT i = 0; i < m_spNumberTextures.size(); ++i)
    {
        m_spNumberTextures[i] = AssetManager::Instance().GetTexture(numberTexturePath.data() + std::to_string(i) + ".png");
    }
}

void GameUIScript::Start()
{
    if (!OwnerValid()) { return; }

    const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

    // 2週目のときにUIが非表示になっているので、再度表示する
    spOwner->SetStateContagion(GameObject::State::eActive);

    //x----- コンポーネントの検索 / 復元 -----x//
    m_wpStageScript = FindObjectByNameAndGetComponent<StageScript>(m_stageScriptObjectName);
    m_wpPlayerScript = FindObjectByNameAndGetComponent<PlayerScript>(m_playerScriptObjectName);

    // 時間UIオブジェクトの検索
    {
        auto&& timeUIObj = FindChildByNameRecursive(m_timeUIObjectName, spOwner);
        if (!timeUIObj.expired())
        {
            auto&& spriteComps = GetSpriteComponentsSortedByX(timeUIObj.lock());

            for (auto&& spriteComp : spriteComps)
            {
                m_wpTimeUIObjHasSpriteComps.emplace_back(spriteComp);
            }
        }
    }

    // 背景画像を取得 -> 有効ならばアルファ値を取得 -> 設定
    if (const auto& spBackImageObj = SceneManager::Instance().GetNowScene()->FindObject("BackGround"))
    {
        spBackImageObj->SetState(GameObject::State::ePaused);
    }

    m_isFirstUpdate = true;

}

void GameUIScript::PreUpdate()
{
    // 初回更新の時のみに一部ゲームUIの設定を行う
    if (!m_isFirstUpdate) { return; }

    if (m_wpOwnerObj.expired() ||
        m_wpStageScript.expired())
    {
        return;
    }

    const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

    int stageChildNum = m_wpStageScript.lock()->GetMaxCollectiveCount();

    // スコアUIオブジェクトの検索
    // ※ Startでの取得だと更新順により、取得できない場合があるため、ここで取得 & ステージの子どもの数を設定
    {
        auto&& scoreUIObj = FindChildByNameRecursive(m_scoreUIObjectName, spOwner);
        if (!scoreUIObj.expired())
        {
            auto spriteComps = GetSpriteComponentsSortedByX(scoreUIObj.lock());

            spriteComps[1]->SetMainTexture(m_spNumberTextures[stageChildNum]);

            // 操作が必要なのは一番左のスプライトコンポーネントのみなので、それだけを取得
            m_wpScoreUIHasSpriteComp = spriteComps[0];
        }
    }

    m_isFirstUpdate = false;
}

void GameUIScript::Update()
{
    if (m_wpPlayerScript.expired() ||
        m_wpScoreUIHasSpriteComp.expired() ||
        m_wpTimeUIObjHasSpriteComps.empty() ||
        m_wpStageScript.expired())
    {
        return;
    }

    int nowChildCnt = m_wpPlayerScript.lock()->GetChildCount();
    m_wpScoreUIHasSpriteComp.lock()->SetMainTexture(m_spNumberTextures[nowChildCnt]);

    // 秒数を[分, 秒]に変換
    int nowTimeSec = m_wpStageScript.lock()->GetTimeFromSeconds();

    int min = nowTimeSec / 60;
    m_wpTimeUIObjHasSpriteComps[0].lock()->SetMainTexture(m_spNumberTextures[min%10]);

    int sec = nowTimeSec % 60;
    // 秒数の10の位
    m_wpTimeUIObjHasSpriteComps[1].lock()->SetMainTexture(m_spNumberTextures[sec / 10]);
    // 秒数の1の位
    m_wpTimeUIObjHasSpriteComps[2].lock()->SetMainTexture(m_spNumberTextures[sec % 10]);

}

void GameUIScript::Release()
{
    for(UINT i = 0; i < m_spNumberTextures.size(); ++i)
    {
        m_spNumberTextures[i].reset();
        m_spNumberTextures[i] = nullptr;
    }
}

void GameUIScript::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::GameUIScript::PlayerScriptObjectName.data()] = m_playerScriptObjectName;
    _json[jsonKey::Comp::GameUIScript::StageScriptObjectName.data()] = m_stageScriptObjectName;
    _json[jsonKey::Comp::GameUIScript::TimeUIObjectName.data()] = m_timeUIObjectName;
    _json[jsonKey::Comp::GameUIScript::ScoreUIObjectName.data()] = m_scoreUIObjectName;
}

void GameUIScript::Deserialize(const Json& _json)
{
    m_playerScriptObjectName = _json[jsonKey::Comp::GameUIScript::PlayerScriptObjectName.data()];
    m_stageScriptObjectName = _json[jsonKey::Comp::GameUIScript::StageScriptObjectName.data()];
    m_timeUIObjectName = _json[jsonKey::Comp::GameUIScript::TimeUIObjectName.data()];
    m_scoreUIObjectName = _json[jsonKey::Comp::GameUIScript::ScoreUIObjectName.data()];
}

void GameUIScript::ImGuiUpdate()
{
    if (ImGui::Button(U8_TEXT("PlayerScriptをセットする")))
    {
        m_wpPlayerScript = FindObjectByNameAndGetComponent<PlayerScript>(m_playerScriptObjectName);
    }
    ImGui::Text(U8_TEXT("PlayerScriptを持っているオブジェクトの名前: %s"), m_playerScriptObjectName.c_str());
    utl::ImGuiHelper::InputTextWithString("##PlayerScriptObjName", m_playerScriptObjectName);

    ImGui::Separator();
    if (ImGui::Button(U8_TEXT("StageScriptをセットする")))
    {
        m_wpStageScript = FindObjectByNameAndGetComponent<StageScript>(m_stageScriptObjectName);
    }
    utl::ImGuiHelper::InputTextWithString("##StageScriptObjName", m_stageScriptObjectName);

    ImGui::Separator();
    if (ImGui::Button(U8_TEXT("タイマーUIオブジェクトのセット")))
    {
        auto&& timeUIObj = FindChildByNameRecursive(m_timeUIObjectName, m_wpOwnerObj.lock());
        if (!timeUIObj.expired())
        {
            auto&& spriteComps = GetSpriteComponentsSortedByX(timeUIObj.lock());

            for (auto&& spriteComp : spriteComps)
            {
                m_wpTimeUIObjHasSpriteComps.emplace_back(spriteComp);
            }
        }
    }
    utl::ImGuiHelper::InputTextWithString("##TimeUIObjName", m_timeUIObjectName);

    ImGui::Separator();
    if (ImGui::Button(U8_TEXT("スコアUIオブジェクトのセット")))
    {
        auto&& scoreUIObj = FindChildByNameRecursive(m_scoreUIObjectName, m_wpOwnerObj.lock());
        if (!scoreUIObj.expired())
        {
            auto spriteComps = GetSpriteComponentsSortedByX(scoreUIObj.lock());

            // 操作が必要なのは一番左のスプライトコンポーネントのみなので、それだけを取得
            m_wpScoreUIHasSpriteComp = spriteComps[0];
        }
    }
    utl::ImGuiHelper::InputTextWithString("##ScoreUIObjName", m_scoreUIObjectName);
}
