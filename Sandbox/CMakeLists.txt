project(sandbox LANGUAGES CXX)

file(GLOB_RECURSE SANDBOX_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Sandbox/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Sandbox/*.c"
)

add_executable(sandbox ${SANDBOX_SRC})

target_include_directories(sandbox PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(sandbox PUBLIC cxx_std_23)

target_link_libraries(sandbox PUBLIC engine)
