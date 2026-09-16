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
    golden_cross_backend_test.cpp
    shortcut_parse_test.cpp
    file_filter_parse_test.cpp
    # Engine sources under test are compiled directly (not linked from
    # DeclarativeUI) so they provably build without any backend.
    ${CMAKE_SOURCE_DIR}/frameworks_core/LayoutFlags.cpp
    ${CMAKE_SOURCE_DIR}/frameworks_core/LayoutEngine.cpp
)

target_include_directories(layout_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
)

add_test(NAME layout_tests COMMAND layout_tests)
