#pragma once

#include "../../BaseComponent.h"

/**
* @class CollisionComponent
* @brief
* @details
*
*/
class CollisionComponent
    : public BaseComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] _owner - オーナーオブジェクトのポインタ
    * @param[in] _name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズするかどうか
    */
    CollisionComponent(const std::shared_ptr<GameObject>& _owner, const std::string& _name, bool _enableSerialize)
        : BaseComponent(_owner, _name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~CollisionComponent() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // 当たり判定の形状を取得
    const std::unique_ptr<KdCollider>& GetCollider() { return m_upCollider; }

    // 当たるオブジェクトの追加
    void AddHitObj(const std::shared_ptr<GameObject>& _spObj) const { m_upCollisionHelper->AddColObj(_spObj); }

    // 当たり判定のタイプを設定: KdCollider::Type を設定する
    void SetColliderType(UINT _type) { m_colliderType = _type; }
    void AddColliderType(UINT _type) { m_colliderType |= _type; }
    void RemoveColliderType(UINT _type) { m_colliderType &= ~_type; }

    // あたり判定する形状のタイプを設定: KdCollider::Type を設定する
    void AddCollisionSphereType(UINT _type) const { m_upCollisionHelper->AddSphereColType(_type); }
    void AddCollisionRayType(UINT _type) const { m_upCollisionHelper->AddRayColType(_type); }
    void SetRayDirNorm(const Math::Vector3& _dir) const { m_upCollisionHelper->SetRayDirAndNormal(_dir); }

    // あたり判定データの取得
    const CollisionHelper::SphereHitData& GetSphereHitData() const { return m_upCollisionHelper->GetSphereHitData(); }
    const CollisionHelper::RayHitData& GetRayHitData() const { return m_upCollisionHelper->GetRayHitData(); }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @fn void Awake()
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;

    /**
    * @fn void Start()
    * @brief Awakeを経て初期化された後、1度だけ呼びだされる
    * @details
    *	Awakeの後に呼び出される
    *	他のコンポーネントとの依存関係にある初期化処理や
    *	Awakeの段階ではできない初期化を行う
    */
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void PreUpdate() override;
    void Update() override;
    void PostUpdate() override;

    /* @fn Release() @brief 終了 */
    void Release() override;

    /* @brief 保存 / 読みこみ */
    void Serialize(Json& _json) const override;

    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    //	当たり判定形状の登録
    void RegisterCollider();
    // 当たった判定の検出ヘルパーを登録
    void RegisterCollisionHelper();

    // 当たり判定のタイプ設定 //
    template <typename Enum>
    void SetColliderType(Enum _colType)
    {
        bool onCheck = false;

        if (utl::ImGuiHelper::UpdateBitFlagWithCheckbox(utl::str::EnumToString(_colType).data(), _colType,
                                                        m_colliderType, onCheck))
        {
            // チェックボックスが変更されたら、当たり判定のタイプを追加 / 削除
            if(onCheck)
            {
                m_upCollider->AddTypeAll(m_colliderType & _colType);
            }
            else
            {
                m_upCollider->RemoveTypeAll(m_colliderType & _colType);
            }
        }
    }

    // あたり判定ヘルパークラス : 当たり判定対象のオブジェクトとの当たり判定を行う
    std::unique_ptr<CollisionHelper> m_upCollisionHelper = nullptr;

    // 当たる側から呼び出される //
    std::unique_ptr<KdCollider> m_upCollider = nullptr;
    UINT m_colliderType = 0; // 呼び出されたときの当たり判定のタイプ
};


// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace CollisionComponent
    {
        constexpr std::string_view ColliderType = "ColliderType";
    }
}
