# Backend-free unit tests (docs/specs/custom_layout_system/tasks.md).
# layout_tests deliberately does NOT link DeclarativeUI: the layout engine and
# its tests must compile without any backend, which this target enforces.

add_executable(layout_tests
    test_main.cpp
    smoke_test.cpp
    layout_flags_test.cpp
    layout_node_test.cpp
    mock_backend_test.cpp
    layout_engine_measure_test.cpp
    layout_engine_sizegroup_test.cpp
    layout_engine_arrange_test.cpp
    layout_engine_grid_test.cpp
    layout_engine_scroll_test.cpp
    layout_engine_splitter_test.cpp
    layout_engine_expander_test.cpp
    layout_engine_hidden_test.cpp
    layout_engine_foreach_test.cpp
    golden_cross_backend_test.cpp
    shortcut_parse_test.cpp
    file_filter_parse_test.cpp
    richtext_parse_test.cpp
    ref_watch_test.cpp
    calendar_math_test.cpp
    # Engine sources under test are compiled directly (not linked from
    # DeclarativeUI) so they provably build without any backend.
    ${CMAKE_SOURCE_DIR}/frameworks_core/LayoutFlags.cpp
    ${CMAKE_SOURCE_DIR}/frameworks_core/LayoutEngine.cpp
)

target_include_directories(layout_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
)

add_test(NAME layout_tests COMMAND layout_tests)

# Backend behaviour tests: the opposite of layout_tests. These link the real
# library for the configured backend and drive real native widgets (ImGui
# headless, Qt on the offscreen platform, wx in a real or virtual display), to
# catch what an engine-only test cannot -- grouping, labels, popups, bindings.
option(BUILD_BACKEND_TESTS "Build the per-backend behaviour tests" ON)
if(BUILD_BACKEND_TESTS)
    if(UI_FRAMEWORK STREQUAL "IMGUI")
        set(BACKEND_TEST_SOURCES backend/imgui_backend_test.cpp)
    elseif(UI_FRAMEWORK STREQUAL "QT")
        set(BACKEND_TEST_SOURCES backend/qt_backend_test.cpp)
    elseif(UI_FRAMEWORK STREQUAL "WX")
        set(BACKEND_TEST_SOURCES backend/wx_backend_test.cpp)
        if(APPLE)
            list(APPEND BACKEND_TEST_SOURCES backend/wx_native_click.mm)
        endif()
    endif()

    add_executable(backend_tests ${BACKEND_TEST_SOURCES})
    target_link_libraries(backend_tests PRIVATE DeclarativeUI)
    # An icon file the tests can load whatever the working directory.
    target_compile_definitions(backend_tests PRIVATE
        DUI_TEST_ICON="${CMAKE_SOURCE_DIR}/images/icon_add.png")
    if(UI_FRAMEWORK STREQUAL "WX" AND APPLE)
        target_link_libraries(backend_tests PRIVATE "-framework AppKit")
    endif()
    if(UI_FRAMEWORK STREQUAL "WX" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Symbol names in the assert handler's backtrace.
        target_link_options(backend_tests PRIVATE -rdynamic)
    endif()
    add_test(NAME backend_tests COMMAND backend_tests)
endif()
