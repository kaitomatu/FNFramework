#include "StageScript.h"

#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"
#include "Application/Component/Script/ChildController/ChildController.h"

void StageScript::Start()
{
    if (m_stageTimer.IsRunning())
    {
        m_stageTimer.Reset();
    }
    m_stageTimer.Start();

    // ステージオブジェクトの再設置
    for (auto&& stageObj : m_registerStageObjectPos)
    {
        const std::shared_ptr<GameObject>& spObject = SceneManager::Instance().GetNowScene()->FindObject(stageObj.first);
        if (!spObject) { continue; }

        stageObj.second.Object = spObject;
        spObject->GetTransformComponent()->SetPosition(stageObj.second.Pos);
    }

    if (m_maxCollectiveCount == 0)
    {
        FNENG_ASSERT_LOG("ステージに子クラゲが配置されていません", false);
    }
    if (m_maxCollectiveCount > 9)
    {
        FNENG_ASSERT_LOG("ステージに配置できる子クラゲの数が多すぎます", false);
    }

    // Startオブジェクトが存在した場合は Playerオブジェクトの座標を設定
    if (const std::shared_ptr<GameObject>& spStartObj = SceneManager::Instance().GetNowScene()->FindObject("Start"))
    {
        const std::shared_ptr<GameObject>& spPlayer = SceneManager::Instance().GetNowScene()->FindObject("Player");

        const Math::Vector3& startPos = spStartObj->GetTransformComponent()->GetWorldPos();

        spPlayer->GetTransformComponent()->SetPosition(startPos);
    }

    // シーンのリザルトUIを非表示にしておく
    const auto& spResultUIObj = SceneManager::Instance().GetNowScene()->FindObject("ResultUI");

    if (spResultUIObj)
    {
        // リザルトUIオブジェクトとその子孫に状態を伝播
        spResultUIObj->SetStateContagion(GameObject::State::ePaused);
    }

    // リザルトUIの背景のアルファ値を0にする
    const auto& spResultUIBackObj = SceneManager::Instance().GetNowScene()->FindObject("BackGround");

    if (spResultUIBackObj)
    {
        const auto& spSpriteComp = spResultUIBackObj->GetComponent<SpriteComponent>();

        if (spSpriteComp)
        {
            spSpriteComp->SetAlpha(0.0f);
        }
    }
}

void StageScript::Update()
{
    // デバッグ表示
    for (auto&& stageObj : m_registerStageObjectPos)
    {
        if (!stageObj.second.IsDebug) { continue; }

        // モデルとかが存在するオブジェクトの座標
        const Math::Vector3& origObjPos = stageObj.second.Object.lock()->GetTransformComponent()->GetWorldPos();
        const Math::Vector3& stageObjePos = stageObj.second.Pos;

        SceneManager::Instance().GetDebugWire()->AddDebugLine(origObjPos, stageObjePos, Color::Blue);
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(origObjPos, Color::Green);
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(stageObjePos, Color::Green);
    }


    // タイマーが 9分59秒(599秒)を超えたらタイマーを止める
    if (m_stageTimer.Elapsed<Timer::Seconds>().count() >= 600 - 1)
    {
        if (m_stageTimer.IsRunning())
        {
            m_stageTimer.Stop();
        }
    }
}

void StageScript::Release()
{
    m_stageTimer.Reset();
}

void StageScript::Serialize(Json& _json) const
{

    // 登録されたオブジェクトとその座標を保存
    Json objectListJson;
    for (const auto& stageObj : m_registerStageObjectPos)
    {
        Json objJson;
        objJson[jsonKey::Comp::StageScript::ObjectName.data()] = stageObj.second.ObjectName;
        objJson[jsonKey::Comp::StageScript::Pos.data()]["x"] = stageObj.second.Pos.x;
        objJson[jsonKey::Comp::StageScript::Pos.data()]["y"] = stageObj.second.Pos.y;
        objJson[jsonKey::Comp::StageScript::Pos.data()]["z"] = stageObj.second.Pos.z;

        objectListJson.push_back(objJson);
    }

    _json[jsonKey::Comp::StageScript::MaxCollectiveCount.data()] = m_maxCollectiveCount;

    _json[jsonKey::Comp::StageScript::StageObjectList.data()] = objectListJson;

    _json[jsonKey::Comp::StageScript::NextStageName.data()] = m_nextStageName;
}

