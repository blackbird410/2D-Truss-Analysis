# tests/integration/CMakeLists.txt
#
# Integration test binaries:
#   integration_tests     — multi-layer workflow tests, domain + infra stacks.
#   gui_integration_test  — GUI signal/slot wiring tests (BUILD_GUI only).
#
# test_e2e_workflows.cpp has no Qt dependency — it is compiled
# unconditionally so it executes in CLI-only and headless CI configurations.

# ============================================================================
# INTEGRATION TESTS — domain + infrastructure stack (no display)
# ============================================================================
add_executable(integration_tests
    test_simple_truss_analysis.cpp
    test_simple_integration.cpp
    test_working_integration.cpp
    test_minimal_analysis.cpp
    test_cli_workflows.cpp
    test_lifecycle_integration.cpp
    test_load_workflow.cpp
    test_facade_adapters_integration.cpp
    test_e2e_workflows.cpp
)

# TrussCLI transitively exposes TrussInterface → TrussApplication →
# TrussInfrastructure → TrussCore.  Listing them separately produces
# duplicate .a entries on the linker line (ld duplicate-library warning).
target_link_libraries(integration_tests PRIVATE
    TrussCLI
    GTest::gtest_main
    GTest::gmock
    $<$<BOOL:${BUILD_GUI}>:Qt6::Core>
    $<$<BOOL:${BUILD_GUI}>:Qt6::Widgets>
    $<$<BOOL:${BUILD_GUI}>:Qt6::Test>
)

target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

add_test(NAME integration_tests COMMAND integration_tests)
set_tests_properties(integration_tests PROPERTIES
    TIMEOUT 60
    LABELS "integration;gtest"
)
if(ENABLE_COVERAGE)
    set_tests_properties(integration_tests PROPERTIES FIXTURES_REQUIRED gcda_cleanup)
endif()

# ============================================================================
# GUI INTEGRATION TESTS — QApplication + offscreen platform plugin
# Exercises the complete GUI signal/slot wiring (MainWindowController and
# all sub-controllers end-to-end).
# ============================================================================
if(BUILD_GUI)
    add_executable(gui_integration_test
        test_gui_integration.cpp
    )

    # TrussGui owns all GUI sources and PUBLIC-links TrussInterface,
    # TrussApplication, TrussInfrastructure, TrussCore, and the Qt6
    # modules (Core, Widgets, Svg).  Qt6::Test adds QSignalSpy/QTest.
    # GTest::gtest is kept explicitly because this target provides its
    # own main() and therefore does not link GTest::gtest_main.
    target_link_libraries(gui_integration_test PRIVATE
        TrussGui
        GTest::gtest
        GTest::gmock
        Qt6::Test
    )

    target_include_directories(gui_integration_test PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )

    add_test(NAME gui_integration_tests COMMAND gui_integration_test)
    set_tests_properties(gui_integration_tests PROPERTIES
        TIMEOUT 120
        LABELS "gui;integration;gtest"
        # QApplication requires a platform plugin; offscreen satisfies that
        # requirement without a physical display or X11 server.
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
    if(ENABLE_COVERAGE)
        set_tests_properties(gui_integration_tests PROPERTIES FIXTURES_REQUIRED gcda_cleanup)
    endif()
endif()
