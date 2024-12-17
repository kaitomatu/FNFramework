#include "Fade.h"

#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"

void Fade::Init()
{
    if (!m_spSpriteComponent)
    {
        const std::shared_ptr<ShaderResourceTexture>& white = GraphicsDevice::Instance().GetWhiteTex();

        m_spSpriteComponent = std::make_shared<SpriteComponent>(nullptr, "obj", false);

        m_spSpriteComponent->Awake();
        m_spSpriteComponent->StartComponent();

        m_spSpriteComponent->SetMainTexture(std::make_shared<ShaderResourceTexture>(*white));
    }

    m_spSpriteComponent->SetSize({ Screen::Width, Screen::Height });
    m_spSpriteComponent->SetColor(Color::White);
    m_spSpriteComponent->SetAlpha(0.0f);

    m_sin = 0.0f;

    // フェードイン
    m_phase = Phase::eStart;
    m_state = State::ePlay;
}

void Fade::StartUpdate()
{
    if (m_spSpriteComponent->GetColor().w >= 1.0f)
    {
        m_phase = Phase::eBlackOut;
    }
    else
    {
        // Spriteの色を更新
        m_spSpriteComponent->SetAlpha(sinf(m_sin * MathHelper::ToRadians)*2.0f);

        m_sin += Progress;
    }

    // 座標などをレンダラ―へ送る
    m_spSpriteComponent->Update();
}

void Fade::BlackOutUpdate()
{
    // MaxFPS辺りの1秒を算出
    m_timer += 1.0f / 60.0f;

    if(m_timer > BlackOutTime)
    {
        m_phase = Phase::eComplete;
    }

    // 座標などをレンダラ―へ送る
    m_spSpriteComponent->Update();
}

void Fade::CompleteUpdate()
{

    if (m_spSpriteComponent->GetColor().w <= 0.0f)
    {
        m_phase = Phase::eComplete;
        m_state = State::eStop;
        m_sin = 0.0f;
    }
    else
    {
        // Spriteの色を更新
        m_spSpriteComponent->SetAlpha(sinf(m_sin * MathHelper::ToRadians)* 2.0f);

        m_sin -= Progress;
    }

    // 座標などをレンダラ―へ送る
    m_spSpriteComponent->Update();
}
