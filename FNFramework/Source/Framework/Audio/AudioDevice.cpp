#include "AudioDevice.h"
#include "SoundData.h"

//--------------------------------
// WaveData

bool WaveData::Load(std::string_view filePath)
{
    // 初回の場合はメモリ確保を行う
    if (!m_data)
    {
        m_data = std::make_unique<Data>();
    }
    // すでに作成されている場合は音データを一度開放しておく
    else
    {
        delete[] m_data->SoundBuffer;
    }

    HMMIO mmioHandle = nullptr;

    // チャンク情報
    MMCKINFO chunkInfo{};
    // RIFFチャンク用
    MMCKINFO riffChunk{};

    // ファイルパスをワイド文字列に変換
    std::wstring wFilePath = sjis_to_wide(filePath.data());

    // ファイルを開く
    mmioHandle = mmioOpen(
        wFilePath.data(),
        nullptr,
        MMIO_READ
    );

    if (!mmioHandle)
    {
        FNENG_ASSERT_ERROR("ファイルを開けませんでした");
        return false;
    }

    // RIFFチャンクに侵入するためにfccTypeにWAVEを設定をする
    riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');

    // RIIFFチャンクにを解析する
    if (mmioDescend(
        mmioHandle, // MMIOハンドル
        &riffChunk, // チャンク情報
        nullptr, // チャンク情報(親)
        MMIO_FINDRIFF // 取得情報種類
    ) != MMSYSERR_NOERROR)
    {
        FNENG_ASSERT_ERROR("RIFFチャンクを解析できませんでした");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        return false;
    }

    // 解析先のチャンクを"fmt"として設定する
    chunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(
        mmioHandle,
        &chunkInfo,
        &riffChunk,
        MMIO_FINDCHUNK
    ) != MMSYSERR_NOERROR)
    {
        FNENG_ASSERT_ERROR("fmtチャンクがないです");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        return false;
    }

    // fmtデータの読み込み
    DWORD readSize = mmioRead(
        mmioHandle, // MMIOハンドル
        (HPSTR)&m_data->WavFormat, // 読み込み先
        chunkInfo.cksize // 読み込みサイズ
    );

    if (readSize != chunkInfo.cksize)
    {
        FNENG_ASSERT_ERROR("読み込みサイズが違います");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        return false;
    }

    // フォーマットチェック
    if (m_data->WavFormat.wFormatTag != WAVE_FORMAT_PCM)
    {
        FNENG_ASSERT_ERROR("Waveフォーマットではない");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        return false;
    }

    // fmtチャンクを退出する
    if (mmioAscend(mmioHandle, &chunkInfo, 0) != MMSYSERR_NOERROR)
    {
        FNENG_ASSERT_ERROR("fmtチャンク退出失敗");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        return false;
    }

    // dataチャンクを解析
    chunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(mmioHandle, &chunkInfo, &riffChunk, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
    {
        FNENG_ASSERT_ERROR("dataチャンク侵入失敗");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        return false;
    }
    // サイズ保存
    m_data->BuffSize = chunkInfo.cksize;

    // dataチャンクの読み込み
    m_data->SoundBuffer = new char[chunkInfo.cksize];

    readSize = mmioRead(mmioHandle, m_data->SoundBuffer, chunkInfo.cksize);
    if (readSize != chunkInfo.cksize)
    {
        FNENG_ASSERT_ERROR("dataチャンク読み込み失敗");
        mmioClose(mmioHandle, MMIO_FHOPEN);
        delete[] m_data->SoundBuffer;
        return false;
    }

    // ファイルを閉じる
    mmioClose(mmioHandle, MMIO_FHOPEN);

    return true;
}

//--------------------------------
// AudioDevice

bool AudioDevice::Create()
{
    //--------------------------------
    // XAudio2デバイスの作成
    //--------------------------------
    HRESULT result = XAudio2Create(&m_pXAudio2);
    if (FAILED(result))
    {
        FNENG_ASSERT_ERROR("XAudio2デバイスの作成に失敗しました。");
        return false;
    }

    //--------------------------------
    // マスターボイスの作成
    //--------------------------------
    HRESULT hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);

    if (FAILED(hr))
    {
        // エラー処理
        FNENG_ASSERT_ERROR("マスターボイスの作成に失敗しました。");
        return false;
    }

    return true;
}

void AudioDevice::Release()
{
    // マスターボイスの解放
    if (m_pMasteringVoice)
    {
        m_pMasteringVoice->DestroyVoice();
        m_pMasteringVoice = nullptr;
    }

    // XAudio2デバイスの解放
    if (m_pXAudio2)
    {
        m_pXAudio2->Release();
        m_pXAudio2 = nullptr;
    }

    // Waveデータの解放
    if (!m_waveDataMap.empty())
    {
        m_waveDataMap.clear();
    }

    // SoundDataの解放
    if (!m_playList.empty())
    {
        m_playList.clear();
    }
}

void AudioDevice::AddPlayList(const std::shared_ptr<SoundData>& sound)
{
    if (!sound) { return; }

    m_playList[(size_t)sound.get()] = sound;
}

std::shared_ptr<WaveData> AudioDevice::GetSoundData(std::string_view filePath)
{
    // すでに読み込まれている場合はそのまま返す
    auto it = m_waveDataMap.find(filePath.data());
    if (it != m_waveDataMap.end())
    {
        return it->second;
    }
    // 読み込まれていない場合は読み込んでから返す
    // Waveファイルの読み込み
    auto waveData = std::make_shared<WaveData>();
    if (!waveData->Load(filePath))
    {
        FNENG_ASSERT_ERROR("Waveファイルの読み込みに失敗しました。");
        return nullptr;
    }

    // 読み込んだWaveファイルを登録する
    m_waveDataMap.emplace(filePath, waveData);
    return waveData;
}

std::shared_ptr<SoundData> AudioDevice::Play(std::string_view filePath, bool isLoop, float volume)
{
    if (!m_pXAudio2)
    {
        FNENG_ASSERT_ERROR("XAudio2デバイスが無効です");
        return nullptr;
    }

    // m_waveDataMapから音データを取得する
    std::shared_ptr<WaveData> soundData = GetSoundData(filePath);

    if (!soundData) { return nullptr; }

    // 再生するサウンド
    auto sound = std::make_shared<SoundData>();

    if (!sound->CreateSoundVoice(soundData, isLoop))
    {
        FNENG_ASSERT_ERROR("音データの作成に失敗しました。");
        return nullptr;
    }

    sound->SetVolume(volume);
    sound->Play();

    AddPlayList(sound);

    return sound;
}
