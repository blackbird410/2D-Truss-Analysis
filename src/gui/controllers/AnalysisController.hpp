/**
 * @file AnalysisController.hpp
 * @brief Controller for coordinating structural analysis workflow
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This controller orchestrates the analysis workflow:
 * 1. Truss validation before analysis
 * 2. Analysis execution
 * 3. Results retrieval and presentation
 * 4. Results export operations
 * 5. Results lifecycle management
 * 
 * Uses Qt signal/slot mechanism for asynchronous workflow coordination.
 * 
 * Architecture: GUI Controller Layer (MVP Pattern)
 * Dependencies: TrussApplicationService, AnalysisApplicationService,
 *               AnalysisResultsPresenter, ValidationPresenter
 */

#pragma once

#include <QObject>
#include <QString>
#include "application/TrussApplicationService.hpp"
#include "application/AnalysisApplicationService.hpp"
#include "gui/presenters/AnalysisResultsPresenter.hpp"
#include "gui/presenters/ValidationPresenter.hpp"

namespace truss_controllers {

/**
 * @brief Coordinates structural analysis workflow
 * 
 * This controller handles the analysis workflow: validation, analysis execution,
 * results retrieval, and export operations. It mediates between Views and
 * Application layer services.
 */
class AnalysisController : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Construct AnalysisController
     * 
     * @param trussService Reference to TrussApplicationService
     * @param analysisService Reference to AnalysisApplicationService
     * @param analysisPresenter Reference to AnalysisResultsPresenter
     * @param validationPresenter Reference to ValidationPresenter
     * @param parent Qt parent object
     */
    explicit AnalysisController(
        truss::application::TrussApplicationService& trussService,
        truss::application::AnalysisApplicationService& analysisService,
        truss_presenters::AnalysisResultsPresenter& analysisPresenter,
        truss_presenters::ValidationPresenter& validationPresenter,
        QObject* parent = nullptr);
    
    /**
     * @brief Get current results handle
     * 
     * @return size_t Current results handle (0 if invalid)
     */
    size_t getCurrentResults() const { return m_currentResultsHandle; }

public slots:
    /**
     * @brief Handle request to analyze truss
     * 
     * This performs validation first, then executes analysis if valid.
     * 
     * @param trussHandle Handle to truss to analyze
     */
    void onAnalyzeRequested(truss::application::TrussHandle trussHandle);
    
    /**
     * @brief Handle request to export analysis results
     * 
     * @param resultsHandle Handle to results to export
     * @param filepath Export file path (extension determines format)
     */
    void onExportRequested(size_t resultsHandle,
                            const QString& filepath);
    
    /**
     * @brief Handle request to clear analysis results
     */
    void onClearResults();

signals:
    /**
     * @brief Emitted when analysis starts
     */
    void analysisStarted();
    
    /**
     * @brief Emitted when analysis completes successfully
     * 
     * @param handle Handle to new analysis results
     */
    void analysisCompleted(size_t handle);
    
    /**
     * @brief Emitted when analysis fails
     * 
     * @param errorMessage User-friendly error message
     */
    void analysisFailed(const QString& errorMessage);
    
    /**
     * @brief Emitted when validation fails
     * 
     * @param display Formatted validation display data
     */
    void validationFailed(const truss_presenters::ValidationPresenter::ValidationDisplay& display);
    
    /**
     * @brief Emitted when export completes successfully
     * 
     * @param filepath Path where results were exported
     */
    void exportCompleted(const QString& filepath);
    
    /**
     * @brief Emitted when export fails
     * 
     * @param errorMessage User-friendly error message
     */
    void exportFailed(const QString& errorMessage);
    
    /**
     * @brief Emitted to update status message
     * 
     * @param message Status message
     */
    void statusMessageChanged(const QString& message);

private:
    truss::application::TrussApplicationService& m_trussService;
    truss::application::AnalysisApplicationService& m_analysisService;
    truss_presenters::AnalysisResultsPresenter& m_analysisPresenter;
    truss_presenters::ValidationPresenter& m_validationPresenter;
    size_t m_currentResultsHandle;
    truss::application::TrussHandle m_currentTrussHandle;
};

}  // namespace truss_controllers
