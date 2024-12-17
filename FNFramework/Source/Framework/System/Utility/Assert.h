#pragma once

/**
 * std::source_location::current()が呼び出し元で展開されるようにマクロを使用
 */
// マクロでSET_LOCATIONとErrorAssertをセットで呼び出す : isOutputがtrueの場合はログファイルにも出力
#define FNENG_ASSERT_LOG(message, isOutput) \
    Assert::WarningLog(message, isOutput, std::source_location::current());

// マクロでSET_LOCATIONとErrorAssertをセットで呼び出す
#define FNENG_ASSERT_ERROR(message) \
    Assert::ErrorAssert(message, std::source_location::current());

// マクロでSET_LOCATIONとErrorAssert_Fをセットで呼び出す
#define FNENG_ASSERT_ERROR_F(format, ...) \
    Assert::ErrorAssert_F(std::source_location::current(), format, __VA_ARGS__);

#define FNENG_ASSERT_ERROR_BLOB(errorBlob) \
    Assert::OutputErrorBlob(errorBlob, std::source_location::current());

namespace Assert
{
    // 日時を文字列で取得するヘルパー関数
    inline std::string GetCurrentDateTime()
    {
        // 現在の日時を取得
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        // 取得した日時を文字列に変換
        std::tm tm{};
        localtime_s(&tm, &in_time_t);

        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");

        return ss.str();
    }

    // 書き込むファイルのパス
    static const std::string FlolderPath = "Assets/Data/Log/AssertLog/";
    // アプリケーション実行時の日時をファイル名にする
    static const std::string FileName = FlolderPath + "/" + GetCurrentDateTime() + "Error.log";

    // エラーログをファイルに出力する関数
    inline void LogErrorToFile(const std::string& errorMessage, const std::string& filePath, int line)
    {
        std::string datetime = GetCurrentDateTime();

        // フォルダが存在しない場合は作成する
        std::filesystem::create_directories(FlolderPath);

        std::ofstream logFile(FileName, std::ios::out | std::ios::app);

        // ファイルが有効な場合はエラー内容と日時を書き込む
        if (logFile.is_open())
        {
            logFile << "DateTime: " << datetime << "\n"
                << "ErrorMessage: " << errorMessage << "\n"
                << "File: " << filePath << "\n"
                << "Line: " << line << "\n"
                << "-----------------------------------\n";
            logFile.close();
        }
    }

#ifdef _DEBUG
    /* @brief エラーメッセージをログのみに出力 */
    inline void WarningLog(std::string_view _errMsg, bool _outputFile = true, const std::source_location& location = std::source_location::current())
    {
        //==文字列をワイド文字列に変換する===
        std::wstring wErrStr = sjis_to_wide("Message : ") + sjis_to_wide(_errMsg.data());
        std::wstring wErrFile =
            sjis_to_wide("FilePath : ") + sjis_to_wide(location.file_name()) + L'\n' +
            sjis_to_wide("line : ") + sjis_to_wide(std::to_string(location.line()));
        //===========ここまで==============

        std::wstring wOutputStr = wErrFile + L'\n' + wErrStr + L'\n';

        if(_outputFile)
        {
            LogErrorToFile(_errMsg.data(), location.file_name(), (UINT)location.line());
        }

        OutputDebugString(wOutputStr.c_str());
    }

    /* @brief エラーメッセージ出力 - アプリケーション停止 */
    inline void ErrorAssert(std::string_view _errMsg, const std::source_location& location = std::source_location::current())
    {
        //==文字列をワイド文字列に変換する===
        std::wstring wErrStr = sjis_to_wide(_errMsg.data());
        std::wstring wErrFile = sjis_to_wide(location.file_name());
        //===========ここまで==============

        LogErrorToFile(_errMsg.data(), location.file_name(), (UINT)location.line());

        _wassert(wErrStr.data(), wErrFile.data(), (UINT)location.line());
    }

