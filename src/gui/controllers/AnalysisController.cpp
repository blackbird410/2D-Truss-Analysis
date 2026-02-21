#include "AnalysisController.hpp"
#include <stdexcept>

namespace truss_controllers {

AnalysisController::AnalysisController(
    truss::application::ITrussService* trussService,
    truss::application::IAnalysisService* analysisService,
    truss_presenters::AnalysisResultsPresenter& analysisPresenter,
    truss_presenters::ValidationPresenter& validationPresenter,
    QObject* parent)
    : QObject(parent)
    , m_trussService(trussService)
    , m_analysisService(analysisService)
    , m_analysisPresenter(analysisPresenter)
    , m_validationPresenter(validationPresenter)
    , m_currentResultsHandle(0)
    , m_currentTrussHandle(0)
{
    if (!m_trussService) {
        throw std::invalid_argument("AnalysisController: null truss service pointer");
    }
    if (!m_analysisService) {
        throw std::invalid_argument("AnalysisController: null analysis service pointer");
    }
}

void AnalysisController::onAnalyzeRequested(truss::application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        emit analysisFailed("Invalid truss handle");
        return;
    }
    
    // Store current truss handle for export
    m_currentTrussHandle = trussHandle;
    
    emit analysisStarted();
    emit statusMessageChanged("Validating structure...");
    
    // Step 1: Validate truss
    auto validationResult = m_trussService->validateTruss(trussHandle);
    
    if (!validationResult.success) {
        emit analysisFailed(QString::fromStdString(validationResult.errorMessage));
        return;
    }
    
    // Check if validation passed
    if (!validationResult.value.isValid()) {
        auto display = m_validationPresenter.formatValidation(validationResult.value);
        emit validationFailed(display);
        emit statusMessageChanged("Validation failed");
        return;
    }
    
    emit statusMessageChanged("Running analysis...");
    
    // Step 2: Execute analysis
    const auto& truss = m_trussService->getTrussMutable(trussHandle);
    auto analysisResult = m_analysisService->analyze(truss);
    
    if (!analysisResult.success) {
        emit analysisFailed(QString::fromStdString(analysisResult.errorMessage));
        emit statusMessageChanged("Analysis failed");
        return;
    }
    
    // Step 3: Store results and emit success
    m_currentResultsHandle = analysisResult.value;
    emit analysisCompleted(m_currentResultsHandle);
    emit statusMessageChanged("Analysis completed successfully");
}

void AnalysisController::onExportRequested(
    size_t resultsHandle,
    const QString& filepath)
{
    if (resultsHandle == 0) {
        emit exportFailed("No analysis results available");
        return;
    }
    
    if (m_currentTrussHandle == 0) {
        emit exportFailed("No truss model available");
        return;
    }
    
    emit statusMessageChanged("Exporting results...");
    
    const auto& truss = m_trussService->getTrussMutable(m_currentTrussHandle);
    auto result = m_analysisService->exportResults(
        resultsHandle,
        filepath.toStdString(),
        truss
    );
    
    if (result.success) {
        emit exportCompleted(filepath);
        emit statusMessageChanged(QString("Results exported to %1").arg(filepath));
    } else {
        emit exportFailed(QString::fromStdString(result.errorMessage));
        emit statusMessageChanged("Export failed");
    }
}

void AnalysisController::onClearResults() {
    if (m_currentResultsHandle != 0) {
        // Clear results via service if needed
        m_currentResultsHandle = 0;
        emit statusMessageChanged("Results cleared");
    }
}

}  // namespace truss_controllers
