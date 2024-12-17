#include "../ChildState/ChildState.h"
#include "../ItemState/ItemState.h"
#include "../../ChildController.h"
#include "Application/Component/LightComponent/LightComponent.h"
#include "Application/Component/TransformComponent/TransformComponent.h"
#include "Application/Component/Script/ChildController/ChildState/ItemState/ChildKurageItemState.h"
#include "Application/Component/Script/KurageMoveScript/KurageMoveScript.h"
#include "Application/Component/Script/PlayerScript/PlayerScript.h"

void ChildState::Enter(ChildController* _pOwner)
{
    const std::shared_ptr<LightComponent>& spLigComp = _pOwner->GetLightComponent();
    if (spLigComp)
    {
        spLigComp->SetColor(Math::Vector3{ Color::DeepBlue.x, Color::DeepBlue.y, Color::DeepBlue.z });
    }

    // スケールと回転の初期化
    const std::shared_ptr<TransformComponent>& spOwnerTrans = _pOwner->GetOwner()->GetTransformComponent();
    if (!spOwnerTrans) { return; }

    // 基準位置のオフセットを計算（子オブジェクトごとにユニークなオフセット）
    m_basePosOffset = GenerateBasePositionOffset();

    // プレイヤーの位置を基準位置として設定
    if (const std::shared_ptr<PlayerScript>& spPlayerScript = _pOwner->GetPlayerScript())
    {
        const std::shared_ptr<TransformComponent>& spPlayerTrans = spPlayerScript->GetOwner()->GetTransformComponent();

        // 基準点を設定 //
        if (spPlayerTrans)
        {
            Math::Vector3 playerPos = spPlayerTrans->GetWorldPos();
            Math::Vector3 basePos = playerPos + m_basePosOffset;
            _pOwner->SetBasePosition(basePos);
        }

        // 子どもを追加する //
        spPlayerScript->AddChild(PlayerScript::ChildData{ _pOwner->GetOwner(), _pOwner->GetKurageMoveScript() });
    }
    else
    {
        // プレイヤーが存在しない場合、自身の位置を基準位置に設定
        _pOwner->SetBasePosition(spOwnerTrans->GetWorldPos());
    }

    // 方向ベクトルの初期化
    m_currentDir = ChildControllerSetting::CalcNewDir(_pOwner, spOwnerTrans->GetWorldPos());

    // 経過時間の初期化
    m_elapsedTime = 0.0f;
}

void ChildState::Update(ChildController* _pOwner)
{
    const std::shared_ptr<TransformComponent>& spOwnerTrans = _pOwner->GetOwner()->GetTransformComponent();
    if (!spOwnerTrans) { return; }

    float deltaTime = SceneManager::Instance().FrameDeltaTime();
    m_elapsedTime += deltaTime;

    // 現在の位置と基準位置の距離を計算
    const Math::Vector3& currentPosition = spOwnerTrans->GetWorldPos();
    const Math::Vector3& basePosition = _pOwner->GetBasePosition();
    float distanceToBase = (basePosition - currentPosition).Length();

    // 子オブジェクトが離れすぎた場合の処理
    if (distanceToBase > m_followDistanceThreshold)
    {
        if (const std::shared_ptr<KurageMoveScript>& spKurageMove = _pOwner->GetKurageMoveScript())
        {
            spKurageMove->SetLightUpdate(true);
        }

        // 基準位置を自身の位置に設定
        _pOwner->GetChildController().AddState<ChildKurageItemState>(true);
        return;
    }

    // 基準位置をプレイヤーの位置にオフセットを加えた位置に更新
    if (const std::shared_ptr<PlayerScript>& spPlayerScript = _pOwner->GetPlayerScript())
    {
        const std::shared_ptr<TransformComponent>& spPlayerTrans = spPlayerScript->GetOwner()->GetTransformComponent();
        if (spPlayerTrans)
        {
            Math::Vector3 playerPos = spPlayerTrans->GetWorldPos();
            Math::Vector3 basePos = playerPos + m_basePosOffset;
            _pOwner->SetBasePosition(basePos);
        }
    }

    // デバッグ表示
    // if (ChildController::m_debugFlg)
    {
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(basePosition, Color::White, 0.2f);
        SceneManager::Instance().GetDebugWire()->AddDebugSphere(currentPosition, Color::Red, m_followDistanceThreshold);
        float dist = Math::Vector3(basePosition - currentPosition).Length();
        SceneManager::Instance().GetDebugWire()->AddDebugLine(currentPosition, m_currentDir, dist, Color::Green);
        SceneManager::Instance().GetDebugWire()->AddDebugLine(currentPosition, basePosition, Color::White);
    }

    // 一定間隔で新しい方向を計算
    if (m_elapsedTime >= m_calcDirIntervalSeconds)
    {
        //Math::Vector3 newDir = ChildControllerSetting::CalcNewDir(_pOwner, currentPosition);

        //// 現在の方向と新しい方向を線形補間
        //m_currentDir = Math::Vector3::Lerp(newDir, m_currentDir, m_previewDirRate);
        //m_currentDir.Normalize();

        m_currentDir = _pOwner->GetBasePosition() - currentPosition;
        m_currentDir.Normalize();

        m_elapsedTime = 0.0f;
    }

    // 計算した方向を KurageMoveScript にセット
    if (const std::shared_ptr<KurageMoveScript>& spKurageMove = _pOwner->GetKurageMoveScript())
    {

        if (spKurageMove->GetKurageState() == KurageMoveScript::KurageState::eDefault)
        {
            // 基本的にはこのスクリプトでライトの更新を行うため、ライトの更新を一時的に停止する
            spKurageMove->SetLightUpdate(false);

            // プレイヤーとの距離に応じてライトの色を変更する //
            if(const std::shared_ptr<LightComponent>& spLigComp = _pOwner->GetLightComponent())
            {
                // 初期状態 / 現在のライトの色 //
                const Math::Vector3& defaultColor = Math::Vector3{ Color::DeepGray.x, Color::DeepGray.y, Color::DeepGray.z };
                const Math::Vector3& childModeLigColor = Math::Vector3{ Color::DeepBlue.x, Color::DeepBlue.y, Color::DeepBlue.z };

                float t = distanceToBase / m_followDistanceThreshold;

                const Math::Vector3& color = MathHelper::Lerp(childModeLigColor, defaultColor, t);

                spLigComp->SetColor(color);
            }
        }

        // ノックバック状態の場合はノックバック側で操作するため、早期リターン //
        if (spKurageMove->GetKurageState() == KurageMoveScript::KurageState::eKnockBack)
        {
            return;
        }

        // 海藻に絡まっている場合は反対方向に設定する //
        if (spKurageMove->GetKurageState() == KurageMoveScript::KurageState::eTangled)
        {
            m_currentDir = -m_currentDir;
        }

        m_currentDir.y = 0.0f;
        spKurageMove->SetMoveFlg(true);
        spKurageMove->SetInputDirection(m_currentDir);
    }
}

