/**
 * @file main_window_v2.hpp
 * @brief New MainWindow skeleton (Phase 4).
 *
 * Temporary name @e v2 avoids symbol conflict with the legacy @c main_window.cpp
 * while both coexist in the CMake target during the migration.  In Phase 8
 * this file will be renamed to @c main_window.cpp after the legacy version is
 * deleted.
 *
 * Phase 4: Full QMainWindow skeleton — QSplitter (65/35), bottom QDockWidget,
 *          menu bar stubs, toolbar stubs, status bar labels, MainWindowController
 *          ownership.  Panels are placeholder QWidgets pending Phase 5.
 * Phase 5: InspectorPanel / AnalysisControlBar / ResultsDockPanel replace placeholders.
 * Phase 6: Full signal/slot wiring; closeEvent dirty-state guard.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "gui/controllers/main_window_controller.hpp"
#include "gui/state/workspace_state.hpp"

#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

#include <memory>

namespace truss::interface { class ITrussAnalysisFacade; }
namespace truss::gui { class TrussCanvasWidget; }

namespace truss::gui {

/**
 * @brief New-architecture main window for the 2D Truss Analysis application.
 *
 * Layout (Phase 4):
 * @code
 * ┌──────────────────────────────────────────────────┐
 * │ MenuBar                                          │
 * ├──────────────────────────────────────────────────┤
 * │ ToolBar                                          │
 * ├────────────────────────────┬─────────────────────┤
 * │                            │                     │
 * │   TrussCanvasWidget        │  Inspector          │
 * │   (65 % of splitter)       │  (placeholder,      │
 * │                            │   35 %)             │
 * ├──────────────────────────────────────────────────┤
 * │ Results Dock (bottom, dismissible)               │
 * ├──────────────────────────────────────────────────┤
 * │ StatusBar  [Phase]  [Nodes N  Members M]         │
 * └──────────────────────────────────────────────────┘
 * @endcode
 *
 * Constructor takes @em only @c ITrussAnalysisFacade&; all internal
 * dependencies are constructed inside the constructor.
 */
class MainWindowV2 : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Construct the new main window.
     *
     * @param facade  Application facade (non-owning); must outlive this window.
     * @param parent  Qt parent widget (usually nullptr for a top-level window).
     */
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
    TrussCanvasWidget*  m_canvas{nullptr};         ///< Primary rendering area

    /// Right-panel placeholder — replaced by InspectorPanel in Phase 5.
    QWidget*            m_inspectorPlaceholder{nullptr};

    /// Bottom dock widget — contains m_resultsPlaceholder (Phase 4),
    /// replaced by full ResultsDockPanel in Phase 5.
    QDockWidget*        m_resultsDock{nullptr};

    /// Results content placeholder — replaced by ResultsDockPanel in Phase 5.
    QWidget*            m_resultsPlaceholder{nullptr};

    QSplitter*          m_centralSplitter{nullptr};///< Horizontal 65/35 splitter

    // -------------------------------------------------------------------
    // Status bar widgets
    // -------------------------------------------------------------------
    QLabel*             m_phaseLabel{nullptr};     ///< Shows current WorkspacePhase name
    QLabel*             m_statsLabel{nullptr};     ///< Shows "Nodes: N  Members: M"

    // -------------------------------------------------------------------
    // Controller
    // -------------------------------------------------------------------
    std::unique_ptr<ctrl::MainWindowController> m_controller;
};

}  // namespace truss::gui
