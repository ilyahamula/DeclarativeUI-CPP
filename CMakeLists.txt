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


#option(USE_WX_WIDGETS "Fetch and link wxWidgets" ON)

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

add_executable(DeclarativeUI main.cpp)
target_include_directories(DeclarativeUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(DeclarativeUI PRIVATE ${WX_TARGET})
target_compile_definitions(DeclarativeUI PRIVATE USE_WX_WIDGETS)
