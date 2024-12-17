#include "Animation.h"

//=================================================================
// AnimationData
//=================================================================

/**
* @tparam T - キーの型 : AnimKeyQuaternion, AnimKeyVector3
 * @fn BinarySearchNextAnimKey(const std::vector<T>& list, float time)
 *
 * @brief 二分探索で、指定時間から次の配列要素のKeyIndexを求める関数
 * @param list - キー配列
 * @param time - 時間
 * @return 次の配列要素のIndex
 */
template <class T>
int BinarySearchNextAnimKey(const std::vector<T>& list, float time)
{
    int low = 0;
    int high = (int)list.size();
    while (low < high)
    {
        int mid = (low + high) / 2;
        float midTime = list[mid].m_time;

        if (midTime <= time) low = mid + 1;
        else high = mid;
    }
    return low;
}

void AnimationData::Channel::Interpolate(Math::Matrix& _rDst, float time)
{
    // ベクターによる拡縮補間
    bool isChange = false;
    Math::Matrix scale;
    Math::Vector3 resultVec;
    if (InterpolateScales(resultVec, time))
    {
        scale = scale.CreateScale(resultVec);
        isChange = true;
    }

    // クォタニオンによる回転補間
    Math::Matrix rotate;
    Math::Quaternion resultQuat;
    if (InterpolateRotations(resultQuat, time))
    {
        rotate = rotate.CreateFromQuaternion(resultQuat);
        isChange = true;
    }

    // ベクターによる座標補間
    Math::Matrix trans;
    if (InterpolateTranslations(resultVec, time))
    {
        trans = trans.CreateTranslation(resultVec);
        isChange = true;
    }

    if (isChange)
    {
        _rDst = scale * rotate * trans;
    }
}

bool AnimationData::Channel::InterpolateTranslations(Math::Vector3& _result, float _time)
{
    if (Translations.size() == 0) { return false; }

    // キー位置検索
    UINT keyIdx = BinarySearchNextAnimKey(Translations, _time);

    // 先頭のキーなら、先頭のデータを返す
    if (keyIdx == 0)
    {
        _result = Translations.front().m_vec;
        return true;
    }

    // 配列外のキーなら、最後のデータを返す
    if (keyIdx >= Translations.size())
    {
        _result = Translations.back().m_vec;
        return true;
    }

    // それ以外(中間の時間)なら、その時間の値を補間計算で求める
    auto& prev = Translations[keyIdx - 1]; // 前のキー
    auto& next = Translations[keyIdx]; // 次のキー

    // 前のキーと次のキーの時間から、0～1間の時間を求める
    float f = (_time - prev.m_time) / (next.m_time - prev.m_time);

    // 補間
    _result = DirectX::XMVectorLerp(
        prev.m_vec,
        next.m_vec,
        f
    );

    return true;
}

bool AnimationData::Channel::InterpolateRotations(Math::Quaternion& _result, float _time)
{
    if (Rotations.size() == 0) { return false; }

    // キー位置検索
    UINT keyIdx = BinarySearchNextAnimKey(Rotations, _time);

    // 先頭のキーなら、先頭のデータを返す
    if (keyIdx == 0)
    {
        _result = Rotations.front().m_quat;
        return true;
    }

    // 配列外のキーなら、最後のデータを返す
    if (keyIdx >= Rotations.size())
    {
        _result = Rotations.back().m_quat;
        return true;
    }

    // それ以外(中間の時間)なら、その時間の値を補間計算で求める
    auto& prev = Rotations[keyIdx - 1]; // 前のキー
    auto& next = Rotations[keyIdx]; // 次のキー

    // 前のキーと次のキーの時間から、0～1間の時間を求める
    float f = (_time - prev.m_time) / (next.m_time - prev.m_time);

    // 補間
    _result = DirectX::XMQuaternionSlerp(
        prev.m_quat,
        next.m_quat,
        f
    );

    return true;
}

bool AnimationData::Channel::InterpolateScales(Math::Vector3& _result, float _time)
{
    if (Scales.size() == 0) { return false; }

    // キー位置検索
    UINT keyIdx = BinarySearchNextAnimKey(Scales, _time);

    // 先頭のキーなら、先頭のデータを返す
    if (keyIdx == 0)
    {
        _result = Scales.front().m_vec;
        return true;
    }

    // 配列外のキーなら、最後のデータを返す
    if (keyIdx >= Scales.size())
    {
        _result = Scales.back().m_vec;
        return true;
    }

    // それ以外(中間の時間)なら、その時間の値を補間計算で求める
    auto& prev = Scales[keyIdx - 1]; // 前のキー
    auto& next = Scales[keyIdx]; // 次のキー

    // 前のキーと次のキーの時間から、0～1間の時間を求める
    float f = (_time - prev.m_time) / (next.m_time - prev.m_time);

    // 補間
    _result = DirectX::XMVectorLerp(
        prev.m_vec,
        next.m_vec,
        f
    );

    return true;
}

//=================================================================
// Animator
//=================================================================
Animator::Animator()
{
    // todo : 今後は外部ファイルなどを用いて設定できるようにする
    m_maxConplementTime = 15.0f;
}

void Animator::AdvanceTime(std::vector<ModelWork::Node>& _rNodes, float _speed)
{
    if (!m_spAnimation) { return; }

    // フレームごとの経過時間を取得
    float deltaTime = SceneManager::Instance().FrameDeltaTime();

    // アニメーションの進行度を更新
    m_time += _speed * deltaTime * GLTFAnimationFrame;

    // ループ処理
    if (m_isLoop)
    {
        if(m_time >= m_spAnimation->MaxFrame)
        {
            m_time = 0;
        }
    }
    else
    {
        if (m_time >= m_spAnimation->MaxFrame)
        {
            m_time = m_spAnimation->MaxFrame;
        }
    }

    // ノードごとにアニメーションを適用
    for (auto& channel : m_spAnimation->Channels)
    {
        // 対応するノードを取得
        auto& node = _rNodes[channel.NodeOffset];

        // 前の変換行列を保存
        Math::Matrix prevTransform = node.mLocalTransform;

        // アニメーションデータによる行列補間
        Math::Matrix animTransform;
        channel.Interpolate(animTransform, m_time);

        // アニメーション切り替え後一定時間は線形補間
        if (m_complementTime > 0.0f)
        {
            float t = (m_maxConplementTime - m_complementTime) / m_maxConplementTime;
            node.mLocalTransform = Math::Matrix::Lerp(prevTransform, animTransform, t);
        }
        else
        {
            node.mLocalTransform = animTransform;
        }
    }

    // 補完時間を更新
    m_complementTime -= _speed * deltaTime;
    if (m_complementTime < 0.0f)
    {
        m_complementTime = 0.0f;
    }
}
