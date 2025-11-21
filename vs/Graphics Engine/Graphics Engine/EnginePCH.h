#pragma once

#include <stdexcept>
#include <windows.h> // For HRESULT
#include <string>    // For std::string
#include <format>    // For std::format (C++20)
#include <comdef.h>  // For _com_error

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        _com_error err(hr);
        std::wstring ws(err.ErrorMessage());
        std::string errorMessage(ws.begin(), ws.end());
        throw std::runtime_error(std::format("DirectX Error: {} (HRESULT: {:#x})", errorMessage, static_cast<unsigned int>(hr)));
    }
}
