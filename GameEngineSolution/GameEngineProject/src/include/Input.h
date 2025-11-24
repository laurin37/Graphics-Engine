#pragma once
#include <windows.h>

class Input
{
public:
    Input();
    ~Input() = default;

    struct MouseState
    {
        int x, y;
        int dx, dy;
    };

    void Initialize(HWND hwnd);
    void Update();

    bool IsKeyDown(int key) const;
    int GetMouseDeltaX() const;
    int GetMouseDeltaY() const;

private:
    HWND m_hwnd;
    bool m_keys[256];
    MouseState m_mouseState;
};
