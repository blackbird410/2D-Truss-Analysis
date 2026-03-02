/**
 * @file analysis_control_bar.hpp
 * @brief Toolbar bar for analysis execution controls.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 5.
 *
 * @note Q_OBJECT is added in Phase 5.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QWidget>

namespace truss::gui::state { struct WorkspaceState; }

namespace truss::gui {

/**
 * @brief Horizontal control bar containing analysis execution controls.
 *
 * Contains:
 *  - Run Analysis button (transitions to indeterminate QProgressBar during Analysing phase)
 *  - Validate button
 *  - Options button (opens AnalysisOptionsDialog)
 *
 * All controls are enabled/disabled based on WorkspacePhase transitions received
 * via the onStateChanged slot.
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement button layout with QStackedWidget
 *       for run/progress swap, connect onStateChanged slot, emit analyzeRequested
 *       and validateRequested signals.
 */
class AnalysisControlBar : public QWidget {
public:
    explicit AnalysisControlBar(QWidget* parent = nullptr) : QWidget(parent) {}

    // TODO Phase 5: public slot: void onStateChanged(const state::WorkspaceState& state)
    // TODO Phase 5: signals: analyzeRequested(AnalysisOptions), validateRequested()
    // TODO Phase 5: signals: optionsRequested()
};

}  // namespace truss::gui
