#pragma once
// clang-format off

// ---- Standard library ----
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
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

// ---- Dependencies ----
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <imgui.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

// ---- Internal ----
#include "Titan/Core.h"
#include "Titan/Core/Log.h"

// ---- Platform ----
#ifdef TI_PLATFORM_WINDOWS
#include <Windows.h>
#endif
// clang-format on