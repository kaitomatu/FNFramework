#include "TrackingCameraComponent.h"

#include "Application/Application.h"
#include "Application/Component/Renderer/AnimationComponent/AnimationComponent.h"
#include "Application/Component/Renderer/ModelComponent/ModelComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Object/Camera/Camera.h"

void TrackingCameraComponent::Awake()
{
    m_spCamera = std::make_shared<Camera>();

    m_sensitivity = 3.0f;

    m_trackingMouseData.DegAng = Math::Vector3{ 25.0f, 0.0f, 0.0f };
}

void TrackingCameraComponent::Start()
{
    // ここの段階でカメラが設定されていない場合は警告
    if (!m_spCamera)
    {
        FNENG_ASSERT_ERROR(m_spCamera->GetCameraName() + " : が無効状態です")
    }


    if (!m_targetName.empty())
    {
        if (const auto& nowScene = SceneManager::Instance().GetNowScene())
        {
            for (auto&& obj : nowScene->GetObjectList())
            {
                if (obj->GetName() == m_targetName)
                {
                    SetTargetObj(obj);
                    break;
                }
            }
        }
    }
}

void TrackingCameraComponent::PostUpdate()
{
    // カメラの更新後、GPU用のバッファにカメラを登録
    m_spCamera->UpdateViewMat();
    m_spCamera->UpdateProjMat();

    // GPU用のバッファにカメラを登録
    ShaderManager::Instance().RegisterCamData(m_spCamera);

    if (!m_targetObj.expired())
    {
        FollowsTheTarget();
    }
    else
    {
        // ターゲットがない場合は、カメラのローカル座標でビュー行列を更新
        const auto& spTrans = m_wpOwnerObj.lock()->GetTransformComponent();
        if (spTrans)
        {
            //	カメラの回転行列を作成
            CalcScreenPosToMoveVal(m_trackingMouseData.DegAng);

            // 回転行列を作成
            m_mRotation =
                Math::Matrix::CreateRotationX(MathHelper::ConvertToRadians(m_trackingMouseData.DegAng.x)) *
                Math::Matrix::CreateRotationY(MathHelper::ConvertToRadians(m_trackingMouseData.DegAng.y)) *
                Math::Matrix::CreateRotationZ(MathHelper::ConvertToRadians(m_trackingMouseData.DegAng.z));

            Math::Matrix mTrans = spTrans->GetWorldMatrix() + Math::Matrix::CreateTranslation(m_localCameraPos);

            // カメラの座標を計算
            Math::Matrix mCamMat = m_mLocalPos * m_mRotation * mTrans;

            // ビュー行列をカメラにセット
            m_spCamera->SetViewMat(mCamMat.Invert());
        }
    }
}

