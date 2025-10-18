cmake_minimum_required(VERSION 3.16)
project(engine LANGUAGES CXX)

# ---- Sources ----
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Titan/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Titan/*.c"
)

# ---- Library ----
add_library(engine SHARED ${ENGINE_SRC})

# ---- Dependencies ----
target_link_libraries(engine PUBLIC spdlog glfw glad imgui)

# ---- Include directories ----
target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# ---- C++ Standard ----
target_compile_features(engine PUBLIC cxx_std_23)

# ---- Precompiled Headers ----
target_precompile_headers(engine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Titan/PCH.h"
)

# ---- Platform detection ----
if(WIN32)
    target_compile_definitions(engine PUBLIC TI_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(engine PUBLIC TI_PLATFORM_MAC)
elseif(UNIX)
    target_compile_definitions(engine PUBLIC TI_PLATFORM_LINUX)
endif()

# ---- Debug/Release detection ----
target_compile_definitions(engine PUBLIC
    $<$<CONFIG:Debug>:TI_BUILD_DEBUG>
    $<$<CONFIG:Release>:TI_BUILD_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:TI_BUILD_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:TI_BUILD_RELEASE_WITH_DEBUG_INFO>
    $<$<CONFIG:MinSizeRel>:TI_BUILD_RELEASE>
    $<$<CONFIG:MinSizeRel>:TI_BUILD_RELEASE_MINIMAL_SIZE>
)


# ---- DLL export macro ----
target_compile_definitions(engine PRIVATE TI_BUILD_DLL)