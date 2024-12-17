#pragma once

/**
* @class Collider
* @brief 当たり判定をサポートするクラス
* @detailed 後々、各形状(球体、AABB、OBB、平面、カプセル、メッシュ)のクラスに移動する
*/
namespace Collider
{
    //==========================
    // 線分関係
    //==========================
    /**
    * @brief 線分と平面の当たり判定
    * @param[in] line  - 線分
    * @param[in] plane - 平面
    * @param[out] lineIntersectPos 線分と平面の交点の位置を相対的に示すパラメータ
    * @return bool 線分と平面が当たっているかどうか - 当たっている : true / 当たっていない : false
    */
    static bool Intersect(const LineSegment& line, const Plane& plane, float& lineIntersectPos);

    /**
    * @brief 線分と球の判定
    * @param[in] line   - 線分
    * @param[in] sphere - 球
    * @param[out] lineIntersectPos 線分と球の交点の位置を相対的に示すパラメータ
    * @return bool 線分と球が当たっているかどうか - 当たっている : true / 当たっていない : false
    */
    static bool Intersect(const LineSegment& line, const Sphere& sphere, float& lineIntersectPos);

    /**
    * @brief AABBの側面(無限平面)と交差しているかをテストする
    * @param[in] start - 線分の始点
    * @param[in] end   - 線分の終点
    * @param[in] negd  - 平面の負の距離
    * @paramin] norm   - 平面の法線
    * @param[out] out  - 交点の位置を相対的に示すパラメータと平面の法線データ
    * @return bool AABBの側面(無限平面)と交差しているかどうか - 当たっている : true / 当たっていない : false
    *
    * @details 平面は無限平面なので、AABBの側面(無効側)と交差している可能性があるのでテスト用のヘルパー関数
    */
    static bool TestSidePlane(float start, float end, float negd, const Math::Vector3& norm,
                              std::vector<std::pair<float, Math::Vector3>>& out)
    {
        // 線分の長さを求める
        float lineSegmentLength = end - start;

        if (MathHelper::NearZero(lineSegmentLength)) { return false; }

        // 線分の始点から平面までの距離を求める
        float planeOffset = -start + negd;
        float t = planeOffset / lineSegmentLength;

        // tが線分の範囲内にあるのかを判定する
        if (t >= 0.0f && t <= 1.0f)
        {
            out.emplace_back(t, norm);
            return true;
        }

        // 
        return false;
    }

