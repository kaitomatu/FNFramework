#pragma once

//----------------------------
// ComPtr
//----------------------------
using Microsoft::WRL::ComPtr;

//----------------------------
// SimpleMath
//----------------------------
namespace Math = DirectX::SimpleMath;

//----------------------------
// スクリーン用定数
//----------------------------
// ToDo : 今いろんなところにScreen定数をいちいち呼びだしてるが、今後はGraphicsDeviceが単一管理する
namespace Screen
{
    static constexpr int Width = 1280; // 横幅
    static constexpr int Height = 720; // 縦幅
    static constexpr int HalfWidth = Width / 2;
    static constexpr int HalfHeight = Height / 2;
}

//----------------------------
// 色
//----------------------------
namespace Color
{
    constexpr Math::Color Red = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr Math::Color Magenta = {0.3f, 0.1f, 0.2f, 1.0f};
    constexpr Math::Color Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    constexpr Math::Color Green = {0.0f, 1.0f, 0.0f, 1.0f};
    constexpr Math::Color DeepGreen = {0.0f, 0.2f, 0.0f, 1.0f};
    constexpr Math::Color Blue = {0.0f, 0.0f, 1.0f, 1.0f};
    constexpr Math::Color DeepBlue = {0.01f, 0.12f, 0.4f, 1.0f};
    constexpr Math::Color White = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Math::Color Black = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr Math::Color LightGray = {0.7f, 0.7f, 0.7f, 1.0f};
    constexpr Math::Color Gray = {0.5f, 0.5f, 0.5f, 1.0f};
    constexpr Math::Color DeepGray = {0.2f, 0.2f, 0.2f, 1.0f};
}

//=================================
// 便利機能
//=================================

namespace utl
{
    /**
    * @brief 安全にReleaseを行うための関数
    * @param p - Releaseを行いたいポインタ
    */
    template <typename T>
    void SafeRelease(T*& p)
    {
        if (p)
        {
            p->Release();
            p = nullptr;
        }
    }

    /**
    * @brief 安全にDeleteを行うための関数
    * @param p - Deleteを行いたいポインタ
    */
    template <typename T>
    void SafeDelete(T*& p)
    {
        if (p)
        {
            delete p;
            p = nullptr;
        }
    }
}

// c++20では char8_t という型が用意されているためchar8_t->char*として利用するには変換が必要
#define U8_TEXT(text) reinterpret_cast<const char*>(u8##text)
