#include "FPSController.h"

void FPSController::UpdateFPS(bool waitForNextFrame)
{
    // 残りの待機時間を計算
    long long waitTime = static_cast<long long>(m_fpsWaitTime - m_overWaitTime - m_deltaTime);

    m_overWaitTime = 0;

    // フレームが目標より短い場合、必要に応じて待機
    if (waitForNextFrame && waitTime > 0)
    {
        std::this_thread::sleep_for(Timer::Microseconds(waitTime));
    }

    // 待機後の超過時間を計算
    m_overWaitTime = m_timer.Elapsed<Timer::Microseconds>().count() - m_deltaTime - waitTime;

    // FPSの計測
    Monitoring();

    // 経過時間をタイマーから取得（マイクロ秒単位）
    m_deltaTime = m_timer.Elapsed<Timer::Microseconds>().count();

    // 次のフレームに備えてタイマーを再スタート
    m_timer.Start();
}

void FPSController::Monitoring()
{
    // FPSを計測するリフレッシュ間隔
    constexpr float refreshTime = 0.5f;

    // フレームのカウントを増やす
    m_frameCount++;

    // 累積時間に現在のフレームの経過時間を追加
    m_accumulatedTime += m_deltaTime / MicrosecondsInSecond;

    // 累積時間がリフレッシュ間隔を超えたらFPSを計測
    if (m_accumulatedTime < refreshTime) { return; }

    // FPS = フレーム数 / 経過時間
    m_nowFPS = static_cast<int>(static_cast<double>(m_frameCount) / m_accumulatedTime);

    // カウントをリセット
    m_frameCount = 0;
    m_accumulatedTime = 0.0;
}
