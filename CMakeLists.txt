cmake_minimum_required(VERSION 3.20)
project(TrussAnalysis VERSION 3.0.0 LANGUAGES CXX)

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

# Coverage options (enabled when CMAKE_BUILD_TYPE=Coverage)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(nlohmann_json REQUIRED)

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
endif()

# Enable Qt6 automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create library for core functionality
add_library(TrussCore STATIC
    src/core/Application.cpp
    src/core/Application.hpp
    # Domain model in model/ subdirectory
    src/core/model/Types.hpp
    src/core/model/Node.cpp
    src/core/model/Node.hpp
    src/core/model/Member.cpp
    src/core/model/Member.hpp
    src/core/model/Truss.cpp
    src/core/model/Truss.hpp
    src/core/model/Load.cpp
    src/core/model/Load.hpp
    # Domain assembly utilities
    src/core/assembly/TrussAssembler.cpp
    src/core/assembly/TrussAssembler.hpp
    # Domain validation services
    src/core/validation/TrussValidator.cpp
    src/core/validation/TrussValidator.hpp
    # Phase 2 - Analysis decomposition
    src/core/analysis/StiffnessAssembler.cpp
    src/core/analysis/StiffnessAssembler.hpp
    src/core/analysis/BoundaryConditionHandler.cpp
    src/core/analysis/BoundaryConditionHandler.hpp
    src/core/analysis/ILinearSolver.hpp
    src/core/analysis/DirectSolver.cpp
    src/core/analysis/DirectSolver.hpp
    src/core/analysis/IterativeSolver.cpp
    src/core/analysis/IterativeSolver.hpp
    src/core/analysis/SolverFactory.cpp
    src/core/analysis/SolverFactory.hpp
    src/core/analysis/AnalysisOrchestrator.cpp
    src/core/analysis/AnalysisOrchestrator.hpp
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
    src/infrastructure/adapters/ConsoleOutputAdapter.cpp
    src/infrastructure/adapters/ConsoleOutputAdapter.hpp
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
    src/infrastructure/io/fileio_factory.cpp
    src/infrastructure/io/fileio_factory.hpp
)

