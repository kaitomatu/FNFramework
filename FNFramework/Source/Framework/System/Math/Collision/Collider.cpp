#include "Collider.h"

//==========================
// 線分関係
//==========================
/**
* @brief 線分と平面の当たり判定
* @param[in] line - 線分
* @param[in] plane - 平面
* @param[out] lineIntersectPos 線分と平面の交点の位置を相対的に示すパラメータ
* @return bool 線分と平面が当たっているかどうか - 当たっている : true / 当たっていない : false
*/
bool Collider::Intersect(const LineSegment& line, const Plane& plane, float& lineIntersectPos)
{
    // 線分データ格納
    const LineSegment::LineData& lineData = line.GetLineData();
    // 平面データ格納
    const Plane::PlaneData& planeData = plane.GetPlaneData();

    // ラインの方向ベクトルを求める
    Math::Vector3 lineDir = lineData.End - lineData.Start;

    float dirDotNormal = lineDir.Dot(planeData.Normal);
    // 内積がゼロに近い場合、平面と線分は平行であり交差しない
    if (MathHelper::NearZero(dirDotNormal))
    {
        // 平面と線分の交点を求める
        float distanceToPlane = lineData.Start.Dot(planeData.Normal) - planeData.Distance;
        // 距離がゼロに近い場合、線分の始点は平面上にある
        if (MathHelper::NearZero(distanceToPlane))
        {
            lineIntersectPos = 0.0f;
            return true;
        }
        return false;
    }
    // 平面と線分の交点を求める
    float distanceToPlane = -lineData.Start.Dot(planeData.Normal) - planeData.Distance;
    lineIntersectPos = distanceToPlane / dirDotNormal;

    // tが線分の範囲内にあるのかどうかを判定する
    bool withinLine = (lineIntersectPos >= 0.0f && lineIntersectPos <= 1.0f);
    return withinLine;
}

/**
* @brief 線分と球の判定
* @param[in] line - 線分
* @param[in] sphere - 球
* @param[out] lineIntersectPos 線分と球の交点の位置を相対的に示すパラメータ
* @return bool 線分と球が当たっているかどうか - 当たっている : true / 当たっていない : false
*/
bool Collider::Intersect(const LineSegment& line, const Sphere& sphere, float& lineIntersectPos)
{
    // 線分データ格納
    const LineSegment::LineData& lineData = line.GetLineData();
    // 球体データ格納
    const Sphere::SphereData& sphereData = sphere.GetSphereData();

    // 方程式の X,Y,a,b,c を求める
    Math::Vector3 X = lineData.Start - sphereData.Center;
    Math::Vector3 Y = lineData.End - lineData.Start;
    float a = Y.Dot(Y);
    float b = 2.0f + X.Dot(Y);
    float c = X.Dot(X) - sphereData.Radius * sphereData.Radius;

    // 判別式を求める
    float disc = b * b - 4.0f * a * c;

    if (disc < 0.0f) { return false; }

    // 判別式の平方根を求める
    float sqrtDisc = sqrt(disc);

    // tの解(min / max)を求める
    float tMin = (-b - sqrtDisc) / (2.0f * a);
    float tMax = (-b + sqrtDisc) / (2.0f * a);

    // tが線分の範囲内にあるのかを判定する
    if (tMin >= 0.0f && tMin <= 1.0f)
    {
        lineIntersectPos = tMin;
        return true;
    }
    if (tMax >= 0.0f && tMax <= 1.0f)
    {
        lineIntersectPos = tMax;
        return true;
    }

    // どこにも引っかからない場合は当たっていない
    return false;
}

//==========================
// カプセル関係
//==========================
/**
* @brief カプセルとカプセルの当たり判定
* @param[in] capsule1 - カプセル1
* @param[in] capsule2 - カプセル2
* @return bool カプセルとカプセルが当たっているかどうか - 当たっている : true / 当たっていない : false
*/
bool Collider::Intersect(const Capsule& capsule1, const Capsule& capsule2)
{
    // カプセルデータ格納
    const Capsule::CapsuleData& cd1 = capsule1.GetCapsuleData();
    const Capsule::CapsuleData& cd2 = capsule2.GetCapsuleData();

    // 線分同士の最短距離を求める
    float distSq = LineSegment::MinDistSq(cd1.LineSegment.GetLineData(), cd2.LineSegment.GetLineData());

    // 半径の和を求める
    float sumRadii = cd1.Radius + cd2.Radius;

    return distSq <= (sumRadii * sumRadii);
}

//==========================
// 球関係
//==========================
/**
* @brief 球と球の当たり判定
* @param[in] - sphere1 球1
* @param[in] - sphere2 球2
* @return bool 球と球が当たっているかどうか - 当たっている : true / 当たっていない : false
*/
inline bool Collider::Intersect(const Sphere& sphere1, const Sphere& sphere2)
{
    // 球体データ格納
    const Sphere::SphereData& sd1 = sphere1.GetSphereData();
    const Sphere::SphereData& sd2 = sphere2.GetSphereData();

    // 球体同士の距離を求める
    float distSq = (sd1.Center - sd2.Center).LengthSquared();

    // 半径の和を求める
    float sumRadii = sd1.Radius + sd2.Radius;

    // 球の距離が半径の和より小さい場合は衝突している
    return distSq <= (sumRadii * sumRadii);
}

/**
* @brief 球スイープの交差判定
*
* @details
*	球スイープとは、球が移動することによって生じる交差判定のこと
*	tを 0～1 の間で進めることで、フレームで移動した交差判定を網羅することができる
*
* @param[in] sp1_before - 移動前(1つ前のフレーム)の球1
* @param[in] sp1_now    - 移動後(現在のフレーム)の球1
* @param[in] sp2_before - 移動前(1つ前のフレーム)の球2
* @param[in] sp2_now    - 移動後(現在のフレーム)の球2
* @param[out] outT		- 交差した時のtの値
* @return bool 球スイープが交差しているかどうか - 交差している : true / 交差していない : false
*/
bool Collider::SweptSphere(const Sphere& sp1_before, const Sphere& sp1_now, const Sphere& sp2_before,
                           const Sphere& sp2_now, float& outT)
{
    // １つ目の球体データ格納
    const Sphere::SphereData& p0 = sp1_before.GetSphereData(); // 1フレーム前のデータ
    const Sphere::SphereData& p1 = sp1_now.GetSphereData(); // 現在のフレームのデータ
    // ２つ目の球体データ格納
    const Sphere::SphereData& q0 = sp2_before.GetSphereData(); // 1フレーム前のデータ
    const Sphere::SphereData& q1 = sp2_now.GetSphereData(); // 現在のフレームのデータ

    // 公式の X, Y, a, b, c を求める
    Math::Vector3 X = p0.Center - q0.Center; // 移動前の球同士の距離
    Math::Vector3 Y = p1.Center - p0.Center - (q1.Center - q0.Center);
    float a = Y.Dot(Y);
    float b = 2.0f * X.Dot(Y);
    float sumRadii = p0.Radius + q0.Radius;
    float c = X.Dot(X) - sumRadii * sumRadii;

    // 判別式を求める
    float disc = b * b - 4.0f * a * c;

    // 判別式が負の値の場合は交差していない
    if (disc < 0.0f) { return false; }

    disc = sqrt(disc);

    // 球形なら2点ヒットする可能性があるので、より近い方を選択する
    outT = (-b - disc) / (2.0f * a);

    if (outT >= 0.0f && outT <= 1.0f) { return true; }

    // tが 0～1 の範囲外の場合は、線分上でヒットしていない
    return false;
}
