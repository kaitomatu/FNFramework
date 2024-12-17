#pragma once

namespace RndHelper
{
    // 書き込むファイルのパス
    static const std::string FlolderPath = "Assets/Data/Json/Random/";
    // アプリケーション実行時の日時をファイル名にする
    static const std::string FileName = FlolderPath + "random.json";
}

namespace utl
{
    /**
    * @class RandomHelper
    * @brief 乱数生成のヘルパークラス
    * @details
    *   - std::mt19937を使用して乱数を生成する
    *   - 乱数のシード値や履歴を一貫させるためにシングルトンで実装
    */
    class RandomHelper
        : public utl::Singleton<RandomHelper>
    {
        friend class utl::Singleton<RandomHelper>;

    public:
        //x--------------------------------x//
        //x            乱数取得             x//
        //x--------------------------------x//
        /**
         * @fn int GetRandomInt(int min, int max)
         * @brief min以上max以下の整数型の乱数を取得
         * @param min : 取得する乱数の最小値
         * @param max : 取得する乱数の最大値
         * @return min以上max以下の整数型の乱数
         */
        int GetRandomInt(int min, int max)
        {
            // min と max が逆転していたら入れ替える
            if (min > max) { std::swap(min, max); }

            std::uniform_int_distribution<int> dist(min, max);

            int rand = dist(m_randMachine);

            // 履歴情報の更新 : m_rangHistoryIdxが最大値を超えたら上書きする
            m_rangHistorys[m_rangHistoryIdx % RandomHistoryMax] = rand;
            m_rangHistoryIdx++;

            return rand;
        }

        /**
         * @fn double GetRandomDouble(double min, double max)
         * @brief min以上max以下の浮動型の乱数を取得
         * @param min : 取得する乱数の最小値
         * @param max : 取得する乱数の最大値
         * @return min以上max以下の浮動型の乱数
         */
        double GetRandomDouble(double min, double max)
        {
            // min と max が逆転していたら入れ替える
            if (min > max) { std::swap(min, max); }

            std::uniform_real_distribution<double> dist(min, max);

            double rand = dist(m_randMachine);

            // 履歴情報の更新 : m_rangHistoryIdxが最大値を超えたら上書きする
            m_rangHistorys[m_rangHistoryIdx % RandomHistoryMax] = static_cast<int>(rand);
            m_rangHistoryIdx++;

            return rand;
        }

        /**
         * @fn bool GetBernoulli(double p)
         * @brief この確率でtrueを返しそれ以外は false を返すベルヌーイ分布に基づく乱数を生成
         * @param p : 0.0 ~ 1.0の確率(0.5 = 50%で true を返す)
         * @return ベルヌーイ分布に基づくtrue / false
         */
        bool GetBernoulli(double p)
        {
            // ベルヌーイ分布に基づく乱数生成
            p = std::clamp(p, 0.0, 1.0);
            std::bernoulli_distribution dist(p);
            return dist(m_randMachine);
        }

        //x--------------------------------x//
        //x          シードの設定            x//
        //x--------------------------------x//

        /**
         * @fn UINT GetCurrentSeed() const
         * @brief 現在のシード値を取得
         * @return 現在のシード値を取得
         */
        UINT GetCurrentSeed() const { return currentSeed; }

        /**
         * @fn void SetSeed(unsigned int seed)
         * @brief 指定されたシードで乱数生成機のシード値を設定
         * @param seed : 乱数生成機のシード値
         */
        void SetSeed(UINT seed)
        {
            m_randMachine.seed(seed);
            currentSeed = seed; // シード値を更新
        }

        /**
         * @fn void ResetSeed()
         * @brief ランダムナシードでに乱数生成器のシード値を設定
         */
        void ResetSeed()
        {
            SetSeed(std::random_device()());
        }

        //x--------------------------------x//
        //x          履歴の操作             x//
        //x--------------------------------x//

        static constexpr int RandomHistoryMax = 100; // 乱数の履歴の最大数

        // 乱数の履歴を取得 / クリア
        const std::array<int, RandomHistoryMax>& GetHistory() const { return m_rangHistorys; }

        void ClearHistory()
        {
            m_rangHistorys = {};
        }

        //----- ファイル出力 / 読み込み -----//
        void SaveState(std::string_view filePath) const
        {
            // Jsonオブジェクトに履歴を保存
            JsonWrapper j;
            j.SetValue("History", m_rangHistorys);
            j.SetValue("Seed", currentSeed);
            j.SaveToFile(filePath);
        }

        void LoadState(std::string_view filePath)
        {
            JsonWrapper jsonWrapper;

            if (!jsonWrapper.LoadFromFile(filePath)) { return; }

            // シード値を復元 -> 乱数生成機のシード値を設定
            currentSeed = jsonWrapper.GetValue<UINT>("Seed", 0);
            m_randMachine.seed(currentSeed);

            // 履歴を復元
            m_rangHistorys = jsonWrapper.GetValue<std::array<int, RandomHistoryMax>>("History", {});
        }

    private:
        //----- 乱数生成機 -----//
        // 乱数生成機 : ここにアクセスして乱数を生成する
        std::mt19937 m_randMachine;
        // 現在のシード値
        UINT currentSeed = 0;

        //----- 履歴関係の変数 -----//
        // 履歴にアクセスするためのインデックス / 履歴の最大数
        UINT m_rangHistoryIdx = 0;
        // 生成された乱数の履歴
        std::array<int, RandomHistoryMax> m_rangHistorys = {};

    private:
        // コンストラクタで乱数エンジンを初期化
        RandomHelper()
        {
            // 一度だけ random_device() を呼び出して、そのシードを両方に設定
            ResetSeed();
        }

        ~RandomHelper() override
        {
            SaveState(RndHelper::FileName);
            ClearHistory();
        }
    };
}
