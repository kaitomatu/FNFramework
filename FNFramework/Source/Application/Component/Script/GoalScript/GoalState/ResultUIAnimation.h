#pragma once

class GoalScript;
class SpriteComponent;
/**
* @class ResultUIAnimation
* @brief
* @details
*   -
*/
class ResultUIAnimation
{
public:

    enum class ResultUIState
    {
        eIsClear = 0,
        eTargetTime,
        eCollective,
        eCount
    };

    struct InitResultUIData
    {
        struct InitResultUI
        {
            // クラゲの下地: グレーのクラゲ
            std::shared_ptr<SpriteComponent> spBaseStarSpriteComponent;
            // クラゲの星: メインのクラゲ
            std::shared_ptr<SpriteComponent> spStarSpriteComponent;
            //x 達成条件のテキストのインスタンス x//
            std::shared_ptr<GameObject> spClearTextObject;
        } InitResultUIs[static_cast<size_t>(ResultUIState::eCount)];
    };

    void Init(const GoalScript* _spOwner, InitResultUIData _initData);
    void Update();

    bool IsAchieveClear(ResultUIState _state);

private:

    /**
     * 構成要素
     *  - 目標が達成されているかどうか
     *  - 星(クラゲのテクスチャ操作)のインスタンス
     *  - ↑に紐づいているテキスト(スプライトコンポーネント)
     */
    // リザルト画面で利用するアニメーション情報
    struct ResultUIAnimatorInfo
    {
        bool IsClearAchieve = false;
        // クラゲの下地: グレーのクラゲ
        std::weak_ptr<SpriteComponent> wpBaseStarSpriteComponent;
        // クラゲの星: メインのクラゲ
        std::weak_ptr<SpriteComponent> wpStarSpriteComponent;
        float Alpha = 0.0f;

        Math::Vector2 IconBasePos = Math::Vector2::Zero;
        Math::Vector2 IconMoveDir = Math::Vector2::Zero;

        //x 達成条件のテキストのインスタンス x//
        std::shared_ptr<GameObject> spClearTextObject;

        float MoveInterval = 0.0f;
    };

    // クリア条件のチェック
    void CheckClearAchieve(const GoalScript* _pOwner);

    void SetClearTextSpriteDataFromTargetTime(const std::shared_ptr<GameObject>& _spObj);
    void SetClearTextSpriteDataFromCollective(const std::shared_ptr<GameObject>& _spObj);

    // UIアニメーションの変更
    void ChangeUIState();

    // クリア条件テキストの有効 / 無効の切替
    void ChangeClearTextUpdateActive();

    // クラゲUIの移動アニメーション制御
    void KurageUIAnimationControl();
    // 移動方向の計算
    void CalcNewDir(int idx, Math::Vector2& newDir);

    ResultUIState m_state = ResultUIState::eIsClear;

    // ステージに存在する | ステージクリア時のクラゲの数
    int m_stageChildNum = 0;
    int m_stageClearChildNum = 0;

    // ステージクリアの目標時間 | ステージクリア時の時間
    int m_targetSecTime = 0;
    int m_clearSecTime = 0;

    std::array<ResultUIAnimatorInfo, static_cast<size_t>(ResultUIState::eCount)> m_resultUIAnimatorInfos;
};
