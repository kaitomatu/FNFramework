#include "SoundData.h"

bool SoundData::CreateSoundVoice(const std::shared_ptr<WaveData>& waveData, bool isLoop)
{
    if (!waveData)
    {
        FNENG_ASSERT_ERROR("WaveDataが無効です");
        return false;
    }

    const std::unique_ptr<WaveData::Data>& data = waveData->GetData();

    //------------------
    // SoundVoice作成
    //------------------
    WAVEFORMATEX waveFormat{};

    // 波形フォーマットの設定
    std::memcpy(&waveFormat, &data->WavFormat, sizeof(data->WavFormat));

    // 1サンプル当たりのバッファサイズを計算する
    waveFormat.wBitsPerSample = data->WavFormat.nBlockAlign * 8 / data->WavFormat.nChannels;

    // ソースボイスの作成 ここではフォーマットのみ渡っている
    HRESULT result = AudioDevice::Instance().GetXAudio2()->CreateSourceVoice(&m_pSourceVoice, &waveFormat);
    if (FAILED(result))
    {
        FNENG_ASSERT_ERROR("SourceVoice作成失敗");
        return false;
    }

    //==================================
    // 読み込んだデータ(音データ本体)を
    // ソースボイスにセットする
    //==================================
    XAUDIO2_BUFFER xAudio2Buffer{};
    xAudio2Buffer.pAudioData = (BYTE*)data->SoundBuffer;
    xAudio2Buffer.Flags = XAUDIO2_END_OF_STREAM;
    xAudio2Buffer.AudioBytes = data->BuffSize;

    // 三項演算子を用いて、ループするか否かの設定をする
    xAudio2Buffer.LoopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;
    result = m_pSourceVoice->SubmitSourceBuffer(&xAudio2Buffer);

    if (FAILED(result))
    {
        // エラー処理
        FNENG_ASSERT_ERROR("音データのセットに失敗しました。");
        return false;
    }

    return true;
}

void SoundData::Play()
{
    if (!m_pSourceVoice)
    {
        FNENG_ASSERT_ERROR("SourceVoiceが無効です");
        return;
    }

    if (!m_isRunning)
    {
        m_pSourceVoice->Start();
        m_isRunning = true;
    }

    // これを一括で管理するクラスを作る
    IsSoundQueueZero();
}
