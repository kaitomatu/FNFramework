#pragma once

/**
* @class Timer
* @brief 時間計測クラス
* @details
*   時間計測を行うクラス : 1インスタンスごとに1つの時間計測を行える
*
* todo : 現在はスレッドセーフな処理になっていないので、std::mutexなどを利用したスレッドセーフなタイマークラスを作成する
*      : HasElapsedMultipleOfの挙動がおかしいので修正する
*/
class Timer
{
public:
    // タイマー用の型エイリアス //
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    // 時間の型エイリアス //
    using Hours = std::chrono::hours;
    using Minutes = std::chrono::minutes;
    using Seconds = std::chrono::seconds;
    using Milliseconds = std::chrono::milliseconds;
    using Microseconds = std::chrono::microseconds;
    using Nanoseconds = std::chrono::nanoseconds;

    //x--------------------------x
    // コンストラクタ / デストラクタ
    //x--------------------------x
    Timer(std::string_view name = "Timer")
        : m_name(name)
          , m_isRunning(false)
          , m_isPaused(false)
          , m_elapsedPausedTime(Milliseconds(0))
          , m_nextTargetTime(Milliseconds(0))
          , m_startTime({})
          , m_endTime({})
    {
    }

    ~Timer() = default;

    //x--------------------------x
    //     経過時間の取得
    //x--------------------------x
    /**
     * @fn template<typename DurationType = Milliseconds> DurationType Elapsed() const
     * @brief 経過時間の取得
     *
     * @tparam DurationType : 時間の型(Timer::Hoursなど)
     *
     * @return 型指定された経過時間
     */
    template <typename DurationType = Milliseconds>
    DurationType Elapsed() const
    {
        if (m_isPaused)
        {
            return std::chrono::duration_cast<DurationType>(m_elapsedPausedTime);
        }
        if (m_isRunning)
        {
            return std::chrono::duration_cast<DurationType>(Clock::now() - m_startTime);
        }
        return std::chrono::duration_cast<DurationType>(m_endTime - m_startTime);
    }

    //x--------------------------x
    //     時間経過の確認
    //x--------------------------x
    /**
     * @fn template<typename DurationType = Milliseconds> bool HasElapsedMoreThan(const DurationType& duration) const
     * @brief 経過時間が指定時間より大きいかどうか
     *  memo : 一定時間経過後に何かさせたい場合に利用する
     *
     * @tparam DurationType : 時間の型(Timer::Hoursなど)
     * @param duration : 比較する時間
     *
     * @return 経過時間が指定時間より大きいかどうか
     */
    template <typename DurationType = Milliseconds>
    bool HasElapsedMoreThan(const DurationType& duration) const
    {
        return Elapsed<DurationType>() >= duration;
    }

    /**
     * @fn template<typename DurationType = Milliseconds> bool HasElapsedLessThan(const DurationType& duration) const
     * @brief 経過時間が指定時間より小さいかどうか
     *  memo : duration秒間は真を返すなどの処理がしたい場合に利用する
     *
     * @tparam DurationType : 時間の型(Timer::Hoursなど)
     * @param duration : 比較する時間
     *
     * @return 経過時間が指定時間より小さいかどうか
     */
    template <typename DurationType = Milliseconds>
    bool HasElapsedLessThan(const DurationType& duration) const
    {
        return Elapsed<DurationType>() <= duration;
    }

    /**
     * @fn template<typename DurationType = Milliseconds> bool HasElapsedMultipleOf(const DurationType& duration)
     * @brief 経過時間が指定時間に達しているかどうか
     *  memo : 一定間隔で何かさせたい場合に利用する
     *
     * @tparam DurationType : 時間の型(Timer::Hoursなど)
     * @param duration : 次のタイマーセット時の時間
     *
     * @return 経過時間が指定時間に達しているかどうか
     */
    template <typename DurationType = Milliseconds>
    bool HasElapsedMultipleOf(const DurationType& duration)
    {
        // m_nextTargetTimeの場合は duration をセットする
        if(m_nextTargetTime == Milliseconds(0))
        {
            SetNextTargetTime(duration);
            return false;
        }

        if (Elapsed<DurationType>() >= m_nextTargetTime)
        {
            SetNextTargetTime(duration); // 次の目標時間を更新
            return true;
        }
        return false;
    }

    // 次の目標時間を設定する
    template <typename DurationType = Milliseconds>
    void SetNextTargetTime(const DurationType& duration)
    {
        m_nextTargetTime = Elapsed<DurationType>() + duration;
    }

    //x--------------------------x
    //         動作関係
    //x--------------------------x
    /**
     * @fn void Start()
     * @brief 計測開始
     */
    void Start();

    /**
     * @fn void Stop()
     * @brief 計測終了
     */
    void Stop();

    /**;
     * @fn void Pause()
     * @brief 一時停止
     * @details
     *  この関数が呼ばれたときに一度タイマーの計測を中止し、Resume()が呼ばれたときの時間を計測時間に足すことで処理を行う
     */
    void Pause();

    /**
     * @fn void Resume()
     * @brief 計測再開
     */
    void Resume();

    /**
     * @fn void Reset()
     * @brief 計測リセット
     */
    void Reset();

    //x--------------------------x
    //      各フラグ関係取得
    //x--------------------------x
    /**
     * @fn bool IsRunning() const
     * @brief 計測中かどうか
     * @return bool : 計測中ならtrue
     */
    bool IsRunning() const { return m_isRunning; }

    /**
     * @fn bool IsPaused() const
     * @brief 一時停止中かどうか
     * @return bool : 一時停止中ならtrue
     */
    bool IsPaused() const { return m_isPaused; }

    /**
     * @fn const std::string& GetName() const
     * @brief タイマーの名前を取得
     * @return std::string : タイマーの名前
     */
    const std::string& GetName() const { return m_name; }

    /**
     * @fn std::string ElapsedTimeAsString() const
     * @brief 経過時間を文字列で取得
     * @return std::string : 経過時間
     */
    std::string ElapsedTimeAsString() const
    {
        long long elapsed = this->Elapsed<Milliseconds>().count();

        Hours hours = std::chrono::duration_cast<Hours>(Milliseconds(elapsed));
        elapsed -= std::chrono::duration_cast<Milliseconds>(hours).count();

        Minutes minutes = std::chrono::duration_cast<Minutes>(Milliseconds(elapsed));
        elapsed -= std::chrono::duration_cast<Milliseconds>(minutes).count();

        Seconds seconds = std::chrono::duration_cast<Seconds>(Milliseconds(elapsed));
        elapsed -= std::chrono::duration_cast<Milliseconds>(seconds).count();

        Milliseconds milliseconds = Milliseconds(elapsed);

        std::ostringstream oss;
        if (hours.count() > 0) oss << hours.count() << "h ";
        if (minutes.count() > 0 || hours.count() > 0) oss << minutes.count() << "m ";
        oss << seconds.count() << "s " << milliseconds.count() << "ms";

        return oss.str();
    }

private:
    // タイマーの名前
    std::string m_name;

    // 計測開始しているかどうか
    bool m_isRunning;

    // ポーズ中かどうか
    bool m_isPaused;

    // ポーズ中に経過した時間
    Milliseconds m_elapsedPausedTime;

    // 計測開始時間
    TimePoint m_startTime;

    // 計測終了時間
    TimePoint m_endTime;

    // 次の目標時間
    Milliseconds m_nextTargetTime;
};
