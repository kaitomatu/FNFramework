#include "SeaweedRenderingScript.h"

#include "../../../Component/TransformComponent/TransformComponent.h"

void SeaweedRenderingScript::SetAnimation(
    std::shared_ptr<Animator>& spAnimator,
    std::string_view animName, bool isLoop)
{
    // モデルの読み込みがまだの場合はデータを読み込んでから処理を行う
    if (!IsLoadedModelData())
    {
        LoadModelData();
    }

    const auto& animList = m_spModelData->GetModelData()->GetAnimationList();

    for (int i = 0; i < animList.size(); ++i)
    {
        if (animList[i]->Name == animName)
        {
            m_animIdx = i;
            break;
        }
    }

    SetAnimation(spAnimator, m_spModelData->GetAnimation(animName), isLoop);
}

void SeaweedRenderingScript::SetAnimation(
    std::shared_ptr<Animator>& spAnimator,
    int animIdx, bool isLoop)
{
    // モデルの読み込みがまだの場合はデータを読み込んでから処理を行う
    if (!IsLoadedModelData())
    {
        LoadModelData();
    }

    m_animIdx = animIdx;

    SetAnimation(
        spAnimator,
        m_spModelData->GetAnimation(animIdx),
        isLoop);

}

void SeaweedRenderingScript::SetAnimation(
    std::shared_ptr<Animator>& spAnimator,
    const std::shared_ptr<AnimationData>& spAnimData,
    bool isLoop)
{
    // 実体化されていない場合はメモリを確保する
    if (!spAnimator)
    {
        spAnimator = std::make_shared<Animator>();
    }

    spAnimator->SetAnimation(spAnimData, isLoop);

}

void SeaweedRenderingScript::Awake()
{
    m_area.Center  = Math::Vector3{};
    m_area.Extents = Math::Vector3(0.5f, 0.5f, 0.5f); // サイズの半分
    m_area.Orientation = Math::Quaternion::Identity;
}

void SeaweedRenderingScript::Start()
{
    ModelComponent::Start();

    // animationはとりあえず最初のアニメーションを設定
    m_spAnimationInfo.resize(5);

    for (int i = 0; i < m_spAnimationInfo.size(); ++i)
    {
        if (!m_spAnimationInfo[i].spAnimator)
        {
            m_spAnimationInfo[i].spAnimator = std::make_shared<Animator>();
        }
        SetAnimation(m_spAnimationInfo[i].spAnimator, 0, true);

        double maxFrame = static_cast<double>(m_spAnimationInfo[i].spAnimator->GetMaxFrame());

        float progress =
            static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(0.0, maxFrame));

        m_spAnimationInfo[i].spAnimator->SetProgressTime(progress);
        m_spAnimationInfo[i].DummyModelData = std::make_shared<ModelWork>(*m_spModelData);
    }
}

