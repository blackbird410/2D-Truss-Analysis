/**
 * @file AnalysisApplicationService.hpp
 * @brief Application facade for structural analysis operations
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This service provides a high-level API for Interface layer (GUI/CLI) to perform
 * structural analysis without direct dependency on Domain analysis implementation.
 *
 * Responsibilities:
 * - Run structural analysis
 * - Export analysis results
 * - Manage analysis configuration
 * - Manage results lifecycle
 *
 * Design Pattern: Facade (simplifies complex subsystems)
 * Architecture: Application Layer (orchestrates Domain + Infrastructure)
 */

#pragma once

#include "../core/analysis/AnalysisOrchestrator.hpp"
#include "../core/analysis/DirectSolver.hpp"
#include "../core/analysis/IterativeSolver.hpp"
#include "../core/interfaces/IAnalysisResultsView.hpp"
#include "../core/model/Truss.hpp"
#include "../core/validation/TrussValidator.hpp"
#include "../infrastructure/export/exporter_factory.hpp"
#include "TrussApplicationService.hpp"  // For Result<T> template
#include "interfaces/IAnalysisService.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace truss::application {

/**
 * @brief Application service for structural analysis operations
 *
 * This service coordinates analysis workflow:
 * 1. Validate truss structure
 * 2. Configure analysis options
 * 3. Execute analysis via AnalysisOrchestrator
 * 4. Store results for later export/visualization
 * 5. Export results in various formats
 *
 * Thread Safety: Not thread-safe (intended for single-threaded applications)
 *
 * Usage Example:
 * @code
 * AnalysisApplicationService analysisService;
 * TrussApplicationService trussService;
 *
 * auto trussResult = trussService.loadTruss("bridge.json");
 * if (trussResult) {
 *     auto& truss = trussService.getTrussMutable(trussResult.value);
 *     auto analysisResult = analysisService.analyze(truss);
 *     if (analysisResult) {
 *         const auto& results = analysisService.getResultsView(analysisResult.value);
 *         analysisService.exportResults(
 *             analysisResult.value,
 *             infrastructure::export_::ExportFormat::CSV,
 *             "results.csv",
 *             truss,
 *             {}
 *         );
 *     }
 * }
 * @endcode
 */
class AnalysisApplicationService : public IAnalysisService {
public:
    AnalysisApplicationService();
    ~AnalysisApplicationService() override = default;

    // Disable copy (manages unique resources)
    AnalysisApplicationService(const AnalysisApplicationService&) = delete;
    AnalysisApplicationService& operator=(const AnalysisApplicationService&) = delete;

    // Allow move
    AnalysisApplicationService(AnalysisApplicationService&&) noexcept = default;
    AnalysisApplicationService& operator=(AnalysisApplicationService&&) noexcept = default;

    /**
     * @brief Run structural analysis on truss
     * @param truss Truss model to analyze
     * @param options Analysis configuration options
     * @return Result containing ResultsHandle on success
     */
    Result<ResultsHandle> analyze(const core::Truss& truss,
                                  const core::analysis::AnalysisOptions& options = {}) override;

    /**
     * @brief Get read-only view of analysis results
     * @param handle Results handle
     * @return Reference to IAnalysisResultsView interface
     * @throws std::invalid_argument if handle is invalid
     */
    const core::interfaces::IAnalysisResultsView&
    getResultsView(ResultsHandle handle) const override;

    /**
     * @brief Get mutable access to analysis results
     * @param handle Results handle
     * @return Reference to AnalysisResults
     * @throws std::invalid_argument if handle is invalid
     */
    core::analysis::AnalysisResults& getResults(ResultsHandle handle);

    /**
     * @brief Export analysis results to file
     * @param handle Results handle
     * @param format Export format (CSV, JSON, XML, etc.)
     * @param filepath Output file path
     * @param truss Truss model (for context/metadata)
     * @param options Export options
     * @return Result indicating success/failure
     */
    Result<bool> exportResults(ResultsHandle handle,
                               infrastructure::export_::ExportFormat format,
                               const std::filesystem::path& filepath,
                               const core::Truss& truss,
                               const infrastructure::export_::ExportOptions& options = {}) override;

    /**
     * @brief Export results with auto-detected format
     * @param handle Results handle
     * @param filepath Output file path (format detected from extension)
     * @param truss Truss model (for context/metadata)
     * @param options Export options
     * @return Result indicating success/failure
     */
    Result<bool> exportResults(ResultsHandle handle,
                               const std::filesystem::path& filepath,
                               const core::Truss& truss,
                               const infrastructure::export_::ExportOptions& options = {}) override;

    /**
     * @brief Clear analysis results
     * @param handle Results handle to remove
     * @return true if results were deleted, false if handle was invalid
     */
    bool clearResults(ResultsHandle handle) override;

    /**
     * @brief Clear all analysis results
     */
    void clearAll() override;

    /**
     * @brief Check if handle is valid
     * @param handle Results handle to check
     * @return true if handle references existing results
     */
    bool isValidHandle(ResultsHandle handle) const override;

    /**
     * @brief Get count of stored results
     * @return Number of result sets currently managed
     */
    size_t getResultsCount() const { return m_results.size(); }

private:
    std::unordered_map<ResultsHandle, std::shared_ptr<core::analysis::AnalysisResults>> m_results;
    ResultsHandle m_nextHandle{1};

    ResultsHandle generateHandle() { return m_nextHandle++; }
};

}  // namespace truss::application
