/**
 * @file export_controller.hpp
 * @brief Controller mediating export requests from result panels and
 *        IAnalysisService::exportResults calls.
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

#include <QObject>

namespace truss::application { class IAnalysisService; }

namespace truss::gui::ctrl {

/**
 * @brief Handles export requests from ResultsDockPanel.
 *
 * Receives exportRequested(format, path) signals from ResultsDockPanel and
 * calls IAnalysisService::exportResults(resultsHandle, format, path).
 *
 * On success:  emits exportCompleted(outputPath)
 * On failure:  emits exportFailed(errorMessage) → NotificationRail
 *
 * @note ExportController receives IAnalysisService* (not the full facade)
 *       because it only needs export operations.
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement onExportRequested slot with
 *       IAnalysisService call, hold resultsHandle updated from stateChanged.
 */
class ExportController : public QObject {
public:
    explicit ExportController(QObject* parent = nullptr) : QObject(parent) {}

    // TODO Phase 5: explicit ExportController(application::IAnalysisService* service,
    //                                          QObject* parent = nullptr)
    // TODO Phase 5: public slots:
    //   void onExportRequested(ExportFormat format, const QString& filePath)
    //   void onResultsHandleUpdated(std::size_t resultsHandle)
    // TODO Phase 5: signals:
    //   void exportCompleted(const QString& outputPath)
    //   void exportFailed(const QString& errorMessage)
};

}  // namespace truss::gui::ctrl
