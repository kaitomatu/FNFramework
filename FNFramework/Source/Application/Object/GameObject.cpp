#include "GameObject.h"

#include "../Component/TransformComponent/TransformComponent.h"
#include "../Component/Renderer/ModelComponent/ModelComponent.h"

void GameObject::Serialize(Json& _json) const
{
    Json componentsData = Json::array(); // コンポーネント配列として初期化

    // 各コンポーネントのデータを保存
    for (size_t i = 0; i < m_spComponents.size(); ++i)
    {
        // シリアライズを行わないオブジェクトの場合は次へ
        if (!m_spComponents[i]->EnableSerialize()) { continue; }

        Json compJson;
        compJson[jsonKey::Object::Key_ComponentsName.data()] = m_spComponents[i]->GetComponentName(); // コンポーネント名を保存
        compJson[jsonKey::Object::Key_ComponentEnable.data()] = m_spComponents[i]->IsEnable(); // コンポーネントの有効無効を保存
        compJson[jsonKey::Object::Key_ComponentsID.data()] = i; // コンポーネントのインデックスをIDとして保存
        m_spComponents[i]->Serialize(compJson); // 各コンポーネントをシリアライズ
        componentsData.push_back(compJson); // 配列として追加
    }

    // オブジェクト名とコンポーネントデータをJSONに保存
    _json[jsonKey::Object::Key_Name.data()] = m_objName;
    _json[jsonKey::Object::Key_Components.data()] = componentsData;

    // 親オブジェクトの名前をシリアライズ
    if (auto parent = m_wpParent.lock())
    {
        _json[jsonKey::Object::Key_ParentName.data()] = parent->GetName();
    }
    else
    {
        _json[jsonKey::Object::Key_ParentName.data()] = "";
    }
}

void GameObject::Deserialize(const Json& _json)
{
    // オブジェクト名を復元
    m_objName = _json.at(jsonKey::Object::Key_Name.data()).get<std::string>();

    // 親オブジェクトの名前を保存
    {
        auto it = _json.find(jsonKey::Object::Key_ParentName);
        if (it != _json.end())
        {
            m_parentName = it->get<std::string>(); // 一時的に親の名前を保存
        }
    }

    //x------ コンポーネントのデシリアライズ -----x//
    {
        auto it = _json.find(jsonKey::Object::Key_Components);
        if (it == _json.end()) { return; }

        const Json& componentsJson = it.value();

        const std::string_view transformName = typeid(TransformComponent).name();

        // 各コンポーネントを配列からデシリアライズ
        for (const auto& compJson : componentsJson)
        {
            // コンポーネント名を取得
            std::string compName = compJson.at(jsonKey::Object::Key_ComponentsName.data()).get<std::string>();

            // コンポーネントIDを取得
            int componentID = compJson.at(jsonKey::Object::Key_ComponentsID.data()).get<int>();
            bool enable = compJson.at(jsonKey::Object::Key_ComponentEnable.data()).get<bool>();

            // TransformComponentは必ず存在するためAddComponentを呼ばずにデシリアライズ
            if (compName != transformName)
            {
                // コンポーネントを取得してデシリアライズ（IDを基にして特定）
                SceneManager::Instance().AddComponentToObject(compName, shared_from_this());
            }

            if (const auto& spComp = GetComponentByID(componentID))
            {
                spComp->SetEnable(enable);
                spComp->Deserialize(compJson); // コンポーネントのデシリアライズ
            }
        }
    }
}

void GameObject::AddChild(const std::shared_ptr<GameObject>& child)
{
    if (!child) { return; }

    // 子供がすでに他の親を持っている場合、親から削除する
    if (auto oldParent = child->GetParent())
    {
        oldParent->RemoveParentChildRelation(child);
    }

    // 親オブジェクトとして自分を設定し、親子関係を確立
    child->m_wpParent = shared_from_this();
    m_wpChildren.emplace_back(child);

    // 子供のワールド座標を取得
    const Math::Vector3& childWorldPos = child->GetTransformComponent()->GetWorldPos();

    // 親のワールド行列の逆行列を取得
    const Math::Matrix& parentWorldMatrixInv = GetTransformComponent()->GetWorldMatrix().Invert();

    // 子オブジェクトのワールド座標を親のローカル座標に変換
    const Math::Vector3 localPos = Math::Vector3::Transform(childWorldPos, parentWorldMatrixInv);
    child->GetTransformComponent()->SetPosition(localPos);
}