void StageScript::Deserialize(const Json& _json)
{

    // 登録されたオブジェクトとその座標を読み込み
    const auto& objectListJson = _json.at(jsonKey::Comp::StageScript::StageObjectList.data());
    for (const auto& objJson : objectListJson)
    {
        EditData editData;
        editData.ObjectName = objJson.at(jsonKey::Comp::StageScript::ObjectName.data()).get<std::string>();
        editData.Pos.x = objJson.at(jsonKey::Comp::StageScript::Pos.data()).at("x").get<float>();
        editData.Pos.y = objJson.at(jsonKey::Comp::StageScript::Pos.data()).at("y").get<float>();
        editData.Pos.z = objJson.at(jsonKey::Comp::StageScript::Pos.data()).at("z").get<float>();

        m_registerStageObjectPos[editData.ObjectName] = editData;
    }

    auto maxCollectiveCount = _json.find(jsonKey::Comp::StageScript::MaxCollectiveCount.data());

    if (maxCollectiveCount != _json.end())
    {
        m_maxCollectiveCount = maxCollectiveCount.value();
    }

    // ステージ名は登録されていない可能性があるのでfindで検索
    auto stageName = _json.find(jsonKey::Comp::StageScript::NextStageName.data());

    if (stageName != _json.end())
    {
        m_nextStageName = stageName.value();
    }
}

void StageScript::ImGuiUpdate()
{

    //x--- 次のステージの名前の入力 ---x//
    utl::ImGuiHelper::InputTextWithString(U8_TEXT("次のステージの名前"), m_nextStageName);

    ImGui::Separator();

    ImGui::Text(U8_TEXT("現在の時間(秒) : %d"), m_stageTimer.Elapsed<Timer::Seconds>());
    ImGui::Text(U8_TEXT("クリアの目標タイム(秒):"));
    ImGui::DragInt("##ClearTargetTime", &m_clearTargetSecTime, 1, 0, 1000);

    ImGui::Separator();

    ImGui::Text(U8_TEXT("ステージの子どもの数"));
    ImGui::DragInt("##MaxCollectiveCount", &m_maxCollectiveCount, 1, 0, 9);

    ImGui::Separator();

    // 現在操作可能なオブジェクトを表示 //
    for (auto& stageObjList : m_registerStageObjectPos)
    {
        std::string id = "##" + stageObjList.first;

        ImGui::PushID(id.data());
        ImGui::Text(U8_TEXT("オブジェクト名 : %s"), stageObjList.first.c_str());
        ImGui::SameLine();
        ImGui::Checkbox(U8_TEXT("デバッグ表示"), &stageObjList.second.IsDebug);

        // デバッグモードのときにのみ座標変更可能に
        if (stageObjList.second.IsDebug)
        {
            ImGui::DragFloat3(U8_TEXT("座標 : %.3f, %.3f, %.3f"), &stageObjList.second.Pos.x, 0.1f);
        }
        else
        {
            ImGui::Text(U8_TEXT("座標 : %.3f, %.3f, %.3f"), stageObjList.second.Pos.x, stageObjList.second.Pos.y, stageObjList.second.Pos.z);
        }
        ImGui::Separator();
        ImGui::PopID();
    }

    //x--- ステージのエディット ---x//
    utl::ImGuiHelper::InputTextWithString(U8_TEXT("現在のステージの構成"), m_stageObjectName);

    RegisterStageObjectPosForImGui();
}

void StageScript::RegisterStageObjectPosForImGui()
{
    // 自分自身をターゲットに追加しようとしている場合は処理を抜ける
    if (m_stageObjectName == m_wpOwnerObj.lock()->GetName()) { return; }

    if (!ImGui::Button(U8_TEXT("当たり判定対象追加"))) { return; }

    // シーンから当たり判定対象のオブジェクトを取得
    const std::shared_ptr<GameObject>& spTarget = SceneManager::Instance().GetNowScene()->FindObject(m_stageObjectName);

    if (!spTarget) { return; }

    // すでに登録されている場合は処理を抜ける
    for (auto&& stageObjList : m_registerStageObjectPos)
    {
        if (stageObjList.first == m_stageObjectName) { return; }
    }

    m_registerStageObjectPos[m_stageObjectName].Object = spTarget;
    m_registerStageObjectPos[m_stageObjectName].ObjectName = spTarget->GetName();
    m_registerStageObjectPos[m_stageObjectName].Pos = spTarget->GetTransformComponent()->GetWorldPos();
}
