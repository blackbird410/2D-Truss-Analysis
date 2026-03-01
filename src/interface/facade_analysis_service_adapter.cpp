/**
 * @file facade_analysis_service_adapter.cpp
 * @brief IAnalysisService adapter implementation backed by TrussAnalysisFacade.
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 */

#include "facade_analysis_service_adapter.hpp"
#include "truss/analysis/analysis_options.hpp"
#include "truss_analysis_facade.hpp"

namespace truss::interface {

FacadeAnalysisServiceAdapter::FacadeAnalysisServiceAdapter(ITrussAnalysisFacade& facade)
    : m_facade(facade) {}

application::Result<application::ResultsHandle>
FacadeAnalysisServiceAdapter::analyze(const core::Truss& truss,
                                      const core::analysis::AnalysisOptions& options) {
    return m_facade.analyze(truss, options);
}

const core::interfaces::IAnalysisResultsView&
FacadeAnalysisServiceAdapter::getResultsView(application::ResultsHandle handle) const {
    return m_facade.getResultsView(handle);
}

application::Result<bool>
FacadeAnalysisServiceAdapter::exportResults(application::ResultsHandle handle,
                                            truss::ExportFormat format,
                                            const std::filesystem::path& filepath,
                                            const core::Truss& truss,
                                            const infrastructure::export_::ExportOptions& options) {
    return m_facade.exportResults(handle, format, filepath, truss, options);
}

application::Result<bool>
FacadeAnalysisServiceAdapter::exportResults(application::ResultsHandle handle,
                                            const std::filesystem::path& filepath,
                                            const core::Truss& truss,
                                            const infrastructure::export_::ExportOptions& options) {
    return m_facade.exportResults(handle, filepath, truss, options);
}

bool FacadeAnalysisServiceAdapter::clearResults(application::ResultsHandle handle) {
    return m_facade.clearResults(handle);
}

void FacadeAnalysisServiceAdapter::clearAll() {
    m_facade.clearAll();
}

bool FacadeAnalysisServiceAdapter::isValidResultsHandle(application::ResultsHandle handle) const {
    return m_facade.isValidResultsHandle(handle);
}

}  // namespace truss::interface
