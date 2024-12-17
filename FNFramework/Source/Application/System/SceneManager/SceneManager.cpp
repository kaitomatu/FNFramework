#include "Application/Application.h"

#include "Framework/System/Device/Keyboard/InputSystem.h"

#include "Application/System/SceneManager/Transition/TransitionEffect/Fade.h"

#include "SceneManager.h"

#include "Application/Object/GameObject.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Component/Renderer/ModelComponent/ModelComponent.h"

#include "Application/Component/Script/PlayerScript/PlayerScript.h"
#include "Application/Component/Script/SkySphereScript/SkySphereScript.h"
#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"

#include "Application/Component/Collision/CollisionComponent/CollisionComponent.h"
#include "Application/Component/Collision/HitGroundComponent/HitGroundComponent.h"
#include "Application/Component/Collision/KnockBackComponent/KnockBackScript.h"
#include "Application/Component/Script/GameUIScript/GameUIScript.h"
#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"
#include "Application/Component/Camera/TrackingCameraComponent.h"
#include "Application/Component/InputMoveComponent/InputMoveComponent.h"
#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/Script/Billboard/BillboardScript.h"
#include "Application/Component/Script/CameraShakeEventScript/CameraShakeEventScript.h"
#include "Application/Component/Script/ChildController/ChildController.h"
#include "Application/Component/Script/GoalScript/GoalScript.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"
#include "Application/Component/Collision/TangledComponent/TangledComponent.h"
#include "Application/Component/LightComponent/LightAnimationScript.h"
#include "Application/Component/Script/DebugTextureScript/DebugTextureScript.h"
#include "Application/Component/Script/SeaweedManageScript/SeaweedManageScript.h"
#include "Application/Component/Script/SeaWeedWallScript/SeaWeedWallScript.h"
#include "Application/Component/Script/SeaweedRenderingScript/SeaweedRenderingScript.h"
#include "Application/Component/Script/StageScript/StageScript.h"
#include "Application/Component/Script/TitleScript/TitleScript.h"
#include "Application/Component/Script/WallEventScript/WallEventScript.h"

void SceneManager::RegisterComponent()
{
    RegisterComponent<AnimationComponent>();
    RegisterComponent<BillboardScript>();
    RegisterComponent<CameraShakeEventScript>();
    RegisterComponent<ChildController>();
    RegisterComponent<CollisionComponent>();
    RegisterComponent<DebugTextureScript>();
    RegisterComponent<GameUIScript>();
    RegisterComponent<GoalScript>();
    RegisterComponent<HitGroundComponent>();
    RegisterComponent<InputMoveComponent>();
    RegisterComponent<KnockBackScript>();
    RegisterComponent<KurageMoveScript>();
    RegisterComponent<LightComponent>();
    RegisterComponent<LightAnimationScript>();
    RegisterComponent<ModelComponent>();
    RegisterComponent<PlayerScript>();
    RegisterComponent<SeaweedManageScript>();
    RegisterComponent<SeaweedRenderingScript>();
    RegisterComponent<SeaWeedWallScript>();
    RegisterComponent<SkySphereScript>();
    RegisterComponent<SpriteComponent>();
    RegisterComponent<StageScript>();
    RegisterComponent<TangledComponent>();
    RegisterComponent<TitleScript>();
    RegisterComponent<TrackingCameraComponent>();
    RegisterComponent<WallEventScript>();
    RegisterComponent<TransformComponent>(false);
}

void SceneManager::Init()
{
    Json setting;
    if (!utl::file::LoadFromFile(setting, SettingFileName))
    {
        FNENG_ASSERT_ERROR("設定ファイルの読み込みに失敗しました")
    }

    RegisterComponent();

    // InputSystemにボタンを追加
    CreateInputButton();

    // デバッグワイヤーフレームの初期化
    m_upDebugWire = std::make_unique<DebugWire>();
    m_upDebugWire->Init();

    // Fadeの初期化
    m_spFade = std::make_shared<Fade>(/* blackOutTime */ 1.5f, /* progress = */ 0.5f);

    m_deltaTime = 0.0f;

    // todo : 今後はオーディオコンポーネントを用意して、そこで再生するようにする
    AudioDevice::Instance().Play("Assets/Audio/BGM/BGM.wav", true, 1.0f);

    ImportSceneFiles();

    // Assets/Scenes にあるシーンファイルを読み込む
    for (auto&& [sceneName, path] : m_umSceneNameToPath)
    {
        FNENG_ASSERT_LOG(path.string() + "シーンの追加", false)
            AddScene(sceneName);
    }

    m_firstLoadSceneName = setting.value(FirstLoadSceneName.data(), "Title");
    ChangeScene(m_firstLoadSceneName);
    //ChangeScene("Title");
}

