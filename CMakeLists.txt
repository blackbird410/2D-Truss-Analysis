cmake_minimum_required(VERSION 3.20)
project(TrussAnalysis VERSION 3.0.0 LANGUAGES CXX)

# Export compile commands for tools like clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# Build options
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(BUILD_GUI "Build GUI application (requires Qt6)" ON)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
        message(STATUS "Code coverage enabled")
    else()
        message(WARNING "Coverage reporting is only supported with GCC/Clang")
    endif()
endif()

# Find required packages
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Qt6 is only required if building GUI
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)
    message(STATUS "Building with GUI support (Qt6)")
else()
    message(STATUS "Building CLI only (no Qt6)")
endif()

# Find tinyxml2 (note: package name in CMake is lowercase)
find_package(tinyxml2 QUIET)
if(NOT tinyxml2_FOUND)
    # Try alternative search in Homebrew
    find_library(TINYXML2_LIBRARY tinyxml2 PATHS /opt/homebrew/lib /usr/local/lib)
    find_path(TINYXML2_INCLUDE_DIR tinyxml2.h PATHS /opt/homebrew/include /usr/local/include)
    if(TINYXML2_LIBRARY AND TINYXML2_INCLUDE_DIR)
        add_library(tinyxml2::tinyxml2 UNKNOWN IMPORTED)
        set_target_properties(tinyxml2::tinyxml2 PROPERTIES
            IMPORTED_LOCATION "${TINYXML2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${TINYXML2_INCLUDE_DIR}"
        )
        set(tinyxml2_FOUND TRUE)
    else()
        message(FATAL_ERROR "tinyxml2 not found")
    endif()
endif()

# Enable testing
include(CTest)
enable_testing()

# Find Google Test (for unit testing)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    message(STATUS "Google Test found: ${GTest_VERSION}")
    
    # Manually create gmock targets if not found (Ubuntu 24.04 requires this)
    if(NOT TARGET GTest::gmock)
        find_library(GMOCK_LIBRARY gmock PATHS /usr/lib /usr/local/lib)
        find_library(GMOCK_MAIN_LIBRARY gmock_main PATHS /usr/lib /usr/local/lib)
        
        if(GMOCK_LIBRARY AND NOT GMOCK_MAIN_LIBRARY)
            message(WARNING "gmock found but gmock_main not found; GTest::gmock_main target will not be created")
        elseif(NOT GMOCK_LIBRARY AND GMOCK_MAIN_LIBRARY)
            message(WARNING "gmock_main found but gmock not found; neither GTest::gmock nor GTest::gmock_main targets will be created")
        endif()

        if(GMOCK_LIBRARY)
            add_library(GTest::gmock UNKNOWN IMPORTED)
            set_target_properties(GTest::gmock PROPERTIES
                IMPORTED_LOCATION "${GMOCK_LIBRARY}"
                INTERFACE_LINK_LIBRARIES "GTest::gtest"
            )
            message(STATUS "Created GTest::gmock target: ${GMOCK_LIBRARY}")
        endif()
        
        if(GMOCK_LIBRARY AND GMOCK_MAIN_LIBRARY)
            add_library(GTest::gmock_main UNKNOWN IMPORTED)
            set_target_properties(GTest::gmock_main PROPERTIES
                IMPORTED_LOCATION "${GMOCK_MAIN_LIBRARY}"
                INTERFACE_LINK_LIBRARIES "GTest::gmock"
            )
            message(STATUS "Created GTest::gmock_main target: ${GMOCK_MAIN_LIBRARY}")
        endif()
    endif()
endif()

# Enable Qt6 automatic MOC, UIC, and RCC (only if building GUI)
if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# ============================================================================
# UTILITY LAYER (no project dependencies — safe for all layers)
# ============================================================================

add_library(TrussUtils STATIC
    src/utilities/string_utils.cpp
    src/utilities/string_utils.hpp
    src/utilities/math_utils.hpp
)

