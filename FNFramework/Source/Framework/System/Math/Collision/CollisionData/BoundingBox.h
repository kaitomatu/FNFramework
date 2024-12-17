#pragma once

//=====================================================================//
// AABB : 軸平行境界ボックス(Axis-Aligned Bouding Box)
//=====================================================================//

/**
* @class AABB
* @brief 軸平行境界ボックス(Axis-Aligned Bouding Box)
* @brief : どの辺もx,y,z軸に平行な直方体
* @brief : Math::Vector3(3D)の場合はz軸も考慮する
*/
template <typename T>
class AABB
    : public CollisionData
{
public:
    // テンプレートの型がMath::Vector2かMath::Vector3かをチェックする
    static_assert(std::is_same_v<T, Math::Vector2>,
                  "class AABB - template<Type T>は Math::Vector2 か Math::Vector3 じゃないといけないです。");
};

// T = Math::Vector3の場合
template <>
class AABB<Math::Vector3>
    : public CollisionData
{
public:
    // AABBデータ構造体
    struct AABBData
    {
        // 最大値と最小値を初期化
        AABBData()
            : Min(MathHelper::VECTOR3_MAX), Max(MathHelper::VECTOR3_MIN)
        {
        }

        Math::Vector3 Min; // 最小値
        Math::Vector3 Max; // 最大値
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    AABB()
    {
    }

    AABB(const Math::Vector3& point)
    {
        UpdateMinMax(point);
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /* @brief AABBデータ取得 @return AABBのデータ */
    const AABBData& GetAABBData() const
    {
        return m_aabbData;
    }

    /* @brief 最小値を取得 @return 最小値 */
    const Math::Vector3& GetMin() const
    {
        return m_aabbData.Min;
    }

    Math::Vector3& WorkMin()
    {
        return m_aabbData.Min;
    }

    void SetMin(const Math::Vector3& min)
    {
        m_aabbData.Min = min;
    }

    /* @brief 最大値を取得 @return 最大値 */
    const Math::Vector3& GetMax() const
    {
        return m_aabbData.Max;
    }

    Math::Vector3& WorkMax()
    {
        return m_aabbData.Max;
    }

    void SetMax(const Math::Vector3& max)
    {
        m_aabbData.Max = max;
    }

    Math::Vector3 GetCenter() const
    {
        return (m_aabbData.Min + m_aabbData.Max) * 0.5f;
    }

    /**
    * @brief サイズを取得
    * @return サイズ
    */
    Math::Vector3 GetSize() const
    {
        return m_aabbData.Max - m_aabbData.Min;
    }

    /**
     * @fn DirectX::BoundingBox ToDirectXBoundingBox()
     *
     * @brief 自作フォーマットのAAABBをDirectX::BoundingBoxに変換する
     * @return DirectX::BoundingBox
     */
    DirectX::BoundingBox ToDirectXBoundingBox() const
    {
        // AABBのMinとMaxを取得
        const Math::Vector3& min = GetMin();
        const Math::Vector3& max = GetMax();

        // 中心点を計算
        Math::Vector3 center = GetCenter();

        // 範囲（半径）を計算
        Math::Vector3 extents = GetSize() * 0.5f;

        // DirectX::BoundingBoxを生成して返す
        return DirectX::BoundingBox(
            { center.x, center.y, center.z },   // 中心点
            { extents.x, extents.y, extents.z } // 範囲
        );
    }
    void ToDirectXBoundingBox(DirectX::BoundingBox& _dxAABB) const
    {
        _dxAABB = ToDirectXBoundingBox();
    }

    //--------------------------------
    // その他関数
    //--------------------------------

    void Reset()
    {
        m_aabbData.Min = Math::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
        m_aabbData.Max = Math::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }

    void SetFromDirectXBoundingBox(const DirectX::BoundingBox& dxBox)
    {
        m_aabbData.Min.x = dxBox.Center.x - dxBox.Extents.x;
        m_aabbData.Min.y = dxBox.Center.y - dxBox.Extents.y;
        m_aabbData.Min.z = dxBox.Center.z - dxBox.Extents.z;

        m_aabbData.Max.x = dxBox.Center.x + dxBox.Extents.x;
        m_aabbData.Max.y = dxBox.Center.y + dxBox.Extents.y;
        m_aabbData.Max.z = dxBox.Center.z + dxBox.Extents.z;
    }

    /**
    * @brief 引数に応じた値に応じて最小値と最大値を更新する
    * @param[in] point : 更新する値
    */
    void UpdateMinMax(const Math::Vector3& point)
    {
        // 各要素を比較して最小値と最大値を更新する
        m_aabbData.Max.x = std::max(m_aabbData.Max.x, point.x);
        m_aabbData.Min.x = std::min(m_aabbData.Min.x, point.x);

        m_aabbData.Max.y = std::max(m_aabbData.Max.y, point.y);
        m_aabbData.Min.y = std::min(m_aabbData.Min.y, point.y);

        m_aabbData.Max.z = std::max(m_aabbData.Max.z, point.z);
        m_aabbData.Min.z = std::min(m_aabbData.Min.z, point.z);
    };

    /**
    * @brief 回転後のAABBを計算する
    * ※この処理だと回転後のAABBは、回転前のAABBよりも大きくなる可能性がある
    * @param[in] q : 回転行列
    */
    void Rotate(const Math::Quaternion& q)
    {
        // ボックスの角の8点を計算する
        std::array<Math::Vector3, 8> points;
        // 最少の点は角にある
        points[0] = m_aabbData.Min;
        // 最大の点は角にある
        points[7] = m_aabbData.Max;

        // 最小の点と最大の点の中間点を計算する
        points[1] = Math::Vector3{m_aabbData.Max.x, m_aabbData.Min.y, m_aabbData.Min.z};
        points[2] = Math::Vector3{m_aabbData.Min.x, m_aabbData.Max.y, m_aabbData.Min.z};
        points[3] = Math::Vector3{m_aabbData.Min.x, m_aabbData.Min.y, m_aabbData.Max.z};
        points[4] = Math::Vector3{m_aabbData.Min.x, m_aabbData.Max.y, m_aabbData.Max.z};
        points[5] = Math::Vector3{m_aabbData.Max.x, m_aabbData.Min.y, m_aabbData.Max.z};
        points[6] = Math::Vector3{m_aabbData.Max.x, m_aabbData.Max.y, m_aabbData.Min.z};

        // 初めの点を回転させる
        Math::Vector3 point = Math::Vector3::Transform(points[0], q);
        // 回転後のAABBの最小値と最大値を更新する
        m_aabbData.Min = point;
        m_aabbData.Max = point;

        // 残りの点を回転させる
        for (UINT i = 1; i < points.size(); ++i)
        {
            point = Math::Vector3::Transform(points[i], q);
            UpdateMinMax(point);
        }
    }

    // Math::Matrix に対応する Rotate 関数をオーバーロード
    void Rotate(const Math::Matrix& mat)
    {
        // ボックスの角の8点を計算する
        std::array<Math::Vector3, 8> points;
        // 最少の点は角にある
        points[0] = m_aabbData.Min;
        // 最大の点は角にある
        points[7] = m_aabbData.Max;

        // 最小の点と最大の点の中間点を計算する
        points[1] = Math::Vector3{m_aabbData.Max.x, m_aabbData.Min.y, m_aabbData.Min.z};
        points[2] = Math::Vector3{m_aabbData.Min.x, m_aabbData.Max.y, m_aabbData.Min.z};
        points[3] = Math::Vector3{m_aabbData.Min.x, m_aabbData.Min.y, m_aabbData.Max.z};
        points[4] = Math::Vector3{m_aabbData.Min.x, m_aabbData.Max.y, m_aabbData.Max.z};
        points[5] = Math::Vector3{m_aabbData.Max.x, m_aabbData.Min.y, m_aabbData.Max.z};
        points[6] = Math::Vector3{m_aabbData.Max.x, m_aabbData.Max.y, m_aabbData.Min.z};

        // 初めの点を回転させる
        Math::Vector3 point = Math::Vector3::Transform(points[0], mat);
        // 回転後のAABBの最小値と最大値を更新する
        m_aabbData.Min = point;
        m_aabbData.Max = point;

        // 残りの点を回転させる
        for (UINT i = 1; i < points.size(); ++i)
        {
            point = Math::Vector3::Transform(points[i], mat);
            UpdateMinMax(point);
        }
    }

    /**
    * @brief 点がAABBの中にあるかどうかを判定する
    * @param[in] point : 判定する点
    * @return 点がAABBの中にあるかどうか - true : 点がAABBの中にある , false : 点がAABBの中にない
    */
    bool Contains(const Math::Vector3& point) const
    {
        // 点がAABBの外側にあるかどうかを判定する
        if (point.x < m_aabbData.Min.x) { return false; } // 点がAABBの左側にある
        if (point.y < m_aabbData.Min.y) { return false; } // 点がAABBの下側にある
        if (point.z < m_aabbData.Min.z) { return false; } // 点がAABBの奥側にある
        if (point.x > m_aabbData.Max.x) { return false; } // 点がAABBの右側にある
        if (point.y > m_aabbData.Max.y) { return false; } // 点がAABBの上側にある
        if (point.z > m_aabbData.Max.z) { return false; } // 点がAABBの手前側にある

        // どれも真じゃない = 点がAABBの中にある
        return true;
    }

    /*
    * @brief 点とAABBの最短距離の二乗を計算する
    * @param[in] point : 計算する点
    * @return 点とAABBの最短距離の二乗
    */
    float MinDistSq(const Math::Vector3& point) const
    {
        // 各軸の差を計算する
        float dx = std::max(m_aabbData.Min.x - point.x, 0.0f);
        dx = std::max(dx, point.x - m_aabbData.Max.x);
        float dy = std::max(m_aabbData.Min.y - point.y, 0.0f);
        dy = std::max(dy, point.y - m_aabbData.Max.y);
        float dz = std::max(m_aabbData.Min.z - point.z, 0.0f);
        dz = std::max(dz, point.z - m_aabbData.Max.z);
        // 距離の二乗を返す
        return dx * dx + dy * dy + dz * dz;
    }

private:
    // AABBデータ
    AABBData m_aabbData;
};

//=====================================================================//
// OBB : 回転する境界ボックス(Oriented Bouding Box)
//=====================================================================//

/**
* @class OBB
* @brief 有向バウンディング(境界)ボックス(Oriented Bouding Box)
* @details : AABBのような軸平行の制限をもたないので、回転を行えるがAABBと比べて計算コストが高い
* @details : 元のオブジェクトの形状に合わせて回転するので、AABBのように誤差が生まれない
*/

class OBB
{
public:
    OBB()
        : Center(0.0f, 0.0f, 0.0f),
          Extents(1.0f, 1.0f, 1.0f),
          Orientation(Math::Quaternion::Identity)
    {
    }

    OBB(const Math::Vector3& center, const Math::Vector3& extents, const Math::Quaternion& orientation)
        : Center(center), Extents(extents), Orientation(orientation)
    {
    }

    // メンバ変数
    Math::Vector3 Center;
    Math::Vector3 Extents;
    Math::Quaternion Orientation;

    // 点が OBB 内にあるかを判定
    bool Contains(const Math::Vector3& point) const
    {
        // 点をローカル座標系に変換
        Math::Quaternion q;
        Orientation.Conjugate(q);
        Math::Vector3 localPoint = Math::Vector3::Transform(point - Center, q);

        // ローカル座標系での判定
        return (std::abs(localPoint.x) <= Extents.x) &&
               (std::abs(localPoint.y) <= Extents.y) &&
               (std::abs(localPoint.z) <= Extents.z);
    }

    // DirectX::BoundingOrientedBox への変換
    DirectX::BoundingOrientedBox ToDirectXOBB() const
    {
        return DirectX::BoundingOrientedBox(
            Center,
            Extents,
            Orientation);
    }
};