void SeaweedRenderingScript::Update()
{
    if (!OwnerValid()) { return; }

    if (m_spAnimationInfo.empty()) { return; }

    const std::shared_ptr<TransformComponent>& spTransform = m_wpOwnerObj.lock()->GetTransformComponent();

    m_area.Center = spTransform->GetWorldPos();
    m_area.Orientation = spTransform->GetQuaternion();

    //--------------------------------
    // アニメーションの更新
    //--------------------------------
    // アニメーションが設定されている場合はアニメーションを進める
    for (int i = 0; i < m_spAnimationInfo.size(); ++i)
    {
        if (!m_spAnimationInfo[i].spAnimator)
        {
            continue;
        }

        m_spAnimationInfo[i].spAnimator->AdvanceTime(
            m_spAnimationInfo[i].DummyModelData->WorkNodes(),
            m_animationSpeed);

        if (m_spAnimationInfo[i].spAnimator->IsAnimationEnd())
        {
            SetAnimation(m_spAnimationInfo[i].spAnimator, 0, true);
        }
    }

    //--------------------------------
    // 海藻の座標の更新
    //--------------------------------
    // 前回のフレームと座標の数が違う場合は配置を再度計算する
    if (m_currentSeaweedIdx != m_seaweedInstanceRenderData.size())
    {
        CalcRenderingPos();
    }

    //--------------------------------
    // レンダラへのデータ送信
    //--------------------------------
    for (const auto& renderMatData : m_seaweedInstanceRenderData)
    {
        int idx = renderMatData.UseAnimatorIdx;

        if (idx < 0 || idx >= static_cast<int>(m_spAnimationInfo.size()))
        {
            continue;
        }

        const std::shared_ptr<ModelWork>& spModelData = m_spAnimationInfo[idx].DummyModelData;

        UINT renderType = m_renderType;

        CullingCheck(
            {
                renderMatData.PosAndRotZ.x,
                renderMatData.PosAndRotZ.y,
                renderMatData.PosAndRotZ.z
            },
            renderType);

        // hack : ここが何かおかしいので見直しする
        renderType = static_cast<UINT>(RenderingData::Model::RenderType::eLit);
        renderType |= static_cast<UINT>(RenderingData::Model::RenderType::eShadow);

        // 試錐台カリングで描画対象外の場合は描画しない
        if (!m_insideFrustum && m_cullingType == CullingType::eFrustum)
        {
            continue;
        }

        Math::Matrix m =
            Math::Matrix::CreateScale(spTransform->GetScale()) *
            spTransform->GetRotationMatrix() *
            //Math::Matrix::CreateRotationY(renderMatData.PosAndRotZ.w) *
            Math::Matrix::CreateTranslation({ renderMatData.PosAndRotZ.x, renderMatData.PosAndRotZ.y, renderMatData.PosAndRotZ.z });

        Renderer::Instance().AddRenderingModelData(spModelData, m, renderType, m_color, m_tilling, m_offset);
    }
}

void SeaweedRenderingScript::CullingCheck(const Math::Vector3& worldPos, UINT& renderType)
{
    m_insideFrustum = CheckFrustumCulling(worldPos);

    if (m_cullingType == CullingType::eIgnoreShadowCulling)
    {
        if (m_insideFrustum)
        {
            renderType |= static_cast<UINT>(RenderingData::Model::RenderType::eShadow);
        }
        // 視界外の場合は影のみの描画にする
        else
        {
            renderType = static_cast<UINT>(RenderingData::Model::RenderType::eShadow);
        }
    }
}

bool SeaweedRenderingScript::CheckFrustumCulling(const Math::Vector3& worldPos)
{
    // カメラのフラスタム情報を取得
    const auto& cameraData = ShaderManager::Instance().FindCameraData(RenderingData::MainCameraName);
    if (!cameraData)
    {
        return true; // カメラ情報がない場合はカリングしない
    }

    // フラスタムをワールド空間に変換
    DirectX::BoundingFrustum frustum(cameraData->GetProjMat());
    Math::Matrix viewInv = cameraData->GetViewMat().Invert();
    frustum.Transform(frustum, viewInv);

    OBB obb;
    obb.Center = worldPos;
    const Math::Vector3& extents = m_colMeshBox.AABB.GetSize() / 2.0f;
    obb.Extents = extents;
    obb.Orientation = m_wpOwnerObj.lock()->GetTransformComponent()->GetQuaternion();

    // ワールド座標がフラスタム内か判定
    if (!frustum.Intersects(obb.ToDirectXOBB()))
    {
        return false;
    }

    return true;
}

