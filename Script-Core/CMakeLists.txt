cmake_minimum_required(VERSION 3.8)

project(Titan-ScriptCore LANGUAGES CSharp)
file(GLOB_RECURSE SCRIPTCORE_SOURCES
    "src/*.cs"
    "props/*.cs"
)

add_library(Titan-ScriptCore SHARED ${SCRIPTCORE_SOURCES})

set_target_properties(Titan-ScriptCore PROPERTIES
    OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts"
)

set(CMAKE_CSharp_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts/Intermediates")

set_property(TARGET Titan-ScriptCore PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")

set_property(TARGET Titan-ScriptCore PROPERTY VS_DOTNET_REFERENCES
    "System"
    "System.Core"
    "System.Xml"
    "System.Xml.Linq"
)

set_property(TARGET Titan-ScriptCore PROPERTY VS_GLOBAL_Optimize "false")
set_property(TARGET Titan-ScriptCore PROPERTY VS_GLOBAL_DebugSymbols "true")
add_custom_command(TARGET Titan-ScriptCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Titan-ScriptCore>"
        "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts/ScriptCore.dll"
)