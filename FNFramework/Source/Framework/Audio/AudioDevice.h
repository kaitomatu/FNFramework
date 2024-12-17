#pragma once

//----------------------
// Wave
//----------------------
// Waveファイルのデータ本体
class WaveData
{
public:
    struct Data
    {
        WAVEFORMATEX WavFormat; // Waveファイルのフォーマット
        char* SoundBuffer; // Waveファイルのデータ(音の波形データ)
        DWORD BuffSize; // Waveファイルのデータサイズ

        ~Data() { delete[] SoundBuffer; }
    };

    WaveData()
    {
    }

    ~WaveData() { m_data = nullptr; }

    const std::unique_ptr<Data>& GetData() const { return m_data; }

    /**
    * @brief Waveファイルの読み込み
    * @param[in] wFilePath - 読み込むWaveファイルのパス
    * @param[out] outData - Waveファイルのデータ
    * @return 読み込みに成功したらtrue
    */
    bool Load(std::string_view filePath);

private:
    // Waveファイルのデータ
    // サウンドエフェクト
    std::unique_ptr<Data> m_data = nullptr;

    // コピー禁止用:単一のデータはコピーできない
    WaveData(const WaveData& src) = delete;
    void operator=(const WaveData& src) = delete;
};

//----------------------
// AudioDevice
// 音関係のデータをまとめて管理しているクラス
//----------------------
class SoundData;

class AudioDevice
    : public utl::Singleton<AudioDevice>
{
    friend class utl::Singleton<AudioDevice>;

public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    AudioDevice()
        : m_pMasteringVoice(nullptr),
          // マスターボイスの解放関数を指定している
          m_pXAudio2(nullptr)
    {
    }

    ~AudioDevice() override { Release(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    IXAudio2* GetXAudio2() const { return m_pXAudio2; }
    IXAudio2MasteringVoice* GetMasteringVoice() const { return m_pMasteringVoice; }

    //--------------------------------
    // その他関数
    //--------------------------------
    bool Create();
    void Release();

    /**
    * @brief PlayListにデータを追加する
    * @param[in] sound - 追加するSoundData
    */
    void AddPlayList(const std::shared_ptr<SoundData>& sound);

    /**
    * @brief WaveDataを取得する
    * @brief すでに読み込まれているデータはそのまま返し、初回登録の場合は読み込んでから返す
    * @param[in] filePath - 読み込むWaveファイルのパス
    * @return 指定されたWaveData
    */
    std::shared_ptr<WaveData> GetSoundData(std::string_view filePath);

    std::shared_ptr<SoundData> Play(std::string_view filePath, bool isLoop = false, float volume = 1.0f);

private:
    /*/* XAudio2デバイス */ /*
	*	  IXAudio2SourceVoice
	*			 V
	*	( IXAudio2SubmixVoice ) - あってもなくてもどちらでもよいが、あると用途ごとに設定ができる？
	*			 V
	*  IXAudio2MasteringVoice
	*/
    IXAudio2MasteringVoice* m_pMasteringVoice = nullptr;
    IXAudio2* m_pXAudio2 = nullptr;
    // 現在再生している音データ
    // m_waveDataMapから取得して再生を行う
    std::map<size_t, std::shared_ptr<SoundData>> m_playList = {};

    // ゲーム内で使用するWaveファイルのデータ
    // 一度読み込んだWaveファイルはこのマップに格納しておく
    std::unordered_map<std::string, std::shared_ptr<WaveData>> m_waveDataMap = {};
};
