# src/cli/CMakeLists.txt
#
# TrussCLI — reusable CLI component library and CLI executable.
#
# TrussCLI (STATIC library) contains all CLI command and presenter
# implementations.  It is linked by both the TrussAnalysisCLI executable
# and by the unit/integration test binaries, eliminating duplicate source
# compilation that existed when CLI_SOURCES was listed in each target.
#
# TrussAnalysisCLI (executable) links TrussCLI and adds the single
# application entry point (src/main_app.cpp).

# ---------------------------------------------------------------------------
# CLI component library
# ---------------------------------------------------------------------------
add_library(TrussCLI STATIC
    argument_parser.cpp
    commands/example_command.cpp
    commands/help_command.cpp
    commands/analyze_command.cpp
    commands/validate_command.cpp
    commands/export_command.cpp
    presenters/console_presenter.cpp
)

target_include_directories(TrussCLI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# TrussInterface transitively exposes TrussApplication → TrussInfrastructure
# → TrussCore, so all of those are available to TrussCLI and its consumers.
target_link_libraries(TrussCLI
    PUBLIC
        TrussInterface
        TrussApplication
        TrussInfrastructure
)

target_compile_features(TrussCLI PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# CLI executable
# ---------------------------------------------------------------------------
add_executable(TrussAnalysisCLI
    ${CMAKE_SOURCE_DIR}/src/main_app.cpp
)

target_link_libraries(TrussAnalysisCLI PRIVATE TrussCLI)

target_compile_features(TrussAnalysisCLI PRIVATE cxx_std_20)

set_target_properties(TrussAnalysisCLI PROPERTIES
    # Lowercase name follows Debian Policy §10.1 and standard Unix conventions.
    OUTPUT_NAME "truss-analysis-cli"
)
