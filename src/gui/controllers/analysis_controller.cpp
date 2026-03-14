/**
 * @file analysis_controller.cpp
 * @brief AnalysisController implementation — QThread background worker (Phase 5).
 *
 * Thread-safety pattern:
 *   1. Main thread copies the Truss by value from the facade.
 *   2. Copy is moved into AnalysisWorker before the thread starts.
 *   3. Worker calls IAnalysisService::analyze(trussCopy, opts) on the worker thread.
 *   4. Results are posted back to the main thread via Qt::QueuedConnection.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/analysis_controller.hpp"

#include "application/interfaces/ianalysis_service.hpp"
#include "core/model/truss.hpp"
#include "interface/itruss_analysis_facade.hpp"

#include <QThread>

namespace truss::gui::ctrl {

// ===========================================================================
// AnalysisWorker — internal helper moved to a background QThread
// ===========================================================================

class AnalysisWorker : public QObject {
    Q_OBJECT

public:
    AnalysisWorker(truss::core::Truss truss,
                   truss::core::analysis::AnalysisOptions opts,
                   truss::application::IAnalysisService* service)
        : m_truss{std::move(truss)}, m_opts{opts}, m_service{service} {}

public slots:
    void execute() {
        auto result = m_service->analyze(m_truss, m_opts);
        if (result) {
            emit finished(result.value);
        } else {
            emit failed(QString::fromStdString(result.errorMessage));
        }
    }

signals:
    void finished(std::size_t resultsHandle);
    void failed(const QString& error);

private:
    truss::core::Truss m_truss;
    truss::core::analysis::AnalysisOptions m_opts;
    truss::application::IAnalysisService* m_service;
};

// ===========================================================================
// AnalysisController
// ===========================================================================

AnalysisController::AnalysisController(truss::interface::ITrussAnalysisFacade& facade,
                                       QObject* parent)
    : QObject{parent}, m_facade{facade} {}

AnalysisController::~AnalysisController() {
    cleanupThread();
}

std::size_t AnalysisController::currentResultsHandle() const noexcept {
    return m_resultsHandle;
}

void AnalysisController::onTrussHandleUpdated(std::size_t trussHandle) {
    m_trussHandle = trussHandle;
}

void AnalysisController::onAnalyzeRequested(const core::analysis::AnalysisOptions& opts) {
    if (m_trussHandle == 0) {
        emit analysisFailed(QStringLiteral("No active truss to analyze."));
        return;
    }

    if (m_thread && m_thread->isRunning()) {
        // Analysis already in progress — ignore duplicate request.
        return;
    }

    // ---- Step 1: copy the Truss on the main thread ----
    truss::core::Truss trussCopy = m_facade.getTrussMutable(m_trussHandle);

    // ---- Step 2: announce start ----
    emit analysisStarted();

    // ---- Step 3: set up worker + thread ----
    m_thread = new QThread{this};

    // IAnalysisService is inherited by ITrussAnalysisFacade
    auto* service = static_cast<truss::application::IAnalysisService*>(&m_facade);

    auto* worker = new AnalysisWorker{std::move(trussCopy), opts, service};
    worker->moveToThread(m_thread);

    // Worker lifetime: deleted when thread finishes
    connect(m_thread, &QThread::finished, worker, &QObject::deleteLater);

    // Results routing (QueuedConnection — worker thread → main thread)
    connect(worker,
            &AnalysisWorker::finished,
            this,
            &AnalysisController::onWorkerFinished,
            Qt::QueuedConnection);
    connect(worker,
            &AnalysisWorker::failed,
            this,
            &AnalysisController::onWorkerFailed,
            Qt::QueuedConnection);

    // Start execution
    connect(m_thread, &QThread::started, worker, &AnalysisWorker::execute);
    m_thread->start();
}

void AnalysisController::onStopRequested() {
    if (m_thread && m_thread->isRunning()) {
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait(3000);
        emit analysisFailed(QStringLiteral("Analysis stopped by user."));
        cleanupThread();
    }
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void AnalysisController::onWorkerFinished(std::size_t resultsHandle) {
    m_resultsHandle = resultsHandle;
    cleanupThread();
    emit analysisCompleted(resultsHandle);
}

void AnalysisController::onWorkerFailed(const QString& error) {
    cleanupThread();
    emit analysisFailed(error);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void AnalysisController::cleanupThread() {
    if (m_thread) {
        if (m_thread->isRunning()) {
            m_thread->quit();
            m_thread->wait();
        }
        m_thread->deleteLater();
        m_thread = nullptr;
    }
}

}  // namespace truss::gui::ctrl

// AUTOMOC cannot find Q_OBJECT classes defined in .cpp files unless the .moc
// is explicitly included at the bottom.
#include "analysis_controller.moc"
