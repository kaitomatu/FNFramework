#pragma once

class GameObject;
class TransformComponent;
class ModelComponent;


// 行われている当たり判定の種類
enum class ActiveCollisionType
{
    eNone = 1 << 0,
    eSphere = 1 << 1,
    eRay = 1 << 2,
};

class CollisionHelper
{
public:
    // 球体によるヒットデータ
    struct SphereHitData
    {
        bool			IsHit = false;    // ヒット判定
        float			MaxOverLap = 0;	    // 当たった時のめり込んだ球の最大値
        Math::Vector3	HitDir = {};	    // 当たった場所の向き
        UINT            HitType = 0;        // 当たったオブジェクトのタイプ
        std::string		HitObjectName = "";
    };

    // レイによるヒットデータ
    struct RayHitData {
        bool			IsHit = false;	// ヒット判定
        Math::Vector3	HitPosition = {};		// 当たった場所の座標
        Math::Vector3   HitNormal = {};		// 当たった場所の法線
        UINT            HitType = 0;		// 当たったオブジェクトのタイプ
        std::string		HitObjectName = ""; 	// 衝突オブジェクト名
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    CollisionHelper() {}
    void Init(
        const std::shared_ptr<GameObject>& owner,
        const std::shared_ptr<TransformComponent>& trans,
        const std::shared_ptr<ModelComponent>& model);
    ~CollisionHelper()
    {
        m_spTransformComp.reset();
    }

    //--------------------------------
    // セッター / ゲッター
    //--------------------------------
    void SetActiveCollisionType(ActiveCollisionType _eType)
    {
        m_activeColType = static_cast<UINT>(_eType);
    }
    // 特定の当たり判定だけなくしたい場合に使用
    void InactiveCollissionType(ActiveCollisionType _eType)
    {
        m_activeColType &= ~static_cast<UINT>(_eType);
    }

    //	当たられる対象のアドレスを保管する
    void AddColObj(const std::shared_ptr<GameObject>& obj);

    // ターゲットリストを取得
    const std::list<std::weak_ptr<GameObject>>& GetColTargetList() const
    {
        return m_wpColTargetList;
    }

    // ターゲット対象のオブジェクトの名前
    const std::vector<std::string>& GetColObjectNames() const
    {
        return m_colObjectNames;
    }

    //--------------------------
    // 当たり判定時のデータの作成
    //--------------------------
    void AddSphereColType(UINT colType)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eSphere);
        m_sphereColliderInfo.m_type = colType;
    }

    void AddRayColType(UINT _eType)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eRay);
        m_rayColliderInfo.m_type |= _eType;
    }

    void SetRayDirAndNormal(const Math::Vector3& _rayDir)
    {
        m_activeColType |= static_cast<UINT>(ActiveCollisionType::eRay);
        m_rayColliderInfo.m_dir = _rayDir;
        m_rayColliderInfo.m_dir.Normalize();
    }

    // あたり判定の結果を取得する
    const RayHitData& GetRayHitData()const { return m_rayHitData; }
    const SphereHitData& GetSphereHitData()const { return m_sphereHitData; }

    void Update();
    void ImGui();

    // 無効オブジェクトの削除 //
    void DeleteInvalidObjects();

    // シリアライズ / デシリアライズ //
    void Serialize(Json& json) const;
    void Deserialize(const Json& json);

private:

    /* @brief 球状物体のあたり判定を取得する用の関数 */
    void SphereCollision(const float colSize);
    /* @brief レイのあたり判定を取得する用の関数 */
    void RayCollision(const float colSize);

    void AddCollisionTargetForImGui();
    void ShowColTargetsForImGui();
    void ChangeSphereColTypeForImGui();
    void ChangeRayColTypeForImGui();

    // あたり判定データを作成するに当たって必要な情報 //
    std::weak_ptr<GameObject> m_wpOwner;

    std::shared_ptr<TransformComponent> m_spTransformComp = nullptr;
    std::shared_ptr<ModelComponent>     m_spModelComp = nullptr;

    //x--- あたり判定関連 ---x//
    //	当たられる対象アドレス保存用 - 当たられる側のオブジェクトを追加する
    std::list<std::weak_ptr<GameObject>> m_wpColTargetList;
    std::string m_addColTargetName;

    //x--- あたり判定の結果 ---x//
    // 現在行っている衝突判定の種類: 球 / レイ判定が有効かどうかが入っている
    UINT m_activeColType = 0;

    // 球体用データ
    KdCollider::SphereInfo m_sphereColliderInfo;    // あたり判定用の球情報
    SphereHitData m_sphereHitData;  // 球によるヒットデータ

    // レイ用データ
    KdCollider::RayInfo m_rayColliderInfo;      // あたり判定用のレイ情報
    RayHitData m_rayHitData;    // レイによるヒットデータ

    // シリアライズ / デシリアライズ用 //
    std::vector<std::string> m_colObjectNames;

    float m_colSize = 0.0f;
};

namespace jsonKey::Comp::ColliderHelper
{
    constexpr std::string_view Key_ColliderTargets = "ColliderTargets";

    constexpr std::string_view Key_SphereCollider = "SphereCollider";
    constexpr std::string_view Key_SphereColliderType = "Type";

    constexpr std::string_view Key_RayCollider = "RayCollider";
    constexpr std::string_view Key_RayColliderType = "Type";
    constexpr std::string_view Key_RayColliderDirection = "Direction";

    constexpr std::string_view Key_ColSize = "ColSize";
}
