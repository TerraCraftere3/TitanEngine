#pragma once

// ---- Standard library ----
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <chrono>
#include <cstdint>

// ---- Dependencies ----
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <GLFW/glfw3.h>

// ---- Internal ----
#include "Terra/Core.h"
#include "Terra/Log.h"

// ---- Platform ----
#ifdef TERRA_PLATFORM_WINDOWS
#include <Windows.h>
#endif