void SceneManager::AddComponentToObject(std::string_view _compName, std::shared_ptr<GameObject> _spObj)
{
    auto itr = m_umCompNameToAddCompFunc.find(_compName.data());

    if (itr == m_umCompNameToAddCompFunc.end()) { return; }

    itr->second(_spObj);
}

void SceneManager::RemoveComponentToObject(std::string_view _compName, std::shared_ptr<GameObject> _spObj)
{
    auto itr = m_umCompNameToRemoveCompFunc.find(_compName.data());

    if (itr == m_umCompNameToRemoveCompFunc.end()) { return; }

    itr->second(_spObj);
}

void SceneManager::Release()
{
    // 設定ファイルの保存 //
    Json setting;
    setting[FirstLoadSceneName.data()] = m_firstLoadSceneName;
    utl::file::SaveToFile(setting, SettingFileName);
}

void SceneManager::PreUpdate()
{
    //--------------------------------
    // 画面遷移
    //--------------------------------

    // if(シーンがちがったら)
    // {
    //     // 画面遷移エフェクトがない場合は、新しい画面遷移エフェクトを追加する
    //     if (m_spFade->GetPhase() == Fade::Phase::eNone ||
    //         m_spFade->GetPhase() == Fade::Phase::eComplete)
    //     {
    //         m_spFade->Init();
    //     }
    //
    //     // フェードの処理を行ったのでフェード処理の更新を行う
    //     TransitionAndChangeScene();
    //
    //     return;
    // }

    const std::shared_ptr<Scene>& scene = GetScene(m_nowSceneName);

    if (!scene) { return; }

    scene->PreUpdate();
}

void SceneManager::Update()
{
    // デルタタイムの取得
    // Memo : ○○Updateで利用する必要が出てきた場合は、さらに上の階層のApplicationクラスに移動する
    m_deltaTime =
        Application::Instance().GetFPSController()->GetDeltaTime();

    // 定数バッファのタイムも更新 //
    ShaderManager::Instance().GetAmbientManager()->AddTime(m_deltaTime);

    // // 画面遷移中なら更新を行わない
    // if (m_spFade->GetState() == BaseTransitionEffect::State::ePlay &&
    //     m_spFade->GetPhase() == BaseTransitionEffect::Phase::eBlackOut)
    // {
    //     return;
    // }

    SceneUpdate();
}

void SceneManager::PostUpdate()
{
    //--------------------------------
    // 画面遷移
    //--------------------------------

    m_spFade->Update();
}

bool SceneManager::IsLoadedScene(std::string_view _sceneName)
{
    auto itr = m_umNameToScene.find(_sceneName.data());

    // 最終端まで到達していない = すでに存在するため早期リターン
    if (itr != m_umNameToScene.end())
    {
        return true;
    }

    return false;
}

void SceneManager::AddScene(std::string_view _sceneName)
{
    // すでに追加されたシーンの場合は何もしない
    if (IsLoadedScene(_sceneName)) { return; }

    const std::string& sceneName = _sceneName.data();

    //x--------- ここまで来たら新しいシーンを追加する ---------x//
    auto spScene = std::make_shared<Scene>(sceneName);

    m_umNameToScene[sceneName] = spScene;

    // もし最初のシーンなら有効なシーンにする
    if (m_nowSceneName.empty())
    {
        m_nowSceneName = sceneName;
    }

    // シーンのセーブファイルがあるかどうかを確認 //
    auto itr = m_umSceneNameToPath.find(sceneName);

    // 見つからない = 新しいシーン
    if (itr == m_umSceneNameToPath.end())
    {
        m_vSceneNames.push_back(spScene->GetSceneName().data());
        return;
    }

    // すでにある場合はファイルから読み込む
    Json sceneData;

    if (!utl::file::LoadFromFile(sceneData, itr->second.string())) { return; }

    m_vSceneNames.push_back(spScene->GetSceneName().data());
    spScene->Deserialize(sceneData);
}

void SceneManager::ChangeScene(std::string_view _sceneName)
{
    auto itr = m_umNameToScene.find(_sceneName.data());

    // シーンの変更前に Release() を読んでおく
    if (auto&& spNowScene = GetScene(m_nowSceneName))
    {
        spNowScene->Release();
    }

    // シーンがない場合は新しく作成
    if (itr == m_umNameToScene.end())
    {
        AddScene(_sceneName);

        m_nowSceneName = _sceneName;
    }
    else
    {
        m_nowSceneName = itr->first;
    }

    // シーンが作成されているはずなのでシーンの初期化を行う
    const std::shared_ptr<Scene>& spScene = GetScene(m_nowSceneName);

    // シーンが取得できない場合はエラーを出力
    if (!spScene)
    {
        FNENG_ASSERT_ERROR("シーンの取得ができませんでした")
            return;
    }

    ShaderManager::Instance().GetAmbientManager()->Init();
    spScene->Init();

    // AddScene でファイルは用意されているはずなので、ここで読み込む
    Json sceneData;
    std::string deserializeFilePath = m_umSceneNameToPath[spScene->GetSceneName().data()].string();

    if (!utl::file::LoadFromFile(sceneData,  deserializeFilePath)) { return; }

    spScene->ClearObjList();

    ShaderManager::Instance().GetAmbientManager()->Deserialize(sceneData);
    spScene->Deserialize(sceneData);
}

