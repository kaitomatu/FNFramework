#include "ResultUIAnimation.h"

#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"
#include "Application/Component/Script/GoalScript/GoalScript.h"
#include "Application/Component/Script/PlayerScript/PlayerScript.h"
#include "Application/Component/Script/StageScript/StageScript.h"

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

// x--- タイム表示用のテクスチャ設定処理を行う関数 ---x
void SetTimeTextures(const std::vector<std::shared_ptr<SpriteComponent>>& spriteComps, int timeInSeconds)
{
    const std::string& TexDir = "Assets/Texture/UI/Numeric/";

    int minutes = timeInSeconds / 60;
    int seconds = timeInSeconds % 60;

    // 9分59秒を超えた場合は 9分59秒にする
    if (minutes >= 10)
    {
        minutes = 9;
        seconds = 59;
    }

    // 分の表示
    spriteComps[0]->SetMainTexture(TexDir + std::to_string(minutes) + ".png");
    // 10の位の秒数
    spriteComps[1]->SetMainTexture(TexDir + std::to_string(seconds / 10) + ".png");
    // 1の位の秒数
    spriteComps[2]->SetMainTexture(TexDir + std::to_string(seconds % 10) + ".png");
}

// クリアタイムの描画用のデータを設定する
// MainTex を変更してタイムを表示する
void ResultUIAnimation::SetClearTextSpriteDataFromTargetTime(const std::shared_ptr<GameObject>& _spObj)
{
    if(!_spObj)
    {
        FNENG_ASSERT_LOG("オブジェクトが存在しません", false);
        return;
    }

    // _spObj の子要素の TargetTimeObject / ClearTimeObject を取得
    for (auto&& wpChild : _spObj->GetChildren())
    {
        if (wpChild.expired()) { continue; }

        const std::shared_ptr<GameObject>& spChild = wpChild.lock();

        if (spChild->GetName() == "TargetTimeObject")
        {
            const auto& spriteComps = GetSpriteComponentsSortedByX(spChild);
            SetTimeTextures(spriteComps, m_targetSecTime);
        }

        if (spChild->GetName() == "ClearTimeObject")
        {
            const auto& spriteComps = GetSpriteComponentsSortedByX(spChild);
            SetTimeTextures(spriteComps, m_clearSecTime);
        }
    }
}

// クリア時の子クラゲと目標数の描画用のデータを設定する
// MainTex を変更して数を表示する
void ResultUIAnimation::SetClearTextSpriteDataFromCollective(const std::shared_ptr<GameObject>& _spObj)
{
    if (!_spObj)
    {
        FNENG_ASSERT_LOG("オブジェクトが存在しません", false);
        return;
    }

    const std::string& TexDir = "Assets/Texture/UI/Numeric/";

    //x--- X 軸でソートする ---x//
    const auto& spriteComps = GetSpriteComponentsSortedByX(_spObj);

    //  [ 0 ・ 最後尾 ]のものに 0 ～ 9 を割り当てる
    if (const auto& spSpriteComp = spriteComps[0])
    {
        spSpriteComp->SetMainTexture(TexDir + std::to_string(m_stageClearChildNum) + ".png");
    }

    if (const auto& spSpriteComp = spriteComps[spriteComps.size() - 1])
    {
        spSpriteComp->SetMainTexture(TexDir + std::to_string(m_stageChildNum) + ".png");
    }
}

