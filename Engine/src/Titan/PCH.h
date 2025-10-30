#pragma once
// clang-format off

/*===========================================
        Precompiled Header (PCH)
        Includes:
        - Standard Library
        - Platform Specific Headers
        - Dependencies
        - Core Engine Headers
===========================================*/

// ---- Standard Library ----
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// ---- Platform ----
#ifdef TI_PLATFORM_WINDOWS
    #include <Windows.h>
#endif

// ---- Logging (spdlog) ----
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// ---- ImGui + ImGuizmo ----
#include <imgui.h>
#include <ImGuizmo.h>

// ---- Math (GLM) ----
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

// ---- Internal Engine Includes ----
#include "Titan/Core.h"
#include "Titan/Debug/Macros.h"
#include "Titan/Debug/Instrumentor.h"
#include "Titan/Core/Log.h"
// clang-format on