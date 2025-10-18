cmake_minimum_required(VERSION 3.16)
project(engine LANGUAGES CXX)

# ---- Sources ----
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Terra/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Terra/*.c"
)

# ---- Library ----
add_library(engine SHARED ${ENGINE_SRC})

# ---- Dependencies ----
target_link_libraries(engine PUBLIC spdlog glfw)

# ---- Include directories ----
target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# ---- C++ Standard ----
target_compile_features(engine PUBLIC cxx_std_23)

# ---- Precompiled Headers ----
target_precompile_headers(engine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Terra/PCH.h"
)

# ---- Platform detection ----
if(WIN32)
    target_compile_definitions(engine PUBLIC TERRA_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(engine PUBLIC TERRA_PLATFORM_MAC)
elseif(UNIX)
    target_compile_definitions(engine PUBLIC TERRA_PLATFORM_LINUX)
endif()

# ---- Debug/Release detection ----
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PUBLIC TERRA_BUILD_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(engine PUBLIC TERRA_BUILD_RELEASE)
endif()

# ---- DLL export macro ----
target_compile_definitions(engine PRIVATE TERRA_BUILD_DLL)