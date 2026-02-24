/**
 * @file results_widget.hpp
 * @brief Widget for displaying analysis results in tables and summaries.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/analysis_application_service.hpp"
#include "application/truss_application_service.hpp"

#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

namespace truss::gui {

/**
 * @brief Widget for visualizing analysis results
 *
 * This widget follows Clean Architecture principles:
 * - Uses dependency injection for services
 * - Does not own or directly access Domain entities
 * - Accesses data through interface abstractions (ITrussView, IAnalysisResultsView)
 * - All rendering is read-only
 *
 * Architecture:
 * - Depends on: TrussApplicationService, AnalysisApplicationService (injected)
 * - Uses: ITrussView, IAnalysisResultsView interfaces for data access
 * - No controller dependency (read-only widget)
 */
class ResultsWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct ResultsWidget with dependency injection
     * @param trussService Application service for truss data
     * @param analysisService Application service for analysis results
     * @param parent Qt parent widget
     */
    explicit ResultsWidget(application::TrussApplicationService& trussService,
                           application::AnalysisApplicationService& analysisService,
                           QWidget* parent = nullptr);

public slots:
    /**
     * @brief Update results display for a truss with analysis results
     * @param trussHandle Handle to the truss to display (with embedded results)
     */
    void updateResults(application::TrussHandle trussHandle);

    /**
     * @brief Clear all results displays
     */
    void clearResults();

private:
    void setupUI();
    void updateDisplacementsTable(application::TrussHandle trussHandle);
    void updateForcesTable(application::TrussHandle trussHandle);
    void updateReactionsTable(application::TrussHandle trussHandle);
    void updateStiffnessTable(application::TrussHandle trussHandle);
    void updateSummary(application::TrussHandle trussHandle);

    // Service dependencies (injected)
    application::TrussApplicationService& m_trussService;
    application::AnalysisApplicationService& m_analysisService;

    // Current state
    application::TrussHandle m_currentTrussHandle;

    // UI components
    QTableWidget* m_displacementsTable;
    QTableWidget* m_forcesTable;
    QTableWidget* m_reactionsTable;
    QTableWidget* m_stiffnessTable;
    QTextEdit* m_summaryText;
};

}  // namespace truss::gui
