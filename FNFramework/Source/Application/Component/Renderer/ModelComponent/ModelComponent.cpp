#include "ModelComponent.h"
#include "../../TransformComponent/TransformComponent.h"

void ModelComponent::Awake()
{
    m_previewRenderType = m_renderType;
}

void ModelComponent::Start()
{
    // モデルの読み込みがまだの場合はデータを読み込んでから処理を行う
    if (!IsLoadedModelData())
    {
        LoadModelData();
    }
}

// 描画データをRendererに送るだけなのでPostUpdateで行う
void ModelComponent::Update()
{
    // モデルデータがない場合は何もしない
    if (!OwnerValid() || !m_spModelData) { return; }

    UINT renderType;

    // 陰影計算なしのモデルの場合はカリングを行わない
    renderType = CullingCheck();

    // 試錐台カリングで描画対象外の場合は描画しない
    if (!m_insideFrustum && m_cullingType == CullingType::eFrustum) { return; }

    const Math::Matrix& mWorld = m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldMatrix();

    // モデルデータの更新
    Renderer::Instance().AddRenderingModelData(m_spModelData, mWorld, renderType, m_color, m_tilling, m_offset);
}

void ModelComponent::UpdateWorldTransform()
{
    if (!OwnerValid()) { return; }

    // m_modelAABBのワールド変換
    TransformAABB(m_colMeshBox.SrcAABB, m_colMeshBox.AABB);

    // m_drawAABBのワールド変換
    TransformAABB(m_drawMeshBox.SrcAABB, m_drawMeshBox.AABB);
}

void ModelComponent::LoadModelData()
{
    // この時点でモデル名がない場合はデフォルトのモデルを読み込む
    if (m_modelName.empty())
    {
        m_modelName = RenderingData::Model::DefaultModelName;
    }

    if (!m_spModelData)
    {
        m_spModelData = std::make_shared<ModelWork>();
    }

    m_spModelData->SetModelData(m_modelName);

    UpdateModelAABB();

    // 初期段階のボックスを作成
    UpdateWorldTransform();
}

void ModelComponent::Serialize(Json& _json) const
{
    // デフォルトモデルの場合は保存しない
    if (RenderingData::Model::DefaultModelName != m_modelName)
    {
        _json[jsonKey::Comp::ModelComponent::ModelName.data()] = m_modelName;
    }

    _json[jsonKey::Comp::ModelComponent::Color.data()] = { m_color.x, m_color.y, m_color.z, m_color.w };

    _json[jsonKey::Comp::ModelComponent::CullingType.data()] = m_cullingType;

    // RenderTypeのビットフラグを配列として保存
    Json renderTypes = Json::array();
    for (UINT i = 0; i < sizeof(UINT) * 8; ++i)
    {
        if (m_renderType & (1 << i))
        {
            renderTypes.push_back(1 << i); // フラグが立っているビットを保存
        }
    }
    _json[jsonKey::Comp::ModelComponent::RenderType.data()] = renderTypes;

    // タイリング / オフセットの保存
    _json[jsonKey::Comp::ModelComponent::Tilling.data()] = { m_tilling.x, m_tilling.y };
    _json[jsonKey::Comp::ModelComponent::Offset.data()] = { m_offset.x, m_offset.y };
}

