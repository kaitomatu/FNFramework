#include "ChildKurageItemState.h"

#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/Script/ChildController/ChildController.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"
#include "Application/Component/TransformComponent/TransformComponent.h"

void ChildKurageItemState::Enter(ChildController* _pOwner)
{
    auto spLigComp =_pOwner->GetLightComponent();
    if (spLigComp)
    {
        // 仲間クラゲの場合は色を変更する
        if (_pOwner->GetPlayerScript())
        {
            spLigComp->SetColor({ Color::DeepGray.x, Color::DeepGray.y, Color::DeepGray.z });
        }
    }

    // 初期座標の設定
    const auto& spOwnerObjTrans = _pOwner->GetOwner()->GetTransformComponent();

    if (!spOwnerObjTrans)
    {
        FNENG_ASSERT_LOG("ChildKurageItemState::Update() : TransformComponentが存在しません", false);
        return;
    }

    _pOwner->SetBasePosition(spOwnerObjTrans->GetWorldPos());

    ItemState::Enter(_pOwner);
}

void ChildKurageItemState::Update(ChildController* _pOwner)
{
    const auto& spOwnerObjTrans = _pOwner->GetOwner()->GetTransformComponent();
    if (!spOwnerObjTrans)
    {
        FNENG_ASSERT_LOG("ChildKurageItemState::UpdateFloatBehavior() : TransformComponentが存在しません", false);
        return;
    }

    const Math::Vector3& nowPos = spOwnerObjTrans->GetWorldPos();

    float deltaTime = SceneManager::Instance().FrameDeltaTime();

    // デルタタイムを加算
    m_elapsedTime += deltaTime;

    // 一定間隔で方向を変更
    if (m_elapsedTime >= m_calcDirIntervalSeconds)
    {
        m_previewDirRate = std::clamp(m_previewDirRate, 0.0f, 1.0f);

        // 現在の方向と新しい方向を線形補完することで、かくかくとした動きをなくす
        Math::Vector3 newDir = ChildControllerSetting::CalcNewDir(_pOwner, nowPos);
        m_targetDir = Math::Vector3::Lerp(newDir, m_currentDir, m_previewDirRate);
        m_targetDir.Normalize();

        m_elapsedTime = 0.0f;
    }

    // [ 現在の方向 ---> 目標の方向 ]に補間
    m_currentDir = Math::Vector3::Lerp(m_currentDir, m_targetDir, m_interpolationRate);
    m_currentDir.Normalize();

    const std::shared_ptr<KurageMoveScript>& kurageMove = _pOwner->GetKurageMoveScript();

    if (kurageMove)
    {
        m_currentDir.y  = 0.0f;
        kurageMove->SetMoveFlg(true);
        kurageMove->SetInputDirection(m_currentDir);
    }

    //x--------------------------x//
    //      ItemStateの更新       x//
    //x--------------------------x//
    //----- プレイヤーとの距離判定 + ステートの移行 -----//
    ItemState::Update(_pOwner);
}

void ChildKurageItemState::Exit(ChildController* _pOwner)
{
    ItemState::Exit(_pOwner);
}

void ChildKurageItemState::ImGui(ChildController* _pOwner)
{
    ImGui::Text(U8_TEXT("自分の名前 : %s"), _pOwner->GetOwner()->GetName().data());

    ItemState::ImGui(_pOwner);

    ImGui::Text(U8_TEXT("フレーム間でのベクトルの補完率　"));
    ImGui::DragFloat("##InterpolationRate", &m_interpolationRate, 0.01f, 0.0f, 1.0f);

    ImGui::Separator();

    ImGui::Text(U8_TEXT("x----- ぷかぷか挙動の方向の設定 -----x"));
    ImGui::DragFloat(U8_TEXT("移動方向の更新間隔(s)"), &m_calcDirIntervalSeconds, 1, 1, 30);

    ImGui::Text("CurrentDir \n{ \n\tx : %.4f, \n\ty : %.4f, \n\tz : %.4f \n}",
        m_targetDir.x,
        m_targetDir.y,
        m_targetDir.z);

    ImGui::Text(U8_TEXT("前回のベクトルの影響度"));
    ImGui::DragFloat("##previewDirRate", &m_previewDirRate, 0.01f, 0.0f, 1.0f);
}
