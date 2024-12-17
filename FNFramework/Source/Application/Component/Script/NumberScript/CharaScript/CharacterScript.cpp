#include "CharacterScript.h"

#include "Application/Component/Renderer/SpriteComponent/SpriteComponent.h"

void CharacterScript::Awake()
{
    m_basePos = Math::Vector2::Zero;
    m_displayLen = 0;
}

void CharacterScript::Start()
{
    // この時点でm_spSampleSourceTextureにはテクスチャが格納されているはずなのでサイズが0の場合はエラー
    if (m_spSampleSourceTexture.size() == 0)
    {
        FNENG_ASSERT_ERROR("CharacterScript::Start() : m_spSampleSourceTexture のサイズが 0 です");
        return;
    }

    // m_maxLenの数分スプライトを初期化しておく
    m_spSpriteComponents.resize(m_maxLen);
    for (int i = 0; i < m_maxLen; ++i)
    {
        auto spSprite = m_wpOwnerObj.lock()->AddComponent<SpriteComponent>();
        spSprite->Awake();

        m_spSpriteComponents[i] = spSprite;
    }
}

void CharacterScript::Update()
{
    if(m_spSampleSourceTexture.size() < m_samplingIdx)
    {
        m_samplingIdx = 0;
    }

    if (m_checkSampleSourceTexture)
    {
        UpdateSampleSourceTexture();
        return;
    }

    // 表示する文字数が最大の文字数を超えている場合は最大の文字数に設定する
    if (m_displayLen > m_maxLen)
    {
        // todo : 今後の拡張として、maxLenが超えた場合は動的にSpriteComponentを追加するようにする
        // 懸念点として、一時的に必要なだけでそれ以降必要がなかった場合に無駄なリソースを消費する可能性がある
        m_displayLen = m_maxLen;
    }

    // todo : 複数種類のテクスチャをm_spSpriteComponentsから参照したい場合はここでm_basePosのオフセットを計算する必要がある(NumberScript::Updateを参照)

    // 表示する文字数分スプライトを表示する
    for (int i = 0; i < m_maxLen; ++i)
    {
        const std::shared_ptr<SpriteComponent>& spSprite = m_spSpriteComponents[i].lock();
        if (!PreClassValid(spSprite)) { continue; }

        if (i <= m_displayLen)
        {
            const auto& samplingTex = m_spSampleSourceTexture[m_samplingIdx];

            spSprite->SetPos({m_basePos.x + static_cast<float>((i - 1) * samplingTex->GetWidth()), m_basePos.y});
            spSprite->SetMainTexture(samplingTex);
            spSprite->SetSize({
                static_cast<float>(samplingTex->GetWidth()), static_cast<float>(samplingTex->GetHeight())
            });

            spSprite->SetEnable(true);
        }
        else
        {
            spSprite->SetEnable(false);
        }
    }
}

void CharacterScript::UpdateSampleSourceTexture()
{
    for (int i = 0; i < m_spSpriteComponents.size(); ++i)
    {
        const auto& spSprite = m_spSpriteComponents[i].lock();
        if (!PreClassValid(spSprite)) { continue; }

        const auto& texture = m_spSampleSourceTexture[i];
        spSprite->SetMainTexture(texture);
        spSprite->SetPos({m_basePos.x + static_cast<float>((i - 1) * texture->GetWidth()), m_basePos.y});
        spSprite->SetSize({static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())});
        spSprite->SetEnable(true);
    }
}

void CharacterScript::ImGuiUpdate()
{
    ImGui::DragFloat2("ScreenPos", &m_basePos.x, 0.1f);

    // テクスチャの選択
    ImGui::DragInt("SamplingTexIndex", &m_samplingIdx, 1, 0,
        static_cast<int>(m_spSampleSourceTexture.size()) - 1);

    // 表示する文字数の設定
    ImGui::DragInt("DisplayLen", &m_displayLen, 1, 0, m_maxLen);
}

void CharacterScript::Release()
{
    m_spSampleSourceTexture.clear();
    m_spSpriteComponents.clear();
}
