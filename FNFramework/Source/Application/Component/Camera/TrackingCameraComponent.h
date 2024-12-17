#pragma once

#include "../BaseComponent.h"

/**
* @class TrackingCameraComponent
* @brief カメラ基底クラス
* @details
* todo : 今後追従機能を外し、行列更新 + RTのみを保持したSimpleCameraComponentを作成 -> TrackingCameraComponentを継承して作成
*/
class TrackingCameraComponent
    : public BaseComponent
{
public:
    struct TrackingMouseData
    {
        //	カメラ回転モード
        bool IsCamRot = true;

        //	回転度
        Math::Vector3 DegAng = {};
        POINT PrevMousePos = {};
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    TrackingCameraComponent(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~TrackingCameraComponent() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // ローカルの回転行列の設定
    void SetLocalCameraPos(const Math::Vector3& _localCameraPos) { m_localCameraPos = _localCameraPos; }
    const Math::Vector3& GetLocalCameraPos() const { return m_localCameraPos; }

    // ターゲットオブジェクトの設定
    void SetTargetObj(const std::weak_ptr<GameObject>& targetObj) { m_targetObj = targetObj; }

    // カメラの設定 / 取得
    void SetCamera(const std::shared_ptr<Camera>& camera) { m_spCamera = camera; }
    const std::shared_ptr<Camera>& GetCamera() const { return m_spCamera; }

    // カメラの名前の設定 / 取得
    void SetCameraName(const std::string& name) const { m_spCamera->SetCameraName(name); }
    const std::string& GetCameraName() const { return m_spCamera->GetCameraName(); }

    // プロジェクション行列の設定 / 取得
    void InitProjMatInfo(const CameraProjMatInfo::PerspectiveInfo& perspInfo, float nearClip = 0.01f,
                         float farClip = 1000.0f) const
    {
        m_spCamera->InitProjMatInfo(perspInfo, nearClip, farClip);
    }

    void InitProjMatInfo(const CameraProjMatInfo::OrthographicInfo& orthoInfo, float nearClip = 0.01f,
                         float farClip = 1000.0f) const
    {
        m_spCamera->InitProjMatInfo(orthoInfo, nearClip, farClip);
    }

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

    /* @fn void OnUpdateWorldTransform() @brief ワールド行列更新 */
    void PostUpdate() override;

    /* @fn Release() @brief 終了 */
    void Release() override;

    // シリアライズ / デシリアライズ
    void Serialize(Json& _json) const override;
    void Deserialize(const Json& _json) override;

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @fn ImGuiUpdate() @brief 更新 */
    void ImGuiUpdate() override;
    void ImGuiPerspectiveInfo();
    void ImGuiOrthographicInfo();

    //------------------
    //    データ更新
    //------------------

    void FollowsTheTarget();

    //------------------
    //    データ計算
    //------------------
    /**
     * @fn void FollowsTheTarget(Math::Vector3& _degAng)
     * @brief マウス操作によるカメラの回転角度の変更
     *
     * @param _degAng : カメラの回転角度
     */
    void CalcScreenPosToMoveVal(Math::Vector3& _degAng);

    /**
     * @fn Math::Vector3 CalcOnCameraPosNode(const std::shared_ptr<BaseObject>& target)
     * @brief ターゲットに設定されているオブジェクトにカメラ位置の指定があった際に、その位置を取得してくる
     *
     * @param target : ターゲットオブジェクト
     * @return 取得されたカメラ位置
     *
     * todo : パラメータ(座標など)の外部ファイル化により必要なくなるかも
     */
    Math::Vector3 CalcOnCameraPosNode(const std::shared_ptr<GameObject>& target);


    //------------------
    //  各種パラメータ
    //------------------
    // ターゲットオブジェクト : 設定されている場合に追従する
    std::weak_ptr<GameObject> m_targetObj;
    std::string m_targetName = "";

    // カメラ本体
    std::shared_ptr<Camera> m_spCamera = nullptr;

    float m_sensitivity = 0.0f;

    TrackingMouseData m_trackingMouseData;

    Math::Vector3 m_localCameraPos = Math::Vector3::Zero;

    // カメラの調整用の行列データ
    Math::Matrix m_mLocalPos;
    Math::Matrix m_mRotation;

    std::string m_guiCamName;
};

namespace jsonKey::Comp
{
    namespace TrackingCameraComponent
    {
        // TrackingCameraComponent に関するキー
        constexpr std::string_view Name = "CameraName";
        constexpr std::string_view LocalCameraPos = "LocalCameraPos";
        constexpr std::string_view MouseDegAng = "MouseDegAng";
        constexpr std::string_view Sensitivity = "Sensitivity";
        constexpr std::string_view TargetObject = "TargetObject";

        // ViewMatrix に関するキー
        constexpr std::string_view ViewMat = "ViewMat";

        // Camera に関するプロジェクション行列情報のキー
        constexpr std::string_view ProjMat_NearClip = "NearClip";
        constexpr std::string_view ProjMat_FarClip = "FarClip";
        constexpr std::string_view ProjMat_Type = "ProjectionType";
        constexpr std::string_view ProjMat_ViewAngle = "ViewAngle";
        constexpr std::string_view ProjMat_Aspect = "Aspect";
        constexpr std::string_view ProjMat_Width = "Width";
        constexpr std::string_view ProjMat_Height = "Height";
    }
}
