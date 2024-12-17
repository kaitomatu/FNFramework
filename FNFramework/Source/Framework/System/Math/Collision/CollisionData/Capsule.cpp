#include "Capsule.h"

bool Capsule::Contains(const Math::Vector3& point) const
{
    // 点と線分の最短距離(2乗)を求める
    float distSq = m_capsuleData.LineSegment.DistanceFromPointSq(point);

    // 最短距離が半径の2乗より小さい場合は点はカプセルの内側にある
    return distSq <= (m_capsuleData.Radius * m_capsuleData.Radius);
}
