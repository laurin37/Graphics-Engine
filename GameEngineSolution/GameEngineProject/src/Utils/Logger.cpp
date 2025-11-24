#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <ctime>

#include "../../include/Utils/EnginePCH.h"
#include "../../include/Utils/Logger.h"

Logger& Logger::Get()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : m_minLevel(Level::Debug)
    , m_fileLoggingEnabled(true)
    , m_consoleEnabled(true)
    , m_consoleHandle(nullptr)
{
    // Get console handle for color output
    m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Create logs directory if it doesn't exist
    std::filesystem::create_directories("logs");

    // Create log file with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);
    
    std::ostringstream filename;
    filename << "logs/engine_"
             << std::put_time(&tm_now, "%Y%m%d_%H%M%S")
             << ".log";

    m_logFile.open(filename.str(), std::ios::out);
    
    if (m_logFile.is_open())
    {
        m_logFile << "=== Graphics Engine Log ===" << std::endl;
        m_logFile << "Session started: " << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << std::endl;
        m_logFile << "============================" << std::endl << std::endl;
    }
}

Logger::~Logger()
{
    if (m_logFile.is_open())
    {
        m_logFile << std::endl << "=== Session ended ===" << std::endl;
        m_logFile.close();
    }
}

void Logger::Debug(const std::string& message)
{
    Log(Level::Debug, message);
}

void Logger::Info(const std::string& message)
{
    Log(Level::Info, message);
}

void Logger::Warning(const std::string& message)
{
    Log(Level::Warning, message);
}

void Logger::Error(const std::string& message, const char* file, int line)
{
    Log(Level::Error, message, file, line);
}

void Logger::Fatal(const std::string& message, const char* file, int line)
{
    Log(Level::Fatal, message, file, line);
}

void Logger::SetMinLevel(Level level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

void Logger::EnableFileLogging(bool enabled)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_fileLoggingEnabled = enabled;
}

void Logger::EnableConsoleOutput(bool enabled)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleEnabled = enabled;
}

void Logger::Log(Level level, const std::string& message, const char* file, int line)
{
    // Filter by minimum level
    if (level < m_minLevel)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    std::string timestamp = GetTimestamp();
    std::string levelStr = LevelToString(level);
    
    // Build the formatted message
    std::ostringstream formatted;
    formatted << "[" << timestamp << "] [" << levelStr << "] " << message;
    
    // Add file and line info for errors and fatal errors
    if ((level == Level::Error || level == Level::Fatal) && file != nullptr)
    {
        // Extract just the filename from the full path
        std::string filename = file;
        size_t lastSlash = filename.find_last_of("\\/");
        if (lastSlash != std::string::npos)
            filename = filename.substr(lastSlash + 1);
        
        formatted << " [" << filename << ":" << line << "]";
    }

    std::string finalMessage = formatted.str();

    // Output to Visual Studio Debug Output
    OutputDebugStringA((finalMessage + "\n").c_str());

    // Output to console with color
    if (m_consoleEnabled)
    {
        SetConsoleColor(level);
        std::cout << finalMessage << std::endl;
        ResetConsoleColor();
    }

    // Output to file
    if (m_fileLoggingEnabled && m_logFile.is_open())
    {
        m_logFile << finalMessage << std::endl;
        m_logFile.flush(); // Ensure it's written immediately
    }
}

std::string Logger::GetTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::LevelToString(Level level) const
{
    switch (level)
    {
    case Level::Debug:   return "DEBUG";
    case Level::Info:    return "INFO ";
    case Level::Warning: return "WARN ";
    case Level::Error:   return "ERROR";
    case Level::Fatal:   return "FATAL";
    default:             return "?????";
    }
}

void Logger::SetConsoleColor(Level level)
{
    if (m_consoleHandle == nullptr)
        return;

    HANDLE handle = static_cast<HANDLE>(m_consoleHandle);
    
    switch (level)
    {
    case Level::Debug:
        SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY); // Gray
        break;
    case Level::Info:
        SetConsoleTextAttribute(handle, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Cyan
        break;
    case Level::Warning:
        SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow
        break;
    case Level::Error:
        SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_INTENSITY); // Bright Red
        break;
    case Level::Fatal:
        SetConsoleTextAttribute(handle, FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_INTENSITY); // Red on Red background
        break;
    }
}

void Logger::ResetConsoleColor()
{
    if (m_consoleHandle == nullptr)
        return;

    HANDLE handle = static_cast<HANDLE>(m_consoleHandle);
    SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // White
}
