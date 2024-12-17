#include "DebugTextureScript.h"

#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"

void DebugTextureScript::Start()
{
    if(!OwnerValid()) { return; }

    //x--- デバッグ表示したいテクスチャをリストに追加する ---x//
    std::vector<std::shared_ptr<ShaderResourceTexture>> spDebugTextures;
    auto shadowTex = ShaderManager::Instance().GetShadowShader()->GetShadowMap()->GetTexture();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(shadowTex));

    //// G-Buffer //
    auto gbDepthTex = ShaderManager::Instance().GetGBufferPass()->GetDepthGB();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(gbDepthTex));
    auto gbAlbedTex = ShaderManager::Instance().GetGBufferPass()->GetAlbedoGB();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(gbAlbedTex));
    auto gbNormalTex = ShaderManager::Instance().GetGBufferPass()->GetNormalGB();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(gbNormalTex));

    // 輝度テクスチャ //
    auto luminanceTex = ShaderManager::Instance().GetBloomShader()->GetLuminanceRenderTexture();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(luminanceTex));

    // 輝度抽出 //
    auto blurTex = ShaderManager::Instance().GetBloomShader()->GetGaussianBlur().GetBlurTexture();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(blurTex));

    // ブルームのRTTex　//
    auto bloomTex = ShaderManager::Instance().GetBloomShader()->GetBloomRenderTexture();
    spDebugTextures.emplace_back(std::make_shared<ShaderResourceTexture>(bloomTex));

    // オーナーにスプライトコンポーネントがアタッチされていない場合は、生成する
    std::string_view spriteCompName = typeid(SpriteComponent).name();

    int i = 0;
    for(auto&& comp : m_wpOwnerObj.lock()->GetComponents())
    {
        // 既にスプライトコンポーネントがアタッチされている場合は、テクスチャを設定する
        if(comp->GetComponentName() == spriteCompName)
        {
            auto sp = std::dynamic_pointer_cast<SpriteComponent>(comp);
            sp->SetMainTexture(spDebugTextures[i]);
            ++i;
        }
    }

    spDebugTextures.clear();

    // リリースビルドのときはデバッグテクスチャは表示しない
#ifndef _DEBUG
    m_wpOwnerObj.lock()->SetState(GameObject::State::ePaused);
#endif
}
