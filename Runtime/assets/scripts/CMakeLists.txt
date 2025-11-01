project(Titan-GameCore LANGUAGES CSharp)

file(GLOB_RECURSE GAMECORE_SOURCES CONFIGURE_DEPENDS
    "*.cs"
)

add_library(Titan-GameCore SHARED ${GAMECORE_SOURCES})

add_dependencies(Titan-GameCore Titan-ScriptCore)

set_property(TARGET Titan-GameCore PROPERTY VS_DOTNET_REFERENCES
    "System"
    "System.Core"
    "System.Xml"
    "System.Xml.Linq"
    "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts/ScriptCore.dll"
)

add_custom_command(TARGET Titan-GameCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Titan-GameCore>"
        "${CMAKE_SOURCE_DIR}/Runtime/Resources/Scripts/Sandbox.dll"
)