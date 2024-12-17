#pragma once

// 数学関連のヘルパー関数を定義する名前空間
namespace MathHelper
{
    //----------------------
    // 定数
    //----------------------

    // 円周率
    static constexpr float PI = 3.14159265359f; // 180度
    static constexpr float TwoPI = PI * 2.0f; // 360度

    // float型の最大 / 最小値
    static constexpr float FLOAT_MAX = std::numeric_limits<float>::max(); // float型の最大値
    static constexpr float FLOAT_MIN = std::numeric_limits<float>::lowest(); // float型の最小値

    // Vector2型の最大値
    static constexpr auto VECTOR2_MAX = Math::Vector2(FLOAT_MAX, FLOAT_MAX);
    // Vector2型の最小値
    static constexpr auto VECTOR2_MIN = Math::Vector2(FLOAT_MIN, FLOAT_MIN);

    // Vector3型の最大値
    static constexpr auto VECTOR3_MAX = Math::Vector3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
    // Vector3型の最小値
    static constexpr auto VECTOR3_MIN = Math::Vector3(FLOAT_MIN, FLOAT_MIN, FLOAT_MIN);

    //----------------------
    // 便利関数
    //----------------------

    /**
     * @fn inline float Lerp(float a, float b, float t)
     * @brief 線形補間を行う
     * @param a : t = 0の時の値 - a
     * @param b : t = 1の時の値 - b
     * @param t : 0.0f ~ 1.0fの間の値
     * @return 補間結果
     */
    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    static constexpr float ToRadians = PI / 180.0f; // ディグリース角をラジアン角に変換するための定数

    // ディグリース角をラジアン角に変換する
    inline float ConvertToRadians() { return ToRadians; }
    inline float ConvertToRadians(float degrees) { return degrees * ToRadians; }
    inline Math::Vector3 ConvertToRadians(const Math::Vector3& degrees) { return degrees * ToRadians; }

    static constexpr float ToDegrees = 180.0f / PI; // ディグリース角をラジアン角に変換するための定数

    // ラジアン角をディグリース角に変換する
    inline float ConvertToDegrees() { return ToDegrees; }
    inline float ConvertToDegrees(float radians) { return radians * ToDegrees; }
    inline Math::Vector3 ConvertToDegrees(const Math::Vector3& radians) { return radians * ToDegrees; }

    /**
     * @brief Vector3型のそれぞれの線形補間を行う
     * @param a : 補間開始点
     * @param b : 補間終了点
     * @param t : 0.0f ~ 1.0f の範囲で補間係数
     * @return 補間結果の Vector3
     */
    inline Math::Vector3 Lerp(const Math::Vector3& a, const Math::Vector3& b, float t)
    {
        return {
            std::lerp(a.x, b.x, t),
            std::lerp(a.y, b.y, t),
            std::lerp(a.z, b.z, t)
        };
    }

    /**
     * @fn inline bool NearZero(float value, double epsilon = 1e-6f)
     * @brief 対象の値が0に近いかどうかを判定する
     * @param[in] value - 対象の値
     * @param[in] epsilon - 許容誤差 : デフォルトは1e-6f(0.000001f)
     * @return 0に近ければtrue
     */
    inline bool NearZero(float value, double epsilon = 1e-6f) { return std::abs(value) < epsilon; }

    /**
     * @fn inline void WorldToScreen(const Math::Vector3& worldPos, POINT& screenPos, const Math::Matrix& mView, const Math::Matrix& mProj)
     * @brief ワールド座標からスクリーン座標へ変換する
     * @param[in] worldPos	- ワールド座標
     * @param[out] screenPos - スクリーン座標
     * @param[in] mView		- ビュー行列
     * @param[in] mProj		- 射影行列
     */
    inline void WorldToScreen(const Math::Vector3& worldPos, POINT& screenPos, const Math::Matrix& mView,
        const Math::Matrix& mProj)
    {
        // ワールド座標を4次元ベクトルに変換
        auto worldPos4 = Math::Vector4(worldPos.x, worldPos.y, worldPos.z, 1.0f);
        // クリップ座標に変換 -
        Math::Vector4 clipCoords = Math::Vector4::Transform(worldPos4, mView * mProj);

        Math::Vector3 ndc; // 正規化デバイス座標(Normalized Device Coordinate)
        ndc.x = clipCoords.x / clipCoords.w;
        ndc.y = clipCoords.y / clipCoords.w;

        screenPos.x = static_cast<LONG>((ndc.x + 1.0f) * 0.5f * Screen::Width);
        screenPos.y = static_cast<LONG>((1.0f - ndc.y) * 0.5f * Screen::Height);
    }

