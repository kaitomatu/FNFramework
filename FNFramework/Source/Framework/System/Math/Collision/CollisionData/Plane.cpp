#include "Plane.h"

void Plane::CreatePlane(const Math::Vector3& p0, const Math::Vector3& p1, const Math::Vector3& p2)
{
    // 各ベクトルを求める
    Math::Vector3 v1 = p1 - p0; // p0->p1
    Math::Vector3 v2 = p2 - p0; // p0->p2

    // 外積と正規化で法線を求める
    m_planeData.Normal = v1.Cross(v2);
    m_planeData.Normal.Normalize();

    // 法線と点の内積でdistanceを求める
    m_planeData.Distance = -m_planeData.Normal.Dot(p0);
}
