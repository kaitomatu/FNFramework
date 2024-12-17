#pragma once

class FPSController
{
public:
    // コンストラクタ：目標FPSを設定
    FPSController(double targetFPS = 180.0)
        : m_fpsWaitTime(1.0 / targetFPS * MicrosecondsInSecond)
        , m_deltaTime(0.0)
        , m_overWaitTime(0.0)
        , m_targetFPS(targetFPS)
    {
        m_timer.Start();
    }

    // FPSコントロールの更新
    void UpdateFPS(bool waitForNextFrame = true);

    // 目標FPSの設定
    double GetTargetFPS() const { return m_targetFPS; }
    void SetTargetFPS(double targetFPS)
    {
        // 目標FPSが 0 になると操作不可能になるため、 1 未満の場合は受け付けない
        if(targetFPS <= 1.0) { return; }

        m_targetFPS = targetFPS;
        m_fpsWaitTime = 1.0 / m_targetFPS * MicrosecondsInSecond;
    }

    // 経過時間を秒単位で取得
    double GetDeltaTime() const
    {
        double delta = m_deltaTime / MicrosecondsInSecond;

        // 大きくなりすぎた場合は clamp する
        delta = std::clamp(delta, 0.0, 0.1);

        return delta;
    }

    int GetCurrentFPS() const { return m_nowFPS; }

private:
    //x--- FPS制御関連 ---x//
    double m_targetFPS;    // 目標FPS
    double m_fpsWaitTime;  // 1フレームに待機する時間（マイクロ秒）
    double m_deltaTime;    // 前フレームからの経過時間
    double m_overWaitTime; // 前のフレームで多めに待った時間

    Timer m_timer;         // タイマーインスタンス

    //x--- FPS算出用 ---x//
    // 一秒ごとにFPSの算出を行う
    void Monitoring();

    int m_frameCount = 0;                // フレームカウント
    double m_fpsMonitorBeginTime = 0.0;  // FPS計測開始時間
    double m_accumulatedTime = 0.0;      // 累積時間
    int m_nowFPS = 0;                    // 現在のFPS

    // マイクロ秒を秒に変換するための定数
    static constexpr double MicrosecondsInSecond = 1000000.0;
};
