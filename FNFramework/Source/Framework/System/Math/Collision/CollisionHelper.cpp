#include "CollisionHelper.h"

#include "Application/Component/Collision/CollisionComponent/CollisionComponent.h"
#include "Application/Component/Renderer/ModelComponent/ModelComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"

void CollisionHelper::Init(
    const std::shared_ptr<GameObject>& owner,
    const std::shared_ptr<TransformComponent>& trans,
    const std::shared_ptr<ModelComponent>& model)
{
    m_wpOwner = owner;
    m_spTransformComp = trans;
    m_spModelComp = model;
}

void CollisionHelper::AddColObj(const std::shared_ptr<GameObject>& obj)
{
    // すでに登録されている場合は処理を抜ける
    for(auto& wpTarget : m_wpColTargetList)
    {
        if (wpTarget.lock() == obj) { return; }
    }

    m_wpColTargetList.emplace_back(obj);
}

void CollisionHelper::Update()
{
    // すべての当たり判定を初期化しておく
    m_sphereHitData = {};
    m_rayHitData = {};

    if (!m_spModelComp)
    {
        FNENG_ASSERT_ERROR("ModelComponentが設定されていません");
        return;
    }

    const AABB<Math::Vector3>& box = m_spModelComp->GetColMeshAABB();

    // 矩形データの半分のサイズ = 生成する当たり判定のサイズとする
    const float colSize = (box.GetSize().Length() + m_colSize) / 2.0f;

    //	当たり判定の更新
    if (m_activeColType & static_cast<UINT>(ActiveCollisionType::eSphere))
    {
        SphereCollision(colSize);

        //	デバッグライン表示
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(
            m_sphereColliderInfo.m_sphere.Center,
            m_sphereHitData.IsHit ? Color::Red : Color::White,
            m_sphereColliderInfo.m_sphere.Radius
        );
    }
    if (m_activeColType & static_cast<UINT>(ActiveCollisionType::eRay))
    {
        RayCollision(colSize);

        //  デバッグライン表示
        SceneManager::Instance().GetDebugWire()->AddDebugLine(
            m_rayColliderInfo.m_pos,
            m_rayColliderInfo.m_dir,
            m_rayColliderInfo.m_range,
            m_rayHitData.IsHit ? Color::Red : Color::White);
    }
}

void CollisionHelper::ImGui()
{
    ImGui::Text(U8_TEXT("当たり判定のサイズ"));
    ImGui::DragFloat("##当たり判定のサイズ", &m_colSize, 0.01f);

    //x--- 当たり判定オブジェクトの管理 ---x//
    if (ImGui::TreeNode(U8_TEXT("当たるオブジェクト関係")))
    {
        // コリジョン対象の追加
        AddCollisionTargetForImGui();
        // コリジョン対象の表示
        ShowColTargetsForImGui();
        ImGui::TreePop();
    }

    //x--- Sphere Collision Type ---x//
    if (ImGui::TreeNode(U8_TEXT("当たり判定を行う球判定のタイプ")))
    {
        ChangeSphereColTypeForImGui();
        ImGui::TreePop();
    }

    //x--- Ray Collision Type ---x//
    if (ImGui::TreeNode(U8_TEXT("当たり判定を行うレイ判定のタイプ")))
    {
        ChangeRayColTypeForImGui();
        ImGui::TreePop();
    }
}

void CollisionHelper::AddCollisionTargetForImGui()
{
    utl::ImGuiHelper::InputTextWithString(U8_TEXT("当たり判定対象の名前"), m_addColTargetName);

    // 自分自身をターゲットに追加しようとしている場合は処理を抜ける
    if (m_addColTargetName == m_wpOwner.lock()->GetName()) { return; }

    if (!ImGui::Button(U8_TEXT("当たり判定対象追加"))) { return; }

    // シーンから当たり判定対象のオブジェクトを取得
    const std::shared_ptr<GameObject>& spTarget = m_wpOwner.lock()->GetScene()->FindObject(m_addColTargetName);

    if (!spTarget) { return; }

    // すでに登録されている場合は処理を抜ける
    for (const auto& wpTarget : m_wpColTargetList)
    {
        if (wpTarget.lock() == spTarget) { return; }
    }

    std::shared_ptr<CollisionComponent> spColComp = spTarget->GetComponent<CollisionComponent>();

    // 対象オブジェクトの当たり判定形状が登録されていない場合は登録する
    if (!spColComp)
    {
        spColComp = spTarget->AddComponent<CollisionComponent>(true);
    }

    AddColObj(spTarget);
}

