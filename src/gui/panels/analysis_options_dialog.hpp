/**
 * @file analysis_options_dialog.hpp
 * @brief Modal dialog for configuring solver options before analysis.
 *
 * Phase 5: Full Q_OBJECT implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "core/analysis/analysis_orchestrator.hpp"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;

namespace truss::gui {

/**
 * @brief Configuration dialog for solver options.
 *
 * Exposes every field of `core::analysis::AnalysisOptions` as an editable
 * form field.  Accepted settings are retrieved via options(); they can be
 * pre-populated via setOptions().
 *
 * This is one of two permitted modal dialogs (the other being unsaved-changes
 * confirmation in ProjectController).  All other feedback uses NotificationRail.
 */
class AnalysisOptionsDialog : public QDialog {
    Q_OBJECT

public:
    explicit AnalysisOptionsDialog(QWidget* parent = nullptr);

    /// @brief Return the options currently shown in the dialog.
    [[nodiscard]] core::analysis::AnalysisOptions options() const;

    /// @brief Pre-populate all fields from @p opts.
    void setOptions(const core::analysis::AnalysisOptions& opts);

private slots:
    void onSolverTypeChanged(int index);

private:
    void buildLayout();

    QComboBox* m_solverCombo{nullptr};
    QDoubleSpinBox* m_toleranceSpin{nullptr};
    QSpinBox* m_maxIterSpin{nullptr};
    QCheckBox* m_computeReactionsCheck{nullptr};
    QCheckBox* m_checkStabilityCheck{nullptr};
    QCheckBox* m_verboseCheck{nullptr};
};

}  // namespace truss::gui
