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