/**
 * @file analysis_control_bar.hpp
 * @brief Toolbar bar for analysis execution controls.
 *
 * Phase 5: Full Q_OBJECT implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "core/analysis/analysis_orchestrator.hpp"
#include "gui/state/workspace_state.hpp"

#include <QWidget>

class QPushButton;
class QProgressBar;
class QStackedWidget;

namespace truss::gui {

/**
 * @brief Horizontal control bar containing analysis execution controls.
 *
 * Contains:
 *  - Run Analysis button  — emits analyzeRequested(opts) with current options.
 *  - Stop button          — emits stopRequested() (visible only while Analysing).
 *  - Validate button      — emits validateRequested().
 *  - Options button       — emits optionsRequested() to open AnalysisOptionsDialog.
 *
 * All controls are enabled/disabled based on WorkspacePhase transitions received
 * via the onStateChanged slot.  During Analysing, Run/Validate are replaced by
 * a QProgressBar + Stop button via a QStackedWidget swap.
 *
 * The current AnalysisOptions held by this widget can be updated externally
 * via setOptions() (e.g., after the AnalysisOptionsDialog is accepted).
 */
class AnalysisControlBar : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisControlBar(QWidget* parent = nullptr);

    /// @brief Update the options that will be emitted with analyzeRequested.
    void setOptions(const core::analysis::AnalysisOptions& opts);

    /// @brief Return the current options.
    [[nodiscard]] const core::analysis::AnalysisOptions& options() const noexcept;

public slots:
    void onStateChanged(const truss::gui::state::WorkspaceState& state);

signals:
    /// Emitted when Run is clicked; carries current solver options.
    void analyzeRequested(const core::analysis::AnalysisOptions& opts);
    /// Emitted when Stop is clicked during analysis.
    void stopRequested();
    /// Emitted when Validate is clicked.
    void validateRequested();
    /// Emitted when the Options gear button is clicked.
    void optionsRequested();

private:
    void buildLayout();

    QStackedWidget* m_actionStack{nullptr};  ///< Swaps run-panel <-> progress-panel
    QPushButton* m_runBtn{nullptr};
    QPushButton* m_stopBtn{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QPushButton* m_validateBtn{nullptr};
    QPushButton* m_optionsBtn{nullptr};

    core::analysis::AnalysisOptions m_opts{};  ///< Carried by analyzeRequested
};

}  // namespace truss::gui
