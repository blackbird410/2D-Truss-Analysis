cmake_minimum_required(VERSION 3.20)
project(TrussAnalysis VERSION 3.0.0 LANGUAGES CXX)

# Export compile commands for tools like clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard (project-wide default; individual targets also declare
# cxx_std_20 via target_compile_features for explicit self-documentation).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ============================================================================
# COMPILER FLAGS
# Note: -march=native is intentionally absent — it produces non-portable
# binaries (SIGILL on CPUs older than the build machine).
# Use -O2 for Release (standard Debian/Ubuntu packaging level).
# ============================================================================
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options("$<$<CONFIG:Release>:-O2>")
    add_compile_options("$<$<CONFIG:Debug>:-O0;-g>")
endif()

# ============================================================================
# BUILD OPTIONS
# ============================================================================
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(BUILD_GUI       "Build GUI application (requires Qt6)" ON)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
        message(STATUS "Code coverage enabled")
    else()
        message(WARNING "Coverage reporting is only supported with GCC/Clang")
    endif()
endif()

# ============================================================================
# PACKAGE DISCOVERY
# All third-party packages are found here so sub-directories can consume
# the resulting imported targets without repeating find_package() calls.
# ============================================================================
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Qt6 is only required if building the GUI application.
if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test Svg)
    message(STATUS "Building with GUI support (Qt6)")
else()
    message(STATUS "Building CLI only (no Qt6)")
endif()

# tinyxml2 — try config-mode first, fall back to a Homebrew manual search.
find_package(tinyxml2 QUIET)
if(NOT tinyxml2_FOUND)
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

# ============================================================================
# TESTING INFRASTRUCTURE
# include(CTest) must precede any BUILD_TESTING guard: it is the module that
# defines the BUILD_TESTING cache variable at its default value (ON).
# ============================================================================
include(CTest)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)
    message(STATUS "Google Test found: ${GTest_VERSION}")

    # Manually create gmock targets if not found (Ubuntu 24.04 requires this).
    if(NOT TARGET GTest::gmock)
        find_library(GMOCK_LIBRARY      gmock      PATHS /usr/lib /usr/local/lib)
        find_library(GMOCK_MAIN_LIBRARY gmock_main PATHS /usr/lib /usr/local/lib)

        if(GMOCK_LIBRARY AND NOT GMOCK_MAIN_LIBRARY)
            message(WARNING "gmock found but gmock_main not found; GTest::gmock_main will not be created")
        elseif(NOT GMOCK_LIBRARY AND GMOCK_MAIN_LIBRARY)
            message(WARNING "gmock_main found but gmock not found; neither target will be created")
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

# Qt6 AUTOMOC/AUTORCC/AUTOUIC are intentionally NOT set globally here.
# They are scoped to the TrussGui and TrussAnalysisGUI targets in
# src/gui/CMakeLists.txt to prevent unintended processing of other targets.

# ============================================================================
# SOURCE AND TEST MODULES
# Each subdirectory owns its own CMakeLists.txt and defines its targets
# independently.  The root file orchestrates; it does not define targets.
# Dependency order is: utilities → core → infrastructure → application →
#                      interface → cli → gui (optional)
# ============================================================================
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Optional utility: generate corrected golden masters (standalone helper tool).
if(EXISTS "${CMAKE_SOURCE_DIR}/generate_corrected_golden_masters.cpp")
    add_executable(generate_corrected_golden_masters
        generate_corrected_golden_masters.cpp
    )
    target_link_libraries(generate_corrected_golden_masters PRIVATE TrussCore)
    target_compile_features(generate_corrected_golden_masters PRIVATE cxx_std_20)
endif()




# ============================================================================
# INSTALL RULES
# Uses GNUInstallDirs for FHS-compliant paths. Installs binaries, desktop
# entry, icon, AppStream metainfo, config templates, docs and examples.
# The include() activates all install() calls in InstallRules.cmake.
# ============================================================================
# Override DOCDIR before GNUInstallDirs computes it from PROJECT_NAME
# ("TrussAnalysis"). Debian Policy 12.3 requires copyright at
# /usr/share/doc/<package-name>, where <package-name> = CPACK_PACKAGE_NAME.
set(CMAKE_INSTALL_DOCDIR "share/doc/2d-truss-analysis")
include(GNUInstallDirs)
include(cmake/InstallRules.cmake)

message(STATUS "Configuring for Linux build")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

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
            DEPENDS all_gtest_tests
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

# ============================================================================
# CPACK PACKAGING CONFIGURATION
# Produces per-component .deb packages and .tar.gz tarballs.
# Run: cd build && cpack -G DEB
# ============================================================================
set(CPACK_GENERATOR "DEB;TGZ")
set(CPACK_PACKAGE_NAME "2d-truss-analysis")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Professional 2D truss structural analysis software")
set(CPACK_PACKAGE_DESCRIPTION
    "A professional-grade 2D truss structural analysis application\n built with modern C++20 and Qt6. Features an interactive drawing\n canvas, direct stiffness method engine, and clean layered architecture.")