void CollisionHelper::ShowColTargetsForImGui()
{
    // 当たり判定対象の名前を表示
    for (const auto& wpTarget : m_wpColTargetList)
    {
        if (wpTarget.expired()) { continue; }

        const std::shared_ptr<GameObject>& spTarget = wpTarget.lock();

        ImGui::Text(spTarget->GetName().data());
    }
}

void CollisionHelper::ChangeSphereColTypeForImGui()
{
    // 各フラグに対してチェックボックスを表示し、トグルできるようにする
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Ground##SphereInfo",KdCollider::TypeGround, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Bump##SphereInfo", KdCollider::TypeBump, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Damage##SphereInfo", KdCollider::TypeDamage, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("KnockBack##SphereInfo", KdCollider::TypeKnockBack, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Obstacle##SphereInfo", KdCollider::TypeObstacle, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Event##SphereInfo", KdCollider::TypeEvent, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Player##SphereInfo", KdCollider::TypePlayer, m_sphereColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Item##SphereInfo", KdCollider::TypeItem, m_sphereColliderInfo.m_type);

    // 何かしらのフラグが設定されていたら当たり判定を有効にする
    if (m_sphereColliderInfo.m_type != 0)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eSphere);
    }
    else
    {
        m_activeColType &= ~static_cast<UINT>(ActiveCollisionType::eSphere);
    }
}

void CollisionHelper::ChangeRayColTypeForImGui()
{
    // 各フラグに対してチェックボックスを表示し、トグルできるようにする
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Ground##RayInfo",KdCollider::TypeGround, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Bump##RayInfo", KdCollider::TypeBump, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Damage##RayInfo", KdCollider::TypeDamage, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("KnockBack##RayInfo", KdCollider::TypeKnockBack, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Obstacle##RayInfo", KdCollider::TypeObstacle, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Event##RayInfo", KdCollider::TypeEvent, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Player##RayInfo", KdCollider::TypePlayer, m_rayColliderInfo.m_type);
    utl::ImGuiHelper::UpdateBitFlagWithCheckbox("Item##RayInfo", KdCollider::TypeItem, m_rayColliderInfo.m_type);

    // 何かしらのフラグが設定されていたら当たり判定を有効にする
    if (m_rayColliderInfo.m_type != 0)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eRay);
    }
    else
    {
        m_activeColType &= ~static_cast<UINT>(ActiveCollisionType::eRay);
    }
}


void CollisionHelper::DeleteInvalidObjects()
{
    // 当たり判定オブジェクトが削除されている場合リストからも削除する
    for (auto it = m_wpColTargetList.begin(); it != m_wpColTargetList.end();)
    {
        if (it->expired()) // weak_ptrが無効かチェック
        {
            it = m_wpColTargetList.erase(it); // 無効ならリストから削除
        }
        else
        {
            ++it; // 次の要素へ
        }
    }
}

