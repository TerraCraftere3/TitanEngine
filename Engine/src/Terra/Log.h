#pragma once
#include "PCH.h"
#include "Core.h"

namespace Terra
{
    class TERRA_API Log
    {
    public:
        static void Init();
        inline static std::shared_ptr<spdlog::logger> GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger> GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}

#ifdef TERRA_BUILD_DEBUG
// ---- Core Logging ----
#define LOG_CORE_TRACE(...) ::Terra::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) ::Terra::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...) ::Terra::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Terra::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Terra::Log::GetCoreLogger()->critical(__VA_ARGS__)

// ---- Client Logging ----
#define LOG_TRACE(...) ::Terra::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::Terra::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::Terra::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Terra::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Terra::Log::GetClientLogger()->critical(__VA_ARGS__)
#else
// ---- Core Logging ----
#define LOG_CORE_TRACE(...)
#define LOG_CORE_INFO(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_ERROR(...) ::Terra::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Terra::Log::GetClientLogger()->critical(__VA_ARGS__)

// ---- Client Logging ----
#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...) ::Terra::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Terra::Log::GetClientLogger()->critical(__VA_ARGS__)
#endif