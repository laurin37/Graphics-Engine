#include "Window.h"
#include <stdexcept>

Window::Window()
    : m_hWnd(nullptr), m_hInstance(nullptr), m_width(0), m_height(0)
{
}

Window::~Window()
{
    if (m_hInstance)
    {
        UnregisterClassW(m_windowClass.c_str(), m_hInstance);
    }
}

void Window::Initialize(
    HINSTANCE hInstance,
    int nCmdShow,
    const std::wstring& windowTitle,
    const std::wstring& windowClass,
    int width,
    int height)
{
    m_hInstance = hInstance;
    m_windowTitle = windowTitle;
    m_windowClass = windowClass;
    m_width = width;
    m_height = height;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Window::WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = m_windowClass.c_str();
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
    {
        throw std::runtime_error("Failed to register window class.");
    }

    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowExW(
        0,
        m_windowClass.c_str(),
        m_windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hWnd)
    {
        throw std::runtime_error("Failed to create window.");
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

HWND Window::GetHWND() const
{
    return m_hWnd;
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* pWindow = nullptr;

    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pWindow);
        pWindow->m_hWnd = hWnd;
    }
    else
    {
        pWindow = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pWindow)
    {
        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}
