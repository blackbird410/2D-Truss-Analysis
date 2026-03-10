# src/gui/CMakeLists.txt
#
# TrussGui — reusable GUI component library and GUI executable.
#
# TrussGui (STATIC library) contains all Qt6 widget, panel, model, and
# controller implementations.  AUTOMOC/AUTORCC/AUTOUIC are scoped to this
# target rather than set globally, which prevents implicit MOC processing
# of unrelated targets.
#
# TrussAnalysisGUI (executable) links TrussGui and adds the single Qt
# entry point (src/gui/main.cpp).
#
# This module is only processed when BUILD_GUI is ON (guarded in
# src/CMakeLists.txt with if(BUILD_GUI) add_subdirectory(gui) endif()).

# ---------------------------------------------------------------------------
# GUI component library
# ---------------------------------------------------------------------------
add_library(TrussGui STATIC
    main_window.hpp
    main_window.cpp

    # Application state (header-only)
    state/workspace_state.hpp

    # Interface contracts (header-only)
    interfaces/iconfirmation_provider.hpp

    # Canvas widget
    widgets/truss_canvas_widget.hpp
    widgets/truss_canvas_widget.cpp

    # Panels
    panels/inspector_panel.hpp
    panels/inspector_panel.cpp
    panels/analysis_control_bar.hpp
    panels/analysis_control_bar.cpp
    panels/results_dock_panel.hpp
    panels/results_dock_panel.cpp
    panels/analysis_options_dialog.hpp
    panels/analysis_options_dialog.cpp
    panels/notification_rail.hpp
    panels/notification_rail.cpp

    # Qt item models
    models/node_table_model.hpp
    models/node_table_model.cpp
    models/member_table_model.hpp
    models/member_table_model.cpp
    models/validation_list_model.hpp
    models/validation_list_model.cpp
    models/results_table_model.hpp
    models/results_table_model.cpp

    # Controllers
    controllers/main_window_controller.hpp
    controllers/main_window_controller.cpp
    controllers/canvas_controller.hpp
    controllers/canvas_controller.cpp
    controllers/inspector_controller.hpp
    controllers/inspector_controller.cpp
    controllers/analysis_controller.hpp
    controllers/analysis_controller.cpp
    controllers/project_controller.hpp
    controllers/project_controller.cpp
    controllers/export_controller.hpp
    controllers/export_controller.cpp

    # Theme loader
    theme_loader.hpp
    theme_loader.cpp

    # Qt resources (AUTORCC compiles QSS themes into the binary)
    resources/resources.qrc
)

# Expose src/ and src/gui/ so that consumers (test binaries) can use
# includes like "gui/controllers/canvas_controller.hpp".
target_include_directories(TrussGui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Qt modules are PUBLIC so that test executables that link TrussGui also
# get Qt include dirs and link flags without repeating them.
# Qt6::Test is intentionally excluded — it is a test-only concern that
# test targets declare themselves.
target_link_libraries(TrussGui
    PUBLIC
        TrussInterface
        TrussApplication
        TrussInfrastructure
        Qt6::Core
        Qt6::Widgets
        Qt6::Svg
)

target_compile_features(TrussGui PUBLIC cxx_std_20)

# AUTOMOC/AUTORCC/AUTOUIC are set per-target, not globally, to prevent
# unintended processing of sources in other targets.
set_target_properties(TrussGui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    POSITION_INDEPENDENT_CODE ON
)

# ---------------------------------------------------------------------------
# GUI executable
# ---------------------------------------------------------------------------
add_executable(TrussAnalysisGUI main.cpp)

target_link_libraries(TrussAnalysisGUI PRIVATE TrussGui)

target_compile_features(TrussAnalysisGUI PRIVATE cxx_std_20)

# main.cpp may contain Q_OBJECT or QApplication usage that requires MOC.
set_target_properties(TrussAnalysisGUI PROPERTIES
    AUTOMOC ON
    # Lowercase name follows Debian Policy §10.1 and standard Unix conventions.
    OUTPUT_NAME "truss-analysis"
)
