#pragma once

class WaveData;

class SoundData
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    SoundData()
    {
    }

    ~SoundData()
    {
        if (!m_pSourceVoice)
        {
            m_pSourceVoice->DestroyVoice();
            m_pSourceVoice = nullptr;
        }
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /* @biref SoundVoiceが再生中かどうか */
    bool IsRunning() const { return m_isRunning; }

    void SetVolume(float volume) { m_pSourceVoice->SetVolume(volume); }
    void SetPitch(float pitch) { m_pSourceVoice->SetFrequencyRatio(pitch); }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief Waveファイルの読み込み
    * @param[in] fileName - 読み込むWaveファイルのパス
    * @param[out] waveData - Waveファイルのデータ
    * @param[in] isLoop - ループ再生するかどうか
    * @return 読み込みに成功したらtrue
    */
    bool CreateSoundVoice(const std::shared_ptr<WaveData>& waveData, bool isLoop = false);

    /* @brief SourceVoiceの破棄 */
    void DestroyVoice() { m_pSourceVoice->DestroyVoice(); }
    /* @brief SoundQueueが0かどうか */
    void IsSoundQueueZero()
    {
        XAUDIO2_VOICE_STATE state;
        m_pSourceVoice->GetState(&state);

        // バッファのキューが0なら再生終了
        if ((state.BuffersQueued > 0) == 0)
        {
            m_pSourceVoice->DestroyVoice();
            //m_pSourceVoice = nullptr;
            m_isRunning = false;
        }
    }

    void Play();

private:
    // 音の基になるデータ(同じ音を鳴らす場合その数分必要)
    IXAudio2SourceVoice* m_pSourceVoice = nullptr;

    bool m_isRunning = false;
};
