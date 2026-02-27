/**
 * @file truss_analysis_facade.hpp
 * @brief Unified facade for complete truss analysis workflows
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 *
 * Provides a simplified, high-level API for complete truss analysis workflows,
 * orchestrating interactions between Application services (TrussApplicationService,
 * AnalysisApplicationService) to reduce complexity for end users.
 *
 * Design Pattern: Facade (simplifies complex subsystem interactions)
 * Architecture: Interface Layer (orchestrates Application services)
 *
 * Usage Example:
 * @code
 * TrussAnalysisFacade facade;
 *
 * // Complete workflow in one call
 * auto result = facade.analyzeFromFile("bridge.json");
 * if (result) {
 *     facade.exportResults(result.handle, ExportFormat::CSV, "results.csv");
 * }
 *
 * // Interactive construction
 * TrussBuilder builder("Custom Truss");
 * builder.addNode(0, 0, SupportType::Pinned)
 *        .addNode(4, 0, SupportType::RollerY)
 *        .addMember(1, 2);
 * auto result2 = facade.analyzeInteractive(builder);
 * @endcode
 */

#pragma once

#include "../application/analysis_application_service.hpp"
#include "../application/truss_application_service.hpp"
#include "../core/analysis/analysis_orchestrator.hpp"
#include "../core/validation/truss_validator.hpp"
#include "../infrastructure/export/exporter_factory.hpp"
#include "truss_builder.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace truss::interface {

class FacadeTrussServiceAdapter;
class FacadeAnalysisServiceAdapter;

/**
 * @brief Result of a complete analysis workflow
 *
 * Encapsulates all handles needed to access analysis results,
 * providing a unified return type for facade methods.
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
        return {true, "", th, rh};
    }

    static AnalysisWorkflowResult Failure(const std::string& error) { return {false, error, 0, 0}; }
};

/**
 * @brief Unified facade for truss analysis workflows
 *
 * TrussAnalysisFacade provides simplified, high-level methods for complete
 * analysis workflows, hiding the complexity of coordinating multiple
 * application services.
 *
 * **Design Pattern:** Facade (with proper encapsulation)  
 * - Provides complete workflow methods (analyzeFromFile, analyzeInteractive, etc.)
 * - Exposes service operations via public methods (NOT friend access)
 * - Allows adapters to implement service interfaces by delegating to Facade public API
 * - Maintains clear encapsulation and SOLID principles
 *
 * Key Features:
 * - One-call complete workflows (load → validate → analyze → export)
 * - Automatic resource management
 * - Clear error reporting
 * - Builder pattern integration
 * - Validation before analysis
 * - Polymorphic service interface implementation via adapters
 *
 * Architecture:
 * - Owns application service instances (m_trussService, m_analysisService)
 * - Provides public methods for all service operations (satisfying ITrussService/IAnalysisService)
 * - Adapters delegate to these public methods (NO friend declarations needed)
 * - GUI uses Facade polymorphically via adapters implementing service interfaces
 *
 * Thread Safety: Not thread-safe (intended for single-threaded use)
 */
class TrussAnalysisFacade final {
public:
    /**
     * @brief Construct a new facade with default services
     */
    TrussAnalysisFacade();

    // Disable copy (manages resources via application services)
    TrussAnalysisFacade(const TrussAnalysisFacade&) = delete;
    TrussAnalysisFacade& operator=(const TrussAnalysisFacade&) = delete;

    // Allow move
    TrussAnalysisFacade(TrussAnalysisFacade&&) noexcept = default;
    TrussAnalysisFacade& operator=(TrussAnalysisFacade&&) noexcept = default;

    ~TrussAnalysisFacade() = default;

    // ============================================================
    // Public Service Operations (for adapter delegation)
    // ============================================================
    // These public methods satisfy ITrussService and IAnalysisService contracts
    // Adapters delegate to these methods (NOT via friend access)

    // ITrussService operations
    application::Result<application::TrussHandle> createTruss(const std::string& name);
    application::Result<application::TrussHandle> loadTruss(const std::filesystem::path& filepath);
    application::Result<bool> saveTruss(application::TrussHandle handle,
                                        const std::filesystem::path& filepath,
                                        bool overwrite = false);
    bool clearTruss(application::TrussHandle handle);
    bool isValidTrussHandle(application::TrussHandle handle) const;

    const core::interfaces::ITrussView& getTrussView(application::TrussHandle handle) const;
    core::Truss& getTrussMutable(application::TrussHandle handle);

    application::Result<core::validation::ValidationResult>
    validateTruss(application::TrussHandle handle);

    application::Result<core::NodeId>
    addNode(application::TrussHandle handle,
            const core::Point2D& position,
            core::SupportType supportType = core::SupportType::Free);

    application::Result<core::MemberId> addMember(application::TrussHandle handle,
                                                  core::NodeId startNodeId,
                                                  core::NodeId endNodeId,
                                                  const application::MaterialSpec& material,
                                                  const application::SectionSpec& section);

    application::Result<bool> removeNode(application::TrussHandle handle,
                                         core::NodeId nodeId);
    application::Result<bool> removeMember(application::TrussHandle handle,
                                           core::MemberId memberId);

    application::Result<bool> setNodeSupport(application::TrussHandle handle,
                                             core::NodeId nodeId,
                                             core::SupportType supportType);

    application::Result<bool> applyNodeLoad(application::TrussHandle handle,
                                            core::NodeId nodeId,
                                            const core::Force2D& force);

