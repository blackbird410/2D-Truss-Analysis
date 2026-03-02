/**
 * @file analysis_controller_v2.hpp
 * @brief New AnalysisController (truss::gui::ctrl namespace) that replaces the
 *        legacy truss_controllers::AnalysisController from Phase 8 onward.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 5.
 *
 * @note NAMING: This file uses the _v2 suffix temporarily to coexist with the
 *       legacy src/gui/controllers/analysis_controller.hpp during Phases 1–7.
 *       In Phase 8 the legacy file is deleted and this file is renamed to
 *       analysis_controller.hpp.
 *
 * @note Q_OBJECT is added in Phase 5.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QObject>

namespace truss::application {
    class IAnalysisService;
    class ITrussService;
}

namespace truss::gui::ctrl {

/**
 * @brief Orchestrates analysis workflow on a background QThread.
 *
 * Constructs AnalysisWorker, moves it to a QThread, and manages the
 * full analysis lifecycle:
 *  1. Takes a value copy of the current truss (thread-safety pattern)
 *  2. Emits analysisStarted() to update UI to Analysing phase
 *  3. Invokes AnalysisWorker::doAnalysis() on the worker thread
 *  4. On completion emits analysisCompleted(ResultsHandle)
 *  5. On failure emits analysisFailed(errorMessage)
 *
 * @note Thread safety: AnalysisWorker::doAnalysis() operates solely on the
 *       value-copied Truss and never calls getTrussView/getTrussMutable from
 *       the worker thread. Results are posted back via Qt::QueuedConnection.
 *
 * @todo Phase 5: Add Q_OBJECT macro, construct AnalysisWorker + QThread,
 *       implement onAnalyzeRequested slot with snapshot copy pattern,
 *       emit analysisStarted/analysisCompleted/analysisFailed signals.
 */
class AnalysisController : public QObject {
public:
    explicit AnalysisController(QObject* parent = nullptr) : QObject(parent) {}

    // TODO Phase 5: explicit AnalysisController(application::IAnalysisService* analysisService,
    //                                             application::ITrussService* trussService,
    //                                             QObject* parent = nullptr)
    // TODO Phase 5: std::size_t currentResultsHandle() const (replaces legacy getCurrentResults())
    // TODO Phase 5: public slots:
    //   void onAnalyzeRequested(AnalysisOptions opts)
    //   void onTrussHandleUpdated(std::size_t trussHandle)
    // TODO Phase 5: signals:
    //   void analysisStarted()
    //   void analysisCompleted(std::size_t resultsHandle)
    //   void analysisFailed(const QString& errorMessage)
};

}  // namespace truss::gui::ctrl
