#pragma once

/**
* @class Polygon
* @brief ポリゴンクラス
*/
class Polygon
    : public CollisionData
{
public:
    // 頂点データ
    struct Vertex
    {
        Math::Vector3 Pos; // 座標
        //Math::Vector2 UV;
        //Math::Vector3 Normal;
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Polygon()
    {
    }

    Polygon(const std::vector<Vertex>& vertex)
        : m_vertex(vertex)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /* @brief カプセルデータ取得 @return カプセルのデータ */
    const std::vector<Vertex>& GetVertices() const { return m_vertex; }
    /* @brief 頂点データの座標取得 @param[out] result - 座標を格納する変数 */
    void GetPosition(std::vector<Math::Vector3>& result);

    /* @brief 頂点の設定 @param[in] vertex - 頂点データ */
    void SetVertices(const std::vector<Vertex>& vertex) { m_vertex = vertex; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @bierf 2Dの凸ポリゴンが点を内包しているかどうか
    * @param[in] point - 点の座標
    * @return 内包しているかどうか - true:内包している false:内包していない
    */
    bool Contains(const Math::Vector2& point) const;

private:
    // 頂点は時計回りで用意する
    std::vector<Vertex> m_vertex; // 頂点
};
