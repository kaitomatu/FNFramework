#pragma once

namespace utl
{
    namespace ImGuiHelper
    {
        /**
         * @brief ○○が有効な場合は△△ボタンを表示する
         * @param _condition : 条件
         * @param _buttonLabelTrue : true の場合のボタンラベル
         * @param _actionTrue : true の場合の処理
         * @param _buttonLabelFalse : false の場合のボタンラベル
         * @param _actionFalse : false の場合の処理
         */
        inline void HandleButton(
            bool _condition,
            const char* _buttonLabelTrue,
            std::function<void()> _actionTrue,
            const char* _buttonLabelFalse,
            std::function<void()> _actionFalse)
        {
            if (_condition)
            {
                if (ImGui::Button(_buttonLabelTrue))
                {
                    _actionTrue();
                }
            }
            else
            {
                if (ImGui::Button(_buttonLabelFalse))
                {
                    _actionFalse();
                }
            }
        };

        /**
         * @fn bool UpdateBitFlagWithCheckbox(const char* _label, int _flagType, UINT& _colliderType, bool& _onCheck)
         * @brief フラグをチェックボックスで制御する
         * @brief Bitフラグをチェックボックスで制御する
         * @param _label : チェックボックスのラベル
         * @param _flagType : フラグの種類
         * @param _colliderType : フラグを持つ変数
         * @param _onCheck : チェックボックスがオンかどうか
         * @return チェックボックスが変更されたかどうか
        */
        static bool UpdateBitFlagWithCheckbox(const char* _label, int _flagType, UINT& _colliderType, bool& _onCheck)
        {
            // フラグが有効かどうかを判定
            _onCheck = (_colliderType & _flagType) != 0;

            // チェックボックスでフラグのオンオフを制御
            if (!ImGui::Checkbox(_label, &_onCheck)) { return false; }

            _colliderType ^= _flagType; // フラグをトグル
            return true;
        }

        /**
         * @fn bool UpdateBitFlagWithCheckbox(const char* _label, int _flagType, UINT& _colliderType)
         * @brief フラグをチェックボックスで制御する
         * @param _label : チェックボックスのラベル
         * @param _flagType : フラグの種類
         * @param _colliderType : フラグを持つ変数
         * @return チェックボックスが変更されたかどうか
         */
        static bool UpdateBitFlagWithCheckbox(const char* _label, int _flagType, UINT& _colliderType)
        {
            bool onCheck;
            return UpdateBitFlagWithCheckbox(_label, _flagType, _colliderType, onCheck);
        }

        /**
         * @fn bool RadioButtonWithLabel(const char* label, T& currentValue, T targetValue)
         * @brief ラジオボタンをラベル付きで表示し、値を変更する
         * @tparam T : ラジオボタンの値の型
         * @param label : ラジオボタンのラベル
         * @param currentValue : 現在の値(変更対象)
         * @param targetValue : 変更する値
         * @return $currentValue$に$targetValue$が設定されたかどうか
         */
        template<typename T>
        bool RadioButtonWithLabel(const char* label, T& currentValue, T targetValue)
        {
            if (ImGui::RadioButton(label, currentValue == targetValue))
            {
                currentValue = targetValue;
                return true;  // 値が変更された場合に true を返す
            }
            return false;  // 変更されなかった場合は false
        }

        /**
         * @fn void InputTextWithString(const char* _label, std::string& _str)
         * @brief std::string を直接 ImGui::InputText で扱うための関数
         * @param _label : ImGui で表示するラベル
         * @param _str : 入力を受け付ける std::string
         * @details std::string をコールバックを利用して安全に扱えるようにするためのラッパー関数
         */
        bool InputTextWithString(const char* _label, std::string& _str);

        /**
         * @fn bool SelectModelPath(const char* _label, std::string& _path)
         * @brief ファイルパスを選択するための関数
         * @param _label : ImGui で表示するラベル
         * @param _path : ファイルパス
         * @return bool : ファイルパスが選択されたかどうか
         */
        bool SelectModelPath(const char* _label, std::string& _path);

        /**
         * @fn bool SelectSpritePath(const char* _label, std::string& _path)
         * @brief スプライトパスを選択するための関数
         * @param _label : ImGui で表示するラベル
         * @param _path : スプライトパス
         * @return bool : スプライトパスが選択されたかどうか
         */
        bool SelectSpritePath(const char* _label, std::string& _path);
    }
}
