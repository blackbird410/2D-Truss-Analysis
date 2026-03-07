/**
 * @file analysis_controller.hpp
 * @brief New AnalysisController (truss::gui::ctrl namespace) replacing the
 *        legacy truss_controllers::AnalysisController from Phase 8 onward.
 *
 * Phase 5: Full Q_OBJECT implementation with background QThread worker.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "core/analysis/analysis_orchestrator.hpp"

#include <QObject>
#include <QString>

#include <cstddef>

class QThread;

namespace truss::interface {
class ITrussAnalysisFacade;
}

namespace truss::gui::ctrl {

/**
 * @brief Orchestrates analysis workflow on a background QThread.
 *
 * Constructs AnalysisWorker, moves it to a QThread, and manages the
 * full analysis lifecycle:
 *  1. Takes a value copy of the current truss on the main thread (thread-safety)
 *  2. Emits analysisStarted() to update UI to Analysing phase
 *  3. Worker calls IAnalysisService::analyze(trussCopy, opts) on a background thread
 *  4. On completion emits analysisCompleted(ResultsHandle)
 *  5. On failure emits analysisFailed(errorMessage)
 *
 * @note AnalysisWorker is defined in the .cpp; including this header does not
 *       require QThread to be complete — hence forward declaration is sufficient.
 */
class AnalysisController : public QObject {
    Q_OBJECT

public:
    explicit AnalysisController(truss::interface::ITrussAnalysisFacade& facade,
                                 QObject*                                 parent = nullptr);
    ~AnalysisController() override;

    /// @brief Handle of the most recent successful analysis result (0 = none).
    [[nodiscard]] std::size_t currentResultsHandle() const noexcept;

public slots:
    void onAnalyzeRequested(const truss::core::analysis::AnalysisOptions& opts);
    /// Request to stop an in-progress analysis.
    void onStopRequested();
    /// Update the active truss handle (called by MainWindowController).
    void onTrussHandleUpdated(std::size_t trussHandle);

signals:
    void analysisStarted();
    void analysisCompleted(std::size_t resultsHandle);
    void analysisFailed(const QString& errorMessage);

private slots:
    void onWorkerFinished(std::size_t resultsHandle);
    void onWorkerFailed(const QString& error);

private:
    void cleanupThread();

    truss::interface::ITrussAnalysisFacade& m_facade;
    std::size_t                              m_trussHandle{0};
    std::size_t                              m_resultsHandle{0};
    QThread*                                 m_thread{nullptr};
};

}  // namespace truss::gui::ctrl
