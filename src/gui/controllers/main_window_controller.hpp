/**
 * @file main_window_controller.hpp
 * @brief Top-level orchestrator controller: owns WorkspaceState, all sub-controllers,
 *        and all Qt Item Models.  Receives domain events and cascades model refreshes.
 *
 * Phase 4: Q_OBJECT, WorkspaceState ownership, stateChanged signal.
 * Phase 6: Sub-controller creation + ownership, model refresh cascade,
 *          full onTrussModified / onAnalysisCompleted / onAnalysisFailed
 *          implementations with model refresh.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "gui/state/workspace_state.hpp"

#include <QObject>
#include <QString>

namespace truss::interface { class ITrussAnalysisFacade; }

namespace truss::gui::ctrl {

/**
 * @brief Owns WorkspaceState and orchestrates all GUI sub-controllers.
 *
 * MainWindowController is the single owner of @c WorkspaceState.  It receives
 * domain events from sub-controllers and transitions the workspace state
 * accordingly.  All panels subscribe to @c stateChanged(WorkspaceState).
 *
 * Design constraints:
 * - @c ITrussAnalysisFacade is held by pointer (non-owning); the facade must
 *   outlive the controller.
 * - setState() deep-compares old and new state; only emits stateChanged when
 *   the value actually changes.
 *
 * @todo Phase 6: Wire CanvasController / AnalysisController signals;
 *       implement onTrussModified, onAnalysisCompleted, onAnalysisFailed
 *       with full Qt Item Model refresh cascade.
 */
class MainWindowController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Construct with a mandatory facade reference.
     *
     * @param facade  Application facade; must outlive this controller.
     * @param parent  Qt parent for memory management.
     */
    explicit MainWindowController(truss::interface::ITrussAnalysisFacade& facade,
                                  QObject* parent = nullptr);

    ~MainWindowController() override = default;

    // Disable copy and move — controller has identity semantics
    MainWindowController(const MainWindowController&)            = delete;
    MainWindowController& operator=(const MainWindowController&) = delete;
    MainWindowController(MainWindowController&&)                  = delete;
    MainWindowController& operator=(MainWindowController&&)       = delete;

    /// @brief Read-only access to the current workspace state.
    [[nodiscard]] const state::WorkspaceState& state() const noexcept;

    /**
     * @brief Replace the workspace state and emit @c stateChanged if it changed.
     *
     * This is the sole write path for WorkspaceState.  Sub-controllers and
     * background workers must call this (via Qt::QueuedConnection if off-thread)
     * rather than mutating any shared state directly.
     *
     * @param newState  Desired new state.
     */
    void setState(state::WorkspaceState newState);

signals:
    /**
     * @brief Emitted whenever the workspace state transitions to a new value.
     *
     * @param newState  The new state after the transition.
     */
    void stateChanged(const truss::gui::state::WorkspaceState& newState);

public slots:
    /**
     * @brief Called when the structural model has been modified.
     *
     * Transitions the phase to @c ModelBuilding and updates the truss handle.
     *
     * @todo Phase 6: Trigger NodeTableModel / MemberTableModel refresh cascade.
     *
     * @param trussHandle  Handle to the modified truss.
     */
    void onTrussModified(std::size_t trussHandle);

    /**
     * @brief Called when a background analysis has completed successfully.
     *
     * Transitions the phase to @c ResultsReady and stores the results handle.
     *
     * @todo Phase 6: Trigger ResultsTableModel / MemberTableModel results refresh.
     *
     * @param resultsHandle  Handle to the completed analysis results.
     */
    void onAnalysisCompleted(std::size_t resultsHandle);

    /**
     * @brief Called when a background analysis has failed.
     *
     * Transitions the phase back to @c ModelBuilding and stores the error.
     *
     * @param errorMessage  Human-readable error description.
     */
    void onAnalysisFailed(const QString& errorMessage);

private:
    truss::interface::ITrussAnalysisFacade* m_facade{nullptr};
    state::WorkspaceState                   m_state;
};

}  // namespace truss::gui::ctrl
