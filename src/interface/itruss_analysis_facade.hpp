/**
 * @file itruss_analysis_facade.hpp
 * @brief Abstract interface for TrussAnalysisFacade (for adapters and mocks)
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/interfaces/ianalysis_service.hpp"
#include "application/interfaces/itruss_service.hpp"
#include "application/result.hpp"
#include "core/interfaces/ianalysis_results_view.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "core/model/truss.hpp"
#include "truss/analysis/analysis_options.hpp"
#include "truss/export/export_format.hpp"

#include <filesystem>
#include <string>

namespace truss::interface {

// Forward declaration: TrussBuilder is defined in truss_builder.hpp.
// Consumers that construct a TrussBuilder must include that header themselves.
class TrussBuilder;

/**
 * @brief Result of a complete analysis workflow
 *
 * Encapsulates all handles needed to access analysis results,
 * providing a unified return type for facade workflow methods.
 * Defined here (on the interface) so that all consumers of
 * ITrussAnalysisFacade can use the type without depending on the
 * concrete TrussAnalysisFacade header.
 */
struct AnalysisWorkflowResult {
    bool success{false};
    std::string errorMessage;

    // Resource handles (valid only if success == true)
    application::TrussHandle trussHandle{0};
    application::ResultsHandle resultsHandle{0};

    // Convenience methods
    explicit operator bool() const noexcept { return success; }
    bool isValid() const noexcept { return success; }

    static AnalysisWorkflowResult Success(application::TrussHandle th,
                                          application::ResultsHandle rh) {
        return {.success = true, .errorMessage = "", .trussHandle = th, .resultsHandle = rh};
    }

    static AnalysisWorkflowResult Failure(std::string error) {
        return {.success = false, .errorMessage = std::move(error), .trussHandle = 0, .resultsHandle = 0};
    }
};

/**
 * @brief Combined interface for truss and analysis services
 *
 * This interface combines ITrussService and IAnalysisService into a single
 * unified interface. It resolves the diamond inheritance problem by
 * explicitly declaring the clearAll() method once, and additionally
 * exposes the high-level workflow methods that consumers (e.g. CLI command
 * classes) depend on.  Declaring these methods here — rather than only on
 * the concrete TrussAnalysisFacade — ensures that high-level modules
 * never need to include or name the concrete implementation.
 *
 * Note: isValidTrussHandle and isValidResultsHandle are already inherited
 * from the respective base interfaces and do not need redeclaration.
 */
// NOLINT(misc-multiple-inheritance) -- intentional: combines ITrussService + IAnalysisService interfaces
class ITrussAnalysisFacade : public truss::application::ITrussService,
                             public truss::application::IAnalysisService {
public:
    virtual ~ITrussAnalysisFacade() = default;

    // =========================================================
    // Ambiguity resolution
    // =========================================================

    // Resolve clearAll() ambiguity from multiple inheritance.
    // Both ITrussService and IAnalysisService declare clearAll().
    // Implementations should clear both truss and analysis resources.
    virtual void clearAll() = 0;

    // =========================================================
    // High-level workflow methods
    // =========================================================
    // These methods are the primary API surface for CLI consumers.
    // Declaring them here (as pure virtuals) ensures that CLI command
    // classes depend only on this interface and never on the concrete
    // TrussAnalysisFacade class, satisfying the Dependency Inversion
    // Principle.

    /**
     * @brief Complete workflow: load → validate → analyze
     * @param filepath Path to truss model file
     * @param options  Analysis configuration options
     * @return AnalysisWorkflowResult with handles on success, error on failure
     */
    virtual AnalysisWorkflowResult
    analyzeFromFile(const std::filesystem::path& filepath,
                    const core::analysis::AnalysisOptions& options = {}) = 0;

    /**
     * @brief Complete workflow: build → validate → analyze
     * @param builder TrussBuilder with configured truss (not modified)
     * @param options Analysis configuration options
     * @return AnalysisWorkflowResult with handles on success, error on failure
     */
    virtual AnalysisWorkflowResult
    analyzeInteractive(TrussBuilder& builder,
                       const core::analysis::AnalysisOptions& options = {}) = 0;

    /**
     * @brief Complete workflow: load → validate (no analysis)
     * @param filepath Path to truss model file
     * @return ValidationResult with detailed validation report
     */
    virtual core::validation::ValidationResult
    validateFromFile(const std::filesystem::path& filepath) = 0;

    /**
     * @brief Export analysis results with explicit format
     *
     * Simplified overload that does not require a separate truss parameter;
     * the facade resolves the associated truss from the results handle.
     *
     * @param resultsHandle Handle to analysis results
     * @param format        Explicit export format
     * @param filepath      Export file path
     * @param options       Export options (default: all data)
     * @return true on success, false on failure
     */
    virtual bool exportResults(application::ResultsHandle resultsHandle,
                               truss::ExportFormat format,
                               const std::filesystem::path& filepath,
                               const infrastructure::export_::ExportOptions& options = {}) = 0;

    /**
     * @brief Export analysis results with format auto-detected from filepath extension
     *
     * Simplified overload that does not require a separate truss parameter.
     *
     * @param resultsHandle Handle to analysis results
     * @param filepath      Export file path (extension determines format)
     * @param options       Export options (default: all data)
     * @return true on success, false on failure
     */
    virtual bool exportResults(application::ResultsHandle resultsHandle,
                               const std::filesystem::path& filepath,
                               const infrastructure::export_::ExportOptions& options = {}) = 0;

    // Bring the IAnalysisService::exportResults overloads (which take an
    // explicit Truss and return Result<bool>) back into scope, so they are
    // not hidden by the simpler bool overloads declared above.
    using truss::application::IAnalysisService::exportResults;

    // All other methods are inherited from base interfaces:
    // - ITrussService: createTruss, loadTruss, saveTruss, clearTruss,
    //                  isValidTrussHandle, getTrussView, getTrussMutable,
    //                  validateTruss, addNode, addMember, removeNode,
    //                  removeMember, setNodeSupport, applyNodeLoad, clearNodeLoad,
    //                  updateNode, updateMember
    // - IAnalysisService: analyze, getResultsView,
    //                     exportResults(handle, format, filepath, truss, opts),
    //                     exportResults(handle, filepath, truss, opts),
    //                     clearResults, isValidResultsHandle
};

}  // namespace truss::interface