target_include_directories(TrussUtils PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(TrussUtils PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Create library for core functionality
add_library(TrussCore STATIC
    src/core/application.hpp
    # Domain model in model/ subdirectory
    src/core/model/types.hpp
    src/core/model/node.cpp
    src/core/model/node.hpp
    src/core/model/member.cpp
    src/core/model/member.hpp
    src/core/model/truss.cpp
    src/core/model/truss.hpp
    src/core/model/load.cpp
    src/core/model/load.hpp
    # Domain assembly utilities
    src/core/assembly/truss_assembler.cpp
    src/core/assembly/truss_assembler.hpp
    # Domain validation services
    src/core/validation/truss_validator.cpp
    src/core/validation/truss_validator.hpp
    # Phase 2 - Analysis decomposition
    src/core/analysis/stiffness_assembler.cpp
    src/core/analysis/stiffness_assembler.hpp
    src/core/analysis/boundary_condition_handler.cpp
    src/core/analysis/boundary_condition_handler.hpp
    src/core/analysis/ilinear_solver.hpp
    src/core/analysis/direct_solver.cpp
    src/core/analysis/direct_solver.hpp
    src/core/analysis/iterative_solver.cpp
    src/core/analysis/iterative_solver.hpp
    src/core/analysis/solver_factory.cpp
    src/core/analysis/solver_factory.hpp
    src/core/analysis/analysis_orchestrator.cpp
    src/core/analysis/analysis_orchestrator.hpp
    # Phase 3 - Infrastructure layer (export subsystem)
    src/infrastructure/export/export_types.hpp
    src/infrastructure/export/exporter.hpp
    src/infrastructure/export/exporter_factory.cpp
    src/infrastructure/export/exporter_factory.hpp
    src/infrastructure/export/csv_exporter.cpp
    src/infrastructure/export/csv_exporter.hpp
    src/infrastructure/export/json_exporter.cpp
    src/infrastructure/export/json_exporter.hpp
    src/infrastructure/export/xml_exporter.cpp
    src/infrastructure/export/xml_exporter.hpp
    src/infrastructure/export/html_exporter.cpp
    src/infrastructure/export/html_exporter.hpp
    src/infrastructure/export/latex_exporter.cpp
    src/infrastructure/export/latex_exporter.hpp
    src/infrastructure/export/text_exporter.cpp
    src/infrastructure/export/text_exporter.hpp
    # Phase 4 - Infrastructure layer (logging subsystem)
    src/infrastructure/logging/logger.hpp
    src/infrastructure/logging/console_logger.cpp
    src/infrastructure/logging/console_logger.hpp
    src/infrastructure/logging/file_logger.cpp
    src/infrastructure/logging/file_logger.hpp
    src/infrastructure/logging/logger_factory.cpp
    src/infrastructure/logging/logger_factory.hpp
    # Phase 5A - Infrastructure adapters
    src/infrastructure/adapters/console_output_adapter.cpp
    src/infrastructure/adapters/console_output_adapter.hpp
    # Phase 2 (Infrastructure Refactoring) - File I/O subsystem
    src/infrastructure/io/io_types.hpp
    src/infrastructure/io/truss_reader.hpp
    src/infrastructure/io/truss_writer.hpp
    src/infrastructure/io/json_truss_reader.cpp
    src/infrastructure/io/json_truss_reader.hpp
    src/infrastructure/io/json_truss_writer.cpp
    src/infrastructure/io/json_truss_writer.hpp
    src/infrastructure/io/xml_truss_reader.cpp
    src/infrastructure/io/xml_truss_reader.hpp
    src/infrastructure/io/xml_truss_writer.cpp
    src/infrastructure/io/xml_truss_writer.hpp
    src/infrastructure/io/support_type_serializer.cpp
    src/infrastructure/io/support_type_serializer.hpp
    src/infrastructure/io/fileio_factory.cpp
    src/infrastructure/io/fileio_factory.hpp
)

target_include_directories(TrussCore PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/truss
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/analysis
    ${CMAKE_SOURCE_DIR}/src/infrastructure/export
    ${CMAKE_SOURCE_DIR}/src/infrastructure/logging
    ${CMAKE_SOURCE_DIR}/src/infrastructure/io
)

target_link_libraries(TrussCore PUBLIC
    Eigen3::Eigen
    nlohmann_json::nlohmann_json
    tinyxml2::tinyxml2
    TrussUtils
)

# Set target properties for library
set_target_properties(TrussCore PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# ============================================================================
# APPLICATION LAYER (Phase 4 - Facades for Interface Layer)
# ============================================================================

add_library(TrussApplication STATIC
    src/application/truss_application_service.cpp
    src/application/truss_application_service.hpp
    src/application/analysis_application_service.cpp
    src/application/analysis_application_service.hpp
    src/application/material_library_service.cpp
    src/application/material_library_service.hpp
)

target_include_directories(TrussApplication PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/application
)

target_link_libraries(TrussApplication PUBLIC
    TrussCore
)

set_target_properties(TrussApplication PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# ============================================================================
# INTERFACE LAYER (Public API - Facade + Builder patterns)
# ============================================================================

add_library(TrussInterface STATIC
    src/interface/truss_builder.cpp
    src/interface/truss_builder.hpp
    src/interface/truss_analysis_facade.cpp
    src/interface/truss_analysis_facade.hpp
    src/interface/facade_truss_service_adapter.cpp
    src/interface/facade_truss_service_adapter.hpp
    src/interface/facade_analysis_service_adapter.cpp
    src/interface/facade_analysis_service_adapter.hpp
)

target_include_directories(TrussInterface PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/interface
    ${CMAKE_SOURCE_DIR}/src/infrastructure/export
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/truss
)

target_link_libraries(TrussInterface PUBLIC
    TrussApplication
)

set_target_properties(TrussInterface PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# ============================================================================
# CLI APPLICATION
# ============================================================================

# CLI source files
set(CLI_SOURCES
    src/cli/argument_parser.cpp
    src/cli/commands/example_command.cpp
    src/cli/commands/help_command.cpp
    src/cli/commands/analyze_command.cpp
    src/cli/commands/validate_command.cpp
    src/cli/commands/export_command.cpp
    src/cli/presenters/console_presenter.cpp
    src/infrastructure/adapters/console_output_adapter.cpp
)

# Main application CLI executable
add_executable(TrussAnalysisCLI
    src/main_app.cpp
    ${CLI_SOURCES}
)

target_link_libraries(TrussAnalysisCLI PRIVATE
    TrussInterface
    TrussApplication
    TrussCore
)

set_target_properties(TrussAnalysisCLI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "TrussAnalysisCLI"
)

# Utility: Generate corrected golden masters (optional - only if file exists)
if(EXISTS ${CMAKE_SOURCE_DIR}/generate_corrected_golden_masters.cpp)
    add_executable(generate_corrected_golden_masters
        generate_corrected_golden_masters.cpp
    )
    target_link_libraries(generate_corrected_golden_masters PRIVATE TrussCore)
endif()

# ============================================================================
# UNIT TESTS CONFIGURATION
# ============================================================================

if(BUILD_TESTING)
    message(STATUS "Building unit tests")
    
    # GTest integration validation test
    add_executable(test_gtest_integration tests/test_gtest_integration.cpp)
    target_link_libraries(test_gtest_integration PRIVATE GTest::gtest GTest::gtest_main)
    add_test(NAME gtest_integration COMMAND test_gtest_integration)
    set_tests_properties(gtest_integration PROPERTIES
        TIMEOUT 10
        LABELS "gtest;integration"
    )
    
    # ============================================================================
    # NEW GTEST-BASED TEST TARGETS (Phase 1 migrated tests)
    # ============================================================================
    
    # Unit tests (core functionality tests)
    set(UNIT_TEST_SOURCES
        # Domain layer tests - MODEL
        tests/unit/core/model/test_node.cpp
        tests/unit/core/model/test_member.cpp
        tests/unit/core/model/test_truss.cpp
        tests/unit/core/model/test_load.cpp
        tests/unit/core/model/test_stiffness_matrix_bug.cpp
        # Domain layer tests - VALIDATION
        tests/unit/core/validation/test_truss_validator.cpp
        tests/unit/core/validation/test_validation_advanced.cpp
        # Domain layer tests - ANALYSIS
        tests/unit/core/analysis/test_stiffness_assembler.cpp
        tests/unit/core/analysis/test_boundary_condition_handler.cpp
        tests/unit/core/analysis/test_linear_solvers.cpp
        tests/unit/core/analysis/test_analysis_orchestrator.cpp
        tests/unit/core/analysis/test_displacement_sign_check.cpp
        tests/unit/core/analysis/test_warren_displacement.cpp
        # Infrastructure layer tests
        tests/unit/infrastructure/export/test_csv_exporter.cpp
        tests/unit/infrastructure/export/test_html_exporter.cpp
        tests/unit/infrastructure/export/test_json_exporter.cpp
        tests/unit/infrastructure/export/test_latex_exporter.cpp
        tests/unit/infrastructure/export/test_text_exporter.cpp
        tests/unit/infrastructure/export/test_xml_exporter.cpp
        tests/unit/infrastructure/export/test_exporter_factory.cpp
        tests/unit/infrastructure/export/test_export_advanced.cpp
        tests/unit/infrastructure/logging/test_logger.cpp
        tests/unit/infrastructure/logging/test_logger_advanced.cpp
        tests/unit/infrastructure/io/test_fileio.cpp
        tests/unit/infrastructure/io/test_fileio_advanced.cpp
        tests/unit/infrastructure/io/test_support_type_serializer.cpp
        tests/unit/infrastructure/adapters/test_console_output_adapter.cpp
        # Application layer tests (Phase 4)
        tests/unit/application/test_truss_application_service.cpp
        tests/unit/application/test_truss_application_service_gui_methods.cpp
        tests/unit/application/test_material_library_service.cpp
        tests/unit/application/test_analysis_application_service.cpp
        # Interface layer tests (Public API)
        tests/unit/interface/test_truss_builder.cpp
        tests/unit/interface/test_truss_analysis_facade.cpp
        tests/unit/interface/test_facade_truss_service_adapter.cpp
        tests/unit/interface/test_facade_analysis_service_adapter.cpp
        # CLI layer tests (Phase 5A & 5B & Phase 6)
        tests/unit/cli/test_console_presenter.cpp
        tests/unit/cli/test_help_command.cpp
        tests/unit/cli/test_argument_parser.cpp
        tests/unit/cli/test_analyze_command.cpp
        tests/unit/cli/test_validate_command.cpp
        tests/unit/cli/test_export_command.cpp
        # CLI source files (needed for linking CLI tests)
        ${CLI_SOURCES}
    )
    
    # Conditionally add GUI tests and sources only if BUILD_GUI is enabled
    if(BUILD_GUI)
        list(APPEND UNIT_TEST_SOURCES
            # GUI-dependent validation tests
            tests/unit/core/validation/test_mock_compatibility.cpp
            tests/unit/core/validation/test_mock_analysis_compatibility.cpp
            # GUI layer tests (Controller API Stabilization)
            tests/unit/gui/controllers/test_truss_edit_controller.cpp
            tests/unit/gui/controllers/test_truss_edit_controller_advanced.cpp
            tests/unit/gui/controllers/test_project_controller.cpp
            tests/unit/gui/controllers/test_analysis_controller.cpp
            tests/unit/gui/presenters/test_truss_data_presenter.cpp
            # GUI source files needed for GUI tests
            src/gui/presenters/truss_data_presenter.cpp
            src/gui/presenters/analysis_results_presenter.cpp
            src/gui/presenters/validation_presenter.cpp
            src/gui/controllers/truss_edit_controller.cpp
            src/gui/controllers/analysis_controller.cpp
            src/gui/controllers/project_controller.cpp
        )
    endif()
    
    add_executable(unit_tests ${UNIT_TEST_SOURCES})
    
    target_link_libraries(unit_tests PRIVATE
        TrussInterface
        TrussApplication
        TrussCore
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        $<$<BOOL:${BUILD_GUI}>:Qt6::Core>
        $<$<BOOL:${BUILD_GUI}>:Qt6::Widgets>
        $<$<BOOL:${BUILD_GUI}>:Qt6::Test>
    )
    
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/application
        ${CMAKE_SOURCE_DIR}/src/interface
        ${CMAKE_SOURCE_DIR}/src/infrastructure/export
        ${CMAKE_SOURCE_DIR}/src/infrastructure/logging
        ${CMAKE_SOURCE_DIR}/src/infrastructure/io
        ${CMAKE_SOURCE_DIR}/src/gui
        ${CMAKE_SOURCE_DIR}/tests  # For mocks directory
    )
    
    add_test(NAME unit_tests COMMAND unit_tests)
    set_tests_properties(unit_tests PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        TIMEOUT 30
        LABELS "unit;gtest"
    )
    
    # Integration tests (multi-component workflow tests)
    set(INTEGRATION_TEST_SOURCES
        tests/integration/test_simple_truss_analysis.cpp
        tests/integration/test_simple_integration.cpp
        tests/integration/test_working_integration.cpp
        tests/integration/test_minimal_analysis.cpp
        tests/integration/test_cli_workflows.cpp
        tests/integration/test_lifecycle_integration.cpp
        tests/integration/test_load_workflow.cpp
        tests/integration/test_facade_adapters_integration.cpp
        # CLI source files needed for integration tests
        ${CLI_SOURCES}
    )
    
    # Conditionally add GUI integration tests only if BUILD_GUI is enabled
    if(BUILD_GUI)
        list(APPEND INTEGRATION_TEST_SOURCES
            tests/integration/test_gui_lifecycle.cpp
            tests/integration/test_ui_load_diagnosis.cpp
            tests/integration/test_load_viewport_fix.cpp
            tests/integration/test_gui_file_load.cpp
            tests/integration/test_e2e_workflows.cpp
            # GUI controllers needed for GUI lifecycle tests
            src/gui/controllers/truss_edit_controller.cpp
            src/gui/controllers/project_controller.cpp
            src/gui/presenters/truss_data_presenter.cpp
        )
    endif()
    
    add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
    
    target_link_libraries(integration_tests PRIVATE
        TrussInterface
        TrussCore
        TrussApplication
        $<$<BOOL:${BUILD_GUI}>:Qt6::Core>
        $<$<BOOL:${BUILD_GUI}>:Qt6::Widgets>
        $<$<BOOL:${BUILD_GUI}>:Qt6::Test>
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )
    
    target_include_directories(integration_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/infrastructure/export
        ${CMAKE_SOURCE_DIR}/src/infrastructure/logging
    )
    
    add_test(NAME integration_tests COMMAND integration_tests)
    set_tests_properties(integration_tests PROPERTIES
        TIMEOUT 60
        LABELS "integration;gtest"
    )
    
    # Convenience target to build all new GTest-based tests
    add_custom_target(all_gtest_tests
        DEPENDS unit_tests integration_tests test_gtest_integration
        COMMENT "Building all GTest-based tests"
    )

endif()

# GUI Integration Tests (Qt Test framework) - only if Qt is available
if(BUILD_GUI)
    message(STATUS "Building GUI integration tests")
    
    # GUI Integration Tests (Qt Test framework)
    add_executable(gui_integration_test
        tests/integration/test_gui_integration.cpp
        # GUI source files needed for integration tests
        src/gui/presenters/truss_data_presenter.cpp
        src/gui/presenters/analysis_results_presenter.cpp
        src/gui/presenters/validation_presenter.cpp
        src/gui/controllers/truss_edit_controller.cpp
        src/gui/controllers/analysis_controller.cpp
        src/gui/controllers/project_controller.cpp
    )
    
    target_link_libraries(gui_integration_test PRIVATE
        TrussApplication
        TrussCore
        Qt6::Test
        Qt6::Widgets
        Qt6::Core
    )
    
    target_include_directories(gui_integration_test PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/gui
        ${CMAKE_SOURCE_DIR}/src/application
    )
    
    add_test(NAME gui_integration_tests COMMAND gui_integration_test)
    set_tests_properties(gui_integration_tests PROPERTIES
        TIMEOUT 120
        LABELS "gui;integration;qt"
    )
    
    # Add GUI tests to all_gtest_tests if they exist
    if(TARGET all_gtest_tests)
        add_dependencies(all_gtest_tests gui_integration_test)
    endif()
    
endif()

# Set target properties for CLI executable
# ============================================================================
# GUI APPLICATION (Optional - requires Qt6)
# ============================================================================

if(BUILD_GUI)
    # GUI executable
    add_executable(TrussAnalysisGUI
    src/gui/main.cpp
    src/gui/main_window.cpp
    src/gui/main_window.hpp
    # Widgets
    src/gui/widgets/node_input_widget.cpp
    src/gui/widgets/node_input_widget.hpp
    src/gui/widgets/member_input_widget.cpp
    src/gui/widgets/member_input_widget.hpp
    src/gui/widgets/load_input_widget.cpp
    src/gui/widgets/load_input_widget.hpp
    src/gui/widgets/data_table_widget.cpp
    src/gui/widgets/data_table_widget.hpp
    src/gui/widgets/results_widget.cpp
    src/gui/widgets/results_widget.hpp
    src/gui/widgets/plot_widget.cpp
    src/gui/widgets/plot_widget.hpp
    src/gui/widgets/interactive_drawing_widget.cpp
    src/gui/widgets/interactive_drawing_widget.hpp
    src/gui/widgets/deformed_truss_widget.cpp
    src/gui/widgets/deformed_truss_widget.hpp
    # Phase 3A - Presenters
    src/gui/presenters/analysis_results_presenter.cpp
    src/gui/presenters/truss_data_presenter.cpp
    src/gui/presenters/validation_presenter.cpp
    # Phase 3A - Controllers
    src/gui/controllers/truss_edit_controller.cpp
    src/gui/controllers/analysis_controller.cpp
    src/gui/controllers/project_controller.cpp
)

target_link_libraries(TrussAnalysisGUI PRIVATE
    TrussInterface
    TrussApplication
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
endif()  # BUILD_GUI

# Install rules for packaging
if(BUILD_GUI)
    install(TARGETS TrussAnalysisCLI TrussAnalysisGUI
        RUNTIME DESTINATION bin
    )
else()
    install(TARGETS TrussAnalysisCLI
        RUNTIME DESTINATION bin
    )
endif()

# Linux-specific settings
message(STATUS "Configuring for Linux build")

if(BUILD_GUI)
    # Set RPATH for better library linking
    set_target_properties(TrussAnalysisGUI PROPERTIES
        INSTALL_RPATH "\$ORIGIN/../lib;\$ORIGIN/lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH ON
    )
endif()

set_target_properties(TrussAnalysisCLI PROPERTIES
    INSTALL_RPATH "\$ORIGIN/../lib;\$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH OFF
    INSTALL_RPATH_USE_LINK_PATH ON
)

# Strip debug symbols in release builds for smaller binaries
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(BUILD_GUI)
        add_custom_command(TARGET TrussAnalysisGUI POST_BUILD
            COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:TrussAnalysisGUI>"
            COMMENT "Stripping debug symbols from TrussAnalysisGUI"
        )
    endif()
    
    add_custom_command(TARGET TrussAnalysisCLI POST_BUILD
        COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:TrussAnalysisCLI>"
        COMMENT "Stripping debug symbols from TrussAnalysisCLI"
    )
endif()

message(STATUS "Linux optimizations enabled")

# Coverage reporting targets
if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    
    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Running tests to generate coverage data..."
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} -E echo "Capturing coverage data..."
            COMMAND ${LCOV} --capture --directory . --output-file coverage.info --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov,mismatch,negative
            COMMAND ${CMAKE_COMMAND} -E echo "Filtering coverage data..."
            COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/build/*' '*/tests/*' '*/external/*' '*/v1/*' --output-file coverage_filtered.info --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov,unused,mismatch,negative
            COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML report..."
            COMMAND ${GENHTML} coverage_filtered.info --output-directory coverage_html --rc branch_coverage=1 --ignore-errors inconsistent,category,mismatch,negative
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated: coverage_html/index.html"
            COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage summary..."
            COMMAND ${LCOV} --summary coverage_filtered.info --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov,mismatch,negative
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS unit_tests test_gtest_integration integration_tests
            COMMENT "Generating code coverage report"
        )
        
        add_custom_target(coverage-clean
            COMMAND ${CMAKE_COMMAND} -E remove -f coverage.info coverage_filtered.info
            COMMAND ${CMAKE_COMMAND} -E remove_directory coverage_html
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Cleaning coverage data"
        )
        
        message(STATUS "Coverage targets available: 'make coverage' and 'make coverage-clean'")
    else()
        message(WARNING "lcov or genhtml not found - coverage target unavailable")
    endif()
endif()

# CPack configuration for Linux packaging
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "2d-truss-analysis")
set(CPACK_PACKAGE_VENDOR "Civil Engineering Software Solutions")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional 2D truss structural analysis software")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Debian package specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Civil Engineering Software Solutions <support@civil-engineering-software-solutions.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)

# ============================================================================
# Documentation Generation (Doxygen)
# ============================================================================

find_package(Doxygen OPTIONAL_COMPONENTS dot)

if(DOXYGEN_FOUND)
    # Set project description for Doxyfile
    set(PROJECT_DESCRIPTION "Professional 2D truss structural analysis software")
    
    # Configure Doxyfile with project metadata
    configure_file(
        ${CMAKE_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_BINARY_DIR}/Doxyfile
        @ONLY
    )
    
    # Add custom target for generating documentation
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    # Optional: Add install target for documentation
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/api/html
        DESTINATION share/doc/${PROJECT_NAME}
        OPTIONAL
        COMPONENT documentation
    )
    
    message(STATUS "Doxygen found - 'make docs' target available")
    if(DOXYGEN_DOT_FOUND)
        message(STATUS "  - Graphviz (dot) found - diagrams enabled")
    else()
        message(STATUS "  - Graphviz (dot) not found - diagrams disabled")
        message(STATUS "    Install graphviz for call graphs and class diagrams")
    endif()
    message(STATUS "  Output: ${PROJECT_SOURCE_DIR}/docs/api/html/index.html")
else()
    message(STATUS "Doxygen not found - 'make docs' target unavailable")
    message(STATUS "  Install doxygen to generate API documentation")
endif()

# ============================================================================
# Phase 7 Coverage Analysis Enhancement Targets
# ============================================================================

# Add enhanced coverage target for Phase 7 analysis with detailed metrics
if(ENABLE_COVERAGE AND LCOV AND GENHTML)
    # Add custom target for detailed coverage analysis
    add_custom_target(coverage-analysis
        COMMAND ${CMAKE_COMMAND} -E echo "=== Phase 7 Coverage Analysis ==="
        COMMAND ${CMAKE_COMMAND} -E echo "Capturing detailed coverage metrics..."
        COMMAND ${LCOV} --capture --directory . --output-file coverage_analysis.info 
                --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov
        COMMAND ${CMAKE_COMMAND} -E echo "Filtering external code..."
        COMMAND ${LCOV} --remove coverage_analysis.info '/usr/*' '*/build/*' '*/tests/*' 
                '*/external/*' '*/v1/*' '*/opt/homebrew/*' 
                --output-file coverage_analysis_filtered.info 
                --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov,unused
        COMMAND ${CMAKE_COMMAND} -E echo "Generating module-level coverage statistics..."
        COMMAND ${LCOV} --summary coverage_analysis_filtered.info 
                --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Phase 7: Detailed coverage analysis complete"
        VERBATIM
    )

    # Add target for generating coverage dashboard
    add_custom_target(coverage-dashboard
        COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage dashboard..."
        COMMAND ${CMAKE_COMMAND} -E make_directory coverage_dashboard
        COMMAND ${GENHTML} coverage_analysis_filtered.info 
                --output-directory coverage_dashboard
                --title "Phase 7: 2D Truss Analysis Coverage Dashboard"
                --prefix ${CMAKE_SOURCE_DIR}
                --rc branch_coverage=1
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Coverage dashboard generated at: coverage_dashboard/index.html"
        DEPENDS coverage-analysis
        VERBATIM
    )

    message(STATUS "Phase 7 Coverage Targets Added:")
    message(STATUS "  - make coverage-analysis   : Run analysis with detailed metrics")
    message(STATUS "  - make coverage-dashboard  : Generate dashboard (implies analysis)")
    message(STATUS "  - make coverage            : Original coverage target (existing)")
    message(STATUS "  - make coverage-clean      : Clean coverage data (existing)")
endif()