void SceneManager::RemoveScene(std::string_view _sceneName)
{
    if (m_nowSceneName == _sceneName) { return; }

    // イテレータで要素を検索
    auto it = std::find(m_vSceneNames.begin(), m_vSceneNames.end(), _sceneName);

    if (it != m_vSceneNames.end())
    {
        m_vSceneNames.erase(it);
    }

    m_umNameToScene.erase(_sceneName.data());
}

std::shared_ptr<Scene> SceneManager::GetScene(std::string_view _sceneName)
{
    auto itr = m_umNameToScene.find(_sceneName.data());

    if (itr == m_umNameToScene.end())
    {
        return nullptr;
    }

    return itr->second;
}

std::shared_ptr<Scene> SceneManager::GetNowScene()
{
    return GetScene(m_nowSceneName);
}

void SceneManager::SceneUpdate()
{
    const std::shared_ptr<Scene>& scene = GetScene(m_nowSceneName);

    if (!scene) { return; }

    scene->Update();
}

bool SceneManager::SaveScene(std::string_view _sceneName)
{
    auto itr = m_umNameToScene.find(_sceneName.data());

    if (itr == m_umNameToScene.end()) { return false; }

    SaveScene(itr->second);

    return true;
}

bool SceneManager::SaveScene(std::shared_ptr<Scene> _scene)
{
    Json sceneData;
    _scene->Serialize(sceneData);

    const std::string& sceneName = _scene->GetSceneName();
    auto itr = m_umSceneNameToPath.find(sceneName);

    std::string fileName;

    // 初めて保存されるシーンならファイルを作成
    if (itr == m_umSceneNameToPath.end())
    {
        fileName = ScenesDirectoryPath.data();
        fileName = fileName + "/" + sceneName + ".json";
        m_umSceneNameToPath[sceneName] = fileName;
    }
    else
    {
        fileName = itr->second.string();
    }

    bool sccess = utl::file::SaveToFile(sceneData, fileName);

    return sccess;
}

void SceneManager::ImportSceneFiles()
{
    for (auto&& entry : std::filesystem::directory_iterator(ScenesDirectoryPath))
    {
        // ファイルじゃなければ次のパスを検索
        if (!std::filesystem::is_regular_file(entry.status()))
        {
            continue;
        }

        if (entry.path().extension() != ".json")
        {
            continue;
        }

        // '\\' があると問題が発生するため、代入前に '/' に変換してから代入する
        m_umSceneNameToPath[entry.path().filename().stem().string()] = entry.path().generic_string();
    }
}

void SceneManager::TransitionAndChangeScene()
{
    if (!m_spFade) { return; }
    // 画面遷移状態がブラックアウト(Phase == eBlackOut)になっている場合にシーンの読み込みを行う
    if (!m_spFade->IsBlackOut()) { return; }

    //ChangeScene();
}

void SceneManager::CreateInputButton()
{
    auto AddInputButton = [](std::string_view key, std::vector<int> keyCode)
        {
            InputButton ib;
            ib.AddKeyCode(keyCode);
            InputSystem::Instance().AddButton(key, ib);
        };

    AddInputButton("ImGui", { VK_F1 });
    AddInputButton("Debug", { 'B' });
    AddInputButton("CameraRot", { 'M' });

    AddInputButton("Right", { VK_RIGHT, 'D' });
    AddInputButton("Left", { VK_LEFT, 'A' });
    AddInputButton("Forward", { VK_UP, 'W' });

    // todo : 統一する
    AddInputButton("Back", { VK_DOWN, 'S' });
    AddInputButton("Backward", { VK_DOWN, 'S' });

    AddInputButton("Reset", { 'R' });
    AddInputButton("Enter", { VK_RETURN });

    // マウス
    AddInputButton("RightClick", { VK_RBUTTON });
    AddInputButton("LeftClick", { VK_LBUTTON });
    AddInputButton("Click", { VK_LBUTTON, VK_RBUTTON });

    // ctrl / shift / alt
    AddInputButton("Ctrl", { VK_CONTROL });
    AddInputButton("Shift", { VK_SHIFT });
    AddInputButton("Alt", { VK_MENU });
    AddInputButton("Space", { VK_SPACE });
}