    /**
     * @fn inline void ScreenToWorld(const POINT& screenPos, Math::Vector3& worldPos, float depthValue, const Math::Matrix& mView, const Math::Matrix& mProj)
     * @brief スクリーン座標からワールド座標へ変換する
     * @param[in] screenPos	- スクリーン座標
     * @param[out] worldPos	- ワールド座標
     * @param[in] depthValue	- 深度値
     * @param[in] mView		- ビュー行列
     * @param[in] mProj		- 射影行列
     */
    inline void ScreenToWorld(const POINT& screenPos, Math::Vector3& worldPos, float depthValue,
        const Math::Matrix& mView, const Math::Matrix& mProj)
    {
        Math::Vector3 ndc; // 正規化デバイス座標(Normalized Device Coordinate)
        ndc.x = (static_cast<float>(screenPos.x) * 2.0f) / Screen::Width - 1.0f;
        ndc.y = 1.0f - (static_cast<float>(screenPos.y) * 2.0f) / Screen::Height;
        ndc.z = depthValue;

        Math::Vector4 clipCoords(ndc.x, ndc.y, ndc.z, 1.0f);

        // ビュー行列とプロジェクション行列の逆行列を求める
        Math::Matrix invViewProj = mProj.Invert() * mView.Invert();
        Math::Vector4 worldPos4 = Math::Vector4::Transform(clipCoords, invViewProj);

        worldPos = Math::Vector3(worldPos4.x, worldPos4.y, worldPos4.z) / worldPos4.w;
    }

    namespace Easing
    {

        /**
         * @fn inline float EaseInOutSine(float progress)
         * @param progress - 進行度(0.0f ~ 1.0f)
         * @brief
         *  - 緩やかに始まり、中央で最も速くなり、また緩やかに終わる。
         *  - 自然な推進や停止の動きに適しており、負荷は低い。
         */
        inline float EaseInOutSine(float progress)
        {
            return -(std::cos(PI * progress) - 1.0f) / 2.0f;
        }

        /**
         * @fn inline float EaseInOutQuad(float progress)
         * @param progress - 進行度(0.0f ~ 1.0f)
         * @brief
         *  - 緩やかに始まり、途中で加速し、最後に減速する。
         *  - 比較的軽量で、動きのスムーズさを表現する際に適している。
         */
        inline float EaseInOutQuad(float progress)
        {
            return (progress < 0.5f)
                ? 2.0f * progress * progress
                : 1.0f - std::pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
        }

        /**
         * @fn inline float EaseInOutCubic(float progress)
         * @param progress - 進行度(0.0f ~ 1.0f)
         * @brief
         *  - より急な加速と減速を行う。緩やかな開始と急激な推進をシミュレートできる。
         *  - 処理負荷は軽量だが、強い動きの変化を表現できる。
         */
        inline float EaseInOutCubic(float progress)
        {
            return (progress < 0.5f)
                ? 4.0f * progress * progress * progress
                : 1.0f - std::pow(-2.0f * progress + 2.0f, 3.0f) / 2.0f;
        }

        /**
         * @fn inline float EaseInOutQuart(float progress)
         * @param progress - 進行度(0.0f ~ 1.0f)
         * @brief
         *  - 初めは非常にゆっくり進み、急激に加速して最後に急減速する。
         *  - ※ 処理負荷が高いため、利用には注意が必要。
         *  - 動きの始まりと終わりが大きく異なる場合に適している。
         */
        inline float EaseInOutExpo(float progress)
        {
            progress = std::clamp(progress, 0.0f, 1.0f);
            return (progress < 0.5f)
                ? std::pow(2.0f, 20.0f * progress - 10.0f) / 2.0f
                : (2.0f - std::pow(2.0f, -20.0f * progress + 10.0f)) / 2.0f;
        }

