#pragma once

/**
 * @class BaseTransitionEffect
 *
 * @brief 画面遷移の基底クラス
 * @brief このクラスを継承して画面遷移のエフェクトを作成する場合は、
 *		  各フェーズごとの処理を作成して、外部からのPhaseの変更を行うことで画面遷移を完成させる
 *
 * @details このクラスを継承して画面遷移のエフェクトを作成する
 *			Draw関数は持っておらずUpdate関数でRendererへ描画命令を飛ばす形で処理を行う
 */
class BaseTransitionEffect
{
public:
    /* エフェクトの現在の進捗度をあらわす */
    enum class Phase
    {
        eNone,      // なし
        eStart,     // 開始
        eBlackOut,  // 暗転
        eComplete   // 完了
    };

    /* 画面遷移の状態をあらわす */
    enum class State
    {
        ePlay,  // 再生中
        eStop   // 停止中
    };


    BaseTransitionEffect()
    {
    }

    virtual ~BaseTransitionEffect()
    {
    }

    /* @brief 初期化 */
    virtual void Init() = 0;
    /* @brief 解放 */
    virtual void Release()
    {
    };

    /**
    * @brief 更新
    * @brief 画面遷移の進捗度を更新する
    * @brief Update関数内でRendererへ描画命令を飛ばす
    */
    void Update();

    void SetState(State state) { m_state = state; }
    State GetState() const { return m_state; }

    void SetPhase(Phase phase) { m_phase = phase; }
    Phase GetPhase() const { return m_phase; }

    bool IsPlay() const { return GetState() == State::eStop; }

    bool IsBlackOut() const { return GetPhase() == Phase::eBlackOut; }
    bool IsFinish() const { return GetPhase() == Phase::eComplete; }

protected:
    /* @brief Phase::eStart状態の更新処理 */
    virtual void StartUpdate() = 0;
    /* @brief Phase::eBlackOut状態の更新処理 */
    virtual void BlackOutUpdate() = 0;
    /* @brief Phase::eComplete状態の更新処理 */
    virtual void CompleteUpdate() = 0;

    /* 現在の進捗度 */
    Phase m_phase = Phase::eNone;
    /* 画面遷移の状態 */
    State m_state = State::eStop;
};

class BaseTransitionEffect;
/*
*
*/
class TransitionController
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    TransitionController()
    {
    }

    ~TransitionController() { Release(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    template <typename T>
    std::shared_ptr<T> GetFrontTransitionEffect() const
    {
        // リストが空なら何もしない
        if (m_spTransitionEffectList.empty()) { return nullptr; }

        // ダウンキャストして返す
        return std::dynamic_pointer_cast<T>(m_spTransitionEffectList.front());
    }

    void AddTransitionEffect(const std::shared_ptr<BaseTransitionEffect>& spTransitionEffect)
    {
        m_spTransitionEffectList.emplace_back(spTransitionEffect);
    }

    const std::shared_ptr<BaseTransitionEffect> GetTransitionEffectAndEmptyCheck() const
    {
        if (m_spTransitionEffectList.empty())
        {
            Assert::ErrorAssert("TransitionEffectがありません");
            return nullptr;
        }

        return m_spTransitionEffectList.front();
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    void Release();

    void Update()
    {
        // 画面遷移エフェクトの更新
        PlayTransitionEffect();

        // 画面遷移エフェクトの削除
        DeleteTransitionEffect();
    }

private:
    /**
    * @brief 画面遷移エフェクトの削除
    * @brief 画面遷移状態がPhase::eCompleteのものを削除する
    * @Hack SceneManager::PreUpdateで呼び出すことで更新中に削除するとエラーにならない？
    */
    void DeleteTransitionEffect();

    /**
    * @brief 画面遷移エフェクトの更新
    */
    void PlayTransitionEffect()
    {
        if (auto transition = GetFrontTransitionEffect<BaseTransitionEffect>(); transition)
        {
            transition->Update();
        }
    }

    /**
    * @brief 画面遷移エフェクトのリスト
    * @Hack リストの先頭のみを更新するのでキュー構造の方がいい？
    */
    std::list<std::shared_ptr<BaseTransitionEffect>> m_spTransitionEffectList = {};
};
