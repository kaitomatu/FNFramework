#include "LineSegment.h"

float LineSegment::DistanceFromPoint(const Math::Vector3& point) const
{
    // 各ベクトルの設定
    Math::Vector3 ab = m_lineData.End - m_lineData.Start; // a->b
    Math::Vector3 ba = m_lineData.Start - m_lineData.End; // b->a
    Math::Vector3 ac = point - m_lineData.Start; // a->c
    Math::Vector3 bc = point - m_lineData.End; // b->c

    //----------------
    // 内積の計算
    //----------------
    if (ab.Dot(ac) < 0.0f) // a->bとa->cの内積値が負の値なら、点cは点aに近い
    {
        return ac.Length(); // よってacの長さを返す
    }
    if (ab.Dot(bc) < 0.0f) // a->bとa->cの内積値が負の値なら、点cは点aに近い
    {
        return bc.Length(); // よってbcの長さを返す
    }
    // 点cは点aと点bの間にある
    // よって、点aと点bを通る直線と点cの距離を求める
    return ab.Cross(ac).Length() / ab.Length();
}

float LineSegment::DistanceFromPointSq(const Math::Vector3& point) const
{
    /*
    * 平方根の処理は比較的重たい処理なので、利用しない場合にこっちを使う
    * 利用例 : 2点間の距離を比較する場合など
    */

    // 各ベクトルの設定
    Math::Vector3 ab = m_lineData.End - m_lineData.Start; // a->b
    Math::Vector3 ba = m_lineData.Start - m_lineData.End; // b->a
    Math::Vector3 ac = point - m_lineData.Start; // a->c
    Math::Vector3 bc = point - m_lineData.End; // b->c

    //----------------
    // 内積の計算
    //----------------
    if (ab.Dot(ac) < 0.0f) // a->bとa->cの内積値が負の値なら、点cは点aに近い
    {
        return ac.LengthSquared(); // よってacの長さ^2を返す
    }
    if (ab.Dot(bc) < 0.0f) // a->bとa->cの内積値が負の値なら、点cは点aに近い
    {
        return bc.LengthSquared(); // よってbcの長さ^2を返す
    }
    // 点cは点aと点bの間にある
    // よって、点aと点bを通る直線と点cの距離を求める
    float scalar = ac.Dot(ab) / ab.Dot(ab);
    Math::Vector3 p = ab * scalar;
    return (ac - p).LengthSquared();
}

// GJKアルゴリズムを基にした線分同士の最短距離を求める関数
float LineSegment::MinDistSq(const LineData& line1, const LineData& line2)
{
    // ベクトルの定義
    Math::Vector3 u = line1.End - line1.Start; // 線分1の方向ベクトル
    Math::Vector3 v = line2.End - line2.Start; // 線分2の方向ベクトル
    Math::Vector3 w = line1.Start - line2.Start; // 開始点間のベクトル

    // 内積の計算
    float a = u.Dot(u); // uの長さの二乗
    float b = u.Dot(v); // uとvの内積
    float c = v.Dot(v); // vの長さの二乗
    float d = u.Dot(w);
    float e = v.Dot(w);
    float D = a * c - b * b; // 判別式

    // パラメータの初期値の設定
    float m, n; // m : 線分1上の最短距離を表すパラメータ , n : 線分2上の最短距離を表すパラメータ
    float numerator, denominator; // numerator : 分子 , denominator : 分母

    // 判別式がゼロに近い場合の計算（線分が平行な場合）
    if (MathHelper::NearZero(D))
    {
        // 線分1の始点から線分2への垂直ベクトルの長さを計算
        m = 0.0f;
        denominator = c;
        n = (denominator != 0.0f) ? (e / denominator) : 0.0f;
    }
    else
    {
        // 判別式がゼロでない場合のパラメータ計算
        numerator = b * e - c * d;
        denominator = D;
        m = numerator / denominator;
        n = (a * e - b * d) / denominator;
    }

    // mが[0,1]の範囲外にある場合の補正
    if (m < 0.0f)
    {
        m = 0.0f;
        n = (e < 0.0f) ? 0.0f : ((e > c) ? 1.0f : (e / c));
    }
    else if (m > 1)
    {
        m = 1.0f;
        n = (e + b < 0.0f) ? 0.0f : ((e + b > c) ? 1.0f : ((e + b) / c));
    }

    // 最短距離を表すベクトルの計算
    Math::Vector3 dp = w + m * u - n * v;

    // 最短距離の二乗を返す
    return dp.LengthSquared();
}