void ChildState::Exit(ChildController* _pOwner)
{
    const std::shared_ptr<PlayerScript>& spPlayerScript = _pOwner->GetPlayerScript();

    if (!spPlayerScript) { return; }

    _pOwner->GetKurageMoveScript()->SetKurageState(KurageMoveScript::KurageState::eDefault);
    spPlayerScript->RemoveChild(_pOwner->GetOwner());
}

void ChildState::ImGui(ChildController* _pOwner)
{
    ImGui::Text(U8_TEXT("自分の名前 : %s"), _pOwner->GetOwner()->GetName().data());

    ImGui::Separator();

    ImGui::Text(U8_TEXT("方向を計算する間隔"));
    ImGui::DragFloat("##CalcDirIntervalSeconds", &m_calcDirIntervalSeconds, 0.1f, 0.0f, 10.0f);

    ImGui::Text("CurrentDir \n{ \n\tx : %.4f, \n\ty : %.4f, \n\tz : %.4f \n}", m_currentDir.x, m_currentDir.y, m_currentDir.z);

    ImGui::Text(U8_TEXT("前回のベクトルの影響度"));
    ImGui::DragFloat("##PreviewDirRate", &m_previewDirRate, 0.01f, 0.0f, 1.0f);

    ImGui::Separator();

    const auto& spOwnerTrans = _pOwner->GetOwner()->GetTransformComponent();
    const Math::Vector3& currentPosition = spOwnerTrans->GetWorldPos();
    const Math::Vector3& basePosition = _pOwner->GetBasePosition();
    float distanceToBase = (basePosition - currentPosition).Length();

    ImGui::Text(U8_TEXT("プレイヤーとの距離: %f"), distanceToBase / m_followDistanceThreshold);

    ImGui::Text(U8_TEXT("基準位置との距離の閾値"));
    ImGui::DragFloat("##FollowDistanceThreshold", &m_followDistanceThreshold, 0.1f, 0.0f, 100.0f);

    ImGui::Text("BasePos Offset: { x: %.2f, y: %.2f, z: %.2f }", m_basePosOffset.x, m_basePosOffset.y, m_basePosOffset.z);

    ImGui::Text(U8_TEXT("オフセット半径"));
    ImGui::DragFloat("##Offset", &m_offsetRadius, 0.1f, 0.0f, 10.0f);
    ImGui::Text(U8_TEXT("オフセットの高さ"));
    ImGui::DragFloat("##Offset", &m_offsetHeight, 0.1f, -5.0f, 5.0f);
}

Math::Vector3 ChildState::GenerateBasePositionOffset()
{
    // 子オブジェクトごとに異なるオフセットを生成
    // 半径と角度を用いて円周上に配置
    float angle = static_cast<float>(utl::RandomHelper::Instance().GetRandomDouble(0.0, 360.0));
    float angleRad = MathHelper::ConvertToRadians(angle);

    float offsetX = m_offsetRadius * cosf(angleRad);
    float offsetZ = m_offsetRadius * sinf(angleRad);
    float offsetY = m_offsetHeight;

    return Math::Vector3(offsetX, offsetY, offsetZ);
}
