#pragma once

namespace utl::str
{
    /**
        * @brief Enumをstd::stringに変換する
        * @param[in] toString - 変換したい列挙型
        * @result 変換された文字列
        *
        * ※ nameofライブラリを使用
        * https://github.com/Neargye/nameof
        */
    template <typename E>
    const std::string EnumToString(E toString)
    {
        return std::string(nameof::nameof_enum(toString));
    }

    /*
     * @brief 文字列の中の指定した単語を削除する
     *
     * @param  text		 - 編集したい文字列
     * @param  removeWord - 削除したい単語
     */
    inline void RemoveWord(std::string& text, const std::string_view removeWord)
    {
        std::string replaceString = "";

        // 文字列内で検索して置換を行う
        std::size_t foundPos = text.find(removeWord);
        if (foundPos != std::string::npos)
        {
            text.replace(foundPos, removeWord.length(), replaceString);
        }
    }

    inline void WideToSJis(std::string& text, const std::wstring& wstring)
    {
        text = wide_to_sjis(wstring);
    }

    inline void SJisToWide(std::wstring& wstring, const std::string& string)
    {
        wstring = sjis_to_wide(string);
    }
}