void SeaweedRenderingScript::CalcRenderingPos()
{
    m_seaweedInstanceRenderData.clear();
    // 海藻インスタンスの数を更新
    m_seaweedInstanceRenderData.resize(m_currentSeaweedIdx);

    // 親の回転情報も適応しておく
    for (auto& renderMatData : m_seaweedInstanceRenderData)
    {
        int roopCounter = 0;

        // 1フェーズ目: 設定されたエリア内の [x, z] 平面上にランダム配置
        Math::Vector3 randomPos = Math::Vector3::Zero;
        do
        {
            // エリア内でランダムなポイントを生成
            randomPos.x = utl::RandomHelper::Instance().GetRandomDouble(-m_area.Extents.x, m_area.Extents.x);
            randomPos.z = utl::RandomHelper::Instance().GetRandomDouble(-m_area.Extents.z, m_area.Extents.z);

            // x,z 軸の座標の更新
            // OBB のローカル空間からワールド空間に変換
            randomPos = Math::Vector3::Transform(randomPos, m_area.Orientation) + m_area.Center;

            // 100回以上ループしたら強制終了 //
            if (roopCounter >= 100)
            {
                break;
            }
            ++roopCounter;
        } while (!m_area.Contains(randomPos));

        // 2フェーズ目: y 軸の座標を設定
        switch (m_placementPosition)
        {
            // 上下方向に少しずらす
        case PlacementPosition::eDefault:
        {
            double randomY = utl::RandomHelper::Instance().GetRandomDouble(-2.0, 2.0);
            randomPos.y = static_cast<float>(randomY);
        }
        break;
        // 上面に配置
        case PlacementPosition::eTop:
        {
            randomPos.y += m_area.Extents.y;
        }
        break;
        // 底面に配置
        case PlacementPosition::eBottom:
        {
            randomPos.y -= m_area.Extents.y;
        }
        break;
        }

        // 回転情報の追加
        double randomW = utl::RandomHelper::Instance().GetRandomDouble(-180.0, 180.0);
        float rot = static_cast<float>(randomW);

        renderMatData.PosAndRotZ = Math::Vector4{
            // x, y, z は座標情報 //
            randomPos.x,
            randomPos.y,
            randomPos.z,
            rot
        };

        renderMatData.UseAnimatorIdx = utl::RandomHelper::Instance().GetRandomInt(0, m_spAnimationInfo.size() - 1);
    }
}

void SeaweedRenderingScript::Serialize(Json& _json) const
{
    ModelComponent::Serialize(_json);

    _json[jsonKey::Comp::SeaweedRenderingScript::OBBExtents.data()] = {
        m_area.Extents.x,
        m_area.Extents.y,
        m_area.Extents.z
    };
    _json[jsonKey::Comp::SeaweedRenderingScript::PlacementPosition.data()] = m_placementPosition;

    _json[jsonKey::Comp::SeaweedRenderingScript::CurrentSeaweedIdx.data()] = m_currentSeaweedIdx;

    // アニメーション速度を保存
    _json[jsonKey::Comp::SeaweedRenderingScript::AnimationSpeed.data()] = m_animationSpeed;
}

void SeaweedRenderingScript::Deserialize(const Json& _json)
{
    ModelComponent::Deserialize(_json);

    // アニメーション速度を復元
    m_animationSpeed = _json.at(jsonKey::Comp::SeaweedRenderingScript::AnimationSpeed.data()).get<float>();

    auto eventSize = _json.value(jsonKey::Comp::SeaweedRenderingScript::OBBExtents.data(), Json::array({ 0.0f, 0.0f, 0.0f }));
    m_area.Extents = Math::Vector3(eventSize[0], eventSize[1], eventSize[2]);

    m_placementPosition = _json.value(jsonKey::Comp::SeaweedRenderingScript::PlacementPosition.data(), PlacementPosition::eDefault);

    m_currentSeaweedIdx = _json.value(jsonKey::Comp::SeaweedRenderingScript::CurrentSeaweedIdx.data(), 0);
}

void SeaweedRenderingScript::Release()
{
    ModelComponent::Release();

    for(int i = 0; i < m_spAnimationInfo.size(); ++i)
    {
        m_spAnimationInfo[i].spAnimator.reset();
        m_spAnimationInfo[i].spAnimator = nullptr;
        m_spAnimationInfo[i].DummyModelData.reset();
        m_spAnimationInfo[i].DummyModelData = nullptr;
    }
    m_spAnimationInfo.clear();
    m_seaweedInstanceRenderData.clear();
}

