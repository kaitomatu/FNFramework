#pragma once

namespace utl
{
    /**
    * @class Singleton
    * @brief 簡易的にシングルトンを扱えるようにしたクラス
    * @details
    *   このクラスを継承することでシングルトンパターンを実装できる
    *   ※ 派生先でコンストラクタ / デストラクトをprivateで宣言する必要がある
    */
    template <typename T>
    class Singleton
    {
    public:
        static T& Instance()
        {
            static T instance;
            return instance;
        }

    protected:
        Singleton()
        {
        }

        virtual ~Singleton()
        {
        }

    private:
        // シングルトンなので、コピー / ムーブは禁止
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;
    };
}
