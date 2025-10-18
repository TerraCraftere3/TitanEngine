cmake_minimum_required(VERSION 3.16)
project(sandbox LANGUAGES CXX)

# ---- Sources ----
file(GLOB_RECURSE SANDBOX_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Sandbox/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Sandbox/*.c"
)

# ---- Executable ----
add_executable(sandbox ${SANDBOX_SRC})

# ---- Include directories ----
target_include_directories(sandbox PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# ---- C++ Standard ----
target_compile_features(sandbox PUBLIC cxx_std_23)

# ---- Libraries ----
target_link_libraries(sandbox PUBLIC engine)
