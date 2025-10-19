#pragma once
#include "Titan/Core.h"
#include "Titan/Core/MathLogExtension.h"
#include "Titan/PCH.h"

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
#define TI_CORE_TRACE(...) ::Titan::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TI_CORE_INFO(...) ::Titan::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TI_CORE_WARN(...) ::Titan::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TI_CORE_ERROR(...) ::Titan::Log::GetCoreLogger()->error(__VA_ARGS__)
#define TI_CORE_CRITICAL(...) ::Titan::Log::GetCoreLogger()->critical(__VA_ARGS__)

// ---- Client Logging ----
#define TI_TRACE(...) ::Titan::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TI_INFO(...) ::Titan::Log::GetClientLogger()->info(__VA_ARGS__)
#define TI_WARN(...) ::Titan::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TI_ERROR(...) ::Titan::Log::GetClientLogger()->error(__VA_ARGS__)
#define TI_CRITICAL(...) ::Titan::Log::GetClientLogger()->critical(__VA_ARGS__)
#else
// ---- Core Logging ----
#define TI_CORE_TRACE(...)
#define TI_CORE_INFO(...)
#define TI_CORE_WARN(...)
#define TI_CORE_ERROR(...) ::Titan::Log::GetClientLogger()->error(__VA_ARGS__)
#define TI_CORE_CRITICAL(...) ::Titan::Log::GetClientLogger()->critical(__VA_ARGS__)

// ---- Client Logging ----
#define TI_TRACE(...)
#define TI_INFO(...)
#define TI_WARN(...)
#define TI_ERROR(...) ::Titan::Log::GetClientLogger()->error(__VA_ARGS__)
#define TI_CRITICAL(...) ::Titan::Log::GetClientLogger()->critical(__VA_ARGS__)
#endif
