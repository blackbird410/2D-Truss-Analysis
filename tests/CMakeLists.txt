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