void CollisionHelper::Serialize(Json& json) const
{
    // コライダー対象オブジェクトの名前を保存
    Json colTargetNames = Json::array();
    for (const auto& wpTarget : m_wpColTargetList)
    {
        if(wpTarget.expired()) { continue; }


        colTargetNames.push_back(wpTarget.lock()->GetName());
    }

    using namespace jsonKey::Comp::ColliderHelper;

    json[Key_ColliderTargets.data()] = colTargetNames;

    // 球体コライダー情報を保存
    json[Key_SphereCollider.data()][Key_SphereColliderType.data()] = m_sphereColliderInfo.m_type;

    // レイコライダー情報を保存
    json[Key_RayCollider.data()][Key_RayColliderType.data()] = m_rayColliderInfo.m_type;
    json[Key_RayCollider.data()][Key_RayColliderDirection.data()] = { m_rayColliderInfo.m_dir.x, m_rayColliderInfo.m_dir.y, m_rayColliderInfo.m_dir.z };

    json[Key_ColSize.data()] = m_colSize;
}

void CollisionHelper::Deserialize(const Json& json)
{
    using namespace jsonKey::Comp::ColliderHelper;

    // コライダー対象オブジェクトの名前を読み込み、リストに追加
    m_colObjectNames.clear();

    const auto& colObjectsJson = json.at(Key_ColliderTargets.data());
    for (const auto& objName : colObjectsJson)
    {
        for (const auto& addedObjName : m_colObjectNames)
        {
            if (objName == addedObjName)
            {
                continue;
            }
        }

        m_colObjectNames.push_back(objName); 
    }

    //x--- 球体コライダー情報の読み込み ---x//
    const auto& sphereColliderJson = json.at(Key_SphereCollider.data());

    m_sphereColliderInfo.m_type = sphereColliderJson.at(Key_SphereColliderType.data());

    // 球のタイプが設定されている = 当たり判定をアクティブにする
    if (m_sphereColliderInfo.m_type != 0)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eSphere);
    }

    //x--- レイコライダー情報の読み込み ---x//
    const auto& rayColliderJson = json.at(Key_RayCollider.data());

    m_rayColliderInfo.m_type = rayColliderJson.at(Key_RayColliderType.data());

    // レイのタイプが設定されている = 当たり判定をアクティブにする
    if (m_rayColliderInfo.m_type != 0)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eRay);
    }

    m_rayColliderInfo.m_dir = Math::Vector3{
        rayColliderJson.at(Key_RayColliderDirection.data())[0],
        rayColliderJson.at(Key_RayColliderDirection.data())[1],
        rayColliderJson.at(Key_RayColliderDirection.data())[2]
    };

    m_colSize = json.value(Key_ColSize.data(), 0.f);
}

void CollisionHelper::SphereCollision(const float colSize)
{
    const std::shared_ptr<GameObject>& Owner = m_wpOwner.lock();

    if (!Owner)
    {
        FNENG_ASSERT_LOG("Ownerが不正です", false);
        return;
    }
    if (!m_spTransformComp)
    {
        FNENG_ASSERT_LOG("TransformComponentが設定されていません", false);
        return;
    }

    if (m_sphereColliderInfo.m_type == 0) { FNENG_ASSERT_LOG("SphereInfoのTypeが設定されていません", false); }

    //-------------------------------
    // あたり判定用の球情報生成
    //-------------------------------
    m_sphereColliderInfo.m_sphere.Center = m_spTransformComp->GetWorldPos();
    m_sphereColliderInfo.m_sphere.Radius = colSize;

    //-------------------------------
    // 当たり判定処理
    //-------------------------------
    //	球に当たったオブジェクト情報保存
    std::list<KdCollider::CollisionResult> retSphereList;

    for (const std::weak_ptr<GameObject>& wpTargetObj : m_wpColTargetList)
    {
        if (wpTargetObj.expired())
        {
            FNENG_ASSERT_LOG("Targetが不正です", false)
            continue;
        }

        const std::shared_ptr<GameObject>& spTarget = wpTargetObj.lock();

        const std::shared_ptr<CollisionComponent>& spTargetColComp = spTarget->GetComponent<CollisionComponent>();

        if (!spTargetColComp)
        {
            FNENG_ASSERT_LOG("CollisionComponentが設定されていません", false)
            continue;
        }

        // ターゲットのコライダーが有効な場合のみあたり判定を行う
        if (const std::unique_ptr<KdCollider>& collider = spTargetColComp->GetCollider())
        {
            collider->Intersects(
                m_sphereColliderInfo,
                spTarget->GetTransformComponent()->GetWorldMatrix(),
                &retSphereList);
        }
        else
        {
            FNENG_ASSERT_LOG("TargertのColliderが登録されていません", false)
            continue;
        }
    }

    //-------------------------------
    // 結果格納
    //-------------------------------
    //	球に当たったリスト情報から一番近いオブジェクトを検出する
    for (auto& ret : retSphereList)
    {
        //	一番近くで当たったものを探す
        if (m_sphereHitData.MaxOverLap < ret.m_overlapDistance)
        {
            m_sphereHitData.IsHit = true;
            m_sphereHitData.MaxOverLap = ret.m_overlapDistance;
            m_sphereHitData.HitDir = ret.m_hitDir;
            m_sphereHitData.HitType = ret.m_type;
            m_sphereHitData.HitObjectName = ret.m_collidedObjName;
        }
    }
}

