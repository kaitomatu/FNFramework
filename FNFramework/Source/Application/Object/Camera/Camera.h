#pragma once

/**
* @brief カメラのプロジェクション行列の情報
* @details
*	透視投影か平行投影を選択することで必要な情報が切り替わる
*
*	関数の定義はCamera.cppに記述
*/
class CameraProjMatInfo
{
public:
    // Memo : 構造体として定義しておくことで、インスタンス時の初期化処理の判定処理を簡略化する
    // 透視射影行列特有のの情報
    struct PerspectiveInfo
    {
        PerspectiveInfo()
        {
        }

        PerspectiveInfo(float viewAngle)
            : ViewAngle(viewAngle)
        {
        }

        float ViewAngle = 0.1f; // 画角
        float Aspect = 0.0f; // アスペクト比
    };

    // 平行投影行列特有の情報
    struct OrthographicInfo
    {
        OrthographicInfo()
        {
        }

        OrthographicInfo(float width, float height)
            : Width(width), Height(height)
        {
        }

        float Width = 1.0f; // 横幅
        float Height = 1.0f; // 縦幅
    };

    //--------------------------------
    // セッター
    //--------------------------------
    // 平行投影か透視投影かを切り替える処理がこの関数を通して行われる
    void SetPerspectiveInfo(float viewAngle); // 透視投影
    void SetOrthographicInfo(float width, float height); // 平行投影
    void SetNearAndFar(float nearClip, float farClip);

    //---------------------------------
    // 変数
    //---------------------------------
    bool IsPerspective = true; // 透視投影かどうか
    bool IsNeedUpdate = true; // 更新が必要かどうか

    float Near = 0.1f; // NearClip
    float Far = 1.0f; // FarClip

    // IsPerspectiveによって行列合成の処理が切り替わる
    PerspectiveInfo PerspectInfo; // 透視投影
    OrthographicInfo OrthoInfo; // 平行投影
};

// ビュー行列用の情報
struct CameraViewMatInfo
{
    bool IsNeedUpdate = true;

    Math::Matrix mView;       // ビュー行列
    Math::Matrix mViewInv;    // ビュー行列の逆行列 - カメラの向きなどを計算する際に使用する

    Math::Vector3 Pos       = {0.0f, 0.0f, 0.0f };    // カメラの位置
    Math::Vector3 Up        = Math::Vector3::Up;        // カメラの上方向
    Math::Vector3 Forward   = Math::Vector3::Backward;  // カメラの前方向
    Math::Vector3 Right     = Math::Vector3::Right;     // カメラの右方向
};

/**
* @brief カメラクラス本体
* @details
*	更新などを行う
*/
class Camera
{
public:
    using PerspectiveInfo = CameraProjMatInfo::PerspectiveInfo;
    using OrthographicInfo = CameraProjMatInfo::OrthographicInfo;

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Camera()
    {
    }

    Camera(std::string_view camName)
        : m_cameraName(camName)
    {
    }

    virtual ~Camera()
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    // カメラ定数バッファ用のデータを取得
    CBufferData::Camera GetCBData();

    const std::string& GetCameraName() const { return m_cameraName; }
    void SetCameraName(const std::string& camName)
    {
        m_cameraName = camName;
    }

    // どちらかのデータが更新されていたらShaderManagerでセットを行う
    bool IsUpdate() const { return m_isUpdate; }
    void SetIsUpdate(bool isUpdate) { m_isUpdate = isUpdate; }

    //-----------
    // 座標関係
    //-----------
    const Math::Vector3& GetPos() const { return m_viewMatInfo.Pos; }

    void SetPos(const Math::Vector3& pos)
    {
        m_viewMatInfo.Pos = pos;
        m_viewMatInfo.IsNeedUpdate = true;
    }

    //-----------
    // 移動関係
    //-----------
    const Math::Vector3& GetUp() const { return m_viewMatInfo.Up; }
    void SetUp(const Math::Vector3& up);

    const Math::Vector3& GetForward() const { return m_viewMatInfo.Forward; }
    const Math::Vector3& GetRight() const { return m_viewMatInfo.Right; }

    /**
    * @brief カメラの移動処理
    * @param[in] move - 移動量
    */
    void Move(const Math::Vector3& move);
    /**
    * @brief カメラの視点の移動
    * @param[in] move - 移動量
    */
    void MovePos(const Math::Vector3& move);

    /**
    * @brief カメラの上方向に平行移動
    * @param[in] move - 移動量
    */
    void MoveUp(float move);
    /**
    * @brief カメラの前方向に平行移動
    * @param[in] move - 移動量
    */
    void MoveForward(float move);
    /**
    * @brief カメラの右方向に平行移動
    * @param[in] move - 移動量
    */
    void MoveRight(float move);

    //-----------
    // 行列関係
    //-----------

    // カメラ用の各種行列の取得
    const CameraViewMatInfo& GetViewMatInfo() const { return m_viewMatInfo; }
    CameraViewMatInfo& WorkViewMatInfo() { return m_viewMatInfo; }
    const CameraProjMatInfo& GetProjMatInfo() const { return m_projMatInfo; }
    CameraProjMatInfo& WorkProjMatInfo()  { return m_projMatInfo; }

    /* @brief プロジェクション行列を取得 */
    const Math::Matrix& GetProjMat()
    {
        if (m_projMatInfo.IsNeedUpdate) { UpdateProjMat(); }
        return m_mProj;
    }

    /* @brief ビュー行列のセット */
    void SetViewMat(const Math::Matrix& viewMat)
    {
        m_viewMatInfo.mView = viewMat;
        m_viewMatInfo.IsNeedUpdate = true;
    }

    /* @brief ビュー行列を取得 */
    const Math::Matrix& GetViewMat()
    {
        if (m_viewMatInfo.IsNeedUpdate) { UpdateViewMat(); }
        return m_viewMatInfo.mView;
    }

    /* @brief ビュー行列の逆行列を取得 */
    const Math::Matrix& GetViewMatInv()
    {
        if (m_viewMatInfo.IsNeedUpdate) { UpdateViewMat(); }
        return m_viewMatInfo.mViewInv;
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    void InitProjMatInfo(const PerspectiveInfo& perspInfo, float nearClip = 0.01f, float farClip = 1000.0f);
    void InitProjMatInfo(const OrthographicInfo& orthoInfo, float nearClip = 0.01f, float farClip = 1000.0f);

    void UpdateProjMat();
    void UpdateViewMat();

protected:

    std::string m_cameraName;   // カメラの名前

    bool m_isUpdate = true;     // ShaderManager::m_cameraListの更新が必要かどうか

    CameraProjMatInfo m_projMatInfo;    // プロジェクション行列を作るための情報
    Math::Matrix m_mProj;       // プロジェクション行列

    CameraViewMatInfo m_viewMatInfo;    // ビュー行列を作るための情報

};
