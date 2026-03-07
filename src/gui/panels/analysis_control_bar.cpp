/**
 * @file analysis_control_bar.cpp
 * @brief AnalysisControlBar implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/panels/analysis_control_bar.hpp"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>

namespace truss::gui {

AnalysisControlBar::AnalysisControlBar(QWidget* parent) : QWidget{parent} {
    setObjectName(QStringLiteral("analysisControlBar"));
    buildLayout();
}

void AnalysisControlBar::buildLayout() {
    auto* hbox = new QHBoxLayout{this};
    hbox->setContentsMargins(8, 4, 8, 4);
    hbox->setSpacing(6);

    // ---- Run / Progress swap stack ----
    m_actionStack = new QStackedWidget{this};
    m_actionStack->setObjectName(QStringLiteral("controlBar_actionStack"));

    // Page 0: Run button
    auto* runPage = new QWidget{m_actionStack};
    auto* runLayout = new QHBoxLayout{runPage};
    runLayout->setContentsMargins(0, 0, 0, 0);
    m_runBtn = new QPushButton{QStringLiteral("▶  Run Analysis"), runPage};
    m_runBtn->setObjectName(QStringLiteral("controlBar_runBtn"));
    runLayout->addWidget(m_runBtn);
    m_actionStack->addWidget(runPage);  // index 0

    // Page 1: Progress bar + Stop button
    auto* progressPage = new QWidget{m_actionStack};
    auto* progressLayout = new QHBoxLayout{progressPage};
    progressLayout->setContentsMargins(0, 0, 0, 0);
    m_progressBar = new QProgressBar{progressPage};
    m_progressBar->setObjectName(QStringLiteral("controlBar_progressBar"));
    m_progressBar->setRange(0, 0);  // indeterminate
    m_stopBtn = new QPushButton{QStringLiteral("■  Stop"), progressPage};
    m_stopBtn->setObjectName(QStringLiteral("controlBar_stopBtn"));
    progressLayout->addWidget(m_progressBar, 1);
    progressLayout->addWidget(m_stopBtn);
    m_actionStack->addWidget(progressPage);  // index 1

    hbox->addWidget(m_actionStack, 1);

    // ---- Validate button ----
    m_validateBtn = new QPushButton{QStringLiteral("✔  Validate"), this};
    m_validateBtn->setObjectName(QStringLiteral("controlBar_validateBtn"));
    hbox->addWidget(m_validateBtn);

    // ---- Options button ----
    m_optionsBtn = new QPushButton{QStringLiteral("⚙"), this};
    m_optionsBtn->setObjectName(QStringLiteral("controlBar_optionsBtn"));
    m_optionsBtn->setToolTip(QStringLiteral("Analysis options"));
    m_optionsBtn->setFixedWidth(32);
    hbox->addWidget(m_optionsBtn);

    // ---- Connections ----
    connect(m_runBtn, &QPushButton::clicked, this, [this] { emit analyzeRequested(m_opts); });
    connect(m_stopBtn, &QPushButton::clicked, this, &AnalysisControlBar::stopRequested);
    connect(m_validateBtn, &QPushButton::clicked, this, &AnalysisControlBar::validateRequested);
    connect(m_optionsBtn, &QPushButton::clicked, this, &AnalysisControlBar::optionsRequested);
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

void AnalysisControlBar::setOptions(const core::analysis::AnalysisOptions& opts) {
    m_opts = opts;
}

const core::analysis::AnalysisOptions& AnalysisControlBar::options() const noexcept {
    return m_opts;
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void AnalysisControlBar::onStateChanged(const truss::gui::state::WorkspaceState& state) {
    using truss::gui::state::WorkspacePhase;

    switch (state.phase) {
        case WorkspacePhase::Analysing:
            m_actionStack->setCurrentIndex(1);  // show progress bar
            m_validateBtn->setEnabled(false);
            m_optionsBtn->setEnabled(false);
            break;

        case WorkspacePhase::ModelBuilding:
        case WorkspacePhase::ResultsReady:
            m_actionStack->setCurrentIndex(0);  // show run button
            m_runBtn->setEnabled(state.hasTruss());
            m_validateBtn->setEnabled(state.hasTruss());
            m_optionsBtn->setEnabled(true);
            break;

        case WorkspacePhase::Empty:
        case WorkspacePhase::Validating:
        default:
            m_actionStack->setCurrentIndex(0);
            m_runBtn->setEnabled(false);
            m_validateBtn->setEnabled(false);
            m_optionsBtn->setEnabled(true);
            break;
    }
}

}  // namespace truss::gui
