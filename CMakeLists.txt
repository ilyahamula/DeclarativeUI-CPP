# Ensure dependencies see a full major.minor policy version (some Vcpkg configs
# error out if this is just "3").
if(NOT CMAKE_POLICY_VERSION_MINIMUM OR CMAKE_POLICY_VERSION_MINIMUM STREQUAL "3")
    set(CMAKE_POLICY_VERSION_MINIMUM "3.16" CACHE STRING "" FORCE)
endif()

cmake_minimum_required(VERSION 3.16)

project(DeclarativeUI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep the root-level compile_commands.json symlink pointing at the active
# backend build so VSCode IntelliSense highlights the correct #ifdef blocks.
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UI_FRAMEWORK "WX" CACHE STRING "UI framework to use")
set_property(CACHE UI_FRAMEWORK PROPERTY STRINGS WX QT IMGUI)

option(USE_LOGGER "Enable logging" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# ── Core library ─────────────────────────────────────────────────────────────

add_library(DeclarativeUI STATIC
    frameworks_core/LayoutFlags.cpp
    frameworks_core/ControlWrapper.cpp
    frameworks_core/ControlWrappers.cpp
    frameworks_core/GroupBoxWrapper.cpp
    frameworks_core/DialogWrapper.cpp
    frameworks_core/LayoutWrapper.cpp
    logger/Logger.cpp
)

# include/ holds the public API headers; root exposes frameworks_core/ and logger/
target_include_directories(DeclarativeUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/logger
)

if(USE_LOGGER)
    target_compile_definitions(DeclarativeUI PUBLIC USE_LOGGER)
endif()

# ── Backend configuration ─────────────────────────────────────────────────────

if(UI_FRAMEWORK STREQUAL "WX")
    # Prefer modern CMake package config; fall back to legacy module for platforms like macOS
    # where Homebrew ships the module by default.
    find_package(wxWidgets QUIET CONFIG COMPONENTS core base)
    if(NOT wxWidgets_FOUND)
        find_package(wxWidgets REQUIRED COMPONENTS core base)
        add_library(wxwidgets_port INTERFACE)
        target_include_directories(wxwidgets_port INTERFACE ${wxWidgets_INCLUDE_DIRS})
        target_link_libraries(wxwidgets_port INTERFACE ${wxWidgets_LIBRARIES})
        target_compile_definitions(wxwidgets_port INTERFACE ${wxWidgets_DEFINITIONS})
        set(WX_TARGET wxwidgets_port)
    else()
        set(WX_TARGET wx::core wx::base)
    endif()
    target_link_libraries(DeclarativeUI PUBLIC ${WX_TARGET})
    target_compile_definitions(DeclarativeUI PUBLIC USE_WX)

elseif(UI_FRAMEWORK STREQUAL "QT")
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    target_link_libraries(DeclarativeUI PUBLIC Qt6::Widgets)
    target_compile_definitions(DeclarativeUI PUBLIC USE_QT)

elseif(UI_FRAMEWORK STREQUAL "IMGUI")
    include(FetchContent)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.6
    )
    FetchContent_MakeAvailable(imgui)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    find_package(OpenGL REQUIRED)

    add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)

    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
    )
    FetchContent_GetProperties(stb)
    if(NOT stb_POPULATED)
        FetchContent_Populate(stb)
    endif()

    target_link_libraries(DeclarativeUI PUBLIC imgui_lib)
    target_include_directories(DeclarativeUI PRIVATE ${stb_SOURCE_DIR})
    target_compile_definitions(DeclarativeUI PUBLIC USE_IMGUI)

else()
    message(FATAL_ERROR "Unknown UI_FRAMEWORK: ${UI_FRAMEWORK}. Use WX, QT, or IMGUI.")
endif()

# ── Examples ──────────────────────────────────────────────────────────────────

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
