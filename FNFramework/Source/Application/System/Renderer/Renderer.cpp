#include "Renderer.h"

void Renderer::Render()
{
    // todo : レイトレ用のパイプラインを作成する : 今は非対応のため描画をスキップ
    // if (GraphicsDevice::Instance().IsDXRSupport()) { return; }

    // モデルリストが空じゃなければ、描画処理を行う
    if (!m_GBufferRenderData.empty() || !m_ShadowMapRenderData.empty())
    {
        DrawModel();
    }

    // デバッグ描画
    SceneManager::Instance().GetDebugWire()->Draw();

    if (!m_spriteList.empty())
    {
        DrawSprite();
    }

    ClearList();
}

void Renderer::DrawModel()
{
    if (ShaderManager::Instance().WorkShadowShader()->Begin())
    {

        for (auto& [modelData, instanceList] : m_ShadowMapRenderData)
        {
            if (!modelData || instanceList.InstanceDataList.empty()) { continue; }

            // GBufferPass を使用して描画
            ShaderManager::Instance().WorkShadowShader()->DrawModelInstanced(
                modelData,
                instanceList.InstanceDataList,
                instanceList.ModelWorkList);
        }

        ShaderManager::Instance().WorkShadowShader()->End();
    }

    if (ShaderManager::Instance().WorkGBufferPass()->Begin())
    {
        for (auto& [modelData, instanceList] : m_GBufferRenderData)
        {
            if (!modelData || instanceList.InstanceDataList.empty()) { continue; }

            // GBufferPass を使用して描画
            ShaderManager::Instance().WorkGBufferPass()->DrawModelInstanced(
                modelData,
                instanceList);
        }

        ShaderManager::Instance().WorkGBufferPass()->End();
    }

    // ライティングパスの描画
    ShaderManager::Instance().GetLightingPass()->Rendering();

    ShaderManager::Instance().GetBloomShader()->Rendering();
}

void Renderer::DrawSprite()
{
    //==============================
    // Sprite描画
    //==============================
    if (ShaderManager::Instance().WorkSpriteShader()->Begin())
    {

        for (auto& sprite : m_spriteList)
        {
            if (!sprite.Vertex) { FNENG_ASSERT_ERROR("スプライトの状態が無効です"); continue; }

            // 定数データのセット
            ShaderManager::Instance().WorkSpriteShader()->SetCBSpriteData(sprite.Color, sprite.WorldMatrix, sprite.Tilling, sprite.Offset);

            ShaderManager::Instance().WorkSpriteShader()->DrawSprite(
                sprite.Vertex,
                sprite.MaskTex,
                sprite.PixelPos, sprite.SrcRect,
                sprite.Pivot);
        }
    }
}

void Renderer::ClearList()
{
    m_spriteList.clear();
    m_GBufferRenderData.clear();
    m_ShadowMapRenderData.clear();
}
