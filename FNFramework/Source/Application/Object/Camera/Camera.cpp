#include "Camera.h"

//--------------------------------
// ProjectionMatrix
//--------------------------------
void CameraProjMatInfo::SetNearAndFar(float nearClip, float farClip)
{
    Near = nearClip;
    Far = farClip;
    IsNeedUpdate = true;
}

//---------------
// Perspective
//---------------
void CameraProjMatInfo::SetPerspectiveInfo(float viewAngle)
{
    PerspectInfo = PerspectiveInfo(viewAngle);
    IsPerspective = true;
    IsNeedUpdate = true;
}

//---------------
// Orthographic
//---------------
void CameraProjMatInfo::SetOrthographicInfo(float width, float height)
{
    OrthoInfo = OrthographicInfo(width, height);
    IsPerspective = false;
    IsNeedUpdate = true;
}

//--------------------------------
// カメラ関係処理
//--------------------------------
CBufferData::Camera Camera::GetCBData()
{
    // 各行列に更新が必要な場合は更新処理を行ってから、カメラデータを返す
    if (m_projMatInfo.IsNeedUpdate) { UpdateProjMat(); }
    if (m_viewMatInfo.IsNeedUpdate) { UpdateViewMat(); }

    CBufferData::Camera camDat;

    camDat.CamPos = m_viewMatInfo.mView.Invert().Translation();

    const Math::Matrix& mViewProj = m_viewMatInfo.mView * m_mProj;

    camDat.mViewProj = mViewProj;
    camDat.mViewProjInv = mViewProj.Invert();

    camDat.Height = m_projMatInfo.Far / 2.0f; // 高さ補正を行う

    return camDat;
}


//---------------
// 移動関連
//---------------
void Camera::SetUp(const Math::Vector3& up)
{
    m_viewMatInfo.Up = up;
    m_viewMatInfo.Up.Normalize();
    m_viewMatInfo.IsNeedUpdate = true;
}

void Camera::Move(const Math::Vector3& move)
{
    m_viewMatInfo.Pos += move;
    m_viewMatInfo.IsNeedUpdate = true;
}

void Camera::MovePos(const Math::Vector3& move)
{
    m_viewMatInfo.Pos += move;
    m_viewMatInfo.IsNeedUpdate = true;
}

void Camera::MoveForward(const float move)
{
    Move(m_viewMatInfo.Forward * move);
}

void Camera::MoveRight(float move)
{
    Move(m_viewMatInfo.Right * move);
}

void Camera::MoveUp(float move)
{
    Move(m_viewMatInfo.Up * move);
}

//--------------------------------
// その他関数
//--------------------------------
void Camera::InitProjMatInfo(const PerspectiveInfo& perspInfo, float nearClip, float farClip)
{
    m_projMatInfo.SetNearAndFar(nearClip, farClip);
    m_projMatInfo.SetPerspectiveInfo(MathHelper::ConvertToRadians(perspInfo.ViewAngle));

    UpdateProjMat();
}

void Camera::InitProjMatInfo(const OrthographicInfo& orthoInfo, float nearClip, float farClip)
{
    m_projMatInfo.SetNearAndFar(nearClip, farClip);
    m_projMatInfo.SetOrthographicInfo(orthoInfo.Width, orthoInfo.Height);

    UpdateProjMat();
}

void Camera::UpdateProjMat()
{
    // FarClipが正しく設定されていないということは、
    // 正しくPsojMatInfoが生成されてない可能性があるためエラーを出す
    if (m_projMatInfo.Near >= m_projMatInfo.Far)
    {
        FNENG_ASSERT_ERROR("NearClipの方がFarClipより大きいです。\nInitProjMatInfo()でセットし直してください");
        return;
    }

    // アスペクト比を取得
    m_projMatInfo.PerspectInfo.Aspect = static_cast<float>(Screen::Width) / Screen::Height;

    // 直近でデータが更新された場合、ProjectionMatrixを更新する
    if (!m_projMatInfo.IsNeedUpdate) { return; }

    if (m_projMatInfo.IsPerspective)
    {
        // 透視投影行列の場合
        m_mProj = DirectX::XMMatrixPerspectiveFovLH(
            m_projMatInfo.PerspectInfo.ViewAngle,
            m_projMatInfo.PerspectInfo.Aspect,
            m_projMatInfo.Near,
            m_projMatInfo.Far
        );
    }
    else
    {
        // 平行投影行列の場合
        m_mProj = DirectX::XMMatrixOrthographicLH(
            m_projMatInfo.OrthoInfo.Width,
            m_projMatInfo.OrthoInfo.Height,
            m_projMatInfo.Near,
            m_projMatInfo.Far
        );
    }

    // ShaderManagerのCameraListに更新しが必要なことを伝える
    m_isUpdate = true;

    m_projMatInfo.IsNeedUpdate = false;
}

void Camera::UpdateViewMat()
{
    // 直近でデータが更新された場合、ViewMatrixを更新する
    if (!m_viewMatInfo.IsNeedUpdate) { return; }

    // memo : ビュー行列は継承先で更新されているものとして、ここではセットしない
    // 座標と注視点を基にビュー行列を生成
    m_viewMatInfo.mViewInv = m_viewMatInfo.mView.Invert();

    // ビュー行列から各ベクトルを取得
    m_viewMatInfo.Forward = m_viewMatInfo.mViewInv.Backward();
    m_viewMatInfo.Forward.Normalize();
    m_viewMatInfo.Right = m_viewMatInfo.mViewInv.Right();
    m_viewMatInfo.Right.Normalize();

    // ShaderManagerのCameraListに更新しが必要なことを伝える
    m_isUpdate = true;

    m_viewMatInfo.IsNeedUpdate = false;
}
