/**
 * @file main_window.hpp
 * @brief MainWindow — top-level application window with full signal/slot wiring.
 *
 * QMainWindow with QSplitter (65/35), bottom QDockWidget, menu bar,
 * toolbar, InspectorPanel, AnalysisControlBar, ResultsDockPanel, and
 * NotificationRail.  All actions, keyboard shortcuts, and dirty-state
 * guards are wired through MainWindowController.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "gui/controllers/main_window_controller.hpp"
#include "gui/state/workspace_state.hpp"
#include "truss/export/export_format.hpp"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

#include <memory>

namespace truss::interface {
class ITrussAnalysisFacade;
}
namespace truss::gui {
class TrussCanvasWidget;
class InspectorPanel;
class AnalysisControlBar;
class ResultsDockPanel;
}  // namespace truss::gui

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
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(truss::interface::ITrussAnalysisFacade& facade, QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    /// React to workspace state transitions from the controller.
    void onStateChanged(const truss::gui::state::WorkspaceState& newState);

    /// Show QFileDialog then hand path to ExportController.
    void onExportRequested(truss::ExportFormat format, const QString& suggestedFilename);

    /// Show AnalysisOptionsDialog and push result back to AnalysisControlBar.
    void onOptionsRequested();

    /// Trigger facade validation and show result in status bar.
    void onValidateRequested();

    /// Handle toolbar tool-mode actions.
    void onToolActionTriggered();

private:
    void setupCentralWidget();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void connectSignals();

    // -------------------------------------------------------------------
    // Owned widgets
    // -------------------------------------------------------------------
    TrussCanvasWidget* m_canvas{nullptr};
    InspectorPanel* m_inspectorPanel{nullptr};
    AnalysisControlBar* m_analysisBar{nullptr};
    ResultsDockPanel* m_resultsDockPanel{nullptr};
    QDockWidget* m_resultsDock{nullptr};
    QSplitter* m_centralSplitter{nullptr};

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

    // Tool-mode QActions (exclusive)
    QAction* m_actToolSelect{nullptr};
    QAction* m_actToolNode{nullptr};
    QAction* m_actToolMember{nullptr};
    QAction* m_actToolDelete{nullptr};

    // Analysis toolbar QActions (stored so connectSignals() can wire them)
    QAction* m_actRun{nullptr};
    QAction* m_actStop{nullptr};

    // Display mode toolbar QActions (exclusive checkable)
    QAction* m_actModeGeometry{nullptr};
    QAction* m_actModeStress{nullptr};
    QAction* m_actModeDeformed{nullptr};

    // -------------------------------------------------------------------
    // Controller
    // -------------------------------------------------------------------
    std::unique_ptr<ctrl::MainWindowController> m_controller;
};

}  // namespace truss::gui
