# Ensure dependencies see a full major.minor policy version (some Vcpkg configs
# error out if this is just "3").
if(NOT CMAKE_POLICY_VERSION_MINIMUM OR CMAKE_POLICY_VERSION_MINIMUM STREQUAL "3")
    set(CMAKE_POLICY_VERSION_MINIMUM "3.16" CACHE STRING "" FORCE)
endif()

cmake_minimum_required(VERSION 3.16)

project(DeclarativeUI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UI_FRAMEWORK "WX" CACHE STRING "UI framework to use")
set_property(CACHE UI_FRAMEWORK PROPERTY STRINGS WX QT IMGUI)

option(USE_LOGGER "Enable logging" ON)

add_executable(DeclarativeUI
    main.cpp
    src/frameworks_core/LayoutFlags.cpp
    src/frameworks_core/ControlWrapper.cpp
    src/frameworks_core/ControlWrappers.cpp
    src/frameworks_core/GroupBoxWrapper.cpp
    src/frameworks_core/DialogWrapper.cpp
    src/frameworks_core/LayoutWrapper.cpp
    logger/Logger.cpp
)
target_include_directories(DeclarativeUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/logger
)

if(USE_LOGGER)
    target_compile_definitions(DeclarativeUI PRIVATE USE_LOGGER)
endif()

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
    target_link_libraries(DeclarativeUI PRIVATE ${WX_TARGET})
    target_compile_definitions(DeclarativeUI PRIVATE USE_WX)

elseif(UI_FRAMEWORK STREQUAL "QT")
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    target_link_libraries(DeclarativeUI PRIVATE Qt6::Widgets)
    target_compile_definitions(DeclarativeUI PRIVATE USE_QT)

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

    target_link_libraries(DeclarativeUI PRIVATE imgui_lib)
    target_compile_definitions(DeclarativeUI PRIVATE USE_IMGUI)

else()
    message(FATAL_ERROR "Unknown UI_FRAMEWORK: ${UI_FRAMEWORK}. Use WX, QT, or IMGUI.")
endif()
