target_sources(DeclarativeUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/LayoutBackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ControlWrappers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DialogWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WindowWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageBoxWrapper.cpp
)

if(APPLE)
    # Objective-C++ helper for the Cocoa button bezel (see OsxButtonBezel.hpp);
    # wx exposes no portable API for it. OBJCXX is enabled by the root
    # CMakeLists -- enabling a language here would not reach the directory
    # that owns the target.
    target_sources(DeclarativeUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/OsxButtonBezel.mm)
    set_target_properties(DeclarativeUI PROPERTIES
        OBJCXX_STANDARD 20
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF
    )
endif()
