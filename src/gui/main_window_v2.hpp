/**
 * @file main_window_v2.hpp
 * @brief New MainWindow with full Phase 6 signal/slot wiring.
 *
 * Temporary name @e v2 avoids symbol conflict with the legacy @c main_window.cpp
 * while both coexist in the CMake target during migration.  Phase 8 renames
 * this to @c main_window.cpp after deleting the legacy version.
 *
 * Phase 4: QMainWindow skeleton — QSplitter (65/35), bottom QDockWidget,
 *          menu bar stubs, toolbar stubs, status bar, MainWindowController.
 * Phase 5: InspectorPanel / AnalysisControlBar / ResultsDockPanel replace placeholders.
 * Phase 6: Full signal/slot wiring; closeEvent dirty-state guard; toolbar wiring.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "gui/controllers/main_window_controller.hpp"
#include "gui/state/workspace_state.hpp"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

#include <memory>

namespace truss::interface { class ITrussAnalysisFacade; }
namespace truss::gui {
class TrussCanvasWidget;
class InspectorPanel;
class AnalysisControlBar;
class ResultsDockPanel;
}

namespace truss::gui {

/**
 * @brief New-architecture main window for the 2D Truss Analysis application.
 *
 * Layout (Phase 6):
 * @code
 * ┌────────────────────────────────────────────────────┐
 * │ MenuBar                                            │
 * ├────────────────────────────────────────────────────┤
 * │ ToolBar                                            │
 * ├────────────────────────────┬───────────────────────┤
 * │                            │  AnalysisControlBar   │
 * │   TrussCanvasWidget        │  ──────────────────── │
 * │   (65 % of splitter)       │  InspectorPanel       │
 * │                            │  (35 %)               │
 * ├────────────────────────────┴───────────────────────┤
 * │ ResultsDockPanel (QDockWidget, bottom)              │
 * ├────────────────────────────────────────────────────┤
 * │ StatusBar  [Phase badge]  [Nodes: N  Members: M]   │
 * └────────────────────────────────────────────────────┘
 * @endcode
 */
class MainWindowV2 : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindowV2(truss::interface::ITrussAnalysisFacade& facade,
                          QWidget* parent = nullptr);
    ~MainWindowV2() override = default;

private slots:
    /// @brief React to workspace state transitions emitted by the controller.
    void onStateChanged(const truss::gui::state::WorkspaceState& newState);

private:
    // -------------------------------------------------------------------
    // Setup helpers (called from constructor)
    // -------------------------------------------------------------------
    void setupCentralWidget();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void connectSignals();

    // -------------------------------------------------------------------
    // Owned widgets
    // -------------------------------------------------------------------
    TrussCanvasWidget*   m_canvas{nullptr};
    InspectorPanel*      m_inspectorPanel{nullptr};
    AnalysisControlBar*  m_analysisBar{nullptr};
    ResultsDockPanel*    m_resultsDockPanel{nullptr};
    QDockWidget*         m_resultsDock{nullptr};
    QSplitter*           m_centralSplitter{nullptr};

    // Status bar labels
    QLabel* m_phaseLabel{nullptr};
    QLabel* m_statsLabel{nullptr};
    QLabel* m_cursorLabel{nullptr};

    // Menu QActions stored for later connection in connectSignals()
    QAction* m_actNew{nullptr};
    QAction* m_actOpen{nullptr};
    QAction* m_actSave{nullptr};
    QAction* m_actSaveAs{nullptr};
    QAction* m_actQuit{nullptr};
    QAction* m_actThemeDark{nullptr};
    QAction* m_actThemeLight{nullptr};

    // -------------------------------------------------------------------
    // Controller
    // -------------------------------------------------------------------
    std::unique_ptr<ctrl::MainWindowController> m_controller;
};

}  // namespace truss::gui