void TrackingCameraComponent::FollowsTheTarget()
{
    const std::shared_ptr<GameObject>& target = m_targetObj.lock();

    //	カメラの座標行列を作成
    Math::Vector3 camPos = Math::Vector3::Zero;

    /**
    * ターゲットのモデルにカメラ座標が設定されている場合
    * カメラ座標を設定された座標にする
    */
    camPos = CalcOnCameraPosNode(target);

    const Math::Vector3 cameraWorldPos = camPos + m_spCamera->GetPos() + m_localCameraPos;

    m_mLocalPos = Math::Matrix::CreateTranslation(cameraWorldPos);

    //	カメラにターゲットの行列をセット(座標行列のみ)
    Math::Matrix targetMat;

    // ターゲットの座標を元に、移動行列を作成
    if (const std::shared_ptr<TransformComponent>& targetTrans = target->GetTransformComponent())
    {
        Math::Vector3 targetPos = targetTrans->GetWorldPos();
        targetMat = Math::Matrix::CreateTranslation(targetPos);
    }

    //	カメラの回転行列を作成
    CalcScreenPosToMoveVal(m_trackingMouseData.DegAng);

    // 角度を360度の範囲に収める
    m_trackingMouseData.DegAng.x = std::fmod(m_trackingMouseData.DegAng.x, 360.0f);
    m_trackingMouseData.DegAng.y = std::fmod(m_trackingMouseData.DegAng.y, 360.0f);
    m_trackingMouseData.DegAng.z = std::fmod(m_trackingMouseData.DegAng.z, 360.0f);

    // 角度が負の場合に360度を足して正の範囲に収める
    if (m_trackingMouseData.DegAng.x < 0) { m_trackingMouseData.DegAng.x += 360.0f; }
    if (m_trackingMouseData.DegAng.y < 0) { m_trackingMouseData.DegAng.y += 360.0f; }
    if (m_trackingMouseData.DegAng.z < 0) { m_trackingMouseData.DegAng.z += 360.0f; }

    // 回転行列を作成
    m_mRotation =
        Math::Matrix::CreateRotationX(MathHelper::ConvertToRadians(m_trackingMouseData.DegAng.x)) *
        Math::Matrix::CreateRotationY(MathHelper::ConvertToRadians(m_trackingMouseData.DegAng.y)) *
        Math::Matrix::CreateRotationZ(MathHelper::ConvertToRadians(m_trackingMouseData.DegAng.z));

    const auto& spTrans = m_wpOwnerObj.lock()->GetTransformComponent();
    Math::Matrix mCamMat = m_mLocalPos * m_mRotation * targetMat * spTrans->GetWorldMatrix();

    m_spCamera->SetViewMat(mCamMat.Invert());
}

void TrackingCameraComponent::Release()
{
}

void TrackingCameraComponent::Serialize(Json& json) const
{
    json[jsonKey::Comp::TrackingCameraComponent::Name.data()] = m_spCamera->GetCameraName();
    json[jsonKey::Comp::TrackingCameraComponent::LocalCameraPos.data()] = { m_localCameraPos.x, m_localCameraPos.y, m_localCameraPos.z };
    json[jsonKey::Comp::TrackingCameraComponent::MouseDegAng.data()] = { m_trackingMouseData.DegAng.x, m_trackingMouseData.DegAng.y, m_trackingMouseData.DegAng.z };
    json[jsonKey::Comp::TrackingCameraComponent::Sensitivity.data()] = m_sensitivity;

    // カメラのビュー行列情報 //
    const auto& viewMatInfo = m_spCamera->GetViewMatInfo();

    json[jsonKey::Comp::TrackingCameraComponent::ViewMat.data()]["Pos"] = {
        viewMatInfo.Pos.x,
        viewMatInfo.Pos.y,
        viewMatInfo.Pos.z
    };

    json[jsonKey::Comp::TrackingCameraComponent::ViewMat.data()]["Up"] = {
        viewMatInfo.Up.x,
        viewMatInfo.Up.y,
        viewMatInfo.Up.z
    };

    json[jsonKey::Comp::TrackingCameraComponent::ViewMat.data()]["Forward"] = {
        viewMatInfo.Forward.x,
        viewMatInfo.Forward.y,
        viewMatInfo.Forward.z
    };

    json[jsonKey::Comp::TrackingCameraComponent::ViewMat.data()]["Right"] = {
        viewMatInfo.Right.x,
        viewMatInfo.Right.y,
        viewMatInfo.Right.z
    };

    // プロジェクション行列情報 //
    json[jsonKey::Comp::TrackingCameraComponent::ProjMat_NearClip.data()] = m_spCamera->WorkProjMatInfo().Near;
    json[jsonKey::Comp::TrackingCameraComponent::ProjMat_FarClip.data()] = m_spCamera->WorkProjMatInfo().Far;

    if (m_spCamera->WorkProjMatInfo().IsPerspective)
    {
        json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Type.data()] = typeid(Camera::PerspectiveInfo).name();
        json[jsonKey::Comp::TrackingCameraComponent::ProjMat_ViewAngle.data()] = m_spCamera->WorkProjMatInfo().PerspectInfo.ViewAngle;
        json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Aspect.data()] = m_spCamera->WorkProjMatInfo().PerspectInfo.Aspect;
    }
    else
    {
        json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Type.data()] = typeid(Camera::OrthographicInfo).name();
        json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Width.data()] = m_spCamera->WorkProjMatInfo().OrthoInfo.Width;
        json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Height.data()] = m_spCamera->WorkProjMatInfo().OrthoInfo.Height;
    }

    // ターゲットオブジェクト名を保存
    if (auto target = m_targetObj.lock())
    {
        json[jsonKey::Comp::TrackingCameraComponent::TargetObject.data()] = target->GetName();
    }
}