set(CPACK_PACKAGE_VENDOR "Neil Taison Rigaud")
set(CPACK_PACKAGE_CONTACT "Neil Taison Rigaud <ntsrigaud14@gmail.com>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
# Debian revision suffix appended to upstream version: 3.0.0 -> 3.0.0-1
set(CPACK_DEBIAN_PACKAGE_RELEASE "1")
# Use standard Debian file naming: <name>_<version>-<release>_<arch>.deb
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
# Enforce correct file permissions in generated packages (Debian Policy §10.9)
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
# Strip installed binaries to reduce package size
set(CPACK_STRIP_FILES ON)

# ---- Component-based packaging ----
# Produces separate .deb files for cli, gui, and common components.
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL cli gui common)
set(CPACK_COMPONENT_CLI_DISPLAY_NAME "2D Truss Analysis CLI")
set(CPACK_COMPONENT_GUI_DISPLAY_NAME "2D Truss Analysis GUI")
set(CPACK_COMPONENT_COMMON_DISPLAY_NAME "2D Truss Analysis Shared Data")
set(CPACK_COMPONENT_GUI_DEPENDS common)
set(CPACK_COMPONENT_CLI_DEPENDS common)

# ---- Debian global settings ----
set(CPACK_DEBIAN_PACKAGE_MAINTAINER
    "Neil Taison Rigaud <ntsrigaud14@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE
    "https://github.com/blackbird410/2D-Truss-Analysis")
# Use dpkg-shlibdeps to auto-detect Qt6 and other .so runtime dependencies.
# This is version-agnostic and works across Ubuntu 22.04, 24.04, and derivatives.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# common contains only architecture-independent data; disable shlibdeps for it.
set(CPACK_DEBIAN_COMMON_PACKAGE_SHLIBDEPS OFF)

# ---- CLI component .deb ----
set(CPACK_DEBIAN_CLI_PACKAGE_NAME "2d-truss-analysis-cli")
set(CPACK_DEBIAN_CLI_PACKAGE_DESCRIPTION
    "Command-line tool for 2D truss structural analysis\n Analyses 2D truss structures using the displacement (stiffness) method.\n Reads JSON and XML project files and exports results in multiple formats.")

# ---- GUI component .deb ----
set(CPACK_DEBIAN_GUI_PACKAGE_NAME "2d-truss-analysis")
set(CPACK_DEBIAN_GUI_PACKAGE_DESCRIPTION
    "Interactive graphical application for 2D truss structural analysis\n Full-featured Qt6 GUI for designing, visualising, and analysing 2D truss\n structures. Exports results to CSV, JSON, XML, HTML, LaTeX, and plain text.")
# Extra runtime dep not auto-detected by shlibdeps
set(CPACK_DEBIAN_GUI_PACKAGE_RECOMMENDS "xdg-utils")
# Hook scripts update the desktop database and icon cache after install/remove
set(CPACK_DEBIAN_GUI_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/debian/postinst;"
    "${CMAKE_SOURCE_DIR}/packaging/debian/postrm")

# ---- Common component .deb ----
set(CPACK_DEBIAN_COMMON_PACKAGE_NAME "2d-truss-analysis-common")
# Architecture: all — this component contains only architecture-independent data.
set(CPACK_DEBIAN_COMMON_PACKAGE_ARCHITECTURE "all")
set(CPACK_DEBIAN_COMMON_PACKAGE_DESCRIPTION
    "Shared data files for 2D Truss Analysis\n Configuration templates, examples, and documentation shared by the\n CLI and GUI components of 2D Truss Analysis.")

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
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/docs/api
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    # Optional: Add install target for documentation
    install(DIRECTORY ${CMAKE_BINARY_DIR}/docs/api/html
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
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
    message(STATUS "  Output: ${CMAKE_BINARY_DIR}/docs/api/html/index.html")
else()
    message(STATUS "Doxygen not found - 'make docs' target unavailable")
    message(STATUS "  Install doxygen to generate API documentation")
endif()

# ============================================================================
# Coverage Analysis Enhancement Targets
# ============================================================================

# Add enhanced coverage target with detailed metrics
if(ENABLE_COVERAGE AND LCOV AND GENHTML)
    # Add custom target for detailed coverage analysis
    add_custom_target(coverage-analysis
        COMMAND ${CMAKE_COMMAND} -E echo "=== Coverage Analysis ==="
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
        COMMENT "Detailed coverage analysis complete"
        VERBATIM
    )

    # Add target for generating coverage dashboard
    add_custom_target(coverage-dashboard
        COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage dashboard..."
        COMMAND ${CMAKE_COMMAND} -E make_directory coverage_dashboard
        COMMAND ${GENHTML} coverage_analysis_filtered.info
                --output-directory coverage_dashboard
            --title "2D Truss Analysis Coverage Dashboard"
                --prefix ${CMAKE_SOURCE_DIR}
                --rc branch_coverage=1
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Coverage dashboard generated at: coverage_dashboard/index.html"
        DEPENDS coverage-analysis
        VERBATIM
    )

    message(STATUS "Coverage targets added:")
    message(STATUS "  - make coverage-analysis   : Run analysis with detailed metrics")
    message(STATUS "  - make coverage-dashboard  : Generate dashboard (implies analysis)")
    message(STATUS "  - make coverage            : Original coverage target (existing)")
    message(STATUS "  - make coverage-clean      : Clean coverage data (existing)")
endif()

