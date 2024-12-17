#include "InputButton.h"

void InputButton::GetKeyCode(std::list<int>& retKeyCode)
{
    for (const auto& key : m_keyCodeList)
    {
        retKeyCode.emplace_back(key);
    }
}

void InputButton::AddKeyCode(int _keyCode)
{
    m_keyCodeList.emplace_back(_keyCode);
}

void InputButton::AddKeyCode(const std::initializer_list<int>& _keyCode)
{
    for (int keyCode : _keyCode)
    {
        m_keyCodeList.emplace_back(keyCode);
    }
}

void InputButton::AddKeyCode(const std::vector<int>& _keyCode)
{
    for (int keyCode : _keyCode)
    {
        m_keyCodeList.emplace_back(keyCode);
    }
}

void InputButton::Init()
{
    m_state = ButtonState::eNone;
}

void InputButton::Release()
{
    m_keyCodeList.clear();
}

void InputButton::Update()
{
    short keyState = 0;

    // 設定されているキーコードの中で、押されているものがあるかを調べる
    for (int key : m_keyCodeList)
    {
        keyState |= GetAsyncKeyState(key);
    }

    if (keyState & 0x8000)
    {
        // 押されている
        if (m_state == ButtonState::ePressed || m_state == ButtonState::eHold)
        {
            m_state = ButtonState::eHold;
        }
        else
        {
            m_state = ButtonState::ePressed;
        }
    }
    else
    {
        // 押されていない
        if (m_state == ButtonState::eReleased || m_state == ButtonState::eNone)
        {
            m_state = ButtonState::eNone;
        }
        else
        {
            m_state = ButtonState::eReleased;
        }
    }
}
