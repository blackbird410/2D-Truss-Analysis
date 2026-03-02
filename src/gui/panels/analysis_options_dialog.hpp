/**
 * @file analysis_options_dialog.hpp
 * @brief Modal dialog for configuring solver options before analysis.
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

#include <QDialog>

namespace truss::gui {

/**
 * @brief Configuration dialog for analysis solver options.
 *
 * Appears when the user clicks the "Options" gear button in AnalysisControlBar.
 * Exposes solver settings from AnalysisOptions (tolerance, max iterations,
 * solver type) as form fields.
 *
 * @note This is one of two permitted modal dialogs (the other being unsaved-
 *       changes confirmation in ProjectController). All other feedback uses
 *       the NotificationRail.
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement QFormLayout with AnalysisOptions
 *       fields, OK/Cancel buttons, validate input ranges before accepting.
 */
class AnalysisOptionsDialog : public QDialog {
public:
    explicit AnalysisOptionsDialog(QWidget* parent = nullptr) : QDialog(parent) {}

    // TODO Phase 5: AnalysisOptions getOptions() const
    // TODO Phase 5: void setOptions(const AnalysisOptions& options)
};

}  // namespace truss::gui
