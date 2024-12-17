#pragma once

/**
* @brief バウンディング用ボックスクラス
*/
class Box
    : public CollisionData
{
public:
    // ボックスデータ構造体
    struct BoxData
    {
        BoxData()
        {
        }

        BoxData(const Math::Vector3& center, const Math::Vector3& extents)
            : Center(center), Extents(extents)
        {
        }

        Math::Vector3 Center; // ボックスの中心座標
        Math::Vector3 Extents; // ボックスの各軸に沿った半分のサイズ - つまり、ボックスの幅は Extents.x * 2 となる
    };

    //---------------------------------------
    // コンストラクタ / デストラクタ
    //---------------------------------------
    Box()
    {
    }

    Box(const Math::Vector3& center, const Math::Vector3& extents)
        : m_boxData(center, extents)
    {
    }

    Box(const BoxData& initData)
        : m_boxData(initData)
    {
    }

    //---------------------------------------
    // ゲッター / セッター
    //---------------------------------------
    /* @brief ボックスのデータを取得する @return ボックスデータ */
    const BoxData& GetSphereData() const { return m_boxData; }

    /* @brief ボックスの中心位置設定 @param[in] center - 中心座標 */
    void SetCenter(const Math::Vector3& center) { m_boxData.Center = center; }
    /* @brief ボックスの大きさ設定 @param[in] extents - ボックスのサイズ(半分) */
    void SetExtents(const Math::Vector3& extents) { m_boxData.Extents = extents; }

private:
    // ボックス用データ
    BoxData m_boxData;
};
