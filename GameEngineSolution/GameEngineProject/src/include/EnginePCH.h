#pragma once

#include <stdexcept>
#include <windows.h> // For HRESULT
#include <string>    // For std::string
#include <format>    // For std::format (C++20)
#include <comdef.h>  // For _com_error
#include "Logger.h"  // Logging system

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        _com_error err(hr);
        const wchar_t* wcs = err.ErrorMessage();
        
        // Calculate buffer size
        int size = WideCharToMultiByte(CP_UTF8, 0, wcs, -1, NULL, 0, NULL, NULL);
        std::string errorMessage(size - 1, 0);
        
        // Convert wstring to string
        WideCharToMultiByte(CP_UTF8, 0, wcs, -1, &errorMessage[0], size, NULL, NULL);

        throw std::runtime_error(std::format("DirectX Error: {} (HRESULT: {:#x})", errorMessage, static_cast<unsigned int>(hr)));
    }
}
