/**
 * @file facade_analysis_service_adapter.hpp
 * @brief Adapter that exposes TrussAnalysisFacade through IAnalysisService.
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 */

#pragma once
#include "application/interfaces/ianalysis_service.hpp"
#include "interface/itruss_analysis_facade.hpp"
#include "truss/analysis/analysis_options.hpp"

namespace truss::interface {

/**
 * @brief IAnalysisService adapter backed by TrussAnalysisFacade
 *
 * Provides GUI compatibility while routing operations through the
 * facade-owned analysis service.
 */
class ITrussAnalysisFacade;
class FacadeAnalysisServiceAdapter final : public application::IAnalysisService {
public:
    explicit FacadeAnalysisServiceAdapter(truss::interface::ITrussAnalysisFacade& facade);

    application::Result<application::ResultsHandle>
    analyze(const core::Truss& truss, const core::analysis::AnalysisOptions& options = {}) override;

    const core::interfaces::IAnalysisResultsView&
    getResultsView(application::ResultsHandle handle) const override;

    application::Result<bool>
    exportResults(application::ResultsHandle handle,
                  truss::ExportFormat format,
                  const std::filesystem::path& filepath,
                  const core::Truss& truss,
                  const infrastructure::export_::ExportOptions& options = {}) override;

    application::Result<bool>
    exportResults(application::ResultsHandle handle,
                  const std::filesystem::path& filepath,
                  const core::Truss& truss,
                  const infrastructure::export_::ExportOptions& options = {}) override;

    bool clearResults(application::ResultsHandle handle) override;
    void clearAll() override;
    bool isValidResultsHandle(application::ResultsHandle handle) const override;

private:
    truss::interface::ITrussAnalysisFacade& m_facade;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

}  // namespace truss::interface
