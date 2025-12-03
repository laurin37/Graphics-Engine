#include "../../include/Input/Input.h"

Input::Input() : m_hwnd(nullptr)
{
    ZeroMemory(m_keys, sizeof(m_keys));
    ZeroMemory(&m_mouseState, sizeof(m_mouseState));
    m_isMouseLocked = false;
    ShowCursor(TRUE); // Ensure cursor is visible by default

    // Default Bindings
    BindAction(Action::MoveForward, 'W');
    BindAction(Action::MoveBackward, 'S');
    BindAction(Action::MoveLeft, 'A');
    BindAction(Action::MoveRight, 'D');
    BindAction(Action::Jump, VK_SPACE);
    BindAction(Action::Fire, VK_LBUTTON);
    BindAction(Action::AltFire, VK_MBUTTON);
    BindAction(Action::Reload, 'R');
    BindAction(Action::Quit, VK_ESCAPE);
}

void Input::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;
    SetMouseLock(false); // Ensure unlocked and visible by default
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
    if (m_isMouseLocked)
    {
        m_mouseState.dx = currentPos.x - center.x;
        m_mouseState.dy = currentPos.y - center.y;

        // Reset cursor to the center of the window
        SetCursorPos(center.x, center.y);
    }
    else
    {
        m_mouseState.dx = 0;
        m_mouseState.dy = 0;
        
        // Update absolute position for UI
        ScreenToClient(m_hwnd, &currentPos);
        m_mouseState.x = currentPos.x;
        m_mouseState.y = currentPos.y;
    }
}

void Input::SetMouseLock(bool locked)
{
    if (m_isMouseLocked == locked) return;

    m_isMouseLocked = locked;

    if (m_isMouseLocked)
    {
        ShowCursor(FALSE);
        
        // Center cursor immediately to prevent jump
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        POINT center = { (clientRect.right - clientRect.left) / 2, (clientRect.bottom - clientRect.top) / 2 };
        ClientToScreen(m_hwnd, &center);
        SetCursorPos(center.x, center.y);
        
        // Reset delta
        m_mouseState.dx = 0;
        m_mouseState.dy = 0;
    }
    else
    {
        ShowCursor(TRUE);
    }
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

int Input::GetMouseX() const
{
    return m_mouseState.x;
}

int Input::GetMouseY() const
{
    return m_mouseState.y;
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
