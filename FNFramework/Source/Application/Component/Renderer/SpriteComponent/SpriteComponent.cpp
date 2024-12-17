#include "SpriteComponent.h"

#include "Application/Component/TransformComponent/TransformComponent.h"

void SpriteComponent::Awake()
{
    // バッファの作成を行う
    m_spriteRenderingData.Vertex = std::make_shared<SpriteMesh>();
    m_spriteRenderingData.Vertex->Create();
}

void SpriteComponent::Start()
{
}

void SpriteComponent::Update()
{
    if (!OwnerValid()) { return; }

    // todo : イージングアニメーションで変更できるのがアルファ値のみなのはさすがにまずいので LightComponent 方式で行く
    // イージングアニメーションが設定されている場合はアルファ値を更新する
    if (m_easingData.IsEasing)
    {
        if (m_easingData.Easing(SceneManager::Instance().FrameDeltaTime(), m_spriteRenderingData.Color.w))
        {
            m_easingData.Reverse = !m_easingData.Reverse;
        }
    }

    Math::Vector3 objectPos = m_wpOwnerObj.lock()->GetTransformComponent()->GetWorldPos();

    RenderingData::Sprite::Sprite renderData = m_spriteRenderingData;

    renderData.PixelPos.x += objectPos.x;
    renderData.PixelPos.y += objectPos.y;

    Renderer::Instance().AddRenderingSpriteData(renderData);
}

void SpriteComponent::Release()
{
    if (m_spriteRenderingData.Vertex)
    {
        m_spriteRenderingData.Vertex->Release();
        m_spriteRenderingData.Vertex.reset();
        m_spriteRenderingData.Vertex = nullptr;
    }

    if (m_spriteRenderingData.MaskTex)
    {
        m_spriteRenderingData.MaskTex.reset();
        m_spriteRenderingData.MaskTex = nullptr;
    }

    if (m_spriteRenderingData.SrcRect)
    {
        m_spriteRenderingData.SrcRect.reset();
        m_spriteRenderingData.SrcRect = nullptr;
    }
}

void SpriteComponent::Serialize(Json& _json) const
{
    // オーダー順を保存
    _json[jsonKey::Comp::SpriteComponent::UpdateOrder.data()] = m_spriteRenderingData.Order;

    const Math::Vector4& pixelPos = m_spriteRenderingData.PixelPos;
    // ピクセル座標を保存
    _json[jsonKey::Comp::SpriteComponent::PixelPos.data()] = { pixelPos.x, pixelPos.y, pixelPos.z, pixelPos.w };

    const Math::Vector2& pivot = m_spriteRenderingData.Pivot;
    const Math::Color& color = m_spriteRenderingData.Color;
    // ピボットと色を保存
    _json[jsonKey::Comp::SpriteComponent::Pivot.data()] = { pivot.x, pivot.y };
    _json[jsonKey::Comp::SpriteComponent::Color.data()] = { color.x, color.y, color.z, color.w };

    // イージングデータの保存
    m_easingData.Serialize(_json);

    // メインテクスチャが存在していたらパスを保存
    const std::shared_ptr<SpriteMesh>& spSpriteMesh = m_spriteRenderingData.Vertex;
    if (spSpriteMesh && spSpriteMesh->GetMainTex() && !spSpriteMesh->GetMainTex()->GetFilePath().empty())
    {
        _json[jsonKey::Comp::SpriteComponent::MainTexPath.data()] = spSpriteMesh->GetMainTex()->GetFilePath();
    }

    // マスクテクスチャが存在していたらパスを保存
    const std::shared_ptr<ShaderResourceTexture>& spMaskTex = m_spriteRenderingData.MaskTex;
    if (spMaskTex && !spMaskTex->GetFilePath().empty())
    {
        _json[jsonKey::Comp::SpriteComponent::MaskTexPath.data()] = spMaskTex->GetFilePath();
    }

    // タイリング / オフセットを保存
    _json[jsonKey::Comp::SpriteComponent::Tilling.data()] = { m_spriteRenderingData.Tilling.x, m_spriteRenderingData.Tilling.y };
    _json[jsonKey::Comp::SpriteComponent::Offset.data()] = { m_spriteRenderingData.Offset.x, m_spriteRenderingData.Offset.y };
}