    /**
    * @brief 線分とAABBの当たり判定
    * @param[in] line - 線分
    * @param[in] box  - AABB
    * @param[out] lineIntersectPos - 線分とAABBの交点の位置を相対的に示すパラメータ
    * @param[out] outNorm - 線分とAABBの交点の法線
    * @return bool 線分とAABBが当たっているかどうか - 当たっている : true / 当たっていない : false
    */
    static bool Intersect(const LineSegment& line, const AABB<Math::Vector3>& box, float& lineIntersectPos,
                          Math::Vector3& outNorm)
    {
        // 線分データ格納
        const LineSegment::LineData& lineData = line.GetLineData();
        // AABBデータ格納
        const AABB<Math::Vector3>::AABBData& boxData = box.GetAABBData();

        // ヒットする可能性のあるtの値を格納する配列
        std::vector<std::pair<float, Math::Vector3>> tValues;

        // X平面
        TestSidePlane(lineData.Start.x, lineData.End.x, boxData.Min.x, -Math::Vector3::UnitX, tValues);
        TestSidePlane(lineData.Start.x, lineData.End.x, boxData.Max.x, Math::Vector3::UnitX, tValues);
        // Y平面
        TestSidePlane(lineData.Start.y, lineData.End.y, boxData.Min.y, -Math::Vector3::UnitY, tValues);
        TestSidePlane(lineData.Start.y, lineData.End.y, boxData.Max.y, Math::Vector3::UnitY, tValues);
        // Z平面
        TestSidePlane(lineData.Start.z, lineData.End.z, boxData.Min.z, -Math::Vector3::UnitZ, tValues);
        TestSidePlane(lineData.Start.z, lineData.End.z, boxData.Max.z, Math::Vector3::UnitZ, tValues);

        // tの値が小さいもの順にソート
        std::sort(tValues.begin(), tValues.end(), [](
                  const std::pair<float, Math::Vector3>& a,
                  const std::pair<float, Math::Vector3>& b)
                  {
                      return a.first < b.first;
                  });

        // boxに交点が含まれるのかのテスト
        Math::Vector3 point = {};
        for (std::pair<float, Math::Vector3> t : tValues)
        {
            point = line.PointOnSegment(t.first);

            if (box.Contains(point))
            {
                lineIntersectPos = t.first;
                outNorm = t.second;
                return true;
            }
        }

        // ボックス内に交点がない
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
    static bool Intersect(const Capsule& capsule1, const Capsule& capsule2);

    //==========================
    // 球関係
    //==========================
    /**
    * @brief 球と球の当たり判定
    * @param[in] - sphere1 球1
    * @param[in] - sphere2 球2
    * @return bool 球と球が当たっているかどうか - 当たっている : true / 当たっていない : false
    */
    static bool Intersect(const Sphere& sphere1, const Sphere& sphere2);

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
    static bool SweptSphere(const Sphere& sp1_before, const Sphere& sp1_now, const Sphere& sp2_before,
                            const Sphere& sp2_now, float& outT);

    //==========================
    // AABB関係
    //==========================
    /**
    * @brief AABBとAABBの当たり判定
    * @param[in] aabb1 - AABB1
    * @param[in] aabb2 - AABB2
    * @return bool AABBとAABBが当たっているかどうか - 当たっている : true / 当たっていない : false
    */
    template <typename T>
    static bool Intersect(const AABB<T>& aabb1, const AABB<T>& aabb2);

    /**
    * @brief 球とAABBの当たり判定
    * @param[in] sphere - 球
    * @param[in] box - AABB
    * @return bool 球とAABBが当たっているかどうか - 当たっている : true / 当たっていない : false
    */
    template <typename T>
    static bool Intersect(const Sphere& sphere, const AABB<T>& box);
}

//--------------------------------------//
// AABBとAABBの当たり判定
// テンプレートのため、実装はヘッダーに記述
//--------------------------------------//

template <typename T>
static bool Collider::Intersect(const AABB<T>& aabb1, const AABB<T>& aabb2)
{
    // AABBデータを取得
    const typename AABB<T>::AABBData& data1 = aabb1.GetAABBData();
    const typename AABB<T>::AABBData& data2 = aabb2.GetAABBData();

    // 当たっていない条件を調べる
    // x軸でのチェック - aabb1がaabb2の左側にあるか || aabb1がaabb2の右側にあるか
    if (data1.Max.x < data2.Min.x || data2.Max.x < data1.Min.x)
    {
        return false;
    }
    // y軸でのチェック - aabb1がaabb2の下側にあるか || aabb1がaabb2の上側にあるか
    if (data1.Max.y < data2.Min.y || data2.Max.y < data1.Min.y)
    {
        return false;
    }

    // どれか一つでも当たっていない場合は、AABB同士は交差している
    return true;
}

template <>
static bool Collider::Intersect(const AABB<Math::Vector3>& aabb1, const AABB<Math::Vector3>& aabb2)
{
    // AABBデータを取得
    const AABB<Math::Vector3>::AABBData& data1 = aabb1.GetAABBData();
    const AABB<Math::Vector3>::AABBData& data2 = aabb2.GetAABBData();

    // x軸でのチェック - aabb1がaabb2の左側にあるか || aabb1がaabb2の右側にあるか
    if (data1.Max.x < data2.Min.x || data2.Max.x < data1.Min.x)
    {
        return false;
    }
    // y軸でのチェック - aabb1がaabb2の下側にあるか || aabb1がaabb2の上側にあるか
    if (data1.Max.y < data2.Min.y || data2.Max.y < data1.Min.y)
    {
        return false;
    }
    // z軸でのチェック - aabb1がaabb2の奥側にあるか || aabb1がaabb2の手前側にあるか
    if (data1.Max.z < data2.Min.z || data2.Max.z < data1.Min.z)
    {
        return false;
    }

    // どれもあてはまらない場合は、AABB同士は交差している
    return true;
}

//--------------------------------------//
// AABBと球の当たり判定
// テンプレートのため、実装はヘッダーに記述
//--------------------------------------//

template <typename T>
static bool Collider::Intersect(const Sphere& sphere, const AABB<T>& box)
{
    // 球体データを取得
    const Sphere::SphereData& sd = sphere.GetSphereData();

    // ボックスと球の中心点の距離を求める
    float distSq = box.MinDistSq(sd.Center);
    // ボックスの中に球があるかどうかを判定する
    return distSq <= (sd.Radius * sd.Radius);
}

template <>
static bool Collider::Intersect(const Sphere& sphere, const AABB<Math::Vector3>& box)
{
    // 球体データを取得
    const Sphere::SphereData& sd = sphere.GetSphereData();

    // ボックスと球の中心点の距離を求める
    float distSq = box.MinDistSq(sd.Center);
    // ボックスと球の中心点の距離を求める
    return distSq <= (sd.Radius * sd.Radius);
}