void CollisionHelper::RayCollision(const float colSize)
{
    const std::shared_ptr<GameObject>& Owner = m_wpOwner.lock();

    if (!Owner)
    {
        FNENG_ASSERT_LOG("Ownerが不正です", false)
        return;
    }
    if (!m_spTransformComp)
    {
        FNENG_ASSERT_LOG("TransformComponentが設定されていません", false)
        return;
    }
    if (m_rayColliderInfo.m_type == 0)
    {
        FNENG_ASSERT_LOG("RayInfoのTypeが設定されていません", false)
        return;
    }

    //-----------------------
    // あたり判定用のレイ作成
    //-----------------------
    const float HalfCorrectionAmount = colSize;
    //	座標を設定されていなければ
    Math::Vector3 rayPos = m_spTransformComp->GetWorldPos();
    rayPos.y += -HalfCorrectionAmount; //	足元からレイが飛ぶように補正

    m_rayColliderInfo.m_pos = rayPos;

    m_rayColliderInfo.m_range = HalfCorrectionAmount / 2.0f;

    //-----------------------
    // あたり判定処理
    //-----------------------
    //	レイに当たったオブジェクト情報
    std::list<KdCollider::CollisionResult> retRayList;
    for (const std::weak_ptr<GameObject>& wpTargetObj : m_wpColTargetList)
    {
        if (wpTargetObj.expired())
        {
            FNENG_ASSERT_LOG("Targetが不正です", false)
            continue;
        }

        const std::shared_ptr<GameObject>& spTarget = wpTargetObj.lock();

        const std::shared_ptr<CollisionComponent>& spTargetColComp = spTarget->GetComponent<CollisionComponent>();

        if (!spTargetColComp)
        {
            FNENG_ASSERT_LOG("CollisionComponentが設定されていません", false)
            continue;
        }

        // ターゲットのコライダーが有効な場合のみあたり判定を行う
        if (const std::unique_ptr<KdCollider>& collider = spTargetColComp->GetCollider())
        {
            collider->Intersects(
                m_rayColliderInfo,
                spTarget->GetTransformComponent()->GetWorldMatrix(),
                &retRayList);
        }
        else
        {
            FNENG_ASSERT_LOG("TargertのColliderが登録されていません", false)
            continue;
        }
    }

    //-----------------------
    // 結果格納
    //-----------------------
    //	レイに当たったリストから一番近いオブジェクトを検出する
    float maxOverLap = 0;
    for (auto& ret : retRayList)
    {
        //	レイを遮断しオーバーした長さが
        //	一番長いものを探す
        if (maxOverLap < ret.m_overlapDistance)
        {
            m_rayHitData.IsHit = true;

            maxOverLap = ret.m_overlapDistance;
            m_rayHitData.HitPosition = ret.m_hitPos;
            m_rayHitData.HitNormal = ret.m_hitDir;
            m_rayHitData.HitType = ret.m_type;
            m_rayHitData.HitObjectName = ret.m_collidedObjName;
        }
    }
}
