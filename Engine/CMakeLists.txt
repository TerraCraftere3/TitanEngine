cmake_minimum_required(VERSION 3.16)
project(engine LANGUAGES CXX)

# ---- Options ----
option(TI_BUILD_SHARED "Build Titan Engine as a shared library" ON)

# ---- Sources ----
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Titan/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Titan/*.c"
)

# ---- Library ----
if (TI_BUILD_SHARED)
    add_library(engine SHARED ${ENGINE_SRC})
    target_compile_definitions(engine PUBLIC TI_DYNAMIC_LINK)
    target_compile_definitions(engine PRIVATE TI_BUILD_DLL)
else()
    add_library(engine STATIC ${ENGINE_SRC})
    target_compile_definitions(engine PUBLIC TI_STATIC_LINK)
endif()

message(STATUS "Slang Libraries: ${SLANG_LIB_DIR}")
message(STATUS "Slang Include: ${SLANG_INCLUDE_DIR}")
message(STATUS "Mono Libraries: ${MONO_LIB_DIR}")
message(STATUS "Mono Include: ${MONO_INCLUDE_DIR}")

# ---- Dependencies ----
target_link_libraries(engine PUBLIC spdlog glfw imgui glm EnTT yaml-cpp imguizmo box2d OptickCore)
target_link_libraries(engine PRIVATE glad "${SLANG_LIB_DIR}/slang.lib" "${MONO_LIB_DIR}/libmono-static-sgen.lib")
if(MSVC)
    target_link_libraries(engine PRIVATE
    msvcrt.lib
    ucrt.lib
    oldnames.lib
    ws2_32.lib
    bcrypt.lib
    kernel32.lib
    user32.lib
    advapi32.lib
    version.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    winmm.lib
    psapi.lib
)
endif()

add_dependencies(engine Titan-ScriptCore)
add_dependencies(engine Titan-GameCore)

# ---- Include directories ----
target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${STB_INCLUDE_DIR}"
    "${NANOSVG_INCLUDE_DIR}"
    "${SLANG_INCLUDE_DIR}"
    "${MONO_INCLUDE_DIR}"
    "${FILEWATCH_INCLUDE_DIR}"
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

# ---- Definitions ----
target_compile_definitions(engine PUBLIC
    $<$<CONFIG:Debug>:TI_BUILD_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:TI_BUILD_DEBUG>
    $<$<CONFIG:Release>:TI_BUILD_RELEASE>
    $<$<CONFIG:MinSizeRel>:TI_BUILD_RELEASE>
    $<$<CONFIG:MinSizeRel>:TI_BUILD_RELEASE_MINIMAL_SIZE>
    NOMINMAX
)

if (MSVC)
    target_compile_options(engine PRIVATE
        $<$<CONFIG:Debug>:/arch:AVX2>
    )
else()
    target_compile_options(engine PRIVATE
        $<$<CONFIG:Debug>:-mavx2>
        $<$<CONFIG:Debug>:-mfma>
    )
endif()

# ---- Output Name ----
set_target_properties(engine PROPERTIES OUTPUT_NAME "Titan")
