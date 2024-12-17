#pragma once

#include "ItemState.h"

class ChildController;

/**
* @class ChildKurageItemState
* @brief 子クラゲがアイテム状態の場合のステート : ItemStateを継承
* @details
*   - ItemStateの要素利用しつつ、子クラゲがアイテム状態のときの挙動を設定する
*/
class ChildKurageItemState
    : public ItemState
{
public:

    /**
     * @fn void Enter(TempOwnerName* _pOwner)
     * @brief ステートに入ってきた時の処理
     * @param _pOwner : 所有者のポインタ
     */
    void Enter(ChildController* _pOwner) override;
    /**
     * @fn void Update(TempOwnerName* _pOwner)
     * @brief ステートの更新処理
     * @param _pOwner : 所有者のポインタ
     */
    void Update(ChildController* _pOwner) override;
    /**
     * @fn void Exit(TempOwnerName* _pOwner)
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

    //x--------- ぷかぷか挙動の設定 ---------x//
    /**
     * @fn void CalcNewDir(TempOwnerName* _pOwner, Math::Vector3& _dir)
     * @brief ぷかぷかする新しい方向を算出する
     * @param _pOwner : 所有者のポインタ
     * @param _dir : 新しい方向を格納する変数
     */
    float m_elapsedTime = 0.0f;
    float m_calcDirIntervalSeconds = 0.5f;

    // ベクトル抽選時に前回のベクトルの影響をどのくらい受けるか
    float m_previewDirRate = 0.5f;

    // 現在の方向
    Math::Vector3 m_currentDir = Math::Vector3::Zero;
    // 目標とする方向
    Math::Vector3 m_targetDir = Math::Vector3::Zero;
    // フレーム間でのベクトルの補間率 : 1フレームごとにこの割合で m_targetDir の方向へむく
    float m_interpolationRate = 0.1f;
};