    application::Result<bool> clearNodeLoad(application::TrussHandle handle,
                                            core::NodeId nodeId);

    // IAnalysisService operations
    application::Result<application::ResultsHandle>
    analyze(const core::Truss& truss, const core::analysis::AnalysisOptions& options = {});

    const core::interfaces::IAnalysisResultsView&
    getResultsView(application::ResultsHandle handle) const;

    application::Result<bool> exportResults(application::ResultsHandle handle,
                                           infrastructure::export_::ExportFormat format,
                                           const std::filesystem::path& filepath,
                                           const core::Truss& truss,
                                           const infrastructure::export_::ExportOptions& options =
                                               {});

    application::Result<bool> exportResults(application::ResultsHandle handle,
                                           const std::filesystem::path& filepath,
                                           const core::Truss& truss,
                                           const infrastructure::export_::ExportOptions& options =
                                               {});

    bool clearResults(application::ResultsHandle handle);
    bool isValidResultsHandle(application::ResultsHandle handle) const;

    /**
     * @brief Complete workflow: load → validate → analyze
     * @param filepath Path to truss model file
     * @param options Analysis configuration options
     * @return AnalysisWorkflowResult with handles on success, error on failure
     *
     * This method performs:
     * 1. Load truss from file
     * 2. Validate structure
     * 3. Perform structural analysis
     * 4. Return handles for result access/export
     *
     * On failure, cleans up any partially created resources.
     */
    AnalysisWorkflowResult analyzeFromFile(const std::filesystem::path& filepath,
                                           const core::analysis::AnalysisOptions& options = {});

    /**
     * @brief Complete workflow: build → validate → analyze
     * @param builder TrussBuilder with configured truss
     * @param options Analysis configuration options
     * @return AnalysisWorkflowResult with handles on success, error on failure
     *
     * This method performs:
     * 1. Build truss from builder
     * 2. Validate structure
     * 3. Perform structural analysis
     * 4. Return handles for result access/export
     *
     * The builder is not modified by this call.
     */
    AnalysisWorkflowResult analyzeInteractive(TrussBuilder& builder,
                                              const core::analysis::AnalysisOptions& options = {});

    /**
     * @brief Complete workflow: load → validate (no analysis)
     * @param filepath Path to truss model file
     * @return ValidationResult with detailed validation report
     *
     * Loads a truss file and performs comprehensive validation
     * without running structural analysis. Useful for checking
     * model validity before committing to expensive analysis.
     */
    core::validation::ValidationResult validateFromFile(const std::filesystem::path& filepath);

    /**
     * @brief Validate a builder configuration without building
     * @param builder TrussBuilder to validate
     * @return ValidationResult with detailed validation report
     *
     * Builds the truss temporarily and validates it without
     * storing the result. Useful for interactive validation
     * during truss construction.
     */
    core::validation::ValidationResult validateBuilder(TrussBuilder& builder);

    /**
     * @brief Export analysis results with format auto-detection
     * @param resultsHandle Handle to analysis results
     * @param filepath Export file path (extension determines format)
     * @param options Export options (default: all data)
     * @return true on success, false on failure
     *
     * Requires both truss and results handles from previous analysis.
     * Format is auto-detected from file extension.
     */
    bool exportResults(application::ResultsHandle resultsHandle,
                       const std::filesystem::path& filepath,
                       const infrastructure::export_::ExportOptions& options = {});

    /**
     * @brief Export analysis results with explicit format
     * @param resultsHandle Handle to analysis results
     * @param format Explicit export format
     * @param filepath Export file path
     * @param options Export options (default: all data)
     * @return true on success, false on failure
     */
    bool exportResults(application::ResultsHandle resultsHandle,
                       infrastructure::export_::ExportFormat format,
                       const std::filesystem::path& filepath,
                       const infrastructure::export_::ExportOptions& options = {});

    // ============================================================
    // Resource Management (for adapter delegation)
    // ============================================================

    void clearAll();
    void clearWorkflow(application::TrussHandle trussHandle,
                       application::ResultsHandle resultsHandle);

    // ============================================================
    // Helper Methods (lower-level operations)
    // ============================================================

    /**
     * @brief Create a new empty truss
     * @param name Truss name
     * @return TrussHandle on success, 0 on failure
     *
     * Lower-level method for manual truss construction.
     * Prefer using TrussBuilder for fluent construction.
     */
    application::TrussHandle createEmptyTruss(const std::string& name);

    /**
     * @brief Load truss from file without analysis
     * @param filepath Path to truss file
     * @return TrussHandle on success, 0 on failure
     *
     * Lower-level method for loading without immediate analysis.
     */
    application::TrussHandle loadTrussOnly(const std::filesystem::path& filepath);

    /**
     * @brief Analyze an existing truss
     * @param trussHandle Handle to existing truss
     * @param options Analysis options
     * @return ResultsHandle on success, 0 on failure
     *
     * Lower-level method for analyzing already-loaded truss.
     */
    application::ResultsHandle analyzeOnly(application::TrussHandle trussHandle,
                                           const core::analysis::AnalysisOptions& options = {});

private:
    // Application services (owned by facade)
    application::TrussApplicationService m_trussService;
    application::AnalysisApplicationService m_analysisService;

    // Track last truss handle for export convenience
    application::TrussHandle m_lastTrussHandle{0};

    // Helper methods
    std::string formatValidationErrors(const core::validation::ValidationResult& result) const;
};

}  // namespace truss::interface
