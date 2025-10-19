#pragma once
#include "Core.h"
#include "MathLog.h"
#include "PCH.h"

namespace Titan
{
    class TI_API Log
    {
    public:
        static void Init();
        inline static std::shared_ptr<spdlog::logger> GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger> GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
} // namespace Titan

#ifdef TI_ENABLE_LOGGING
// ---- Core Logging ----
#define LOG_CORE_TRACE(...) ::Titan::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) ::Titan::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...) ::Titan::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Titan::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Titan::Log::GetCoreLogger()->critical(__VA_ARGS__)

// ---- Client Logging ----
#define LOG_TRACE(...) ::Titan::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::Titan::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::Titan::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Titan::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Titan::Log::GetClientLogger()->critical(__VA_ARGS__)
#else
// ---- Core Logging ----
#define LOG_CORE_TRACE(...)
#define LOG_CORE_INFO(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_ERROR(...) ::Titan::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Titan::Log::GetClientLogger()->critical(__VA_ARGS__)

// ---- Client Logging ----
#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...) ::Titan::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Titan::Log::GetClientLogger()->critical(__VA_ARGS__)
#endif
