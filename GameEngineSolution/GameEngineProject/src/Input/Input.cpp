#include "../../include/Input/Input.h"

Input::Input() : m_hwnd(nullptr)
{
    ZeroMemory(m_keys, sizeof(m_keys));
    ZeroMemory(&m_mouseState, sizeof(m_mouseState));

    // Default Bindings
    BindAction(Action::MoveForward, 'W');
    BindAction(Action::MoveBackward, 'S');
    BindAction(Action::MoveLeft, 'A');
    BindAction(Action::MoveRight, 'D');
    BindAction(Action::Jump, VK_SPACE);
    BindAction(Action::Fire, VK_LBUTTON);
    BindAction(Action::AltFire, VK_RBUTTON);
    BindAction(Action::Reload, 'R');
    BindAction(Action::Quit, VK_ESCAPE);
}

void Input::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;
}

void Input::Update()
{
    // Update keyboard state
    for (int i = 0; i < 256; ++i)
    {
        m_keys[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }

    // Update mouse state for relative movement
    POINT currentPos;
    GetCursorPos(&currentPos);
    
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    // Calculate the screen coordinates of the window's center
    POINT center = { (clientRect.right - clientRect.left) / 2, (clientRect.bottom - clientRect.top) / 2 };
    ClientToScreen(m_hwnd, &center);

    // Calculate delta from the center of the screen
    m_mouseState.dx = currentPos.x - center.x;
    m_mouseState.dy = currentPos.y - center.y;

    // Reset cursor to the center of the window
    SetCursorPos(center.x, center.y);
}

bool Input::IsKeyDown(int key) const
{
    return m_keys[key];
}

bool Input::IsMouseButtonDown(int button) const
{
    return m_keys[button];
}

int Input::GetMouseDeltaX() const
{
    return m_mouseState.dx;
}

int Input::GetMouseDeltaY() const
{
    return m_mouseState.dy;
}

void Input::BindAction(Action action, int key)
{
    m_actionBindings[action] = key;
}

bool Input::IsActionDown(Action action) const
{
    auto it = m_actionBindings.find(action);
    if (it != m_actionBindings.end())
    {
        return IsKeyDown(it->second);
    }
    return false;
}
