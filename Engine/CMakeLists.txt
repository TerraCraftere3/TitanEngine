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

message(STATUS "Building Titan Engine as a Shared Lib is turned ${TI_BUILD_SHARED}")
message(STATUS "Slang Libraries: ${SLANG_LIB_DIR}")
message(STATUS "Slang Include: ${SLANG_INCLUDE_DIR}")
message(STATUS "Mono Libraries: ${MONO_LIB_DIR}")
message(STATUS "Mono Include: ${MONO_INCLUDE_DIR}")
message(STATUS "Filewatch Include: ${FILEWATCH_INCLUDE_DIR}")
message(STATUS "NanoSVG Include: ${NANOSVG_INCLUDE_DIR}")
message(STATUS "STB Include: ${STB_INCLUDE_DIR}")
message(STATUS "PhysX Root: ${PHYSX_ROOT_DIR}")
message(STATUS "PhysX Include: ${PHYSX_INCLUDE_DIR}")
message(STATUS "PhysX Libraries: ${PHYSX_BIN_DIR}")
message(STATUS "ImReflect Include: ${IMREFLECT_INCLUDE_DIR}")

# ---- Dependencies ----
target_link_libraries(engine PUBLIC spdlog glfw imgui glm EnTT yaml-cpp imguizmo implot box2d OptickCore assimp OpenAL SndFile::sndfile)
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

target_link_libraries(engine PUBLIC
    "${PHYSX_BIN_DIR}/PhysXExtensions_static_64.lib"
    "${PHYSX_BIN_DIR}/PhysX_64.lib"
    "${PHYSX_BIN_DIR}/PhysXPvdSDK_static_64.lib"
    "${PHYSX_BIN_DIR}/PhysXCommon_64.lib"
    "${PHYSX_BIN_DIR}/PhysXFoundation_64.lib"
    "${PHYSX_BIN_DIR}/PhysXCooking_64.lib"
)

file(GLOB PHYSX_DLLS
    "${PHYSX_BIN_DIR}/PhysX_64.dll"
    "${PHYSX_BIN_DIR}/PhysXCommon_64.dll"
    "${PHYSX_BIN_DIR}/PhysXCooking_64.dll"
    "${PHYSX_BIN_DIR}/PhysXFoundation_64.dll"
    "${PHYSX_BIN_DIR}/PVDRuntime_64.dll"
)

get_target_property(ENGINE_OUTPUT_DIR engine RUNTIME_OUTPUT_DIRECTORY)
if(NOT ENGINE_OUTPUT_DIR)
    set(ENGINE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Engine")
endif()

# Loop and add a post-build copy for each DLL
foreach(_dll ${PHYSX_DLLS})
    add_custom_command(TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_dll}"
            "${ENGINE_OUTPUT_DIR}"
    )
endforeach()
# ---- Include directories ----
target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${STB_INCLUDE_DIR}"
    "${NANOSVG_INCLUDE_DIR}"
    "${SLANG_INCLUDE_DIR}"
    "${MONO_INCLUDE_DIR}"
    "${FILEWATCH_INCLUDE_DIR}"
    "${PHYSX_INCLUDE_DIR}"
    "${IMREFLECT_INCLUDE_DIR}"
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
    $<$<CONFIG:RelWithDebInfo>:TI_BUILD_DEBUG_OPTIMIZED>
    $<$<CONFIG:Release>:TI_BUILD_RELEASE>
    $<$<CONFIG:MinSizeRel>:TI_BUILD_RELEASE>
    $<$<CONFIG:MinSizeRel>:TI_BUILD_RELEASE_MINIMAL_SIZE>
    NOMINMAX
)
target_compile_options(engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4068>
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
