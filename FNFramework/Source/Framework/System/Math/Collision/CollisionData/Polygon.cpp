#include "Polygon.h"

void Polygon::GetPosition(std::vector<Math::Vector3>& result)
{
    m_vertex.resize(result.size());

    for (size_t i = 0; i < result.size(); ++i)
    {
        result[i] = m_vertex[i].Pos;
    }
}

bool Polygon::Contains(const Math::Vector2& point) const
{
    /** 
    * 今回はpoint->頂点のベクトルとの内積値を用いて判定しているが、Rayとの交差判定を用いる方法もある
    */

    float sum = 0.0f;
    Math::Vector2 a, b;

    // 点->頂点のベクトルを求める
    for (size_t i = 0; i < m_vertex.size() - 1; ++i)
    {
        // 点から第1頂点へのベクトル
        a = m_vertex[i].Pos - point;
        a.Normalize();
        // 点から第2頂点へのベクトル
        b = m_vertex[i + 1].Pos - point;
        b.Normalize();

        // 求めたベクトルの内積値を加算していく
        sum += a.Dot(b);
    }

    // 最後と最初の頂点を結ぶベクトル
    a = m_vertex.back().Pos - point;
    a.Normalize();
    b = m_vertex.front().Pos - point;
    b.Normalize();
    sum += a.Dot(b);

    // 内積値の合計が2π(360度)ならば点は多角形の内側にある
    return MathHelper::NearZero(sum - MathHelper::TwoPI);
}