        /**
         *
         * @param progress
         * @return
         */
        inline float EaseInOutBack(float progress)
        {
            const float c1 = 1.70158f;
            const float c2 = c1 * 1.525f;

            return progress < 0.5
                ? (std::pow(2 * progress, 2) * ((c2 + 1) * 2 * progress - c2)) / 2
                : (std::pow(2 * progress - 2, 2) * ((c2 + 1) * (progress * 2 - 2) + c2) + 2) / 2;
        }

        /**
         *
         * @param progress :
         * @return
         */
        inline float EaseOutBack(float progress)
        {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;

            return 1 + c3 * std::pow(progress - 1, 3) + c1 * std::pow(progress - 1, 2);
        }

        enum class EasingType
        {
            Default,
            EaseInOutSine,
            EaseInOutQuad,
            EaseInOutCubic,
            EaseInOutExpo,
            EaseInOutBack,
            EaseOutBack,
            Count
        };

        // ImGuiで利用するためのイージングタイプの選択肢 //
        static class EasingTypeItems
        {
        public:
            // イージングタイプの選択肢を定義
            std::vector<std::string> TypeNames;
            std::vector<const char*> TypeNamePtrs;
        
            EasingTypeItems()
            {
                // イージングタイプの数を取得
                size_t typeCount = static_cast<size_t>(EasingType::Count);

                // ベクターのサイズを設定
                TypeNames.resize(typeCount);
                TypeNamePtrs.resize(typeCount);

                // 各イージングタイプの名前を取得し、ポインタを保存
                for (size_t i = 0; i < typeCount; ++i)
                {
                    EasingType type = static_cast<EasingType>(i);
                    std::string easingTypeStr = utl::str::EnumToString<EasingType>(type);
                    TypeNames[i] = easingTypeStr;
                    TypeNamePtrs[i] = TypeNames[i].c_str();
                }
            }
        };
        inline static EasingTypeItems g_EasingTypeItems;

        class EasingData
        {
        public:

            EasingType Type = EasingType::EaseInOutSine;

            bool IsLoop = true;
            bool IsEasing = false;
            bool Reverse = false;

            float AnimationSpeed = 1.0f;
            float Progress = 0.0f; // 進行度
            float Duration = 1.0f; // どのくらいの時間でアニメーションを行うか
            Math::Vector2 ClampValue = Math::Vector2{ 0.0f, 1.0f }; // x : min, y : max

            // 0.0f ~ 1.0f の範囲で進行度を更新
            bool Easing(float delta, float& value);
            // _minvalue ~ _maxValue の範囲で進行度を更新
            bool Easing(float _delta, float& _value, float _minValue, float _maxValue);

            void ImGuiUpdate();

            // シリアライズ / デシリアライズ
            void Serialize(Json& _json) const;
            void Deserialize(const Json& _json);

        private:

            // ※ イージング関数が増えたらここに追加 ※ //
            float ApplyEasingFunction(float _t)
            {
                switch (Type)
                {
                case EasingType::EaseInOutSine:
                    return Easing::EaseInOutSine(_t);

                case EasingType::EaseInOutQuad:
                    return Easing::EaseInOutQuad(_t);

                case EasingType::EaseInOutCubic:
                    return Easing::EaseInOutCubic(_t);

                case EasingType::EaseInOutExpo:
                    return Easing::EaseInOutExpo(_t);

                case EasingType::EaseInOutBack:
                    return Easing::EaseInOutBack(_t);

                case EasingType::EaseOutBack:
                    return Easing::EaseOutBack(_t);

                default:
                    return _t; // デフォルトは線形補間
                }
            }
        };
    }
}

namespace jsonKey::Easing
{
    // イージングデータ
    constexpr std::string_view EasingType = "EasingType";
    constexpr std::string_view IsLoop = "IsLoop";
    constexpr std::string_view IsEasing = "IsEasing";
    constexpr std::string_view Reverse = "Reverse";
    constexpr std::string_view AnimationSpeed = "AnimationSpeed";
    constexpr std::string_view Duration = "Duration";
    constexpr std::string_view ClampValue = "ClampValue";
}
