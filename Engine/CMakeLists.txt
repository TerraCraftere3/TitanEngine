project(engine LANGUAGES CXX)

file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/*.c"
)

add_library(engine SHARED ${ENGINE_SRC})

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(engine PUBLIC cxx_std_23)