void ModelComponent::Deserialize(const Json& _json)
{
    // モデル名の復元 (デフォルト値: 空文字列)
    m_modelName = _json.value(jsonKey::Comp::ModelComponent::ModelName.data(), std::string{});

    // カリングタイプの復元
    m_cullingType = _json.value(jsonKey::Comp::ModelComponent::CullingType.data(), CullingType::eFrustum);

    // タイリング / オフセットの復元
    auto tillingArray = _json.value(jsonKey::Comp::ModelComponent::Tilling.data(), Json::array({ 1.0f, 1.0f }));
    if (tillingArray.size() == 2)
    {
        m_tilling = Math::Vector2{ tillingArray[0].get<float>(), tillingArray[1].get<float>() };
    }

    auto offsetArray = _json.value(jsonKey::Comp::ModelComponent::Offset.data(), Json::array({ 0.0f, 0.0f }));
    if (offsetArray.size() == 2)
    {
        m_offset = Math::Vector2{ offsetArray[0].get<float>(), offsetArray[1].get<float>() };
    }

    // 色の復元 (デフォルト値: 白色)
    auto colorArray = _json.value(jsonKey::Comp::ModelComponent::Color.data(), Json::array({ 1.0f, 1.0f, 1.0f, 1.0f }));
    if (colorArray.size() == 4)
    {
        m_color = Math::Vector4{
            colorArray[0].get<float>(),
            colorArray[1].get<float>(),
            colorArray[2].get<float>(),
            colorArray[3].get<float>()
        };
    }

    // RenderTypeのビットフラグを配列から復元
    m_renderType = 0; // 一旦リセット
    auto renderTypes = _json.value(jsonKey::Comp::ModelComponent::RenderType.data(), Json::array());
    for (const auto& type : renderTypes)
    {
        AddRenderType(static_cast<RenderingData::Model::RenderType>(type.get<UINT>()));
    }

    // モデルデータのロード
    LoadModelData();
}

void ModelComponent::Release()
{
    if(m_spModelData)
    {
        m_spModelData.reset();
        m_spModelData = nullptr;
    }
}

void ModelComponent::ImGuiUpdate()
{
    Math::Vector3 size = m_colMeshBox.AABB.GetSize();

    //--------------------------------
    // モデルデータ関連
    //--------------------------------
    ImGui::Text(U8_TEXT("メインテクスチャのタイリング"));
    ImGui::DragFloat2("##Tilling", &m_tilling.x, .01f, 0.0f, 100.0f);
    ImGui::Text(U8_TEXT("メインテクスチャのオフセット"));
    ImGui::DragFloat2("##Offset", &m_offset.x);

    ImGui::Text("ModelName : %s", m_modelName.c_str());

    std::string modelName = m_modelName;
    if (utl::ImGuiHelper::SelectModelPath(std::string(m_modelName + "###ModelComp").c_str(), modelName))
    {
        if (!modelName.empty())
        {
            m_modelName = modelName;
            // モデル名をセットしてモデルをロード
            LoadModelData();
        }
    }

    ImGui::Separator();

    // nodeデータを表示
    ImGuiShowNodeData();

    ImGui::Separator();
    //--------------------------------
    // 描画タイプ関連
    //--------------------------------
    ChangeRenderType();

    ImGui::Separator();

    //--------------------------------
    // カリング関係
    //--------------------------------
    ImGui::Text(U8_TEXT("カリングされているかどうか: %s"), m_insideFrustum ? "true" : "false");
    utl::ImGuiHelper::RadioButtonWithLabel(U8_TEXT("カリングしない"), m_cullingType, CullingType::eNotCulling);
    utl::ImGuiHelper::RadioButtonWithLabel(U8_TEXT("試錐代カリング"), m_cullingType, CullingType::eFrustum);
    utl::ImGuiHelper::RadioButtonWithLabel(U8_TEXT("影だけ除外する"), m_cullingType, CullingType::eIgnoreShadowCulling);

    ImGui::Separator();

    //--------------------------------
    // 頂点色変更
    //--------------------------------
    if (ImGui::ColorEdit4("col", &m_color.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float))
    {
        // 色が変更された場合の処理
    }
}


