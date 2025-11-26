#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

class Logger
{
public:
    enum class Level
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    // Singleton access
    static Logger& Get();

    // Logging methods
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message, const char* file = nullptr, int line = 0);
    void Fatal(const std::string& message, const char* file = nullptr, int line = 0);

    // Configuration
    void SetMinLevel(Level level);
    void EnableFileLogging(bool enabled);
    void EnableConsoleOutput(bool enabled);

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    void Log(Level level, const std::string& message, const char* file = nullptr, int line = 0);
    std::string GetTimestamp() const;
    std::string LevelToString(Level level) const;
    void SetConsoleColor(Level level);
    void ResetConsoleColor();

    Level m_minLevel;
    bool m_fileLoggingEnabled;
    bool m_consoleEnabled;
    std::ofstream m_logFile;
    std::mutex m_mutex;
    void* m_consoleHandle; // HANDLE type (void* to avoid including windows.h in header)
};

// Convenience macros for easy logging
#define LOG_DEBUG(msg) Logger::Get().Debug(msg)
#define LOG_INFO(msg) Logger::Get().Info(msg)
#define LOG_WARNING(msg) Logger::Get().Warning(msg)
#define LOG_ERROR(msg) Logger::Get().Error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Logger::Get().Fatal(msg, __FILE__, __LINE__)
