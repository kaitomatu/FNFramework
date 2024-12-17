#include "Scene.h"

#include "Application/Application.h"

void Scene::PreUpdate()
{
    // シーンのライト情報をクリア
    ShaderManager::Instance().GetAmbientManager()->ClearLight();

    //----------------------
    // オブジェクトの削除
    //----------------------
    // ループ中に削除するとイテレータが無効になるため、効率は多少悪いが一時的に保存しておく方式を採用
    std::list<std::shared_ptr<GameObject>> objectsToDelete;

    // 削除対象のオブジェクトを検索
    for (const auto& obj : m_spObjectList)
    {
        // オブジェクトが死亡予定かどうかを確認
        if (obj->GetState() != GameObject::State::eDead) { continue; }

        // 親が設定されている場合、親のリストから削除
        if (auto parent = obj->GetParent())
        {
            parent->RemoveParentChildRelation(obj); // 死亡予定のオブジェクトのみ親のリストから削除
        }

        // 削除予定のオブジェクトを一時リストに追加
        objectsToDelete.push_back(obj);
    }

    // リストから削除する
    for (const auto& obj : objectsToDelete)
    {
        m_spObjectList.remove(obj); // 死亡予定のオブジェクトをリストから削除
    }
}

void Scene::Init()
{
}

void Scene::Release()
{
    // Start のフラグを下げておく
    for (auto&& obj : m_spObjectList)
    {
        for (auto&& comp : obj->GetComponents())
        {
            comp->DownStartFlg();
        }
    }

    // デバッグビルドの場合シーンを保存するか確認
#ifdef _DEBUG
    //if(MessageBox(Application::Instance().GetWindowHandle(), L"シーンを保存しますか？", L"確認", MB_YESNO) == IDYES)
    //{
    //    SceneManager::Instance().SaveScene(shared_from_this());
    //}
#endif // _DEBUG
}

void Scene::Serialize(Json& json) const
{
    // シーンのライト情報をシリアライズ
    ShaderManager::Instance().GetAmbientManager()->Serialize(json);

    Json& objectsJson = json[jsonKey::Key_Objects.data()];
    objectsJson = Json::array();  // JSON配列として初期化

    // オブジェクトを順番に配列に追加
    for (auto&& obj : m_spObjectList)
    {
        Json objJson;
        obj->Serialize(objJson);  // 各オブジェクトをシリアライズ
        objectsJson.push_back(objJson);  // 配列に追加
    }
}

void Scene::Deserialize(const Json& json)
{
    // シーンのライト情報をシリアライズ
    ShaderManager::Instance().GetAmbientManager()->Deserialize(json);

    const Json& objectsJson = json.at(jsonKey::Key_Objects.data());

    // 配列形式でシリアライズされているため、配列として読み込む
    for (const auto& objJson : objectsJson)
    {
        // オブジェクト名はJSON内のデータから取得する
        std::string objName = objJson.at(jsonKey::Object::Key_Name.data()).get<std::string>();

        // 新しいオブジェクトを作成し、名前を設定
        const std::shared_ptr<GameObject>& obj = AddObject(GameObject::State::eActive, objName);

        // オブジェクトのデシリアライズ
        obj->Deserialize(objJson);
    }

    // Scene::Deserialize で、オブジェクトを全て復元した後に親子関係を設定
    for (auto&& obj : m_spObjectList)
    {
        const std::string& parentName = obj->GetParentName();
        if (!parentName.empty())
        {
            auto parent = FindObject(parentName);

            if (parent)
            {
                parent->AddChild(obj);  // 親子関係を復元
            }
        }
    }
}

void Scene::Update()
{
    //----------------------
    // オブジェクトの更新
    //----------------------
    m_isUpdatingObject = true;

    for (auto&& obj : m_spObjectList)
    {
        obj->Start();
    }

    static int i = 0;

    // オブジェクトごとの更新
    for (auto&& obj : m_spObjectList)
    {
        obj->Update();
    }

    if (m_spImGuiUpdate)
    {
        m_spImGuiUpdate->Update();
    }

    m_isUpdatingObject = false;

    // 保留中のオブジェクトがあれば、実際のオブジェクトリストに追加する
    if (!m_spPendingObjectList.empty())
    {
        for (auto&& pending : m_spPendingObjectList)
        {
            const std::string& objName = GenerateUniqueName(m_spObjectList, pending->GetName());
            pending->SetName(objName);
            m_spObjectList.emplace_back(pending);
        }
        m_spPendingObjectList.clear();
    }
}

