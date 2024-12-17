#include "CollisionComponent.h"

#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"
#include "Application/Component/Renderer/ModelComponent/ModelComponent.h"

void CollisionComponent::Awake()
{
}

void CollisionComponent::Start()
{
    if (!OwnerValid()) { return; }

    // 当たられ判定の登録
    RegisterCollider();
    // 当たり判定のヘルパーの登録
    RegisterCollisionHelper();

    const std::shared_ptr<Scene>& spScene = m_wpOwnerObj.lock()->GetScene();

    for (const auto& targetName : m_upCollisionHelper->GetColObjectNames())
    {
        auto&& spTargetObj = spScene->FindObject(targetName);

        if (!spTargetObj) { continue; }

        m_upCollisionHelper->AddColObj(spTargetObj);
    }
}

void CollisionComponent::PreUpdate()
{
    if (!m_upCollisionHelper || !m_upCollider) { return; }

    // 無効なオブジェクトを削除
    m_upCollisionHelper->DeleteInvalidObjects();
    // 当たり判定の更新
    m_upCollisionHelper->Update();
}

void CollisionComponent::Update()
{
}

void CollisionComponent::PostUpdate()
{
}

void CollisionComponent::Release()
{
}

void CollisionComponent::Serialize(Json& json) const
{
    json[jsonKey::Comp::CollisionComponent::ColliderType.data()] = m_colliderType;

    m_upCollisionHelper->Serialize(json);
}

void CollisionComponent::Deserialize(const Json& json)
{
    m_colliderType = json.at(jsonKey::Comp::CollisionComponent::ColliderType.data());

    if(!m_upCollisionHelper)
    {
        m_upCollisionHelper = std::make_unique<CollisionHelper>();
    }

    m_upCollisionHelper->Deserialize(json);
}

void CollisionComponent::ImGuiUpdate()
{
    if (!m_upCollider) { return; }

    if(ImGui::TreeNode(U8_TEXT("x--- 呼び出されたときの判定タイプ ---x")))
    {
        SetColliderType(KdCollider::TypeGround);
        SetColliderType(KdCollider::TypeBump);
        SetColliderType(KdCollider::TypeDamage);
        SetColliderType(KdCollider::TypeKnockBack);
        SetColliderType(KdCollider::TypeObstacle);
        SetColliderType(KdCollider::TypeEvent);
        SetColliderType(KdCollider::TypePlayer);
        SetColliderType(KdCollider::TypeItem);

        ImGui::TreePop();
    }

    if(!m_upCollisionHelper)
    {
        // ヘルパーが存在しない場合は登録ボタンを表示
        if(ImGui::Button(U8_TEXT("当たり判定ヘルパーの登録")))
        {
            m_upCollisionHelper = std::make_unique<CollisionHelper>();

            const std::shared_ptr<TransformComponent>& spTransComp = m_wpOwnerObj.lock()->GetTransformComponent();

            std::shared_ptr<ModelComponent> spModelComp =  m_wpOwnerObj.lock()->GetComponent<ModelComponent>();

            if(!spModelComp)
            {
                spModelComp = m_wpOwnerObj.lock()->GetComponent<AnimationComponent>();
            }

            m_upCollisionHelper->Init(m_wpOwnerObj.lock(), spTransComp, spModelComp);
        }

        return;
    }

    ImGui::Separator();

    ImGui::Text(U8_TEXT("x--- 自分自身のコライダー ---x"));
    m_upCollisionHelper->ImGui();
}

void CollisionComponent::RegisterCollisionHelper()
{
    const std::shared_ptr<TransformComponent>& spTrans = m_wpOwnerObj.lock()->GetTransformComponent();

    std::shared_ptr<ModelComponent> spModelComp = m_wpOwnerObj.lock()->GetComponent<ModelComponent>(false);

    if(!spModelComp)
    {
        spModelComp = m_wpOwnerObj.lock()->GetComponent<AnimationComponent>();
    }

    if (!spModelComp)
    {
        FNENG_ASSERT_LOG("モデルコンポーネントが存在しません", false)
        return;
    }

    if(!m_upCollisionHelper)
    {
        m_upCollisionHelper = std::make_unique<CollisionHelper>();
    }

    m_upCollisionHelper->Init(m_wpOwnerObj.lock(), spTrans, spModelComp);
}

void CollisionComponent::RegisterCollider()
{
    // すでに登録されている場合は登録しない
    if (!m_upCollider)
    {
        m_upCollider = std::make_unique<KdCollider>();
    }

    const std::shared_ptr<GameObject>& spOwner = m_wpOwnerObj.lock();

    std::shared_ptr<ModelComponent> spModelComponent = spOwner->GetComponent<ModelComponent>(false);

    if(!spModelComponent)
    {
        spModelComponent = spOwner->GetComponent<AnimationComponent>();
    }

    if (!spModelComponent)
    {
        FNENG_ASSERT_LOG("モデルコンポーネントが存在しません", false)
        return;
    }

    // モデルの読み込みがまだの場合はモデル読み込みを先に行う
    if (!spModelComponent->IsLoadedModelData())
    {
        spModelComponent->LoadModelData();
    }

    const std::shared_ptr<ModelData>& modelData = spModelComponent->GetModelData()->GetModelData();

    if (!modelData)
    {
        FNENG_ASSERT_LOG("モデルデータが存在しません", false)
        return;
    }

    m_upCollider->RegisterCollisionShape(
        spOwner->GetName(),
        modelData,
        m_colliderType);
}
