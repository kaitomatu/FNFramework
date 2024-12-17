#include "InputSystem.h"

InputButton::ButtonState InputSystem::GetButtonState(std::string_view key)
{
    if (!m_inputButton[key.data()])
    {
        FNENG_ASSERT_ERROR_F("InputSystemに登録されていないキー[%s]が指定されています。", key.data());

        return InputButton::ButtonState::eNone;
    }

    return m_inputButton[key.data()]->GetState();
}

void InputSystem::AddButton(std::string_view key, InputButton button)
{
    m_inputButton[key.data()] = std::make_shared<InputButton>(button);
}

void InputSystem::AddButton(std::string_view key, std::shared_ptr<InputButton> button)
{
    m_inputButton[key.data()] = button;
}

void InputSystem::Release()
{
    m_inputButton.clear();
}

void InputSystem::Update()
{
    for (const auto& button : m_inputButton)
    {
        button.second->Update();
    }
}

bool InputSystem::IsPressed(std::string_view key)
{
    return GetButtonState(key) == InputButton::ButtonState::ePressed;
}

bool InputSystem::IsFree(std::string_view key)
{
    return GetButtonState(key) == InputButton::ButtonState::eNone;
}

bool InputSystem::IsHold(std::string_view key)
{
    return GetButtonState(key) == InputButton::ButtonState::eHold;
}

bool InputSystem::IsRelease(std::string_view key)
{
    return GetButtonState(key) == InputButton::ButtonState::eReleased;
}
