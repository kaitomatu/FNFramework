#pragma once

#include "../../BaseComponent.h"

class TrackingCameraComponent;
/**
* @class CameraShakeEventScript
* @brief
* @details
*
*/
class CameraShakeEventScript
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
    CameraShakeEventScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~CameraShakeEventScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // カメラの揺れ開始 / 終了
    void ShakeStart(float _shakePower, float _vibrato, float _duration)
    {
        // すでに動作中なら何もしない
        if(m_shakeTime.IsRunning()) { return; }

        m_shakeTime.Start();

        m_shakePower = _shakePower;
        m_vibrato = _vibrato;
        m_duration = _duration;

        m_isShake = true;
        m_isShakeFastFrame = true;
    }

    // 揺らすカメラの設定
    void SetTrackingCameraComponent(const std::weak_ptr<TrackingCameraComponent>& trackingCameraComponent) { m_wpTrackingCameraComponent = trackingCameraComponent; }

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
    void Update() override;

    /* @fn void OnUpdateWorldTransform() @brief ワールド行列更新 */
    void UpdateWorldTransform() override;

    /* @fn Release() @brief 終了 */
    void Release() override;

    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;

    // 揺れフラグ
    bool m_isShake = false;

    // カメラの揺れ始め
    bool m_isShakeFastFrame = false;

    // 揺れ時間の管理用
    Timer m_shakeTime = Timer("ShakeTime");

    // 揺れの強さ : -m_shakePower ~ m_shakePower の間でランダムな値で揺れる
    float m_shakePower = 1.0f;

    // 揺れ幅 : どれくらい揺れるか
    float m_vibrato = 0.1f;

    // 揺れの期間 : 何秒間揺れるか
    float m_duration = 0.2f;

    /* 各コンポーネントアクセス用 */
    // シェイクしたい対象のカメラコンポーネント
    std::weak_ptr<TrackingCameraComponent> m_wpTrackingCameraComponent;
    std::string m_trackingCameraComponentName = "";
};

namespace jsonKey::Comp
{
        namespace CameraShakeEventScript
        {
            constexpr std::string_view TrackingCameraComponentObjName = "TrackingCameraComponentObjName";
        }
}
