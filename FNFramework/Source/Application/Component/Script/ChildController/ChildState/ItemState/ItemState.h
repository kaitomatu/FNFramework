#pragma once

class ChildController;

/**
* @class ItemState
* @brief 子クラゲがアイテム状態の時のステート
* @details
*   - 子クラゲがアイテム状態の時のステート
*   - ぷかぷかしていたり、プレイヤーとの距離判定を行い、プレイヤーに近づいたら追従する
*/
class ItemState
    : public utl::StateBase<ChildController>
{
public:
    // あたり判定の距離の設定 / 取得
    void SetLimitDistance(float _dist) { m_contactDistance = _dist; }
    float GetLimitDistance() const { return m_contactDistance; }

    /**
     * @fn void Enter(ChildController* _pOwner)
     * @brief ステートに入ってきた時の処理
     * @param _pOwner : 所有者のポインタ
     */
    void Enter(ChildController* _pOwner) override;
    /**
     * @fn void Update(ChildController* _pOwner)
     * @brief ステートの更新処理
     * @param _pOwner : 所有者のポインタ
     */
    void Update(ChildController* _pOwner) override;
    /**
     * @fn void Exit(ChildController* _pOwner)
     * @brief ステートから出る時の処理
     * @param _pOwner : 所有者のポインタ
     */
    void Exit(ChildController* _pOwner) override;
    /**
     * @fn void ImGui(ChildController* _pOwner)
     * @brief ImGui表示
     * @param _pOwner : 所有者のポインタ
     */
    void ImGui(ChildController* _pOwner) override;

private:
    // プレイヤと接触する最低の距離 : これより近づいたらステート変更
    float m_contactDistance = 4.0f;

    // 所有者(子クラゲが持っているであろうモデルコンポーネントのポインタ)
    std::weak_ptr<ModelComponent> m_wpModelComponent;
};