std::list<std::weak_ptr<GameObject>>::iterator GameObject::RemoveParentChildRelation(
    const std::shared_ptr<GameObject>& child)
{
    if (auto parent = child->m_wpParent.lock())
    {
        auto it = parent->m_wpChildren.begin();
        while (it != parent->m_wpChildren.end())
        {
            if (it->lock() == child)
            {
                // 子供のローカル座標をワールド座標に変換するため、親のワールド行列を使用
                const Math::Vector3& childLocalPos = child->GetTransformComponent()->GetWorldPos();
                const Math::Matrix& parentWorldMatrix = parent->GetTransformComponent()->GetWorldMatrix();

                // ローカル座標をワールド座標に変換
                Math::Vector3 childWorldPos = Math::Vector3::Transform(childLocalPos, parentWorldMatrix);

                // 子供のワールド座標をセット
                child->GetTransformComponent()->SetPosition(childWorldPos);

                // 親子関係を解消
                child->m_wpParent.reset();
                it = parent->m_wpChildren.erase(it);

                return it; // 削除後の次の要素を返す
            }
            ++it;
        }
    }

    return m_wpChildren.end(); // 削除されなかった場合、end()を返す
}

void GameObject::Init()
{
    if (m_spTransformComponent.expired())
    {
        m_spTransformComponent = AddComponent<TransformComponent>(true);
    }
}

void GameObject::Start()
{
    for (int i = 0; i < m_spComponents.size(); ++i)
    {
        m_spComponents[i]->StartComponent();
    }
}

void GameObject::Update()
{
    //--------------------------------
    // 更新処理
    //--------------------------------

    if (m_state != State::eActive) { return; }

    if (m_spTransformComponent.expired())
    {
        FNENG_ASSERT_ERROR("Transformコンポーネントがありません");
        return;
    }

    // コンポーネントの更新
    for (auto&& comp : m_spComponents)
    {
        if (!comp->IsEnable()) { continue; }

        comp->PreUpdate();
    }

    // ワールド行列の更新
    UpdateWorldTransform();

    // コンポーネントの更新
    for (UINT i = 0; i < m_spComponents.size(); ++i)
    {
        if (!m_spComponents[i]->IsEnable()) { continue; }

        m_spComponents[i]->Update();
    }

    // ワールド行列の更新
    UpdateWorldTransform();

    // コンポーネントの更新
    for (auto&& comp : m_spComponents)
    {
        if (!comp->IsEnable()) { continue; }

        comp->PostUpdate();
    }
}

void GameObject::ImGuiUpdate()
{
    //x--- オブジェクトの名前を変更 ---x//
    if (utl::ImGuiHelper::InputTextWithString(U8_TEXT("オブジェクト名"), m_objName))
    {
        m_objName = Scene::GenerateUniqueName(m_wpScene.lock()->GetObjectList(), m_objName);
    }

    //---------------------------------------------------------
    const char** items = SceneManager::Instance().GetCompNames().data();
    // コンポーネント追加
    ImGui::Separator();

    if (ImGui::Button("AddComponent##GameObject"))
    {
        SceneManager::Instance().AddComponentToObject(SceneManager::Instance().GetCompNames()[m_selectItemCurrent],
            shared_from_this());
    }
    ImGui::SameLine();
    std::string comboName = "###";
    ImGui::Combo(comboName.c_str(), &m_selectItemCurrent, items,
        static_cast<int>(SceneManager::Instance().GetCompNames().size()));

    //x-------- 子オブジェクトの追加 --------x//
    utl::ImGuiHelper::InputTextWithString(U8_TEXT("子どもにしたいオブジェクト名"), m_imguiSerchChildName);
    if (m_imguiSerchChildName != m_objName)
    {
        if (ImGui::Button("AddChild##GameObject"))
        {
            auto spChildObj = m_wpScene.lock()->FindObject(m_imguiSerchChildName);

            if (!spChildObj)
            {
                spChildObj = m_wpScene.lock()->AddObject(State::eActive, m_imguiSerchChildName);
            }

            AddChild(spChildObj);
        }
    }

    ImGui::Separator();

    //---------------------------------------------------------

    /**
     * ToDo : 現在は Update / ImGuiUpdate で分けていることにより、
     *  for文が2回回っているが、class UpdateImGuiにコンポーネントから自身の更新処理を追加する用に
     *  変更することで、for文を1回に減らすことができるか検討する - CurrentScene版のAddTabItemを作成
     */
    ImGui::Text("ComponentSize[%d]", m_spComponents.size());

    // アクティブモード変更
    ChangeActiveMode();

    // コンポーネント用ImGuiの制御
    ComponentImGuiUpdate();

    // 子オブジェクトのImGui更新
    int flags =
        //ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow;

    // 子オブジェクトのみ表示
    for (auto it = m_wpChildren.begin(); it != m_wpChildren.end();)
    {
        if (it->expired())
        {
            it = m_wpChildren.erase(it); // 無効なポインタは削除して次の要素へ
            continue;
        }

        ImGui::Separator();
        ImGui::Separator();

        //x-------- 親子の解消 --------x//
        ImGui::PushID(&it);
        if (ImGui::Button(U8_TEXT("親子の解消")))
        {
            it = RemoveParentChildRelation(it->lock()); // イテレータを更新
            continue;
        }

        //x-------- 子オブジェクトのGUI表示 --------x//
        const std::shared_ptr<GameObject>& spChild = it->lock();

        // 有効フラグ更新
        bool isActiveState = spChild->GetState() == State::eActive;
        std::string enableCheckLabel = "##";
        enableCheckLabel += spChild->GetName();
        ImGui::Checkbox(enableCheckLabel.data(), &isActiveState);
        if (isActiveState)
        {
            spChild->SetState(State::eActive);
        }
        else
        {
            spChild->SetState(GameObject::State::ePaused);
        }
        ImGui::SameLine();

        bool bTreeOpen = ImGui::TreeNode(spChild->GetName().data());

        if (bTreeOpen)
        {
            spChild->ImGuiUpdate();
            ImGui::TreePop();
        }
        ImGui::PopID();

        ++it; // 次の要素に進む
    }
}

