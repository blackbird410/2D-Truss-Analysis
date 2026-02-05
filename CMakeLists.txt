cmake_minimum_required(VERSION 3.20)
project(TrussAnalysis VERSION 2.2.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

# Find required packages
find_package(Eigen3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable testing
include(CTest)
enable_testing()

# Find Google Test (for unit testing)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    message(STATUS "Google Test found: ${GTest_VERSION}")
endif()

# Enable Qt6 automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create library for core functionality
add_library(TrussCore STATIC
    src/core/Logger.cpp
    src/core/Logger.hpp
    src/core/Types.hpp
    src/core/Node.cpp
    src/core/Node.hpp
    src/core/Member.cpp
    src/core/Member.hpp
    src/core/Truss.cpp
    src/core/Truss.hpp
    src/core/AnalysisEngine.cpp
    src/core/AnalysisEngine.hpp
    src/core/ResultsExporter.cpp
    src/core/ResultsExporter.hpp
    src/core/Application.cpp
    src/core/Application.hpp
)

target_include_directories(TrussCore PUBLIC
    src/core
)

target_link_libraries(TrussCore PUBLIC
    Eigen3::Eigen
)

# Set target properties for library
set_target_properties(TrussCore PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Main application CLI executable
add_executable(TrussAnalysisCLI
    src/main_app.cpp
)

target_link_libraries(TrussAnalysisCLI PRIVATE
    TrussCore
)

# ============================================================================
# UNIT TESTS CONFIGURATION
# ============================================================================

if(BUILD_TESTING)
    message(STATUS "Building unit tests")
    
    # GTest integration validation test
    add_executable(test_gtest_integration tests/test_gtest_integration.cpp)
    target_link_libraries(test_gtest_integration PRIVATE GTest::gtest GTest::gtest_main)
    add_test(NAME GTestIntegration COMMAND test_gtest_integration)
    set_tests_properties(GTestIntegration PROPERTIES
        TIMEOUT 10
        LABELS "gtest;integration"
    )
    
    # ============================================================================
    # NEW GTEST-BASED TEST TARGETS (Phase 1 migrated tests)
    # ============================================================================
    
    # Unit tests (core functionality tests)
    add_executable(unit_tests
        tests/unit/core/test_node.cpp
        tests/unit/core/test_member.cpp
        tests/unit/core/test_truss.cpp
        tests/unit/core/test_analysis_engine.cpp
    )
    
    target_link_libraries(unit_tests PRIVATE
        TrussCore
        GTest::gtest
        GTest::gtest_main
    )
    
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
    )
    
    add_test(NAME UnitTests COMMAND unit_tests)
    set_tests_properties(UnitTests PROPERTIES
        TIMEOUT 30
        LABELS "unit;gtest"
    )
    
    # Integration tests (multi-component workflow tests)
    add_executable(integration_tests
        tests/integration/test_simple_truss_analysis.cpp
        tests/integration/test_simple_integration.cpp
        tests/integration/test_working_integration.cpp
        tests/integration/test_minimal_analysis.cpp
    )
    
    target_link_libraries(integration_tests PRIVATE
        TrussCore
        GTest::gtest
        GTest::gtest_main
    )
    
    target_include_directories(integration_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
    )
    
    add_test(NAME IntegrationTests COMMAND integration_tests)
    set_tests_properties(IntegrationTests PROPERTIES
        TIMEOUT 60
        LABELS "integration;gtest"
    )
    
    # Convenience target to build all new GTest-based tests
    add_custom_target(all_gtest_tests
        DEPENDS unit_tests integration_tests test_gtest_integration
        COMMENT "Building all GTest-based tests"
    )
    
    # ============================================================================
    # LEGACY CUSTOM FRAMEWORK TESTS (to be removed in Phase 1 Step 1.6)
    # ============================================================================
    
    # Create a function to add unit tests (legacy, will be replaced)
    function(add_unit_test TEST_NAME TEST_SOURCE)
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_link_libraries(${TEST_NAME} PRIVATE TrussCore)
        target_include_directories(${TEST_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/tests
            ${CMAKE_SOURCE_DIR}/src/core
        )
        target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_TESTING)
        
        # Add as a CTest
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        
        # Set test properties
        set_tests_properties(${TEST_NAME} PROPERTIES
            TIMEOUT 30
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endfunction()
    
    # Add unit tests
    add_unit_test(test_Integration tests/unit/test_Integration.cpp)
    add_unit_test(test_Member tests/unit/test_Member.cpp)
    add_unit_test(test_Truss tests/unit/test_Truss.cpp)
    add_unit_test(test_Node tests/unit/test_Node.cpp)
    add_unit_test(test_ConstraintDebug tests/unit/test_ConstraintDebug.cpp)
    add_unit_test(test_DebugAnalysis tests/unit/test_DebugAnalysis.cpp)
    add_unit_test(test_FreeSystemDebug tests/unit/test_FreeSystemDebug.cpp)
    add_unit_test(test_ManualConstraints tests/unit/test_ManualConstraints.cpp)
    add_unit_test(test_MemberDebug tests/unit/test_MemberDebug.cpp)
    add_unit_test(test_MinimalAnalysis tests/unit/test_MinimalAnalysis.cpp)
    add_unit_test(test_PointerAnalysis tests/unit/test_PointerAnalysis.cpp)
    add_unit_test(test_SimpleIntegration tests/unit/test_SimpleIntegration.cpp)
    add_unit_test(test_WorkingIntegration tests/unit/test_WorkingIntegration.cpp)
    
    # Add a custom target to run all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
        DEPENDS 
            test_Integration test_Member test_Truss test_Node
            test_ConstraintDebug test_DebugAnalysis test_FreeSystemDebug
            test_ManualConstraints test_MemberDebug test_MinimalAnalysis
            test_PointerAnalysis test_SimpleIntegration test_WorkingIntegration
        COMMENT "Running all unit tests"
    )
    
    # Add a custom target that uses our shell script
    add_custom_target(run_shell_tests
        COMMAND bash ${CMAKE_SOURCE_DIR}/tests/run_all_tests.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running unit tests with our custom test runner"
    )
    
endif()

# Set target properties for CLI executable
set_target_properties(TrussAnalysisCLI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "TrussAnalysisCLI"
)

# GUI executable
add_executable(TrussAnalysisGUI
    src/gui/main.cpp
    src/gui/MainWindow.cpp
    src/gui/NodeInputWidget.cpp
    src/gui/MemberInputWidget.cpp
    src/gui/LoadInputWidget.cpp
    src/gui/DataTableWidget.cpp
    src/gui/ResultsWidget.cpp
    src/gui/PlotWidget.cpp
    src/gui/InteractiveDrawingWidget.cpp
    src/gui/ProjectFileManager.cpp
    src/gui/DeformedTrussWidget.cpp
)

target_link_libraries(TrussAnalysisGUI PRIVATE
    TrussCore
    Qt6::Core
    Qt6::Widgets
)

# Set target properties for GUI executable
set_target_properties(TrussAnalysisGUI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "TrussAnalysisGUI"
)

# Install rules for packaging
install(TARGETS TrussAnalysisCLI TrussAnalysisGUI
    RUNTIME DESTINATION bin
)

# Linux-specific settings
message(STATUS "Configuring for Linux build")

# Set RPATH for better library linking
set_target_properties(TrussAnalysisGUI PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../lib;\$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH OFF
    INSTALL_RPATH_USE_LINK_PATH ON
)

set_target_properties(TrussAnalysisCLI PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../lib;\$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH OFF
    INSTALL_RPATH_USE_LINK_PATH ON
)

# Strip debug symbols in release builds for smaller binaries
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET TrussAnalysisGUI POST_BUILD
        COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:TrussAnalysisGUI>"
        COMMENT "Stripping debug symbols from TrussAnalysisGUI"
    )
    
    add_custom_command(TARGET TrussAnalysisCLI POST_BUILD
        COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:TrussAnalysisCLI>"
        COMMENT "Stripping debug symbols from TrussAnalysisCLI"
    )
endif()

message(STATUS "Linux optimizations enabled")

# CPack configuration for Linux packaging
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "2d-truss-analysis")
set(CPACK_PACKAGE_VENDOR "Civil Engineering Software Solutions")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional 2D truss structural analysis software")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Debian package specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)


