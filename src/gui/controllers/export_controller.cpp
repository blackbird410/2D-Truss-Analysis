/**
 * @file export_controller.cpp
 * @brief ExportController implementation (Phase 5).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/export_controller.hpp"

#include "interface/itruss_analysis_facade.hpp"

namespace truss::gui::ctrl {

ExportController::ExportController(truss::interface::ITrussAnalysisFacade& facade,
                                    QObject*                                 parent)
    : QObject{parent}, m_facade{facade}
{}

void ExportController::onResultsHandleUpdated(std::size_t resultsHandle)
{
    m_resultsHandle = resultsHandle;
}

void ExportController::onExportRequested(truss::ExportFormat format, const QString& filePath)
{
    if (m_resultsHandle == 0) {
        emit exportFailed(QStringLiteral("No analysis results available to export."));
        return;
    }

    const bool ok = m_facade.exportResults(
        m_resultsHandle, format, std::filesystem::path{filePath.toStdString()});

    if (ok) {
        emit exportCompleted(filePath);
    } else {
        emit exportFailed(QStringLiteral("Export failed: could not write to ") + filePath);
    }
}

}  // namespace truss::gui::ctrl
