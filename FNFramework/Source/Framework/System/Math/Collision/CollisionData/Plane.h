#pragma once

/**
* @class Plane
*/
class Plane
    : public CollisionData
{
public:
    //---------------
    // 平面のデータ
    //---------------
    struct PlaneData
    {
        Math::Vector3 Normal = {}; // 平面の法線
        float Distance = 0.0f; // 原点から平面までの最短距離(符号付)
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Plane()
    {
    }

    Plane(const Math::Vector3& point0, const Math::Vector3& point1, const Math::Vector3& point2)
    {
        CreatePlane(point0, point1, point2);
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /* @brief 平面データ取得 @return 平面データ */
    const PlaneData& GetPlaneData() const { return m_planeData; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @biref 3点から平面を作成する - 法線から面の向きなどを求める
    * @param point0 1点目
    * @param point1 2点目
    * @param point2 3点目
    */
    void CreatePlane(const Math::Vector3& point0, const Math::Vector3& point1, const Math::Vector3& point2);

    /**
    * @brief  点と平面の距離を求める
    * @param  point - 点
    * @return 点と平面の距離
    */
    float SignedDistance(const Math::Vector3& point) const
    {
        // 結果が負の値である場合、pointは平面の裏側(下)にある
        // 逆に正の値である場合、pointは平面の表側(上)にある
        return point.Dot(m_planeData.Normal) - m_planeData.Distance;
    }

private:
    PlaneData m_planeData;
};