    inline void ErrorAssert_F(const std::source_location& location, const char* format, ...)
    {
        // 初期バッファサイズ
        size_t bufferSize = 1024;
        std::vector<char> buffer(bufferSize);

        // 可変引数リストの初期化
        va_list args;
        va_start(args, format);

        // vsnprintfの戻り値を確認して、バッファが足りない場合に拡張する
        int neededSize = vsnprintf(buffer.data(), buffer.size(), format, args);
        if (neededSize >= static_cast<int>(bufferSize))
        {
            buffer.resize(neededSize + 1); // 必要なサイズにバッファを拡張
            vsnprintf(buffer.data(), buffer.size(), format, args);
        }

        // 可変引数リストのクリーンアップ
        va_end(args);

        //==文字列をワイド文字列に変換する===
        std::wstring wErrStr = sjis_to_wide(buffer.data());
        std::wstring wErrFile = sjis_to_wide(location.file_name());
        //===========ここまで==============

        // ログファイルに出力
        std::string err = std::string(buffer.data());
        LogErrorToFile(err, location.file_name(), (UINT)location.line());

        _wassert(wErrStr.data(), wErrFile.data(), (UINT)location.line());
    }

    /**
    * @brief ID3DBlob内の文字出力
    *
    * @param errorBlob - 出力ウィンドウに出力したいID3DBlob
    */
    inline void OutputErrorBlob(ID3DBlob* errorBlob, const std::source_location& location = std::source_location::current())
    {
        if (!errorBlob) return;

        const char* bufferData = static_cast<char*>(errorBlob->GetBufferPointer());
        size_t bufferSize = errorBlob->GetBufferSize();

        std::string errstr(bufferData, bufferSize);
        errstr += "\n";

        ErrorAssert(errstr.c_str(), location);
    }

#else
    /* @brief エラーメッセージをログのみに出力 */
    inline void WarningLog(std::string_view _errMsg, bool _outputFile = true, const std::source_location& location = std::source_location::current())
    {
        //==文字列をワイド文字列に変換する===
        std::wstring wErrStr = sjis_to_wide("Message : ") + sjis_to_wide(_errMsg.data());
        std::wstring wErrFile =
            sjis_to_wide("FilePath : ") + sjis_to_wide(location.file_name()) + L'\n' +
            sjis_to_wide("line : ") + sjis_to_wide(std::to_string(location.line()));
        //===========ここまで==============

        std::wstring wOutputStr = wErrFile + L'\n' + wErrStr + L'\n';

        if(_outputFile)
        {
            LogErrorToFile(_errMsg.data(), location.file_name(), (UINT)location.line());
        }
    }

    /* @brief エラーメッセージ出力 - アプリケーション停止 */
    inline void ErrorAssert(std::string_view errMsg, const std::source_location& location = std::source_location::current())
    {
        //==文字列をワイド文字列に変換する===
        std::wstring wErrStr = sjis_to_wide(errMsg.data());
        std::wstring wErrFile = sjis_to_wide(location.file_name());
        //===========ここまで==============

        LogErrorToFile(errMsg.data(), location.file_name(), (UINT)location.line());
    }

    inline void ErrorAssert_F(const std::source_location& location, const char* format, ...)
    {
        // 初期バッファサイズ
        size_t bufferSize = 1024;
        std::vector<char> buffer(bufferSize);

        // 可変引数リストの初期化
        va_list args;
        va_start(args, format);

        // vsnprintfの戻り値を確認して、バッファが足りない場合に拡張する
        int neededSize = vsnprintf(buffer.data(), buffer.size(), format, args);
        if (neededSize >= static_cast<int>(bufferSize))
        {
            buffer.resize(neededSize + 1); // 必要なサイズにバッファを拡張
            vsnprintf(buffer.data(), buffer.size(), format, args);
        }

        // 可変引数リストのクリーンアップ
        va_end(args);
        //===========ここまで==============

        // ログファイルに出力
        std::string err = std::string(buffer.data());
        LogErrorToFile(err, location.file_name(), (UINT)location.line());
    }

    /* @brief ID3DBlob内の文字出力 */
    inline void OutputErrorBlob(ID3DBlob* errorBlob, const std::source_location& location = std::source_location::current())
    {
    }
#endif // DEBUG
}
