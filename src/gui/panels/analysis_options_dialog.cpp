/**
 * @file analysis_options_dialog.cpp
 * @brief AnalysisOptionsDialog implementation (Phase 5).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/panels/analysis_options_dialog.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace truss::gui {

AnalysisOptionsDialog::AnalysisOptionsDialog(QWidget* parent) : QDialog{parent}
{
    setWindowTitle(QStringLiteral("Analysis Options"));
    setObjectName(QStringLiteral("analysisOptionsDialog"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    buildLayout();
}

void AnalysisOptionsDialog::buildLayout()
{
    auto* root = new QVBoxLayout{this};
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    // ---- Solver group ----
    auto* solverBox    = new QGroupBox{QStringLiteral("Solver"), this};
    auto* solverLayout = new QFormLayout{solverBox};
    solverLayout->setSpacing(6);

    m_solverCombo = new QComboBox{solverBox};
    m_solverCombo->setObjectName(QStringLiteral("solverCombo"));
    m_solverCombo->addItem(QStringLiteral("Direct  (Gaussian elimination)"));
    m_solverCombo->addItem(QStringLiteral("Iterative  (Conjugate gradient)"));
    solverLayout->addRow(QStringLiteral("Solver type:"), m_solverCombo);

    m_toleranceSpin = new QDoubleSpinBox{solverBox};
    m_toleranceSpin->setObjectName(QStringLiteral("toleranceSpin"));
    m_toleranceSpin->setDecimals(10);
    m_toleranceSpin->setRange(1e-15, 1e-3);
    m_toleranceSpin->setSingleStep(1e-10);
    m_toleranceSpin->setValue(1e-9);
    solverLayout->addRow(QStringLiteral("Convergence tolerance:"), m_toleranceSpin);

    m_maxIterSpin = new QSpinBox{solverBox};
    m_maxIterSpin->setObjectName(QStringLiteral("maxIterSpin"));
    m_maxIterSpin->setRange(10, 100'000);
    m_maxIterSpin->setValue(1000);
    solverLayout->addRow(QStringLiteral("Max iterations:"), m_maxIterSpin);

    root->addWidget(solverBox);

    // ---- Output options group ----
    auto* optBox    = new QGroupBox{QStringLiteral("Output Options"), this};
    auto* optLayout = new QVBoxLayout{optBox};
    optLayout->setSpacing(4);

    m_computeReactionsCheck = new QCheckBox{QStringLiteral("Compute support reactions"), optBox};
    m_computeReactionsCheck->setObjectName(QStringLiteral("computeReactionsCheck"));
    m_computeReactionsCheck->setChecked(true);

    m_checkStabilityCheck = new QCheckBox{QStringLiteral("Check structural stability"), optBox};
    m_checkStabilityCheck->setObjectName(QStringLiteral("checkStabilityCheck"));
    m_checkStabilityCheck->setChecked(true);

    m_verboseCheck = new QCheckBox{QStringLiteral("Verbose output"), optBox};
    m_verboseCheck->setObjectName(QStringLiteral("verboseCheck"));
    m_verboseCheck->setChecked(false);

    optLayout->addWidget(m_computeReactionsCheck);
    optLayout->addWidget(m_checkStabilityCheck);
    optLayout->addWidget(m_verboseCheck);
    root->addWidget(optBox);

    // ---- Dialog buttons ----
    auto* buttons =
        new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_solverCombo,
            &QComboBox::currentIndexChanged,
            this,
            &AnalysisOptionsDialog::onSolverTypeChanged);

    onSolverTypeChanged(m_solverCombo->currentIndex());
}

void AnalysisOptionsDialog::onSolverTypeChanged(int index)
{
    const bool iterative = (index == 1);
    m_toleranceSpin->setEnabled(iterative);
    m_maxIterSpin->setEnabled(iterative);
}

core::analysis::AnalysisOptions AnalysisOptionsDialog::options() const
{
    core::analysis::AnalysisOptions opts;
    opts.useDirectSolver      = (m_solverCombo->currentIndex() == 0);
    opts.convergenceTolerance = m_toleranceSpin->value();
    opts.maxIterations        = m_maxIterSpin->value();
    opts.computeReactions     = m_computeReactionsCheck->isChecked();
    opts.checkStability       = m_checkStabilityCheck->isChecked();
    opts.verbose              = m_verboseCheck->isChecked();
    return opts;
}

void AnalysisOptionsDialog::setOptions(const core::analysis::AnalysisOptions& opts)
{
    m_solverCombo->setCurrentIndex(opts.useDirectSolver ? 0 : 1);
    m_toleranceSpin->setValue(opts.convergenceTolerance);
    m_maxIterSpin->setValue(opts.maxIterations);
    m_computeReactionsCheck->setChecked(opts.computeReactions);
    m_checkStabilityCheck->setChecked(opts.checkStability);
    m_verboseCheck->setChecked(opts.verbose);
    onSolverTypeChanged(m_solverCombo->currentIndex());
}

}  // namespace truss::gui