void ResultUIAnimation::Init(const GoalScript* _pOwner, InitResultUIData _initData)
{
    int i = 0;
    // 初期状態はスターの色はグレーにする
    for (auto& resultUI : m_resultUIAnimatorInfos)
    {
        if (!_initData.InitResultUIs[i].spBaseStarSpriteComponent ||
            !_initData.InitResultUIs[i].spStarSpriteComponent ||
            !_initData.InitResultUIs[i].spClearTextObject)
        {
            FNENG_ASSERT_LOG("星のスプライトコンポーネントが存在しません", false);
            continue;
        }

        resultUI.wpBaseStarSpriteComponent = _initData.InitResultUIs[i].spBaseStarSpriteComponent;
        resultUI.wpStarSpriteComponent = _initData.InitResultUIs[i].spStarSpriteComponent;
        resultUI.spClearTextObject = _initData.InitResultUIs[i].spClearTextObject;

        resultUI.Alpha = 0.0f;

        resultUI.IconBasePos = resultUI.wpStarSpriteComponent.lock()->GetPos();
        resultUI.wpStarSpriteComponent.lock()->SetAlpha(0.0f);

        ++i;
    }

    // ここに入ってくる時点でリザルト画面なので、クリア条件を確認する //
    CheckClearAchieve(_pOwner);

    i = 0;
    for (auto& resultUI : m_resultUIAnimatorInfos)
    {
        // clearTextObject の値によって設定する値を変更する
        switch (static_cast<ResultUIState>(i))
        {
            // クリア条件の場合は特に何もしない
        case ResultUIState::eIsClear:
            break;
            // 目標時間の場合は $TargetTime$ と $ClearTime$ を表示する
            // SpriteComp::SetPos/SetMainTexで設定
        case ResultUIState::eTargetTime:
            SetClearTextSpriteDataFromTargetTime(resultUI.spClearTextObject);
            break;
            // 子どもの数の場合は $Collective$ と $ClearCollective$ を表示する
        case ResultUIState::eCollective:
            SetClearTextSpriteDataFromCollective(resultUI.spClearTextObject);
            break;
        }

        ++i;
    }
}

void ResultUIAnimation::CheckClearAchieve(const GoalScript* _pOwner)
{
    if (!_pOwner)
    {
        FNENG_ASSERT_LOG("GoalScriptが存在しません", false);
        return;
    }

    //x-- ステージをクリアしているかどうか --x//
    int idx = static_cast<int>(ResultUIState::eIsClear);
    m_resultUIAnimatorInfos[idx].IsClearAchieve = true;

    if (_pOwner->GetStageScript().expired()) { return; }

    //x--- 目標時間内にクリアしているかどうか ---x//
    const std::shared_ptr<StageScript>& spStageScript = _pOwner->GetStageScript().lock();

    // クリア時間と目標タイムを取得する
    m_clearSecTime = spStageScript->GetTimeFromSeconds();
    m_targetSecTime = spStageScript->GetClearTargetTime();

    if (m_clearSecTime <= m_targetSecTime)
    {
        idx = static_cast<int>(ResultUIState::eTargetTime);
        m_resultUIAnimatorInfos[idx].IsClearAchieve = true;
    }

    //x--- 一定数の子どもを集めているかどうか ---x//
    // プレイヤーの子どもの数を取得
    m_stageClearChildNum = 0;

    // ステージの子どもの数を取得
    m_stageChildNum = spStageScript->GetMaxCollectiveCount();

    // 子どもの数が最大数以上なら達成度を上げる
    const std::shared_ptr<GameObject>& spPlayerObj = SceneManager::Instance().GetNowScene()->FindObject("Player");

    if (spPlayerObj)
    {
        const std::shared_ptr<PlayerScript>& spPlayerScript = spPlayerObj->GetComponent<PlayerScript>();

        if (spPlayerScript)
        {
            // プレイヤーがゴール到達時に持っている子どもの数を取得 //
            m_stageClearChildNum = spPlayerScript->GetChildCount();
        }
    }

    if (m_stageClearChildNum >= m_stageChildNum)
    {
        idx = static_cast<int>(ResultUIState::eCollective);
        m_resultUIAnimatorInfos[idx].IsClearAchieve = true;
    }
}

void ResultUIAnimation::Update()
{
    ChangeUIState();

    ChangeClearTextUpdateActive();

    KurageUIAnimationControl();
}

bool ResultUIAnimation::IsAchieveClear(ResultUIState _state)
{
    int idx = static_cast<int>(_state);
    return m_resultUIAnimatorInfos[idx].IsClearAchieve;
}

void ResultUIAnimation::ChangeUIState()
{
    int idx = static_cast<int>(m_state);

    if (0 > idx || idx >= m_resultUIAnimatorInfos.size())
    {
        return;
    }

    if (m_resultUIAnimatorInfos[idx].wpStarSpriteComponent.expired())
    {
        FNENG_ASSERT_LOG("星のスプライトコンポーネントが存在しません", false);
        return;
    }

    if (m_resultUIAnimatorInfos[idx].Alpha >= 1.0f)
    {
        // アルファ値が1.0以上になったら次のテキストへ
        m_state = static_cast<ResultUIState>(++idx);
        return;
    }

    // 1 秒かけてアルファ値を 1 にする。
    m_resultUIAnimatorInfos[idx].Alpha += SceneManager::Instance().FrameDeltaTime();

    // 条件を達成している場合にのみアルファ値を変更する
    if (!m_resultUIAnimatorInfos[idx].IsClearAchieve)
    {
        return;
    }

    // m_state を変更する処理
    m_resultUIAnimatorInfos[idx].wpStarSpriteComponent.lock()->SetAlpha(m_resultUIAnimatorInfos[idx].Alpha);
}