void ModelComponent::ImGuiShowNodeData()
{
    // そもそもモデルが存在していない場合は何もしない
    const std::shared_ptr<ModelWork>& spModelData = m_spModelData;

    if (!spModelData) { return; }

    //	ツリーの設定---------------------------------
    int flags =
        //ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow;

    bool open = false;

    ImGui::PushID(this);

    open =
        ImGui::TreeNodeEx(this, flags, "NodeData");

    ImGui::PopID();

    if (!open) { return; }

    const auto& origNode = spModelData->GetModelData()->GetNodes();
    const auto& modelNode = m_spModelData->GetNodes();
    for (UINT i = 0; i < origNode.size(); ++i)
    {
        ImGui::Text("---------------------------");
        ImGui::Text("NodeName : %s", origNode[i].NodeName.c_str());
        ImGui::Text("ParentIdx : %d", origNode[i].ParentIdx);
        ImGui::Text("ChildrenCount : %d", origNode[i].Children.size());

        const Math::Vector3& pos = {
            modelNode[i].mLocalTransform._41, modelNode[i].mLocalTransform._42, modelNode[i].mLocalTransform._43
        };
        ImGui::Text("Position { x : %.3f,  y : %.3f, z : %.3f }", pos.x, pos.y, pos.z);
    }

    ImGui::TreePop();
}

void ModelComponent::ChangeRenderType()
{
    // 描画タイプの名前を初回のみ初期化
    std::vector<std::string> renderTypeNames;
    std::vector<const char*> renderTypeCStrs;
    if (renderTypeNames.empty())
    {
        constexpr int enumCount = RenderingData::Model::ImGui::RenderTypeCount;
        renderTypeNames.reserve(enumCount);
        renderTypeCStrs.reserve(enumCount);

        for (int i = 0; i < enumCount; ++i)
        {
            renderTypeNames.push_back(utl::str::EnumToString(static_cast<RenderingData::Model::RenderType>(1 << i)));
            renderTypeCStrs.push_back(renderTypeNames.back().c_str());
        }
    }

    // 現在の描画タイプを取得
    int currentItem = static_cast<int>(log2(static_cast<int>(m_previewRenderType)));

    // コンボボックスに現在選択できるRenderTypeを表示
    std::string label = "NowRenderType" + utl::str::EnumToString(
        static_cast<RenderingData::Model::RenderType>(1 << currentItem));

    if (ImGui::Combo("RenderType", &currentItem, renderTypeCStrs.data(), static_cast<int>(renderTypeCStrs.size())))
    {
        m_previewRenderType = static_cast<UINT>(1 << currentItem);
    }

    // 選択したRenderTypeを追加するのか削除するのかを決める
    ImGui::RadioButton("add", &m_guiAddRenderType, eAdd);
    ImGui::SameLine();
    ImGui::RadioButton("del", &m_guiAddRenderType, eDel);
    ImGui::SameLine();
    ImGui::RadioButton("set", &m_guiAddRenderType, eSet);
    ImGui::SameLine();

    if (ImGui::Button("RenderTypeCommit"))
    {
        if (m_guiAddRenderType == eAdd)
        {
            // 描画タイプを変更
            m_renderType |= 1 << currentItem;
        }
        else if (m_guiAddRenderType == eDel)
        {
            // 描画タイプを変更
            m_renderType &= ~(1 << currentItem);
        }
        else if (m_guiAddRenderType == eSet)
        {
            // 描画タイプを変更
            m_renderType = 1 << currentItem;
        }
        m_guiAddRenderType = eNone;
    }
}

void ModelComponent::ComputeAABBFromNodes(const std::vector<int>& nodeIndices, AABB<Math::Vector3>& outAABB)
{
    if (!m_spModelData) { return; }

    const std::shared_ptr<ModelData>& data = m_spModelData->GetModelData();
    if (!data) { return; }

    const std::vector<ModelData::Node>& nodes = data->GetNodes();

    if (nodeIndices.empty()) { return; }

    // AABBをリセット
    outAABB.Reset();

    // 指定されたノードインデックスのメッシュのAABBを結合
    for (const auto& idx : nodeIndices)
    {
        const auto& node = nodes[idx];
        const auto& mesh = node.spMesh;

        if (!mesh) { continue; }

        const DirectX::BoundingBox& box = mesh->GetBoundingBox();

        // ボックスの最小・最大値を計算
        const Math::Vector3 boxMin(
            box.Center.x - box.Extents.x,
            box.Center.y - box.Extents.y,
            box.Center.z - box.Extents.z
        );

        const Math::Vector3 boxMax(
            box.Center.x + box.Extents.x,
            box.Center.y + box.Extents.y,
            box.Center.z + box.Extents.z
        );

        outAABB.UpdateMinMax(boxMin);
        outAABB.UpdateMinMax(boxMax);
    }
}

