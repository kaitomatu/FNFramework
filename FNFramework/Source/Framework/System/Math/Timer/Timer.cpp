#include "Timer.h"

void Timer::Start()
{
    if (!m_isPaused)
    {
        m_startTime = Clock::now();
        m_elapsedPausedTime = Milliseconds(0); // リセット
    }
    else
    {
        // 再開する時は経過時間を保持しながらスタート
        m_startTime = Clock::now() - m_elapsedPausedTime;
        m_elapsedPausedTime = Milliseconds(0);
    }

    m_isRunning = true;
    m_isPaused = false;
}

void Timer::Stop()
{
    if (!m_isRunning)
    {
        FNENG_ASSERT_LOG(m_name + "が開始されていません", false)
        return;
    }

    m_isRunning = false;
    m_endTime = Clock::now();
    m_nextTargetTime = Milliseconds(0);
}

void Timer::Pause()
{
    // 計測開始していない場合は何もしない
    if(m_startTime == TimePoint()) { return; }
    if (!m_isRunning && m_isPaused) { return; }

    m_endTime = Clock::now();
    m_elapsedPausedTime += std::chrono::duration_cast<Milliseconds>(m_endTime - m_startTime);

    m_nextTargetTime = Milliseconds(0);

    m_isPaused = true;
    m_isRunning = false; // 実際の計測は停止
}

void Timer::Resume()
{
    if(!m_isPaused)
    {
        FNENG_ASSERT_LOG(m_name +  "のTimer::Pause()が呼ばれていません", /* isOutput = */false)
        return;
    }

    Start();
}

void Timer::Reset()
{
    m_isRunning = false;
    m_isPaused = false;
    m_nextTargetTime = Milliseconds(0);
    m_elapsedPausedTime = Milliseconds(0);
    m_startTime = {};
    m_endTime = {};
}
