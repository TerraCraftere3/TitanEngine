# ---- Subdirectories ----
add_subdirectory(spdlog)
add_subdirectory(glfw)

# ---- Glad Directory ----
file(GLOB_RECURSE GLAD_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/*.c"
)
add_library(glad STATIC ${GLAD_SRC})
target_include_directories(glad PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/glad/include"
)

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
target_link_libraries(imgui PRIVATE glfw)