void SeaweedRenderingScript::ImGuiUpdate()
{
    ImGui::Separator();

    ImGui::Text(U8_TEXT("海藻の数"));
    ImGui::DragInt("##InstanceCount", &m_currentSeaweedIdx, 1, 0, 100);

    ImGui::Text(U8_TEXT("海藻の配置するオフセット位置"));
    bool isChangePlacementPos = false;
    isChangePlacementPos = ImGui::RadioButton("Default",
        &m_placementPosition,
        PlacementPosition::eDefault);
    isChangePlacementPos = ImGui::RadioButton("Top",
        &m_placementPosition,
        PlacementPosition::eTop);
    isChangePlacementPos = ImGui::RadioButton("Bottom",
        &m_placementPosition,
        PlacementPosition::eBottom);

    if (isChangePlacementPos)
    {
        CalcRenderingPos();
    }

    if (ImGui::Button(U8_TEXT("座標の再計算")))
    {
        CalcRenderingPos();
    }

    if (ImGui::TreeNode(U8_TEXT("インスタンスの座標")))
    {
        for (int i = 0; i < m_seaweedInstanceRenderData.size(); ++i)
        {
            ImGui::PushID(&m_seaweedInstanceRenderData[i]);
            ImGui::Text(U8_TEXT("使ってるアニメータの種類: %d"), m_seaweedInstanceRenderData[i].UseAnimatorIdx);
            ImGui::DragFloat4("##InstancePosX", &m_seaweedInstanceRenderData[i].PosAndRotZ.x);
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    ImGui::Separator();

    // イベント範囲のサイズ
    ImGui::Text(U8_TEXT("イベント範囲のサイズ"));
    if (ImGui::DragFloat3("##Area Size", &m_area.Extents.x, 0.1f))
    {
        CalcRenderingPos();
    }

    ImGui::Separator();

    ModelComponent::ImGuiUpdate();

    // アニメーションデータがない場合は何もしない
    if (m_spAnimationInfo.empty() || !m_spModelData) { return; }

    ImGui::Text("--------- AnimationData ----------");

    for (int i = 0; i < m_spAnimationInfo.size(); ++i)
    {
        ImGui::PushID(i);
        if (ImGui::TreeNode("AnimationInfo"))
        {
            ImGui::Text(U8_TEXT("Name: %s"), m_spAnimationInfo[i].spAnimator->GetAnimationName().data());
            ImGui::Text(U8_TEXT("最大アニメーション時間: %.2f"), m_spAnimationInfo[i].spAnimator->GetMaxFrame());
            ImGui::Text(U8_TEXT("アニメーション進捗度: %.3f"), m_spAnimationInfo[i].spAnimator->GetProgressTime());
            ImGui::Text(U8_TEXT("アニメーション進捗度(0～1): %.3f"), m_spAnimationInfo[i].spAnimator->GetNormalizeTime());

            // アニメーションの再生速度
            bool isLoop = m_spAnimationInfo[i].spAnimator->IsLoop();
            if (ImGui::Checkbox("IsLoop", &isLoop))
            {
                m_spAnimationInfo[i].spAnimator->SetLoop(isLoop);
            }
            // アニメーション進行度
            if (ImGui::Button("RestartAnimation"))
            {
                m_spAnimationInfo[i].spAnimator->ResetAnimation();
            }

            float progressTime = m_spAnimationInfo[i].spAnimator->GetProgressTime();
            if (ImGui::DragFloat("Progress", &progressTime, 0.1f, 0.0f, m_spAnimationInfo[i].spAnimator->GetMaxFrame()))
            {
                m_spAnimationInfo[i].spAnimator->SetProgressTime(progressTime);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }


    ImGuiChangeAnimData();

    ImGui::DragFloat("AnimationSpeed", &m_animationSpeed, 0.1f, 0.0f, 10.0f);

}

void SeaweedRenderingScript::ImGuiChangeAnimData()
{
    // モデルに含まれるアニメーションデータを取得
    const auto& animations = m_spModelData->GetModelData()->GetAnimationList();
    if (animations.empty())
    {
        ImGui::Text(U8_TEXT("アニメーションデータが存在しません"));
        return;
    }

    ImGui::DragInt("AnimationIndex", &m_animIdx, 1, 0, static_cast<int>(animations.size()) - 1);

    if (ImGui::Button("ChangeAnimation"))
    {
        if (m_animIdx < 0 || m_animIdx >= static_cast<int>(animations.size()))
        {
            FNENG_ASSERT_LOG("アニメーションインデックスが不正です", false)
                return;
        }

        for (int i = 0; i < m_spAnimationInfo.size(); ++i)
        {
            SetAnimation(m_spAnimationInfo[i].spAnimator, m_animIdx, true);
        }
    }
}
