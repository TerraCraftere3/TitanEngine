# ---- Find Packages ----
find_package(Vulkan REQUIRED)

# ---- Force Static Libraries and Static Runtime ----
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC runtime library" FORCE)

# ---- Configs ----
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tests" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BOX2D_USER_SETTINGS OFF CACHE BOOL "" FORCE)


# ---- Additional Static Library Configs ----
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)

# ---- Force SPIRV-Tools to build static only ----
set(SPIRV_TOOLS_BUILD_STATIC ON CACHE BOOL "" FORCE)

# ---- Subdirectories ----
add_subdirectory(spdlog)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(entt)
add_subdirectory(yaml-cpp)
add_subdirectory(Box2D)
add_subdirectory(SPIRV-Cross)
add_subdirectory(SPIRV-Headers)
add_subdirectory(SPIRV-Tools)
add_subdirectory(glslang)
add_subdirectory(shaderc)

# ---- Glad Directory ----
file(GLOB_RECURSE GLAD_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/*.c"
)
add_library(glad STATIC ${GLAD_SRC})
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")

# ---- ImGui Directory ----
file(GLOB IMGUI_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
)
# ---- ImGuizmo Directory ----
file(GLOB IMGUIZMO_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/*.cpp"
)
add_library(imguizmo STATIC ${IMGUIZMO_SRC})
target_include_directories(imguizmo PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo"
)
target_link_libraries(imguizmo PRIVATE imgui)

target_link_libraries(imgui PRIVATE glfw)
# ---- STB Directory ----
set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stb" PARENT_SCOPE)
set(NANOSVG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/nanosvg/src" PARENT_SCOPE)