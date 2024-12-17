#include "Sphere.h"

bool Sphere::Contains(const Math::Vector3& point)
{
    // 中心と点との距離の2乗を計算
    float distSq = (m_sphereData.Center - point).LengthSquared();

    // 半径の2乗より小さい場合は、点は球の内側にある
    return distSq <= (m_sphereData.Radius * m_sphereData.Radius);
}
