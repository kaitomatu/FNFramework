#include "ImGuiHelper.h"

/**
 * @brief ImGui の InputText 用のコールバック関数
 * @param _data : ImGui から渡される入力データ
 * @details 文字列が入力された際、std::string のサイズが変更されると自動的にバッファサイズを調整
 */
static int InputTextCallback(ImGuiInputTextCallbackData* _data)
{
    if (_data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // std::string のサイズを変更し、バッファポインタを再設定
        std::string* str = static_cast<std::string*>(_data->UserData);
        str->resize(_data->BufTextLen);
        _data->Buf = str->data();  // バッファを ImGui に反映
    }
    return 0;
}

bool utl::ImGuiHelper::InputTextWithString(const char* _label, std::string& _str)
{
    // ImGui::InputText で std::string を入力に使うための関数
    // コールバックを設定して、サイズ変更やバッファ更新が安全に行われるようにする
    return ImGui::InputText(_label, _str.data(), _str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, InputTextCallback, &_str);
}

#include "Application/Application.h"

bool utl::ImGuiHelper::SelectModelPath(const char* _label, std::string& _path)
{
    bool ret = false;

    ImGui::PushID(_path.data());

    // ボタンを押すとファイルダイアログを開く
    if (ImGui::Button(_label))
    {
        // ファイルダイアログを開いてパスを取得
        ret = Application::Instance().GetWindowClass().OpenFileDialog(
            _path,
            "SelectModel",
            "Model Files (*.gltf)\0*.gltf\0All Files (*.*)\0*.*\0\0\0");

        // パスが取得できた場合は相対パスに変換
        if (ret)
        {
            auto current = std::filesystem::current_path();
            // プロジェクトディレクトリを取得
            std::filesystem::path projectDir = current / ModelPath::FileDir;
            std::filesystem::path filePath(_path);

            //  Assets::Modelで読み込める形式に変換する //
            // ファイルパスをプロジェクトディレクトリからの相対パスに変換
            std::filesystem::path relativePath = std::filesystem::relative(filePath, projectDir);

            // 拡張子を削除
            relativePath.replace_extension();

            // Windowsの区切り文字 "\" を UNIXスタイルの "/" に変換
            std::string finalPath = relativePath.generic_string();

            // 最終的な相対パスを取得
            _path = finalPath;

            // カレントディレクトリを元に戻す
            std::filesystem::current_path(current);
        }
    }

    ImGui::PopID();

    return ret;
}

bool utl::ImGuiHelper::SelectSpritePath(const char* _label, std::string& _path)
{
    bool ret = false;

    ImGui::PushID(_path.data());

    // ボタンを押すとファイルダイアログを開く
    if (ImGui::Button(_label))
    {
        // ファイルダイアログを開いてパスを取得
        ret = Application::Instance().GetWindowClass().OpenFileDialog(
            _path,
            "SelectSprite",
            "Sprite Files (*.png)\0*.png\0All Files (*.*)\0*.*\0\0\0");

        // パスが取得できた場合は相対パスに変換
        if (ret)
        {
            auto current = std::filesystem::current_path();

            // プロジェクトディレクトリを取得
            std::filesystem::path projectDir = current;
            std::filesystem::path filePath(_path);

            // ファイルパスをプロジェクトディレクトリからの相対パスに変換
            std::filesystem::path relativePath = std::filesystem::relative(filePath, projectDir);

            // Windowsの区切り文字 "\" を UNIXスタイルの "/" に変換
            std::string finalPath = relativePath.generic_string();

            // 最終的な相対パスを取得
            _path = finalPath;

            // カレントディレクトリを元に戻す
            std::filesystem::current_path(current);
        }
    }

    ImGui::PopID();

    return ret;
}
