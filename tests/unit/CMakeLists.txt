# tests/unit/CMakeLists.txt
#
# Unit test binaries:
#   unit_tests              — domain, infrastructure, application, interface,
#                             CLI, and non-display Qt tests (QCoreApplication).
#   unit_tests_gui_widgets  — Qt widget/panel tests requiring QApplication
#                             and the offscreen platform plugin (BUILD_GUI only).
#
# Source files are ONLY test .cpp files.  All production code is linked
# via library targets (TrussCore, TrussInfrastructure, TrussApplication,
# TrussInterface, TrussCLI, TrussGui) — no source file is listed twice.

# ============================================================================
# UNIT TESTS — non-display (QCoreApplication or no Qt)
# ============================================================================
set(UNIT_TEST_SOURCES
    # Domain layer — MODEL
    core/model/test_node.cpp
    core/model/test_member.cpp
    core/model/test_member_branches.cpp
    core/model/test_truss.cpp
    core/model/test_truss_branches.cpp
    core/model/test_load.cpp
    core/model/test_stiffness_matrix_bug.cpp

    # Domain layer — ASSEMBLY
    core/assembly/test_truss_assembler.cpp

    # Domain layer — VALIDATION
    core/validation/test_truss_validator.cpp
    core/validation/test_truss_validator_branches.cpp
    core/validation/test_validation_advanced.cpp

    # Domain layer — ANALYSIS
    core/analysis/test_stiffness_assembler.cpp
    core/analysis/test_boundary_condition_handler.cpp
    core/analysis/test_linear_solvers.cpp
    core/analysis/test_analysis_orchestrator.cpp
    core/analysis/test_displacement_sign_check.cpp
    core/analysis/test_warren_displacement.cpp

    # Infrastructure layer — EXPORT
    infrastructure/export/test_csv_exporter.cpp
    infrastructure/export/test_html_exporter.cpp
    infrastructure/export/test_json_exporter.cpp
    infrastructure/export/test_latex_exporter.cpp
    infrastructure/export/test_text_exporter.cpp
    infrastructure/export/test_xml_exporter.cpp
    infrastructure/export/test_exporter_factory.cpp
    infrastructure/export/test_export_advanced.cpp

    # Infrastructure layer — LOGGING
    infrastructure/logging/test_logger.cpp
    infrastructure/logging/test_logger_advanced.cpp

    # Infrastructure layer — I/O
    infrastructure/io/test_fileio.cpp
    infrastructure/io/test_fileio_advanced.cpp
    infrastructure/io/test_support_type_serializer.cpp
    infrastructure/io/test_json_reader_branches.cpp
    infrastructure/io/test_xml_reader_branches.cpp

    # Infrastructure layer — ADAPTERS
    infrastructure/adapters/test_console_output_adapter.cpp

    # Application layer
    application/test_truss_application_service.cpp
    application/test_truss_application_service_gui_methods.cpp
    application/test_truss_application_service_update.cpp
    application/test_material_library_service.cpp
    application/test_analysis_application_service.cpp

    # Interface layer
    interface/test_truss_builder.cpp
    interface/test_truss_analysis_facade.cpp
    interface/test_facade_truss_service_adapter.cpp
    interface/test_facade_analysis_service_adapter.cpp

    # CLI layer
    cli/test_console_presenter.cpp
    cli/test_help_command.cpp
    cli/test_argument_parser.cpp
    cli/test_analyze_command.cpp
    cli/test_example_command.cpp
    cli/test_validate_command.cpp
    cli/test_validate_command_mock.cpp
    cli/test_export_command.cpp
    cli/test_export_command_mock.cpp

    # Utility layer
    utilities/test_math_utils.cpp
    utilities/test_string_utils.cpp
    utilities/test_string_utils_branches.cpp
)

# Non-display Qt tests: QAbstractItemModel, QObject controllers, and
# WorkspaceState.  These use QCoreApplication at runtime — no platform
# plugin is loaded, so no display server is required.
if(BUILD_GUI)
    list(APPEND UNIT_TEST_SOURCES
        gui/models/test_node_table_model.cpp
        gui/models/test_member_table_model.cpp
        gui/models/test_results_table_model.cpp

        gui/controllers/test_main_window_controller.cpp
        gui/controllers/test_canvas_controller.cpp
        gui/controllers/test_inspector_controller.cpp
        gui/controllers/test_analysis_controller.cpp
        gui/controllers/test_project_controller.cpp
        gui/controllers/test_export_controller.cpp

        gui/state/test_workspace_state.cpp
    )