void TrackingCameraComponent::Deserialize(const Json& json)
{
    auto cameraName = json[jsonKey::Comp::TrackingCameraComponent::Name.data()];
    m_spCamera->SetCameraName(cameraName);

    // カメラの位置・回転・感度を復元
    auto localPos = json[jsonKey::Comp::TrackingCameraComponent::LocalCameraPos.data()];
    m_localCameraPos = Math::Vector3{ localPos[0], localPos[1], localPos[2] };

    auto mouseDegAng = json[jsonKey::Comp::TrackingCameraComponent::MouseDegAng.data()];
    m_trackingMouseData.DegAng = Math::Vector3{ mouseDegAng[0], mouseDegAng[1], mouseDegAng[2] };

    m_sensitivity = json[jsonKey::Comp::TrackingCameraComponent::Sensitivity.data()];

    // カメラのビュー行列情報の復元
    auto it = json.find(jsonKey::Comp::TrackingCameraComponent::ViewMat);
    if (it != json.end())
    {
        auto& viewMatInfo = m_spCamera->WorkViewMatInfo();

        const auto& pos = (*it)["Pos"];
        viewMatInfo.Pos = Math::Vector3{
            pos[0],
            pos[1],
            pos[2]
        };

        const auto& up = (*it)["Up"];
        viewMatInfo.Up = Math::Vector3{
            up[0],
            up[1],
            up[2]
        };

        const auto& forward = (*it)["Forward"];
        viewMatInfo.Forward = Math::Vector3{
            forward[0],
            forward[1],
            forward[2]
        };

        const auto& right = (*it)["Right"];
        viewMatInfo.Right = Math::Vector3{
            right[0],
            right[1],
            right[2]
        };

        viewMatInfo.IsNeedUpdate = true;
    }

    // プロジェクション行列情報の復元
    auto& projInfo = m_spCamera->WorkProjMatInfo();
    projInfo.Near = json[jsonKey::Comp::TrackingCameraComponent::ProjMat_NearClip.data()];
    projInfo.Far = json[jsonKey::Comp::TrackingCameraComponent::ProjMat_FarClip.data()];

    if (json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Type.data()] == typeid(Camera::PerspectiveInfo).name())
    {
        projInfo.IsPerspective = true;
        projInfo.PerspectInfo.ViewAngle = json[jsonKey::Comp::TrackingCameraComponent::ProjMat_ViewAngle.data()];
        projInfo.PerspectInfo.Aspect = json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Aspect.data()];
    }
    else
    {
        projInfo.IsPerspective = false;
        projInfo.OrthoInfo.Width = json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Width.data()];
        projInfo.OrthoInfo.Height = json[jsonKey::Comp::TrackingCameraComponent::ProjMat_Height.data()];
    }

    // ターゲットオブジェクト名の保存
    auto itr = json.find(jsonKey::Comp::TrackingCameraComponent::TargetObject.data());
    if (itr != json.end())
    {
        m_targetName = itr->get<std::string>();
    }
}

