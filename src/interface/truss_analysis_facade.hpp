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
#include "interface/itruss_analysis_facade.hpp"
#include "truss/analysis/analysis_options.hpp"
#include "truss_builder.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace truss::interface {

class FacadeTrussServiceAdapter;
class FacadeAnalysisServiceAdapter;

// AnalysisWorkflowResult is defined in itruss_analysis_facade.hpp and
// is available here via the #include of that header above.

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
class TrussAnalysisFacade final : public ITrussAnalysisFacade {
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
    application::Result<application::TrussHandle> createTruss(const std::string& name) override;
    application::Result<application::TrussHandle>
    loadTruss(const std::filesystem::path& filepath) override;
    application::Result<bool> saveTruss(application::TrussHandle handle,
                                        const std::filesystem::path& filepath,
                                        bool overwrite = false) override;
    bool clearTruss(application::TrussHandle handle) override;
    bool isValidTrussHandle(application::TrussHandle handle) const override;

    const core::interfaces::ITrussView&
    getTrussView(application::TrussHandle handle) const override;
    core::Truss& getTrussMutable(application::TrussHandle handle) override;

    application::Result<core::validation::ValidationResult>
    validateTruss(application::TrussHandle handle) override;

    application::Result<core::NodeId>
    addNode(application::TrussHandle handle,
            const core::Point2D& position,
            core::SupportType supportType = core::SupportType::Free) override;

    application::Result<core::MemberId> addMember(application::TrussHandle handle,
                                                  core::NodeId startNodeId,
                                                  core::NodeId endNodeId,
                                                  const application::MaterialSpec& material,
                                                  const application::SectionSpec& section) override;

    application::Result<bool> removeNode(application::TrussHandle handle,
                                         core::NodeId nodeId) override;
    application::Result<bool> removeMember(application::TrussHandle handle,
                                           core::MemberId memberId) override;

    application::Result<bool> setNodeSupport(application::TrussHandle handle,
                                             core::NodeId nodeId,
                                             core::SupportType supportType) override;

    application::Result<bool> applyNodeLoad(application::TrussHandle handle,
                                            core::NodeId nodeId,
                                            const core::Force2D& force) override;

    application::Result<bool> clearNodeLoad(application::TrussHandle handle,
                                            core::NodeId nodeId) override;

    application::Result<bool> updateNode(application::TrussHandle handle,
                                         core::NodeId nodeId,
                                         const application::NodeUpdateSpec& update) override;

    application::Result<bool> updateMember(application::TrussHandle handle,
                                           core::MemberId memberId,
                                           const application::MemberUpdateSpec& update) override;
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
    bool isValidResultsHandle(application::ResultsHandle handle) const override;

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
    AnalysisWorkflowResult
    analyzeFromFile(const std::filesystem::path& filepath,
                    const core::analysis::AnalysisOptions& options = {}) override;

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
    AnalysisWorkflowResult
    analyzeInteractive(TrussBuilder& builder,
                       const core::analysis::AnalysisOptions& options = {}) override;

    /**
     * @brief Complete workflow: load → validate (no analysis)
     * @param filepath Path to truss model file
     * @return ValidationResult with detailed validation report
     *
     * Loads a truss file and performs comprehensive validation
     * without running structural analysis. Useful for checking
     * model validity before committing to expensive analysis.
     */
    core::validation::ValidationResult
    validateFromFile(const std::filesystem::path& filepath) override;

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
                       const infrastructure::export_::ExportOptions& options = {}) override;

    /**
     * @brief Export analysis results with explicit format
     * @param resultsHandle Handle to analysis results
     * @param format Explicit export format
     * @param filepath Export file path
     * @param options Export options (default: all data)
     * @return true on success, false on failure
     */
    bool exportResults(application::ResultsHandle resultsHandle,
                       truss::ExportFormat format,
                       const std::filesystem::path& filepath,
                       const infrastructure::export_::ExportOptions& options = {}) override;

    // ============================================================
    // Resource Management (for adapter delegation)
    // ============================================================

    void clearAll() override;
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