void SpriteComponent::Deserialize(const Json& _json)
{
    // オーダー順を復元
    m_spriteRenderingData.Order = _json[jsonKey::Comp::SpriteComponent::UpdateOrder.data()];

    // ピクセル座標を復元
    auto pixelPos = _json[jsonKey::Comp::SpriteComponent::PixelPos.data()];
    m_spriteRenderingData.PixelPos = Math::Vector4{ pixelPos[0], pixelPos[1], pixelPos[2], pixelPos[3] };

    // ピボットと色を復元
    auto pivot = _json[jsonKey::Comp::SpriteComponent::Pivot.data()];
    m_spriteRenderingData.Pivot = Math::Vector2{ pivot[0], pivot[1] };

    auto color = _json[jsonKey::Comp::SpriteComponent::Color.data()];
    m_spriteRenderingData.Color = Math::Color{ color[0], color[1], color[2], color[3] };

    // イージングデータを復元
    m_easingData.Deserialize(_json);

    // メインテクスチャのパスを取得
    auto it = _json.find(jsonKey::Comp::SpriteComponent::MainTexPath.data());
    if (it != _json.end())
    {
        std::string mainTexPath = _json[jsonKey::Comp::SpriteComponent::MainTexPath.data()];
        SetMainTexture(AssetManager::Instance().GetTexture(mainTexPath));
    }

    // マスクテクスチャのパスを取得
    it = _json.find(jsonKey::Comp::SpriteComponent::MaskTexPath.data());
    if (it != _json.end())
    {
        std::string maskTexPath = _json[jsonKey::Comp::SpriteComponent::MaskTexPath.data()];
        SetMaskTexture(AssetManager::Instance().GetTexture(maskTexPath));
    }

    // タイリング / オフセットの復元
    auto tillingArray = _json.value(jsonKey::Comp::SpriteComponent::Tilling.data(), Json::array({ 1.0f, 1.0f }));
    if (tillingArray.size() == 2)
    {
        m_spriteRenderingData.Tilling = Math::Vector2{ tillingArray[0].get<float>(), tillingArray[1].get<float>() };
    }

    auto offsetArray = _json.value(jsonKey::Comp::SpriteComponent::Offset.data(), Json::array({ 0.0f, 0.0f }));
    if (offsetArray.size() == 2)
    {
        m_spriteRenderingData.Offset = Math::Vector2{ offsetArray[0].get<float>(), offsetArray[1].get<float>() };
    }
}

void SpriteComponent::ImGuiUpdate()
{
    ImGui::Text(U8_TEXT("描画順(小さければ手前に描画される)"));
    ImGui::DragInt("##Order", &m_spriteRenderingData.Order, 1, 0, 1000);

    if (ImGui::TreeNode(U8_TEXT("デフォルトのオーダー")))
    {
        if (ImGui::Button("BackGround"))
        {
            m_spriteRenderingData.Order = RenderingData::Sprite::Sprite::Order::eBackGround;
        }

        if (ImGui::Button("Default"))
        {
            m_spriteRenderingData.Order = RenderingData::Sprite::Sprite::Order::eDefault;
        }

        if (ImGui::Button("Front"))
        {
            m_spriteRenderingData.Order = RenderingData::Sprite::Sprite::Order::eFront;
        }
        ImGui::TreePop();
    }

    // オフセット / タイリング //
    ImGui::Text(U8_TEXT("オフセット"));
    ImGui::DragFloat2("##Offset", &m_spriteRenderingData.Offset.x, 0.01f);
    ImGui::Text(U8_TEXT("タイリング"));
    ImGui::DragFloat2("##Tiling", &m_spriteRenderingData.Tilling.x, 0.01f);

    // ピクセル座標の設定 //
    ImGui::Text(U8_TEXT("表示座標"));
    ImGui::DragFloat2("##Position", &m_spriteRenderingData.PixelPos.x, 1.0f);
    ImGui::Text(U8_TEXT("表示サイズ"));
    ImGui::DragFloat2("##Size", &m_spriteRenderingData.PixelPos.z, 1.0f);
    ImGui::Text(U8_TEXT("中心点"));
    ImGui::DragFloat2("##Pivot", &m_spriteRenderingData.Pivot.x, 0.01f, 0.0f, 1.0f);
    ImGui::Text(U8_TEXT("色"));
    ImGui::ColorEdit4("##Color", &m_spriteRenderingData.Color.x);

    ImGui::Separator();

    //------------------
    // MainTexture
    //------------------
    ImGui::Text("--------Main Texture--------");
    if (const std::shared_ptr<SpriteMesh>& spSpriteMesh = m_spriteRenderingData.Vertex)
    {
        if (const std::shared_ptr<ShaderResourceTexture>& spTexture = spSpriteMesh->GetMainTex())
        {
            ImGui::Text("TextureName : %s", spTexture->GetFilePath().c_str());
        }
    }
    // スプライト選択機能
    std::string spriteName;
    if (utl::ImGuiHelper::SelectSpritePath("Select MainTex", spriteName))
    {
        if (!spriteName.empty())
        {
            // スプライトをロードしてテクスチャを設定
            SetMainTexture(AssetManager::Instance().GetTexture(spriteName));
        }
    }

    //------------------
    // MaskTexture
    //------------------
    ImGui::Text("--------Mask Texture--------");
    if (const std::shared_ptr<ShaderResourceTexture>& spMaskTex = m_spriteRenderingData.MaskTex)
    {
        ImGui::Text("TextureName : %s", spMaskTex->GetFilePath().c_str());
    }
    // マスク選択機能
    std::string maskName;
    if (utl::ImGuiHelper::SelectSpritePath("Select MaskTex", maskName))
    {
        if (!maskName.empty())
        {
            // マスクをロードしてテクスチャを設定
            SetMaskTexture(AssetManager::Instance().GetTexture(maskName));
        }
    }

    ImGui::Separator();

    m_easingData.ImGuiUpdate();
}