endif()

add_executable(unit_tests ${UNIT_TEST_SOURCES})

# When BUILD_GUI=ON the test sources include Qt-dependent models/controllers.
# Provide a custom main() that creates QCoreApplication on the stack so Qt's
# destructor runs before any atexit handler (including gcov's .gcda flush).
# When BUILD_GUI=OFF the binary has no Qt linkage; gtest_main is sufficient.
if(BUILD_GUI)
    target_sources(unit_tests PRIVATE test_main.cpp)
endif()

# TrussCLI is linked PUBLIC through TrussInterface → TrussApplication →
# TrussInfrastructure → TrussCore, so only TrussCLI needs to be listed.
# Repeating the transitive deps explicitly causes duplicate .a entries on
# the linker command line (ld duplicate-library warning on macOS/lld).
target_link_libraries(unit_tests PRIVATE
    TrussCLI
    # BUILD_GUI=ON  → custom main() in test_main.cpp; link GTest::gtest only.
    # BUILD_GUI=OFF → no Qt, no custom main; link GTest::gtest_main as usual.
    $<IF:$<BOOL:${BUILD_GUI}>,GTest::gtest,GTest::gtest_main>
    GTest::gmock
    $<$<BOOL:${BUILD_GUI}>:TrussGui>
    $<$<BOOL:${BUILD_GUI}>:Qt6::Test>
)

# tests/ provides the mocks directory.  src/ covers all production headers
# transitively through the linked library targets.
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

add_test(NAME unit_tests COMMAND unit_tests)
set_tests_properties(unit_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    TIMEOUT 30
    LABELS "unit;gtest"
    # Belt-and-suspenders: the controller sources compile against Qt6::Widgets
    # but the test bootstrap only initialises QCoreApplication, so no platform
    # plugin is loaded.  The env var is harmless here and guards against any
    # future test that inadvertently triggers QGuiApplication.
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
# CMake 4.x restricts set_tests_properties to the directory where the test
# was defined, so FIXTURES_REQUIRED must be set here (not in the parent).
if(ENABLE_COVERAGE)
    set_tests_properties(unit_tests PROPERTIES FIXTURES_REQUIRED gcda_cleanup)
endif()

# ============================================================================
# GUI WIDGET UNIT TESTS — QApplication + offscreen platform plugin
# Tests that instantiate QWidget, QGraphicsView, or call QPixmap/QPainter.
# Separated so the rest of the unit suite can run without a platform plugin.
# ============================================================================
if(BUILD_GUI)
    add_executable(unit_tests_gui_widgets
        test_main_gui.cpp  # custom main(): QApplication on stack (see comments in file)
        gui/panels/test_notification_rail.cpp
        gui/models/test_validation_list_model.cpp
        gui/panels/test_inspector_panel.cpp
        gui/panels/test_analysis_control_bar.cpp
        gui/panels/test_results_dock_panel.cpp
        gui/widgets/test_truss_canvas_widget.cpp
    )

    # TrussGui brings in all widget/panel/model/controller sources and
    # transitively supplies TrussInterface, TrussApplication,
    # TrussInfrastructure, and TrussCore.  Qt6::Test adds QSignalSpy.
    # GTest::gtest (not gtest_main): custom main() is in test_main_gui.cpp.
    target_link_libraries(unit_tests_gui_widgets PRIVATE
        TrussGui
        GTest::gtest
        GTest::gmock
        Qt6::Test
    )

    target_include_directories(unit_tests_gui_widgets PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )

    add_test(NAME unit_tests_gui_widgets COMMAND unit_tests_gui_widgets)
    set_tests_properties(unit_tests_gui_widgets PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        TIMEOUT 60
        LABELS "unit;gui-widget;gtest"
        # QApplication requires a platform plugin; offscreen satisfies that
        # requirement without a physical display or X11 server.
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
    if(ENABLE_COVERAGE)
        set_tests_properties(unit_tests_gui_widgets PROPERTIES FIXTURES_REQUIRED gcda_cleanup)
    endif()
endif()
