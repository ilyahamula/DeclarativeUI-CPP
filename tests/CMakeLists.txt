# Backend-free unit tests (docs/specs/custom_layout_system/tasks.md).
# layout_tests deliberately does NOT link DeclarativeUI: the layout engine and
# its tests must compile without any backend, which this target enforces.

add_executable(layout_tests
    test_main.cpp
    smoke_test.cpp
)

target_include_directories(layout_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
)

add_test(NAME layout_tests COMMAND layout_tests)
