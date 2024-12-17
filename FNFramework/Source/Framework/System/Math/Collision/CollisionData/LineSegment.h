#pragma once

/**
* @brief 線分用衝突データ
*/
class LineSegment
    : public CollisionData
{
public:
    //----------------
    // 線分のデータ
    //----------------
    struct LineData
    {
        LineData()
        {
        }

        LineData(const Math::Vector3& start, const Math::Vector3& end)
            : Start(start), End(end)
        {
        }

        Math::Vector3 Start = {}; // 線分の始点
        Math::Vector3 End = {}; // 線分の終点
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    LineSegment()
    {
    }

    LineSegment(const Math::Vector3& start, const Math::Vector3& end)
        : m_lineData(start, end)
    {
    }

    LineSegment(const LineData& initData)
        : m_lineData(initData)
    {
    }

    //--------------------------------
    // ゲッター
    //--------------------------------
    /* @brief 線分のデータを取得する @return 線分データ */
    const LineData& GetLineData() const { return m_lineData; }
    LineData& WorkLineData() { return m_lineData; }
    /**
    * @brief 線分のデータを設定する 
    * @param[in] lineData - 線分のデータ
    */
    void SetLineData(const LineData& lineData)
    {
        m_lineData = lineData;
    }

    /**
    * @brief 線分の始点を設定する
    * @param[in] start - 線分の始点
    */
    void SetLineStart(const Math::Vector3& start)
    {
        m_lineData.Start = start;
    }

    /**
    * @brief 線分の終点を設定する 
    * @param[in] end - 線分の終点
    */
    void SetLineEnd(const Math::Vector3& end)
    {
        m_lineData.End = end;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief 線分上の点を計算する
    * @param[in] t - 0～1の値で線分上のどの位置かを指定
    * @return 線分上の点
    */
    Math::Vector3 PointOnSegment(float t) const
    {
        return Math::Vector3::Lerp(m_lineData.Start, m_lineData.End, t);
    }

    /**
    * @brief 点と線分の最短距離を計算する
    * @param[in] point - 点
    * @return 点と線分の最短距離
    */
    float DistanceFromPoint(const Math::Vector3& point) const;

    /**
    * @brief 点と線分の最短距離(Sqared)を計算する
    * @param[in] point - 点
    * @return 点と線分の最短距離の2乗
    */
    float DistanceFromPointSq(const Math::Vector3& point) const;

    //------------------------------------------------
    // 外部からのアクセス関数
    //------------------------------------------------

    /**
    * @brief 2つの線分間の最短距離(Sqared)を計算する
    * @param[in] line1 - 線分1
    * @param[in] line2 - 線分2
    * @return 点と線分の最短距離の2乗
    */
    static float MinDistSq(const LineData& line1, const LineData& line2);

private:
    LineData m_lineData; // 線分のデータ
};
