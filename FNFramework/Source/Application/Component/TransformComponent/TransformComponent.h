#pragma once

#include "../BaseComponent.h"

/**
* @brief TransformComponent
* @brief 座標 : 回転 : 拡大率の変数を持つ
* @details
*	座標、回転、拡大率を持つコンポーネント
*	このコンポーネントはあくまで変数を持っているのみで、移動などは別コンポーネントで行う
*/
class TransformComponent
    : public BaseComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    TransformComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eTranform)
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    const Math::Matrix& GetWorldMatrix() const
    {
        return m_mWorld;
    }

    const Math::Matrix& GetLocalMatrix() const
    {
        return m_mLocal;
    }

    const Math::Quaternion& GetQuaternion() const
    {
        return m_quaternion;
    }

    Math::Matrix GetRotationMatrix() const
    {
        return Math::Matrix::CreateFromQuaternion(m_quaternion);
    }

    // 親がいる場合は親のワールド行列を適用してワールド座標を計算
    Math::Vector3 GetWorldPos() const;

    const Math::Vector3& GetLocalPos() const
    {
        return m_position;
    }

    const Math::Vector3& GetScale() const
    {
        return m_scale;
    }

    const Math::Vector3& GetRotation() const
    {
        return m_rotate;
    }

    /* @brief 正面ベクトルを取得 @正面ベクトル */
    Math::Vector3 GetForward() const
    {
        return m_mWorld.Backward();
    }

    /* @brief 座標の設定 @param[in] position - 設定する座標 */
    void SetPosition(const Math::Vector3& position);

    void SetPositionX(float x)
    {
        m_isCalcMatrix = true;
        m_position.x = x;
    }

    void SetPositionY(float y)
    {
        m_isCalcMatrix = true;
        m_position.y = y;
    }

    void SetPositionZ(float z)
    {
        m_isCalcMatrix = true;
        m_position.z = z;
    }

    /* @brief 回転率の設定 @param[in] rotation - 設定する回転率 */
    void SetRotation(const Math::Vector3& rotation)
    {
        m_isCalcMatrix = true;
        m_rotate = rotation;

        m_quaternion = Math::Quaternion::CreateFromYawPitchRoll(MathHelper::ConvertToRadians(m_rotate));
        m_quaternion.Normalize();
    }

    void SetQuarternion(const Math::Quaternion& quarternion)
    {
        m_isCalcMatrix = true;
        // クォータニオンは直接変更されているのでフラグは立てない
        m_quaternion = quarternion;
        m_rotate = m_quaternion.ToEuler();
    }

    void SetRotationX(float x)
    {
        m_isUpdateQuaternion = true;
        m_rotate.x = x;
    }

    void SetRotationY(float y)
    {
        m_isUpdateQuaternion = true;
        m_rotate.y = y;
    }

    void SetRotationZ(float z)
    {
        m_isUpdateQuaternion = true;
        m_rotate.z = z;
    }

    /* @brief 拡大率の設定 @param[in] scale - 設定する拡大率 */
    void SetScale(const float& scale)
    {
        SetScale(Math::Vector3{scale});
    }

    void SetScale(const Math::Vector3& scale)
    {
        m_isCalcMatrix = true;
        m_scale = scale;
    }

    void SetScaleX(float x)
    {
        m_isCalcMatrix = true;
        m_scale.x = x;
    }

    void SetScaleY(float y)
    {
        m_isCalcMatrix = true;
        m_scale.y = y;
    }

    void SetScaleZ(float z)
    {
        m_isCalcMatrix = true;
        m_scale.z = z;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;
    void Start() override;

    /* @brief 更新 */
    void Update() override;
    void PostUpdate() override;

    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 更新 */
    void ImGuiUpdate() override;

    // 更新フラグ
    // 行列の計算をおこなうかどうか
    bool m_isCalcMatrix = false;

    // クォータニオンの更新フラグ
    bool m_isUpdateQuaternion = false;

    // 各行列を表すパラメータ
    Math::Vector3 m_position = Math::Vector3::Zero; // 座標
    Math::Vector3 m_rotate = Math::Vector3::Zero; // 回転率
    Math::Vector3 m_scale = Math::Vector3::Zero; // 拡大率

    Math::Quaternion m_quaternion = Math::Quaternion::Identity; // 回転

    Math::Matrix m_mLocal; // ローカル行列
    Math::Matrix m_mWorld; // ワールド行列
};

namespace jsonKey::Comp
{
    namespace TransformComponent
    {
        constexpr std::string_view Position = "Position";
        constexpr std::string_view Rotation = "Rotation";
        constexpr std::string_view Scale = "Scale";
    }
}
