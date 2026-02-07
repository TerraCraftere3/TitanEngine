#pragma once
#include "Titan/Core.h"
#include "Titan/Core/MathLogExtension.h"
#include "Titan/PCH.h"

namespace Titan
{
    enum LogLevel
    {
        Trace = 0,
        Info,
        Warn,
        Error,
        Critical
    };

    class TI_API Logger
    {
    public:
        virtual ~Logger() = default;

        template <typename... Args>
        void Trace(const char* fmt, Args&&... args)
        {
            Log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Info(const char* fmt, Args&&... args)
        {
            Log(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Warn(const char* fmt, Args&&... args)
        {
            Log(LogLevel::Warn, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Error(const char* fmt, Args&&... args)
        {
            Log(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Critical(const char* fmt, Args&&... args)
        {
            Log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
        }

    protected:
        virtual void LogInternal(LogLevel level, fmt::string_view fmt, fmt::format_args args) = 0;

    private:
        template <typename... Args>
        void Log(LogLevel level, const char* fmt, Args&&... args)
        {
            LogInternal(level, fmt, fmt::make_format_args(args...));
        }
    };

    class TI_API Log
    {
    public:
        static void Init();
        inline static Ref<Logger> GetCoreLogger() { return s_CoreLogger; }
        inline static Ref<Logger> GetClientLogger() { return s_ClientLogger; }

    private:
        static Ref<Logger> s_CoreLogger;
        static Ref<Logger> s_ClientLogger;
    };
} // namespace Titan

#ifdef TI_ENABLE_LOGGING
// ---- Core Logging ----
#define TI_CORE_TRACE(...) ::Titan::Log::GetCoreLogger()->Trace(__VA_ARGS__)
#define TI_CORE_INFO(...) ::Titan::Log::GetCoreLogger()->Info(__VA_ARGS__)
#define TI_CORE_WARN(...) ::Titan::Log::GetCoreLogger()->Warn(__VA_ARGS__)
#define TI_CORE_ERROR(...) ::Titan::Log::GetCoreLogger()->Error(__VA_ARGS__)
#define TI_CORE_CRITICAL(...) ::Titan::Log::GetCoreLogger()->Critical(__VA_ARGS__)

// ---- Client Logging ----
#define TI_TRACE(...) ::Titan::Log::GetClientLogger()->Trace(__VA_ARGS__)
#define TI_INFO(...) ::Titan::Log::GetClientLogger()->Info(__VA_ARGS__)
#define TI_WARN(...) ::Titan::Log::GetClientLogger()->Warn(__VA_ARGS__)
#define TI_ERROR(...) ::Titan::Log::GetClientLogger()->Error(__VA_ARGS__)
#define TI_CRITICAL(...) ::Titan::Log::GetClientLogger()->Critical(__VA_ARGS__)
#else
// ---- Core Logging ----
#define TI_CORE_TRACE(...)
#define TI_CORE_INFO(...)
#define TI_CORE_WARN(...)
#define TI_CORE_ERROR(...) ::Titan::Log::GetCoreLogger()->Error(__VA_ARGS__)
#define TI_CORE_CRITICAL(...) ::Titan::Log::GetCoreLogger()->Critical(__VA_ARGS__)

// ---- Client Logging ----
#define TI_TRACE(...)
#define TI_INFO(...)
#define TI_WARN(...)
#define TI_ERROR(...) ::Titan::Log::GetClientLogger()->Error(__VA_ARGS__)
#define TI_CRITICAL(...) ::Titan::Log::GetClientLogger()->Critical(__VA_ARGS__)
#endif