void ModelComponent::TransformAABB(const AABB<Math::Vector3>& srcAABB, AABB<Math::Vector3>& _dstAABB)
{

    const std::shared_ptr<TransformComponent>& spTrans = m_wpOwnerObj.lock()->GetTransformComponent();

    if (!PreClassValid(spTrans)) { return; }

    const Math::Vector3& Scale = spTrans->GetScale();
    const Math::Vector3& Pos = spTrans->GetWorldPos();
    const Math::Matrix& mRot = spTrans->GetRotationMatrix();

    _dstAABB = srcAABB;
    // オブジェクト空間のボックスで初期化
    // サイズ
    _dstAABB.WorkMax() *= Scale;
    _dstAABB.WorkMin() *= Scale;

    // 回転
    _dstAABB.Rotate(mRot);

    // 平行移動
    _dstAABB.WorkMin() += Pos;
    _dstAABB.WorkMax() += Pos;
}

void ModelComponent::UpdateModelAABB()
{
    if (!m_spModelData) { return; }

    const std::shared_ptr<ModelData>& data = m_spModelData->GetModelData();
    if (!data) { return; }

    // コリジョンメッシュに基づくAABBの計算
    const std::vector<int>& collisionMeshNodeIndices = data->GetCollisionMeshNodeIdxLists();
    ComputeAABBFromNodes(collisionMeshNodeIndices, m_colMeshBox.AABB);
    m_colMeshBox.SrcAABB = m_colMeshBox.AABB;

    // 描画メッシュに基づくAABBの計算
    const std::vector<int>& drawMeshNodeIndices = data->GetDrawMeshNodeIdxList();
    ComputeAABBFromNodes(drawMeshNodeIndices, m_drawMeshBox.AABB);
    m_drawMeshBox.SrcAABB = m_drawMeshBox.AABB;
}

UINT ModelComponent::CullingCheck()
{
    // デフォルトの場合は普通に返す
    UINT renderType = m_renderType;

    m_insideFrustum = CheckFrustumCulling();

    if (m_cullingType == CullingType::eIgnoreShadowCulling)
    {
        // 視界外の場合は影のみの描画にする
        if (m_insideFrustum)
        {
            renderType |= static_cast<UINT>(RenderingData::Model::RenderType::eShadow);
        }
        else
        {
            renderType = static_cast<UINT>(RenderingData::Model::RenderType::eShadow);
        }
    }

    return renderType;
}

bool ModelComponent::CheckFrustumCulling()
{
    // カリングに必要なカメラ情報の取得
    const auto& cameraData = ShaderManager::Instance().FindCameraData(RenderingData::MainCameraName);

    if (!cameraData)
    {
        return true; // カメラ情報がない場合はカリングしない
    }

    // フラスタム情報の作成
    DirectX::BoundingFrustum frustum(cameraData->GetProjMat());

    // フラスタムをビュー行列の逆行列で変換（ワールド空間へ）
    Math::Matrix viewInv = cameraData->GetViewMat().Invert();
    frustum.Transform(frustum, viewInv);

    // フラスタムとAABBの交差判定
    if (!frustum.Intersects(m_drawMeshBox.AABB.ToDirectXBoundingBox()))
    {
        return false;  // 交差していなければ描画をスキップ
    }

    return true;
}
