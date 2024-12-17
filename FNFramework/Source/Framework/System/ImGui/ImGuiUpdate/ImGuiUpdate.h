#pragma once

// 今後はサービスロケーター的な運用したい

/**
* @class ImGuiUpdate : friend - SceneManager
*
* @brief ImGuiの更新
*
* @details
*	ImGuiの更新を行うクラス
*   SceneManagerにべた書きだと可読性が悪いため、分離した
*
* @attention
*	Use - SceneManager
*/
class ImGuiUpdate
{
public:

    //----------------------------------
    // コンストラクタ / デストラクタ
    //----------------------------------
    ImGuiUpdate()
    {
    }

    ~ImGuiUpdate()
    {
    }

    //----------------------------------
    // その他処理
    //----------------------------------
    /* @brief 初期化 */
    void Init();
    /* @brief 更新処理 */
    void Update();

private:
    //--------------------------------
    // ApplicationTab
    //--------------------------------
    /* @brief アプリケーションの情報確認用GUIウィンドウ */
    void ApplicationInfoGUI();

    void FPSControllerGUI();
    void WindowGUI();
    void AmbientControllerGUI();

    /* @brief SceneManagerUI用UI @return UI情報 */
    void SceneManagerGUI();

    // CurrentSceneGUI
    void CurrentSceneGUI();

    // シーンの追加 //
    int m_selectItemCurrent = 0;
    std::string m_generateSceneName;

    bool m_isUpdate = false;
};
