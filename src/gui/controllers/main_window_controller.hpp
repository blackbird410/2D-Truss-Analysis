/**
 * @file main_window_controller.hpp
 * @brief Top-level orchestrator controller: owns WorkspaceState and coordinates
 *        all sub-controllers.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 6 (State Management & Interaction Logic).
 *
 * @note Q_OBJECT is added in Phase 4 (skeleton) and completed in Phase 6.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "gui/state/workspace_state.hpp"

#include <QObject>

namespace truss::interface { class ITrussAnalysisFacade; }

namespace truss::gui::ctrl {

/**
 * @brief Owns WorkspaceState and orchestrates all GUI sub-controllers.
 *
 * MainWindowController is the single owner of WorkspaceState. It receives
 * domain events from sub-controllers (trussModified, analysisCompleted,
 * analysisFailed) and transitions WorkspaceState accordingly. All panels
 * subscribe to its stateChanged(WorkspaceState) signal.
 *
 * Sub-interface distribution (Phase 6):
 *  MainWindowController holds ITrussAnalysisFacade*; it passes narrower
 *  sub-interfaces (ITrussService*, IAnalysisService*) to specific controllers
 *  at construction time.
 *
 * @todo Phase 4: Add Q_OBJECT macro, implement setState(WorkspaceState),
 *       hold ITrussAnalysisFacade*, emit stateChanged(WorkspaceState) signal.
 * @todo Phase 6: Wire all sub-controller signals, implement onTrussModified,
 *       onAnalysisCompleted, onAnalysisFailed slots with full model refresh.
 */
class MainWindowController : public QObject {
public:
    explicit MainWindowController(QObject* parent = nullptr) : QObject(parent) {}

    // TODO Phase 4: explicit MainWindowController(truss::interface::ITrussAnalysisFacade& facade,
    //                                              QObject* parent = nullptr)
    // TODO Phase 4: const state::WorkspaceState& state() const noexcept
    // TODO Phase 4: signals: void stateChanged(const state::WorkspaceState& newState)
    // TODO Phase 6: public slots: void onTrussModified(std::size_t trussHandle)
    // TODO Phase 6: public slots: void onAnalysisCompleted(std::size_t resultsHandle)
    // TODO Phase 6: public slots: void onAnalysisFailed(const QString& errorMessage)
};

}  // namespace truss::gui::ctrl
