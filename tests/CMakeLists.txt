# tests/CMakeLists.txt
#
# Test suite root.
# Defines the GTest smoke test, then delegates to unit/ and integration/
# for the full test suite.  A convenience meta-target all_gtest_tests
# aggregates every test binary so that a single build command checks
# everything at once.

# ============================================================================
# GTEST SMOKE TEST — no project dependencies
# ============================================================================
add_executable(test_gtest_integration test_gtest_integration.cpp)

target_link_libraries(test_gtest_integration PRIVATE
    GTest::gtest_main
)

add_test(NAME gtest_integration COMMAND test_gtest_integration)
set_tests_properties(gtest_integration PROPERTIES
    TIMEOUT 10
    LABELS "gtest;integration"
)

# ============================================================================
# MODULE TEST SUITES
# ============================================================================
add_subdirectory(unit)
add_subdirectory(integration)

# ============================================================================
# META-TARGET: all_gtest_tests
# Depends on every test binary defined across the test tree.
# ============================================================================
add_custom_target(all_gtest_tests
    DEPENDS unit_tests integration_tests test_gtest_integration
    COMMENT "Building all GTest-based tests"
)

if(TARGET unit_tests_gui_widgets)
    add_dependencies(all_gtest_tests unit_tests_gui_widgets)
endif()

if(TARGET gui_integration_test)
    add_dependencies(all_gtest_tests gui_integration_test)
endif()

# ============================================================================
# COVERAGE: stale .gcda cleanup fixture
#
# When the project is rebuilt without deleting .gcda files, the gcov runtime
# detects a checksum mismatch between the stale file and the new binary and
# calls abort() — surfaced by CTest as "Exception: SegFault".  A
# FIXTURES_SETUP test deletes all .gcda files before any test binary runs so
# every run starts from a clean coverage slate.
#
# This block MUST appear after add_subdirectory(unit) and
# add_subdirectory(integration) so all test names are already registered.
# ============================================================================
if(ENABLE_COVERAGE)
    add_test(NAME cleanup_gcda
        COMMAND ${CMAKE_COMMAND}
                -DBUILD_DIR=${CMAKE_BINARY_DIR}
                -P ${CMAKE_SOURCE_DIR}/cmake/CleanGcda.cmake
    )
    set_tests_properties(cleanup_gcda PROPERTIES
        FIXTURES_SETUP  gcda_cleanup
        LABELS          "coverage_setup"
    )

    # gtest_integration is defined in this same file — can set properties here.
    # unit_tests, integration_tests, and gui variants are defined in their own
    # subdirectory CMakeLists.txt; their FIXTURES_REQUIRED is set there instead
    # (CMake 4.x scopes test-property visibility to the defining directory).
    set_tests_properties(gtest_integration PROPERTIES
        FIXTURES_REQUIRED gcda_cleanup
    )
endif()
