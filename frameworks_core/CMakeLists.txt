# Backend-agnostic sources (compiled for every backend)
target_sources(DeclarativeUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/LayoutFlags.cpp
)

if(UI_FRAMEWORK STREQUAL "WX")
    add_subdirectory(wx)
elseif(UI_FRAMEWORK STREQUAL "QT")
    add_subdirectory(qt)
elseif(UI_FRAMEWORK STREQUAL "IMGUI")
    add_subdirectory(imgui)
endif()
