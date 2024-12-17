#pragma once

class ChildController;

/**
* @class ChildState
* @brief 子クラゲが特定の目標位置に移動するステート
* @details
*   - 子クラゲはランダム性のある自然な移動を行い、プレイヤーの後ろに到達するとステートを抜ける
*/
class ChildState : public utl::StateBase<ChildController>
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
    // ベクトルを計算する関数（ランダム性を加味）
    //void CalcNewDir(ChildController* _pOwner, Math::Vector3& _targetDir);

    // 追従座標の初期オフセットを生成
    Math::Vector3 GenerateBasePositionOffset();
    Math::Vector3 m_basePosOffset;

    // 現在の方向
    Math::Vector3 m_currentDir = Math::Vector3::Zero;

    // 経過時間
    float m_elapsedTime = 0.0f;
    // 方向を計算する間隔
    float m_calcDirIntervalSeconds = 0.f;
        
    // ベクトル抽選時に前回のベクトルの影響をどのくらい受けるか
    float m_previewDirRate = 0.f;

    // 基準位置との距離の閾値
    float m_followDistanceThreshold = 30.0f;
    // オフセットする基準の半径 / 高さ
    float m_offsetRadius = 3.0f;
    float m_offsetHeight = -1.0f;
};
