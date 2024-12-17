#pragma once

/**
* @class Capsule
* @brief カプセルクラス
* @details カプセルとは半径を持つ線分
* @details カプセルは球体と線分を用いることで表現できる
*/
class Capsule
    : public CollisionData
{
public:
    // カプセル用データ
    struct CapsuleData
    {
        CapsuleData()
            : LineSegment({}), Radius(0.0f)
        {
        }

        CapsuleData(const LineSegment& lineSegment, float radius)
            : LineSegment(lineSegment), Radius(radius)
        {
        }

        LineSegment LineSegment; // 線分
        float Radius; // 半径
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Capsule()
    {
    }

    Capsule(const LineSegment& lineSegment, float radius)
        : m_capsuleData(lineSegment, radius)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /* @brief カプセルデータ取得 @return カプセルのデータ */
    const CapsuleData& GetCapsuleData() const { return m_capsuleData; }

    /* @brief 線分の設定 @param[in] lineSegment */
    void SetLineSegment(const LineSegment& lineSegment) { m_capsuleData.LineSegment = lineSegment; }
    /* @brief 半径の設定 @param[in] radius */
    void SetRadius(float radius) { m_capsuleData.Radius = radius; }

    //--------------------------------
    // その他関数
    //--------------------------------

    /**
    * @brief 点がカプセルに含まれるかどうか
    * @param[in] point 点
    * @return 点がカプセルに含まれるかどうか - true : 含まれる , false : 含まれない
    */
    bool Contains(const Math::Vector3& point) const;

private:
    CapsuleData m_capsuleData; // カプセルデータ
};
