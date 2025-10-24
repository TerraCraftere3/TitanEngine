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

# ---- Libraries ----
target_link_libraries(editor PRIVATE engine glad)

# ---- Definitions ----
target_compile_definitions(editor PUBLIC
    GLM_FORCE_INTRINSICS
    GLM_FORCE_AVX2
    GLM_FORCE_INLINE
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
set_target_properties(editor PROPERTIES OUTPUT_NAME "Atlas")