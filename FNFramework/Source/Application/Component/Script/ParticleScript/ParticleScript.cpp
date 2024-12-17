#include "ParticleScript.h"

#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Component/Renderer/ModelComponent/ModelComponent.h"

void ParticleScript::Awake()
{
    // ステートマシンの初期化
    m_particleStates.SetUp(this);
}

void ParticleScript::Start()
{
    // 子オブジェクトに対するパーティクルデータを初期化
    for (const auto& childWeak : m_wpOwnerObj.lock()->GetChildren())
    {
        InitializeParticle(childWeak);
    }
}

void ParticleScript::Update()
{
    if (!OwnerValid()) { return; }

    // 子オブジェクトの数をチェックしてパーティクルデータを更新
    auto children = m_wpOwnerObj.lock()->GetChildren();

    // パーティクルデータマップを更新
    if (m_particleDataMap.size() != children.size())
    {
        // 子オブジェクトが増えた場合、新たにパーティクルデータを追加
        for (const auto& childWeak : children)
        {
            if (m_particleDataMap.find(childWeak.lock()->GetName()) == m_particleDataMap.end())
            {
                InitializeParticle(childWeak);
            }
        }

        // 子オブジェクトが減った場合、パーティクルデータを削除
        for (auto it = m_particleDataMap.begin(); it != m_particleDataMap.end(); )
        {
            auto spChildObj = m_wpOwnerObj.lock()->GetScene()->FindObject(it->first);

            if (!spChildObj)
            {
                it = m_particleDataMap.erase(it);
            }
            else
            {
                // 子オブジェクトが存在しない場合、パーティクルデータを削除
                if (std::find_if(children.begin(), children.end(),
                    [&](const std::weak_ptr<GameObject>& child) {
                        return child.lock()->GetName() == it->first;
                    }) == children.end())
                {
                    it = m_particleDataMap.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }


    // パーティクルデータを更新
    float deltaTime = SceneManager::Instance().FrameDeltaTime();
    UpdateParticleData(deltaTime);
}

void ParticleScript::Release()
{
}

void ParticleScript::Serialize(Json& _json) const
{
    // m_spawnPointのシリアライズ
    Json spawnPointJson;
    spawnPointJson["Min"] = { m_spawnPoint.GetMin().x, m_spawnPoint.GetMin().y, m_spawnPoint.GetMin().z };
    spawnPointJson["Max"] = { m_spawnPoint.GetMax().x, m_spawnPoint.GetMax().y, m_spawnPoint.GetMax().z };
    _json[jsonKey::Comp::ParticleScript::SpawnPoint.data()] = spawnPointJson;

    // ParticleFlowのシリアライズ
    _json[jsonKey::Comp::ParticleScript::ParticleFlow.data()] = static_cast<int>(m_flow);

    // デフォルトの速度と加速度のシリアライズ
    _json[jsonKey::Comp::ParticleScript::DefaultVelocity.data()] = { m_defaultVelocity.x, m_defaultVelocity.y, m_defaultVelocity.z };
    _json[jsonKey::Comp::ParticleScript::DefaultAcceleration.data()] = { m_defaultAcceleration.x, m_defaultAcceleration.y, m_defaultAcceleration.z };

    // パーティクルデータのシリアライズ
    Json particleDataListJson = Json::array();
    for (const auto& [childName, data] : m_particleDataMap)
    {
        Json particleJson;
        particleJson["Position"] = { data.position.x, data.position.y, data.position.z };
        particleJson["Scale"] = { data.scale.x, data.scale.y, data.scale.z };
        particleJson["Color"] = { data.color.x, data.color.y, data.color.z, data.color.w };
        particleJson["Velocity"] = { data.velocity.x, data.velocity.y, data.velocity.z };
        particleJson["Acceleration"] = { data.acceleration.x, data.acceleration.y, data.acceleration.z };
        particleJson["LifeTime"] = data.lifeTime;
        particleJson["ChildName"] = childName;

        particleDataListJson.push_back(particleJson);
    }
    _json[jsonKey::Comp::ParticleScript::ParticleDataList.data()] = particleDataListJson;
}

void ParticleScript::Deserialize(const Json& _json)
{
    // m_spawnPointのデシリアライズ
    auto spawnPointJson = _json.at(jsonKey::Comp::ParticleScript::SpawnPoint.data());
    auto minArray = spawnPointJson.at("Min");
    auto maxArray = spawnPointJson.at("Max");

    m_spawnPoint.SetMin({ minArray[0].get<float>(), minArray[1].get<float>(), minArray[2].get<float>() });
    m_spawnPoint.SetMax({ maxArray[0].get<float>(), maxArray[1].get<float>(), maxArray[2].get<float>() });

    // ParticleFlowのデシリアライズ
    m_flow = static_cast<ParticleFlow>(_json.at(jsonKey::Comp::ParticleScript::ParticleFlow.data()).get<int>());

    // デフォルトの速度と加速度のデシリアライズ
    auto velocityArray = _json.at(jsonKey::Comp::ParticleScript::DefaultVelocity.data());
    m_defaultVelocity = Math::Vector3{ velocityArray[0].get<float>(), velocityArray[1].get<float>(), velocityArray[2].get<float>() };

    auto accelerationArray = _json.at(jsonKey::Comp::ParticleScript::DefaultAcceleration.data());
    m_defaultAcceleration = Math::Vector3{ accelerationArray[0].get<float>(), accelerationArray[1].get<float>(), accelerationArray[2].get<float>() };

    // パーティクルデータのデシリアライズ
    auto it = _json.find(jsonKey::Comp::ParticleScript::ParticleDataList.data());
    if (it == _json.end())
    {
        return;
    }

    auto particleDataListJson = _json.at(jsonKey::Comp::ParticleScript::ParticleDataList.data());
    for (const auto& particleJson : particleDataListJson)
    {
        ParticleData data;
        auto positionArray = particleJson.at("Position");
        data.position = Math::Vector3{ positionArray[0].get<float>(), positionArray[1].get<float>(), positionArray[2].get<float>() };

        auto scaleArray = particleJson.at("Scale");
        data.scale = Math::Vector3{ scaleArray[0].get<float>(), scaleArray[1].get<float>(), scaleArray[2].get<float>() };

        auto colorArray = particleJson.at("Color");
        data.color = Math::Vector4{ colorArray[0].get<float>(), colorArray[1].get<float>(), colorArray[2].get<float>(), colorArray[3].get<float>() };

        auto velocityArray = particleJson.at("Velocity");
        data.velocity = Math::Vector3{ velocityArray[0].get<float>(), velocityArray[1].get<float>(), velocityArray[2].get<float>() };

        auto accelerationArray = particleJson.at("Acceleration");
        data.acceleration = Math::Vector3{ accelerationArray[0].get<float>(), accelerationArray[1].get<float>(), accelerationArray[2].get<float>() };

        data.lifeTime = particleJson.at("LifeTime").get<float>();
        std::string childName = particleJson.at("ChildName").get<std::string>();

        m_particleDataMap[childName] = data;
    }
}

void ParticleScript::UpdateParticleData(float deltaTime)
{
    for (auto& [childName, data] : m_particleDataMap)
    {
        // 速度と加速度による位置の更新
        data.velocity += data.acceleration * deltaTime;
        data.position += data.velocity * deltaTime;

        // パーティクルがm_spawnPointを超えた場合、反対側から再生成
        bool respawn = false;
        if (!m_spawnPoint.Contains(data.position))
        {
            respawn = true;
        }

        if (respawn)
        {
            RespawnParticle(data);
        }

        // TransformComponentを更新
        if (!data.wpTransformComp.expired())
        {
            data.wpTransformComp.lock()->SetPosition(data.position);
            data.wpTransformComp.lock()->SetScale(data.scale);
        }

        // ModelComponentの色を更新
        if (!data.wpModelComp.expired())
        {
            data.wpModelComp.lock()->SetColor(data.color);
        }

        // ライフタイムの更新（今回は無限ループなので不要）
        // data.lifeTime -= deltaTime;
        // if (data.lifeTime <= 0.0f)
        // {
        //     RespawnParticle(data);
        // }
    }
}

void ParticleScript::InitializeParticle(std::weak_ptr<GameObject> childWeak)
{
    const auto& Pos = m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldPos();

    auto spChild = childWeak.lock();

    if (spChild) { return; }

    ParticleData data;
    // 初期位置をランダムに設定
    data.position = Pos;

    // 初期スケールを設定
    data.scale = Math::Vector3{ 1.0f, 1.0f, 1.0f };

    // 初期色を設定
    data.color = Math::Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };

    // デフォルトの速度と加速度を設定
    data.velocity = m_defaultVelocity;
    data.acceleration = m_defaultAcceleration;

    // ライフタイムを設定
    data.lifeTime = FLT_MAX;

    data.wpTransformComp = spChild->GetTransformComponent();

    auto spModelComp = spChild->GetComponent<ModelComponent>();

    if(!spModelComp)
    {
        spModelComp = spChild->GetComponent<AnimationComponent>();
    }

    data.wpModelComp = spModelComp;

    m_particleDataMap[spChild->GetName().data()] = data;
}

void ParticleScript::RespawnParticle(ParticleData& data)
{
    // 反対側から再生成
    switch (m_flow)
    {
    case ParticleFlow::eUp:
        data.position.y = m_spawnPoint.GetMin().y;
        break;
    case ParticleFlow::eDown:
        data.position.y = m_spawnPoint.GetMax().y;
        break;
    case ParticleFlow::eRight:
        data.position.x = m_spawnPoint.GetMin().x;
        break;
    case ParticleFlow::eLeft:
        data.position.x = m_spawnPoint.GetMax().x;
        break;
    default:
        break;
    }

    // 速度と加速度をリセットまたは再設定（必要に応じてランダム化）
    data.velocity = m_defaultVelocity;
    data.acceleration = m_defaultAcceleration;

    // ライフタイムをリセット
    data.lifeTime = FLT_MAX;
}

void ParticleScript::ImGuiUpdate()
{
    if (ImGui::TreeNode("ParticleScript Settings"))
    {
        // m_spawnPointの設定
        ImGui::Text("Spawn Point");
        Math::Vector3 min = m_spawnPoint.GetMin();
        Math::Vector3 max = m_spawnPoint.GetMax();
        if (ImGui::InputFloat3("Min", &min.x))
        {
            m_spawnPoint.SetMin(min);
        }
        if (ImGui::InputFloat3("Max", &max.x))
        {
            m_spawnPoint.SetMax(max);
        }

        // ParticleFlowの設定
        int flow = static_cast<int>(m_flow);
        const char* flowOptions[] = { "Up", "Down", "Right", "Left" };
        if (ImGui::Combo("Particle Flow", &flow, flowOptions, IM_ARRAYSIZE(flowOptions)))
        {
            m_flow = static_cast<ParticleFlow>(flow);
        }

        // デフォルトの速度と加速度の設定
        ImGui::InputFloat3("Default Velocity", &m_defaultVelocity.x);
        ImGui::InputFloat3("Default Acceleration", &m_defaultAcceleration.x);

        ImGui::TreePop();
    }

    // 各パーティクルデータの表示・編集
    if (ImGui::TreeNode("Particle Data"))
    {
        int index = 0;
        for (auto& [childName, data] : m_particleDataMap)
        {
            std::string nodeName = "Particle " + std::to_string(index++);
            if (ImGui::TreeNode(nodeName.c_str()))
            {
                ImGui::Text("Child Name: %s", childName);
                ImGui::InputFloat3("Position", &data.position.x);
                ImGui::InputFloat3("Scale", &data.scale.x);
                ImGui::ColorEdit4("Color", &data.color.x);
                ImGui::InputFloat3("Velocity", &data.velocity.x);
                ImGui::InputFloat3("Acceleration", &data.acceleration.x);
                ImGui::InputFloat("LifeTime", &data.lifeTime);

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}
