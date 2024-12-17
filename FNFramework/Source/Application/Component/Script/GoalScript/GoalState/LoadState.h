#pragma once

class GoalScript;

/**
* @class LoadState
*/
class LoadState
    : public utl::StateBase<GoalScript>
{
public:
    /**
     * @fn void Enter(GoalScript* _pOwner)
     * @brief ステートに入ってきた時の処理
     * @param _pOwner : 所有者のポインタ
     */
    void Enter(GoalScript* _pOwner) override;
    /**
     * @fn void Update(GoalScript* _pOwner)
     * @brief ステートの更新処理
     * @param _pOwner : 所有者のポインタ
     */
    void Update(GoalScript* _pOwner) override;
    /**
     * @fn void Exit(GoalScript* _pOwner)
     * @brief ステートから出る時の処理
     * @param _pOwner : 所有者のポインタ
     */
    void Exit(GoalScript* _pOwner) override;
    /**
     * @fn void ImGui(GoalScript* _pOwner)
     * @brief ImGui表示
     * @param _pOwner : 所有者のポインタ
     */
    void ImGui(GoalScript* _pOwner) override;

private:
    std::string m_sceneName;

};
