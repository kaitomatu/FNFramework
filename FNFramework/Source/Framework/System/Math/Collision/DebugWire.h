#pragma once

/**
* @class DebugWire
* @brief デバッグワイヤー描画クラス
*/
class DebugWire
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    DebugWire()
    {
        // デバッグビルドの場合はデバッグワイヤー描画を有効にする
#ifdef _DEBUG
        m_enable = true;
#endif
    }
    ~DebugWire() { ClearVertex(); }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
    * @brief デバッグワイヤー描画用頂点取得
    * @return デバッグワイヤー描画用頂点
    */
    const std::vector<MeshVertex>& GetVertices() const { return m_vertices; }

    void EnableDebugWire(bool _enable) { m_enable = _enable; }
    bool IsEnable() const { return m_enable; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief デバッグライン描画用頂点追加
    * @param[in] start 開始位置
    * @param[in] end 終了位置
    * @param[in] color 色
    */
    void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Color& color = Color::White);

    /**
    * @brief デバッグライン描画用頂点追加
    * @param[in] start 開始位置
    * @param[in] dir 方向
    * @param[in] length 長さ
    * @param[in] color 色
    */
    void AddDebugLine(const Math::Vector3& start, const Math::Vector3& dir, float length,
                      const Math::Color& col = Color::White);

    /**
    * @brief デバッグボックス描画用頂点追加
    * @param[in] min 最小値
    * @param[in] max 最大値
    * @param[in] color 色
    */
    void AddDebugBox(const Math::Vector3& min, const Math::Vector3& max, const Math::Color& col = Color::White);
    void AddDebugBox(const AABB<Math::Vector3>& box, const Math::Color& col = Color::White)
    {
        AddDebugBox(box.GetMin(), box.GetMax(), col);
    }
    void AddDebugBox(const OBB& obb, const Math::Color& col = Color::White);


    /**
    * @brief デバッグスフィア描画用頂点追加
    * @param[in] pos 位置
    * @param[in] color 色
    * @param[in] radius 半径
    * @param[in] splitCount 球の分割数
    */
    void AddDebugSphere(const Math::Vector3& centerPos, const Math::Color& color, float radius = 1.0f,
                        int splitCount = 8);

    /**
    * @brief デバッグワイヤー初期化
    */
    void Init();

    /* @brief デバッグワイヤー描画 */
    void Draw();

    /* @brief デバッグワイヤー描画用頂点クリア */
    void ClearVertex();

private:
    static constexpr UINT MaxVerticesNum = 500000; // 最大頂点数 - 必要に応じて変更してください

    // todo : 座標とColorしか使ってないのにMeshVertexで頂点作成しているのはメモリの無駄になるので今後ほかのMeshを作成することを検討
    Mesh vertices; // 描画用頂点配列
    std::vector<MeshVertex> m_vertices; // 描画用頂点配列

    bool m_enable = false; // デバッグワイヤー描画有効かどうか
};
