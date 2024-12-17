#include "TransformComponent.h"

Math::Vector3 TransformComponent::GetWorldPos() const
{
    Math::Vector3 worldPos = m_position;

    // 自身が無効な場合は、ローカル座標を返す
    if(!OwnerValid())
    {
        return m_position;
    }

    // 親がいる場合は親のワールド行列を適用してワールド座標を計算
    if (auto parent = m_wpOwnerObj.lock()->GetParent())
    {
        // 親のワールド行列を取得
        const Math::Matrix& parentWorldMatrix = parent->GetTransformComponent()->GetWorldMatrix();

        // ワールド行列を使ってワールド座標を計算
        worldPos = Math::Vector3::Transform(m_position, parentWorldMatrix);
    }

    return worldPos;
}

void TransformComponent::SetPosition(const Math::Vector3& worldPos)
{
    m_isCalcMatrix = true;
    // 自身が無効な場合は、ローカル座標を設定
    if(!OwnerValid())
    {
        m_position = worldPos;
        return;
    }

    if (auto parent = m_wpOwnerObj.lock()->GetParent())
    {
        // 親のワールド行列の逆行列を使ってローカル座標に変換
        Math::Matrix invParentWorldMatrix = parent->GetTransformComponent()->GetWorldMatrix().Invert();
        m_position = Math::Vector3::Transform(worldPos, invParentWorldMatrix);  // ローカル座標に変換
    }
    else
    {
        m_position = worldPos;  // 親がいない場合はそのままワールド座標を設定
    }
}

void TransformComponent::Awake()
{
    m_position = Math::Vector3::Zero;
    m_scale = Math::Vector3{1.0f, 1.0f, 1.0f};
    m_rotate = Math::Vector3::Zero;

    m_isCalcMatrix = false;
}

void TransformComponent::Start()
{
    // 初期化段階で設定された座標に更新しておく
    Update();
    PostUpdate();
}

void TransformComponent::ImGuiUpdate()
{
    ImGui::Text("WorldPosition : %.2f, %.2f, %.2f", m_mWorld._41, m_mWorld._42, m_mWorld._43);
    if (ImGui::DragFloat3("LocalPosition", &m_position.x, 0.1f)) { m_isCalcMatrix = true; }

    if (ImGui::DragFloat3("Scale", &m_scale.x, 0.1f)) { m_isCalcMatrix = true; }
    if (ImGui::DragFloat3("Rotation", &m_rotate.x, 0.1f)) { m_isUpdateQuaternion = true; }
}

void TransformComponent::Update()
{
    // 角度を360度の範囲に収める
    m_rotate.x = std::fmod(m_rotate.x, 360.0f);
    m_rotate.y = std::fmod(m_rotate.y, 360.0f);
    m_rotate.z = std::fmod(m_rotate.z, 360.0f);

    // 角度が負の場合に360度を足して正の範囲に収める
    if (m_rotate.x < 0) { m_rotate.x += 360.0f; }
    if (m_rotate.y < 0) { m_rotate.y += 360.0f; }
    if (m_rotate.z < 0) { m_rotate.z += 360.0f; }

    if(m_isUpdateQuaternion)
    {
        m_quaternion = Math::Quaternion::CreateFromYawPitchRoll(MathHelper::ConvertToRadians(m_rotate));
        m_quaternion.Normalize();
        m_isUpdateQuaternion = false;

        // 行列の更新を行う
        m_isCalcMatrix = true;
    }

    //---------------------------
    // ローカルの行列合成
    //---------------------------
    if (m_isCalcMatrix)
    {
        m_isCalcMatrix = false;

        Math::Matrix mScale = Math::Matrix::CreateScale(m_scale);
        Math::Matrix mRot = Math::Matrix::CreateFromQuaternion(m_quaternion);
        Math::Matrix mTrans = Math::Matrix::CreateTranslation(m_position);

        m_mLocal = mScale * mRot * mTrans;
    }
}

void TransformComponent::PostUpdate()
{
    if (!OwnerValid()) { return; }

    const std::shared_ptr<GameObject>& spOwnerObj = m_wpOwnerObj.lock();

    //x----- 子オブジェクトのTransformComponentも更新 -----x//
    for (const auto& wpChild : spOwnerObj->GetChildren())
    {
        if(wpChild.expired()) { continue; }

        const auto& spChild = wpChild.lock();
        const auto& spChildTrans = spChild->GetTransformComponent();

        if (!PreClassValid(spChildTrans)) { continue; }

        spChildTrans->PostUpdate();  // 子オブジェクトのTransformを更新
    }

    //---------------------------
    // ワールドの行列合成
    //---------------------------
    if (const auto& spParent = spOwnerObj->GetParent())
    {
        const auto& spParentTrans = spParent->GetTransformComponent();

        // なんらかの状態で 親のTransformComponentが取得できない場合は、 m_mWorld = m_mLocal とする
        if (!spParentTrans)
        {
            m_mWorld = m_mLocal;
        }
        else
        {
            // 親のワールド行列と自身のローカル行列を掛け合わせる
            m_mWorld = m_mLocal * spParentTrans->GetWorldMatrix();
        }
    }
    else
    {
        // 親がいない場合、自身のローカル行列がそのままワールド行列
        m_mWorld = m_mLocal;
    }
}

void TransformComponent::Serialize(Json& _json) const
{
    _json[jsonKey::Comp::TransformComponent::Position.data()] = { m_position.x, m_position.y, m_position.z };
    _json[jsonKey::Comp::TransformComponent::Rotation.data()] = { m_rotate.x, m_rotate.y, m_rotate.z };
    _json[jsonKey::Comp::TransformComponent::Scale.data()] = { m_scale.x, m_scale.y, m_scale.z };
}

void TransformComponent::Deserialize(const Json& _json)
{
    auto pos = _json[jsonKey::Comp::TransformComponent::Position.data()];
    SetPosition(Math::Vector3(pos[0], pos[1], pos[2]));

    auto rot = _json[jsonKey::Comp::TransformComponent::Rotation.data()];
    SetRotation(Math::Vector3(rot[0], rot[1], rot[2]));

    auto scale = _json[jsonKey::Comp::TransformComponent::Scale.data()];
    SetScale(Math::Vector3(scale[0], scale[1], scale[2]));
}
