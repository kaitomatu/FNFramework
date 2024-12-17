#pragma once

/**
* @class バウンディングボリューム用球体クラス
*/
class Sphere
    : public CollisionData
{
public:
    // 球体データ構造体
    struct SphereData
    {
        SphereData()
        {
        }

        SphereData(const Math::Vector3& center, float radius)
            : Center(center), Radius(radius)
        {
        }

        Math::Vector3 Center = {}; // 中心座標
        float Radius = 0.0f; // 半径
    };

    //---------------------------------------
    // コンストラクタ / デストラクタ
    //---------------------------------------
    Sphere()
    {
    }

    Sphere(const Math::Vector3& center, float radius)
        : m_sphereData(center, radius)
    {
    }

    Sphere(const SphereData& initData)
        : m_sphereData(initData)
    {
    }

    //---------------------------------------
    // ゲッター / セッター
    //---------------------------------------
    /* @brief 球体のデータを取得する @return 球体データ */
    const SphereData& GetSphereData() const { return m_sphereData; }

    /* @brief 球体の中心位置設定 @param[in] center - 中心座標 */
    void SetCenter(const Math::Vector3& center) { m_sphereData.Center = center; }
    /* @brief 球体の半径設定　@param[in] radius - 半径設定 */
    void SetRadius(float radius) { m_sphereData.Radius = radius; }

    //---------------------------------------
    // その他関数
    //---------------------------------------
    /**
    * @brief 点が球体の中に入っているかどうか判定する
    * @param[in] point - 判定する点
    * @return 球体の中に入っているかどうか - true:入っている false:入っていない
    */
    bool Contains(const Math::Vector3& point);

private:
    // 球体データ
    SphereData m_sphereData;
};