void TrackingCameraComponent::ImGuiUpdate()
{
    if (!m_spCamera)
    {
        FNENG_ASSERT_ERROR(m_spCamera->GetCameraName() + " : が無効状態です")
            return;
    }

    ImGui::Text("-----------TrackingCameraData-----------");

    ImGui::Text("CameraName : %s", m_spCamera->GetCameraName().c_str());
    if (ImGui::Button("ChangeName"))
    {
        m_spCamera->SetCameraName(m_guiCamName);
    }
    ImGui::SameLine();
    utl::ImGuiHelper::InputTextWithString("###NewCameraName", m_guiCamName);

    if (ImGui::Button("ChangeTarget"))
    {
        if (auto&& spTarget = m_wpOwnerObj.lock()->GetScene()->FindObject(m_targetName))
        {
            SetTargetObj(spTarget);
        }
    }
    ImGui::SameLine();
    utl::ImGuiHelper::InputTextWithString("###TargetName", m_targetName);

    ImGui::DragFloat("Sensitivity", &m_sensitivity, 0.1f, 0.0f, 10.0f);
    ImGui::DragFloat3(U8_TEXT("カメラの角度"), &m_trackingMouseData.DegAng.x, 0.1f);

    ImGui::Text("----------------CameraProjMat----------------");
    CameraProjMatInfo& projMatInfo = m_spCamera->WorkProjMatInfo();
    {
        ImGui::Checkbox("Perspective", &projMatInfo.IsPerspective);

        if (ImGui::DragFloat("NerClip", &projMatInfo.Near, 0.1f, 0.1f, 1000.0f))
        {
            m_spCamera->WorkProjMatInfo().IsNeedUpdate = true;
        }

        if (ImGui::DragFloat("FarClip", &projMatInfo.Far, 0.1f, projMatInfo.Near + 0.1f, 1000.0f))
        {
            m_spCamera->WorkProjMatInfo().IsNeedUpdate = true;
        }

        // パーセクティブとオルソーの設定 : カメラに設定されているデータによって操作する行列を変える
        projMatInfo.IsPerspective ? ImGuiPerspectiveInfo() : ImGuiOrthographicInfo();
    }

    ImGui::Text("-----------CameraViewMat-----------");
    CameraViewMatInfo& viewMatInfo = m_spCamera->WorkViewMatInfo();
    // 座標の更新
    {
        ImGui::Text("CameraPos : %.3f, %.3f, %.3f", viewMatInfo.Pos.x, viewMatInfo.Pos.y, viewMatInfo.Pos.z);
        if (ImGui::DragFloat3("Position", &viewMatInfo.Pos.x, 0.1f))
        {
            viewMatInfo.IsNeedUpdate = true;
        }
    }

    // 上方向の更新
    {
        ImGui::Text("CameraUp : %.3f, %.3f, %.3f", viewMatInfo.Up.x, viewMatInfo.Up.y, viewMatInfo.Up.z);
        Math::Vector3 moveVal = {};
        if (ImGui::DragFloat3("Up", &viewMatInfo.Up.x, 0.1f))
        {
            viewMatInfo.Up.Normalize();
            viewMatInfo.IsNeedUpdate = true;
        }
    }
}

void TrackingCameraComponent::ImGuiPerspectiveInfo()
{
    // Perspectiveの情報
    ImGui::Text("--------ProjectionMat[ Perspective ]--------");

    Camera::PerspectiveInfo& perspectiveInfo = m_spCamera->WorkProjMatInfo().PerspectInfo;

    if (ImGui::DragFloat("Aspect", &perspectiveInfo.Aspect, 0.1f, 0.1f, 10.0f))
    {
        m_spCamera->WorkProjMatInfo().IsNeedUpdate = true;
    }

    float viewAngle = MathHelper::ConvertToDegrees(perspectiveInfo.ViewAngle);
    if (ImGui::DragFloat("ViewAngle", &viewAngle, 0.1f, 0.0f, 360.0f))
    {
        perspectiveInfo.ViewAngle = MathHelper::ConvertToRadians(viewAngle);
        m_spCamera->WorkProjMatInfo().IsNeedUpdate = true;
    }
}

void TrackingCameraComponent::ImGuiOrthographicInfo()
{
    // Orthographicの情報
    ImGui::Text("--------ProjectionMat[ Orthographic ]--------");

    CameraProjMatInfo::OrthographicInfo& orthographicInfo = m_spCamera->WorkProjMatInfo().OrthoInfo;

    if (ImGui::DragFloat("Width", &orthographicInfo.Width, 0.1f, 0.1f, 1000.0f))
    {
        m_spCamera->WorkProjMatInfo().IsNeedUpdate = true;
    }

    if (ImGui::DragFloat("Height", &orthographicInfo.Height, 0.1f, 0.1f, 1000.0f))
    {
        m_spCamera->WorkProjMatInfo().IsNeedUpdate = true;
    }
}

