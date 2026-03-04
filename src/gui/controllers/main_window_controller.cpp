/**
 * @file main_window_controller.cpp
 * @brief MainWindowController implementation (Phase 4).
 *
 * Phase 4: Constructor, WorkspaceState accessor / mutator,
 *          stateChanged signal emission, onTrussModified /
 *          onAnalysisCompleted / onAnalysisFailed skeleton slots.
 *
 * Phase 6 will wire these to sub-controllers and trigger Qt Item Model
 * refresh cascades.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/controllers/main_window_controller.hpp"

#include "interface/itruss_analysis_facade.hpp"

namespace truss::gui::ctrl {

// ============================================================
// Construction
// ============================================================

MainWindowController::MainWindowController(truss::interface::ITrussAnalysisFacade& facade,
                                           QObject* parent)
    : QObject(parent)
    , m_facade(&facade)
{
    // Workspace starts in the Empty phase; no model is loaded yet.
}

// ============================================================
// Accessors
// ============================================================

const state::WorkspaceState& MainWindowController::state() const noexcept
{
    return m_state;
}

void MainWindowController::setState(state::WorkspaceState newState)
{
    if (m_state == newState) return;  // avoid spurious signal emissions
    m_state = std::move(newState);
    emit stateChanged(m_state);
}

// ============================================================
// Slots
// ============================================================

void MainWindowController::onTrussModified(std::size_t trussHandle)
{
    state::WorkspaceState next = m_state;
    next.trussHandle  = trussHandle;
    next.phase        = state::WorkspacePhase::ModelBuilding;
    next.resultsHandle = 0;        // results are stale after any model edit
    next.isDirty      = true;
    next.lastError.clear();
    // TODO Phase 6: trigger NodeTableModel::refresh() and MemberTableModel::refresh()
    setState(std::move(next));
}

void MainWindowController::onAnalysisCompleted(std::size_t resultsHandle)
{
    state::WorkspaceState next = m_state;
    next.resultsHandle = resultsHandle;
    next.phase         = state::WorkspacePhase::ResultsReady;
    next.lastError.clear();
    // TODO Phase 6: trigger ResultsTableModel::refresh() and MemberTableModel results refresh
    setState(std::move(next));
}

void MainWindowController::onAnalysisFailed(const QString& errorMessage)
{
    state::WorkspaceState next = m_state;
    next.phase        = state::WorkspacePhase::ModelBuilding;
    next.resultsHandle = 0;
    next.lastError    = errorMessage.toStdString();
    setState(std::move(next));
}

}  // namespace truss::gui::ctrl
