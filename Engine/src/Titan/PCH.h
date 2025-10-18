#pragma once

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

#include <GLFW/glfw3.h>

// ---- Internal ----
#include "Titan/Core.h"
#include "Titan/Log.h"

// ---- Platform ----
#ifdef TI_PLATFORM_WINDOWS
#include <Windows.h>
#endif