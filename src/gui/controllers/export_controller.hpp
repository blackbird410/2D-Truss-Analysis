/**
 * @file export_controller.hpp
 * @brief Controller mediating export requests from ResultsDockPanel and
 *        ITrussAnalysisFacade::exportResults calls.
 *
 * Phase 5: Full Q_OBJECT implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "truss/export/export_format.hpp"

#include <QObject>
#include <QString>

#include <cstddef>

namespace truss::interface {
class ITrussAnalysisFacade;
}

namespace truss::gui::ctrl {

/**
 * @brief Handles export requests from ResultsDockPanel.
 *
 * Receives exportRequested(format, filepath) and forwards to
 * facade.exportResults(resultsHandle, format, filepath).  The active
 * results handle is updated via onResultsHandleUpdated.
 *
 * On success emits exportCompleted(path).
 * On failure emits exportFailed(errorMessage).
 */
class ExportController : public QObject {
    Q_OBJECT

public:
    explicit ExportController(truss::interface::ITrussAnalysisFacade& facade,
                              QObject* parent = nullptr);

public slots:
    void onExportRequested(truss::ExportFormat format, const QString& filePath);
    /// Update active results handle (called by MainWindowController).
    void onResultsHandleUpdated(std::size_t resultsHandle);

signals:
    void exportCompleted(const QString& outputPath);
    void exportFailed(const QString& errorMessage);

private:
    truss::interface::ITrussAnalysisFacade& m_facade;
    std::size_t m_resultsHandle{0};
};

}  // namespace truss::gui::ctrl
