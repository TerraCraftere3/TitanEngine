#pragma once

#include "Titan/Events/Event.h"
#include "Titan/PCH.h"

#include <spdlog/fmt/bundled/format.h>

// --- Vec types ---
template <>
struct fmt::formatter<glm::vec2> : fmt::formatter<std::string>
{
    auto format(const glm::vec2& v, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(v));
    }
};

template <>
struct fmt::formatter<glm::vec3> : fmt::formatter<std::string>
{
    auto format(const glm::vec3& v, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(v));
    }
};

template <>
struct fmt::formatter<glm::vec4> : fmt::formatter<std::string>
{
    auto format(const glm::vec4& v, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(v));
    }
};

// --- Ivec types ---
template <>
struct fmt::formatter<glm::ivec2> : fmt::formatter<std::string>
{
    auto format(const glm::ivec2& v, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(v));
    }
};

template <>
struct fmt::formatter<glm::ivec3> : fmt::formatter<std::string>
{
    auto format(const glm::ivec3& v, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(v));
    }
};

template <>
struct fmt::formatter<glm::ivec4> : fmt::formatter<std::string>
{
    auto format(const glm::ivec4& v, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(v));
    }
};

// --- Mat types ---
template <>
struct fmt::formatter<glm::mat2> : fmt::formatter<std::string>
{
    auto format(const glm::mat2& m, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(m));
    }
};

template <>
struct fmt::formatter<glm::mat3> : fmt::formatter<std::string>
{
    auto format(const glm::mat3& m, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(m));
    }
};

template <>
struct fmt::formatter<glm::mat4> : fmt::formatter<std::string>
{
    auto format(const glm::mat4& m, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(m));
    }
};

// --- Quaternion ---
template <>
struct fmt::formatter<glm::quat> : fmt::formatter<std::string>
{
    auto format(const glm::quat& q, fmt::format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", glm::to_string(q));
    }
};