void TrackingCameraComponent::CalcScreenPosToMoveVal(Math::Vector3& _degAng)
{
    if (!m_trackingMouseData.IsCamRot) { return; }

    if (!InputSystem::Instance().IsHold("RightClick"))
    {
        // カメラ回転モードが無効の場合、前回の位置をリセット
        m_trackingMouseData.PrevMousePos = {};
        return;
    }

    // ウィンドウのハンドルを取得
    HWND hwnd = Application::Instance().GetWindowHandle();

    // ウィンドウのクライアント領域を取得
    RECT rect;
    GetClientRect(hwnd, &rect);

    // クライアント領域をスクリーン座標に変換
    POINT topLeft = { rect.left, rect.top };
    POINT bottomRight = { rect.right, rect.bottom };
    ClientToScreen(hwnd, &topLeft);
    ClientToScreen(hwnd, &bottomRight);

    int screenWidth = bottomRight.x - topLeft.x;
    int screenHeight = bottomRight.y - topLeft.y;

    // マウスワープ量を初期化
    int mouseWarpX = 0, mouseWarpY = 0;

    // カーソルがウィンドウ外に出た場合の処理
    bool isWarped = Window::WarpMouseToInScreen(hwnd, mouseWarpX, mouseWarpY);

    if(isWarped)
    {
        // ImGui のマウスドラッグ量を更新
        ImGuiIO& io = ImGui::GetIO();

        // マウスのドラッグ量（移動量）を調整
        io.MouseDelta.x += static_cast<float>(-mouseWarpX);
        io.MouseDelta.y += static_cast<float>(-mouseWarpY);
    }

    // 現在のマウスの座標を取得
    POINT currentMousePos = {};
    GetCursorPos(&currentMousePos);

    if (m_trackingMouseData.PrevMousePos.x != 0 && m_trackingMouseData.PrevMousePos.y != 0)
    {
        // マウスの差分を計算
        int deltaX = currentMousePos.x - m_trackingMouseData.PrevMousePos.x;
        int deltaY = currentMousePos.y - m_trackingMouseData.PrevMousePos.y;

        // マウスがワープした場合、移動量を補正
        if (deltaX > screenWidth / 2)
        {
            deltaX -= screenWidth;
        }
        else if (deltaX < -screenWidth / 2)
        {
            deltaX += screenWidth;
        }

        if (deltaY > screenHeight / 2)
        {
            deltaY -= screenHeight;
        }
        else if (deltaY < -screenHeight / 2)
        {
            deltaY += screenHeight;
        }

        // 差分による回転の計算
        _degAng.x += static_cast<float>(deltaY) / m_sensitivity;
        _degAng.y += static_cast<float>(deltaX) / m_sensitivity;
    }

    // 現在のマウス位置を保存
    m_trackingMouseData.PrevMousePos = currentMousePos;
}

Math::Vector3 TrackingCameraComponent::CalcOnCameraPosNode(const std::shared_ptr<GameObject>& target)
{
    // ターゲットのオブジェクトのモデルを取得する
    std::shared_ptr<ModelComponent> spModelComp = target->GetComponent<ModelComponent>(/* assertLog = */false);

    // モデルコンポーネントが存在しない場合はアニメーションコンポーネントを取得する
    if (!spModelComp)
    {
        spModelComp = target->GetComponent<AnimationComponent>();
    }

    if (!spModelComp) { return Math::Vector3::Zero; }

    const std::shared_ptr<ModelData>& modelData = spModelComp->GetOriginalModelData();
    if (!modelData) { return Math::Vector3::Zero; }

    // ノードの検索
    // todo | hack : 全ノードの検索なので処理効率が気になる場合は修正する
    const ModelData::Node* const node = modelData->FindNode("CameraPos");
    if (!node) { return Math::Vector3::Zero; }

    const Math::Vector3& camPos = node->mLocalTransform.Translation();

    return camPos;
}