target_include_directories(TrussCore PUBLIC
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
    src/application/TrussApplicationService.cpp
    src/application/TrussApplicationService.hpp
    src/application/AnalysisApplicationService.cpp
    src/application/AnalysisApplicationService.hpp
    src/application/MaterialLibraryService.cpp
    src/application/MaterialLibraryService.hpp
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
# CLI APPLICATION
# ============================================================================

# CLI source files
set(CLI_SOURCES
    src/cli/ArgumentParser.cpp
    src/cli/commands/ExampleCommand.cpp
    src/cli/commands/HelpCommand.cpp
    src/cli/commands/AnalyzeCommand.cpp
    src/cli/commands/ValidateCommand.cpp
    src/cli/commands/ExportCommand.cpp
    src/cli/presenters/ConsolePresenter.cpp
    src/infrastructure/adapters/ConsoleOutputAdapter.cpp
)

# Main application CLI executable
add_executable(TrussAnalysisCLI
    src/main_app.cpp
    ${CLI_SOURCES}
)

target_link_libraries(TrussAnalysisCLI PRIVATE
    TrussApplication
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
        # Domain layer tests
        tests/unit/core/test_node.cpp
        tests/unit/core/test_member.cpp
        tests/unit/core/test_truss.cpp
        tests/unit/core/test_load.cpp
        tests/unit/core/test_truss_validator.cpp
        tests/unit/core/test_stiffness_assembler.cpp
        tests/unit/core/test_boundary_condition_handler.cpp
        tests/unit/core/test_linear_solvers.cpp
        tests/unit/core/test_analysis_orchestrator.cpp
        # Infrastructure layer tests
        tests/unit/infrastructure/export/test_csv_exporter.cpp
        tests/unit/infrastructure/export/test_html_exporter.cpp
        tests/unit/infrastructure/export/test_json_exporter.cpp
        tests/unit/infrastructure/export/test_latex_exporter.cpp
        tests/unit/infrastructure/export/test_text_exporter.cpp
        tests/unit/infrastructure/export/test_xml_exporter.cpp
        tests/unit/infrastructure/export/test_exporter_factory.cpp
        tests/unit/infrastructure/logging/test_logger.cpp
        tests/unit/infrastructure/io/test_fileio.cpp
        tests/unit/infrastructure/test_console_output_adapter.cpp
        # Application layer tests (Phase 4)
        tests/unit/application/test_truss_application_service.cpp
        tests/unit/application/test_truss_application_service_gui_methods.cpp
        tests/unit/application/test_material_library_service.cpp
        tests/unit/application/test_analysis_application_service.cpp
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
    
    target_link_libraries(unit_tests PRIVATE
        TrussApplication
        TrussCore
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )
    
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/application
        ${CMAKE_SOURCE_DIR}/src/infrastructure/export
        ${CMAKE_SOURCE_DIR}/src/infrastructure/logging
        ${CMAKE_SOURCE_DIR}/src/infrastructure/io
    )
    
    add_test(NAME UnitTests COMMAND unit_tests)
    set_tests_properties(UnitTests PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        TIMEOUT 30
        LABELS "unit;gtest"
    )
    
    # Integration tests (multi-component workflow tests)
    add_executable(integration_tests
        tests/integration/test_simple_truss_analysis.cpp
        tests/integration/test_simple_integration.cpp
        tests/integration/test_working_integration.cpp
        tests/integration/test_minimal_analysis.cpp
        tests/integration/test_cli_workflows.cpp
        ${CLI_SOURCES}
    )
    
    target_link_libraries(integration_tests PRIVATE
        TrussCore
        TrussApplication
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
    src/gui/MainWindow.hpp
    src/gui/NodeInputWidget.cpp
    src/gui/NodeInputWidget.hpp
    src/gui/MemberInputWidget.cpp
    src/gui/MemberInputWidget.hpp
    src/gui/LoadInputWidget.cpp
    src/gui/LoadInputWidget.hpp
    src/gui/DataTableWidget.cpp
    src/gui/DataTableWidget.hpp
    src/gui/ResultsWidget.cpp
    src/gui/ResultsWidget.hpp
    src/gui/PlotWidget.cpp
    src/gui/PlotWidget.hpp
    src/gui/InteractiveDrawingWidget.cpp
    src/gui/InteractiveDrawingWidget.hpp
    src/gui/ProjectFileManager.cpp
    src/gui/DeformedTrussWidget.cpp
    src/gui/DeformedTrussWidget.hpp
    # Phase 3A - Presenters
    src/gui/presenters/AnalysisResultsPresenter.cpp
    src/gui/presenters/TrussDataPresenter.cpp
    src/gui/presenters/ValidationPresenter.cpp
    # Phase 3A - Controllers
    src/gui/controllers/TrussEditController.cpp
    src/gui/controllers/AnalysisController.cpp
    src/gui/controllers/ProjectController.cpp
)

target_link_libraries(TrussAnalysisGUI PRIVATE
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

# Coverage reporting targets
if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    
    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Running tests to generate coverage data..."
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} -E echo "Capturing coverage data..."
            COMMAND ${LCOV} --capture --directory . --output-file coverage.info --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov
            COMMAND ${CMAKE_COMMAND} -E echo "Filtering coverage data..."
            COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/build/*' '*/tests/*' '*/external/*' '*/v1/*' --output-file coverage_filtered.info --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov,unused
            COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML report..."
            COMMAND ${GENHTML} coverage_filtered.info --output-directory coverage_html --rc branch_coverage=1 --ignore-errors inconsistent,category
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated: coverage_html/index.html"
            COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage summary..."
            COMMAND ${LCOV} --summary coverage_filtered.info --rc branch_coverage=1 --ignore-errors inconsistent,format,gcov
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


