#pragma once

//--------------------------------
// アニメーションデータ関係
//--------------------------------

// アニメーションキー(クォータニオン)
struct AnimKeyQuaternion
{
    float				m_time = 0;		// 時間
    Math::Quaternion	m_quat;			// クォータニオンデータ
};

// アニメーションキー(ベクトル)
struct AnimKeyVector3
{
    float				m_time = 0;		// 時間
    Math::Vector3		m_vec;			// 3Dベクトルデータ
};

struct AnimationData
{
    // アニメーションデータ
    std::string Name;

    // アニメーション全体の長さ
    float       MaxFrame = 0;

    // 1ノードごとのアニメーションデータ
    struct Channel
    {
        int     NodeOffset = -1;    // 対象モデルノードのOffset値

        // 各チャンネル
        std::vector<AnimKeyVector3>       Translations;   // 位置キーリスト
        std::vector<AnimKeyQuaternion>    Rotations;      // 回転キーリスト
        std::vector<AnimKeyVector3>       Scales;         // 拡縮キーリスト

        void Interpolate(Math::Matrix& _rDst, float _time);
        bool InterpolateTranslations(Math::Vector3& _result, float _time);
        bool InterpolateRotations(Math::Quaternion& _result, float _time);
        bool InterpolateScales(Math::Vector3& _result, float _time);
    };

    // 全ノード用のアニメーションデータ
    std::vector<Channel> Channels;
};

/**
* @class Animator
* @brief アニメーション管理クラス
* @details
*   アニメーションデータを管理している
*   アニメションの進行度やループの有無などを管理する
*
* todo : アニメーションの予約機能を作りたい場合、ここで行うのではなくアニメーション管理クラスを作成しそこで行う
*/
class Animator
{
public:

    Animator();

    void SetAnimation(const std::shared_ptr<AnimationData>& _rData, bool _isLoop = true)
    {
        m_spAnimation = _rData;
        m_isLoop = _isLoop;

        m_time = 0.0f;
        m_complementTime = m_maxConplementTime;
    }

    // アニメーションが終了してる？
    bool IsAnimationEnd() const
    {
        if (!m_spAnimation) { return false; }
        if (m_time < m_spAnimation->MaxFrame) { return false; }

        return true;
    }

    // アニメーションの更新
    void AdvanceTime( std::vector<ModelWork::Node>& _rNodes, float _speed = 1.0f);

    //---------------------
    // アニメーション進行度関係
    //---------------------
    void ResetAnimation() { m_time = 0.0f; }
    void SetProgressTime(float _time) { m_time = _time; }
    float GetProgressTime() const { return m_time; }

    // アニメーションの最大フレーム数
    float GetMaxFrame() const
    {
        if(!m_spAnimation) { return 0.0f; }
        return m_spAnimation->MaxFrame;
    }

    // アニメーションの進行度を正規化したもの
    float GetNormalizeTime() const
    {
        if(!m_spAnimation) { return 0.0f; }
        return m_time / m_spAnimation->MaxFrame;
    }

    void SetNormalizeTime(float _normalizeTime)
    {
        if(!m_spAnimation || m_spAnimation->MaxFrame <= 0.0f)
        {
            return;
        }

        // 0.0f～1.0fの範囲に正規化
        _normalizeTime = std::clamp(_normalizeTime, 0.0f, 1.0f);

        m_time = _normalizeTime * m_spAnimation->MaxFrame;
    }

    // ループするかどうか
    bool IsLoop() const { return m_isLoop; }
    void SetLoop(bool _isLoop) { m_isLoop = _isLoop; }

    // アニメーションデータの名前
    const std::string& GetAnimationName() const
    {
        if(!m_spAnimation) { return "NonAnim"; }
        return m_spAnimation->Name;
    }

private:

    // GLTFのアニメーションフレームレート
    inline static float GLTFAnimationFrame = 60.0f;

    std::shared_ptr<AnimationData>	m_spAnimation = nullptr;	// 再生するアニメーションデータ

    // アニメーションの進行度
    float m_time = 0.0f;

    // 最大の補完時間
    float m_maxConplementTime = 0.0f;
    // 補完時間
    float m_complementTime = 0.0f;

    // ループするかどうか
    bool m_isLoop = false;
};