void GameObject::UpdateWorldTransform()
{
    if (m_spTransformComponent.expired())
    {
        FNENG_ASSERT_ERROR("TransformComponentが設定されていません");
        return;
    }

    // コンポーネントの更新
    for (auto&& comp : m_spComponents)
    {
        if (!comp->IsEnable()) { continue; }

        comp->UpdateWorldTransform();
    }
}

void GameObject::Release()
{
    for (auto& comp : m_spComponents)
    {
        comp->Release();
    }
}

void GameObject::ChangeActiveMode()
{
    //--------------------------------
    // アクティブモード変更
    //--------------------------------

    constexpr int enumCount = static_cast<int>(State::eCount);

    // 描画タイプの名前を取得
    // const char* を受け取るために、std::string -> const char* に変換する
    std::vector<std::string> stateNames;
    std::vector<const char*> stateCStrs;

    // メモリを確保しておく
    stateNames.reserve(enumCount);
    stateCStrs.reserve(enumCount);

    for (int i = 0; i < enumCount; ++i)
    {
        stateNames.push_back(utl::str::EnumToString(static_cast<State>(i)).c_str());
        stateCStrs.push_back(stateNames.back().c_str());
    }

    // 現在のオブジェクトの状態を取得
    int currentItem = static_cast<int>(m_state);

    std::string label = "NowState" + utl::str::EnumToString(m_state);
    if (ImGui::Combo(label.c_str(), &currentItem, stateCStrs.data(), enumCount))
    {
        // オブジェクトの状態を変更
        m_state = static_cast<State>(currentItem);
    }
}

void GameObject::ComponentImGuiUpdate()
{
    //	ツリーの設定---------------------------------
    int flags =
        //ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow;

    bool bTreeOpen;

    for (UINT i = 0; i < m_spComponents.size(); ++i)
    {
        ImGui::PushID(&m_spComponents[i]);

        bTreeOpen =
            ImGui::TreeNodeEx(&m_spComponents[i], flags, m_spComponents[i]->GetComponentName().c_str());

        ImGui::PopID();
        if (bTreeOpen)
        {
            bool enable = m_spComponents[i]->IsEnable();

            if (ImGui::Button(U8_TEXT("コンポーネントの削除")))
            {
                SceneManager::Instance().RemoveComponentToObject(m_spComponents[i]->GetComponentName(),
                    shared_from_this());
                // 削除したら次のコンポーネントに移動
                ImGui::TreePop();
                continue;
            }

            if (ImGui::Checkbox(U8_TEXT("更新するかどうか？"), &enable))
            {
                m_spComponents[i]->SetEnable(enable);
            }

            if (enable)
            {
                m_spComponents[i]->ImGuiUpdate();
            }
            ImGui::TreePop();

            ImGui::Separator();
            ImGui::Separator();
        }
    }
}