void ResultUIAnimation::ChangeClearTextUpdateActive()
{

    // 現在の状態のテキストを表示する
    int idx = static_cast<int>(m_state);

    if (0 > idx)
    {
        return;
    }

    if (idx >= m_resultUIAnimatorInfos.size())
    {
        idx = m_resultUIAnimatorInfos.size() - 1;
    }

    // いったん全てのテキストを非表示にする
    for (auto&& resultUI : m_resultUIAnimatorInfos)
    {
        resultUI.spClearTextObject->SetStateContagion(GameObject::State::ePaused);
    }

    m_resultUIAnimatorInfos[idx].spClearTextObject->SetStateContagion(GameObject::State::eActive);
}

void ResultUIAnimation::KurageUIAnimationControl()
{
    int activeSize = static_cast<int>(m_state);

    if (activeSize < 0)
    {
        FNENG_ASSERT_LOG("activeSize のインデックスが不正です", false)
            return;
    }

    if (activeSize >= m_resultUIAnimatorInfos.size())
    {
        activeSize = m_resultUIAnimatorInfos.size() - 1;
    }

    // 現在の状態で選択されたものからふわふわを開始する
    for (int i = 0; i <= activeSize; ++i)
    {

        if (m_resultUIAnimatorInfos[i].wpStarSpriteComponent.expired() ||
            m_resultUIAnimatorInfos[i].wpBaseStarSpriteComponent.expired())
        {
            FNENG_ASSERT_LOG("星のスプライトコンポーネントが存在しません", false);
            return;
        }

        //x--- 新しい移動方向を計算 ---x//
        Math::Vector2 newDir;

        if (m_resultUIAnimatorInfos[i].MoveInterval <= 0.0f)
        {
            CalcNewDir(i, newDir);
            m_resultUIAnimatorInfos[i].IconMoveDir = newDir;
            m_resultUIAnimatorInfos[i].MoveInterval = 1.f; // 0.5秒ごとに移動方向を決定する
        }

        const float deltaTime = SceneManager::Instance().FrameDeltaTime();

        // 移動間隔を減らす
        m_resultUIAnimatorInfos[i].MoveInterval -= deltaTime;

        //x--- 移動 ---x//
        Math::Vector2 pos = m_resultUIAnimatorInfos[i].wpStarSpriteComponent.lock()->GetPos();

        const float moveSpeed = 8.0f;
        pos += m_resultUIAnimatorInfos[i].IconMoveDir * moveSpeed * deltaTime;

        // 最終的な移動先へセットする
        m_resultUIAnimatorInfos[i].wpStarSpriteComponent.lock()->SetPos(pos);
        m_resultUIAnimatorInfos[i].wpBaseStarSpriteComponent.lock()->SetPos(pos);
    }
}

void ResultUIAnimation::CalcNewDir(int idx, Math::Vector2& newDir)
{

    Math::Vector2 pos = m_resultUIAnimatorInfos[idx].wpStarSpriteComponent.lock()->GetPos();
    Math::Vector2 basePos = m_resultUIAnimatorInfos[idx].IconBasePos;

    // 目標位置との差分を計算
    Math::Vector2 diff = basePos - pos;

    // ランダムな方向を生成
    Math::Vector2 randomDir{
        static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-1.0, 1.0)),
        static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(-1.0, 1.0))
    };

    float dist = diff.Length(); // 距離を計算
    dist = std::clamp(dist, 0.0f, 1.0f); // dist を 0 ～ 1 に制限

    // basePos に近いほど randomDir が強くなるように調整
    newDir = Math::Vector2{
        (randomDir.x * (1.0f - dist)) + (diff.x * (dist - 0.8f)),
        (randomDir.y * (1.0f - dist)) + (diff.y * (dist - 0.8f))
    };

    newDir.Normalize();
}