std::shared_ptr<GameObject> Scene::AddObject(GameObject::State eState, std::string_view name)
{
    // オブジェクトを生成
    auto obj = std::make_shared<GameObject>(shared_from_this(), eState);

    // 初期化をして置く
    obj->Init();

    // オブジェクトが更新中なら待機リストの方に追加する
    if (m_isUpdatingObject)
    {
        const std::string& objName = GenerateUniqueName(m_spObjectList, name);
        obj->SetName(objName);
        m_spPendingObjectList.emplace_back(obj);
    }
    else
    {
        const std::string& objName = GenerateUniqueName(m_spObjectList, name);
        obj->SetName(objName);
        m_spObjectList.emplace_back(obj);
    }

    return obj;
}

void Scene::AddObjectImGui()
{
    // ImGuiでオブジェクトを追加する
    if (ImGui::Button("AddObject"))
    {
        // オブジェクトの名前が設定されてなかったら、デフォルトの名前を設定する
        if (m_generateObjectName.empty())
        {
            m_generateObjectName = "Object";
        }

        AddObject(GameObject::State::eActive, m_generateObjectName);

        m_generateObjectName = "";
    }
    ImGui::SameLine();
    utl::ImGuiHelper::InputTextWithString("###ObjectName", m_generateObjectName);
}

std::shared_ptr<GameObject> Scene::FindObject(std::string_view name)
{
    // オブジェクトの名前に基づいて検索をする
    auto it = std::find_if(m_spObjectList.begin(), m_spObjectList.end(),
        [&](const std::shared_ptr<GameObject>& obj)
        {
            return obj->GetName() == name;
        });

    // 見つからなかった場合はnullptrを返す
    if (it == m_spObjectList.end())
    {
        FNENG_ASSERT_LOG("オブジェクトが見つかりませんでした", false)
            return nullptr;
    }

    return *it;
}

std::string Scene::GenerateUniqueName(
    const std::list<std::shared_ptr<GameObject>>& container,
    std::string_view baseName)
{
    std::string name = baseName.data(); // 基本となる名前
    int num = 0;

    // todo | hack : 線形探索によるチェックなので、コンテナが大きくなると遅くなるため注意
    // 名前が重複していたら番号をずらす
    for (const auto& obj : container)
    {
        if (obj->GetName() != name) { continue; }

        name = baseName.data() + std::to_string(num);
        num++;
    }

    return name;
}

bool Scene::ChangeObjectName(
    std::list<std::shared_ptr<GameObject>>& container,
    std::string_view oldName,
    std::string_view newName,
    bool isRename)
{
    // 名前を基にオブジェクトを検索
    auto it = std::find_if(container.begin(), container.end(),
        [&](const std::shared_ptr<GameObject>& obj)
        {
            return obj->GetName() == oldName; // BaseObjectのGetNameを使用
        });

    // 未登録の場合は失敗
    if (it == container.end())
    {
        FNENG_ASSERT_LOG("コンテナ内に登録されていません", false)
            return false;
    }

    std::string uniqueNewName;

    if (isRename)
    {
        // 名前が重複していたら番号をずらす
        uniqueNewName = GenerateUniqueName(container, newName);
    }
    else
    {
        // 新しい名前が既に存在するか確認
        auto newIt = std::find_if(container.begin(), container.end(),
            [&](const std::shared_ptr<GameObject>& obj)
            {
                return obj->GetName() == newName;
            });

        if (newIt != container.end())
        {
            FNENG_ASSERT_LOG("新しい名前が既に存在します\n追加したい場合は引数 : isRenameをtrueにしてください", false)
                return false;
        }

        uniqueNewName = newName.data(); // 重複していなければそのまま新しい名前を使用
    }

    // オブジェクトの名前を新しい名前に変更
    (*it)->SetName(uniqueNewName);

    return true;
}
