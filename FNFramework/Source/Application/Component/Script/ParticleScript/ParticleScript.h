#pragma once

#include "../../BaseComponent.h"
#include <unordered_map>
#include <memory>

// 前方宣言
class TransformComponent;
class ModelComponent;

/**
* @class ParticleScript
* @details 自身の子オブジェクトについているモデルコンポーネントそれを制御するスクリプト
*/
class ParticleScript
    : public BaseComponent
{
public:

    enum class ParticleFlow
    {
        eUp,
        eDown,
        eRight,
        eLeft
    };

    // 子オブジェクトについているデータひとつづつに用意されるデータ
    struct ParticleData
    {
        Math::Vector3 position;
        Math::Vector3 scale;
        Math::Vector4 color;
        Math::Vector3 velocity;
        Math::Vector3 acceleration;
        float lifeTime;

        std::weak_ptr<TransformComponent> wpTransformComp;
        std::weak_ptr<ModelComponent> wpModelComp;
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    */
    ParticleScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    void SetSpawnPoint(const AABB<Math::Vector3>& spawnPoint) { m_spawnPoint = spawnPoint; }
    const AABB<Math::Vector3>& GetSpawnPoint() const { return m_spawnPoint; }

    void SetParticleFlow(ParticleFlow flow) { m_flow = flow; }
    ParticleFlow GetParticleFlow() const { return m_flow; }

    void SetDefaultVelocity(const Math::Vector3& velocity) { m_defaultVelocity = velocity; }
    const Math::Vector3& GetDefaultVelocity() const { return m_defaultVelocity; }

    void SetDefaultAcceleration(const Math::Vector3& acceleration) { m_defaultAcceleration = acceleration; }
    const Math::Vector3& GetDefaultAcceleration() const { return m_defaultAcceleration; }

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
    */
    void Start() override;

    /* @fn void Update() @brief 更新 */
    void Update() override;

    /* @fn Release() @brief 終了 */
    void Release() override;

    /* @brief 保存 / 読みこみ */
    void Serialize(Json& _json) const override;

    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn void ImGuiUpdate() @brief ImGui更新 */
    void ImGuiUpdate() override;

    // パーティクルデータを更新する関数
    void UpdateParticleData(float deltaTime);

    // パーティクルを初期化する関数
    void InitializeParticle(std::weak_ptr<GameObject> child);

    // パーティクルを再生成する関数
    void RespawnParticle(ParticleData& data);

    //--------------------------------
    // メンバ変数
    //--------------------------------
    AABB<Math::Vector3> m_spawnPoint;                // 生成範囲
    ParticleFlow m_flow = ParticleFlow::eUp;         // 流れる方向

    Math::Vector3 m_defaultVelocity = { 0.0f, 1.0f, 0.0f };     // デフォルトの速度
    Math::Vector3 m_defaultAcceleration = { 0.0f, 0.0f, 0.0f }; // デフォルトの加速度

    std::unordered_map<std::string, ParticleData> m_particleDataMap;

    // ステートマシン
    utl::StateMachine<ParticleScript> m_particleStates;
};

// シリアライズするためのJsonKey
namespace jsonKey::Comp
{
    namespace ParticleScript
    {
        constexpr std::string_view SpawnPoint = "SpawnPoint";
        constexpr std::string_view ParticleFlow = "ParticleFlow";
        constexpr std::string_view DefaultVelocity = "DefaultVelocity";
        constexpr std::string_view DefaultAcceleration = "DefaultAcceleration";
        constexpr std::string_view ParticleDataList = "ParticleDataList";
    }
}
