cmake_minimum_required(VERSION 3.16)
project(editor LANGUAGES CXX)

# ---- Sources ----
file(GLOB_RECURSE EDITOR_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Atlas/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Atlas/*.c"
)

# ---- Executable ----
add_executable(editor ${EDITOR_SRC})

# ---- Include directories ----
target_include_directories(editor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# ---- C++ Standard ----
target_compile_features(editor PUBLIC cxx_std_23)

target_compile_options(editor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4068>
)

# ---- Libraries ----
target_link_libraries(editor PRIVATE engine)

# ---- Output Name ----
set_target_properties(editor PROPERTIES OUTPUT_NAME "Atlas")