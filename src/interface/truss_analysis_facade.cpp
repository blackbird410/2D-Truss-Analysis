/**
 * @file truss_analysis_facade.cpp
 * @brief Implementation of TrussAnalysisFacade unified workflow API
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 */

#include "truss_analysis_facade.hpp"

#include "truss/analysis/analysis_options.hpp"

#include <sstream>

namespace truss::interface {

// ============================================================
// Constructor
// ============================================================

TrussAnalysisFacade::TrussAnalysisFacade() = default;

// ============================================================
// ITrussService Operations (delegate to m_trussService)
// ============================================================

application::Result<application::TrussHandle>
TrussAnalysisFacade::createTruss(const std::string& name) {
    auto result = m_trussService.createTruss(name);
    if (result) {
        m_lastTrussHandle = result.value;
    }
    return result;
}

application::Result<application::TrussHandle>
TrussAnalysisFacade::loadTruss(const std::filesystem::path& filepath) {
    auto result = m_trussService.loadTruss(filepath);
    if (result) {
        m_lastTrussHandle = result.value;
    }
    return result;
}

application::Result<bool> TrussAnalysisFacade::saveTruss(application::TrussHandle handle,
                                                         const std::filesystem::path& filepath,
                                                         bool overwrite) {
    return m_trussService.saveTruss(handle, filepath, overwrite);
}

bool TrussAnalysisFacade::clearTruss(application::TrussHandle handle) {
    bool result = m_trussService.clearTruss(handle);
    if (result && m_lastTrussHandle == handle) {
        m_lastTrussHandle = 0;
    }
    return result;
}

const core::interfaces::ITrussView&
TrussAnalysisFacade::getTrussView(application::TrussHandle handle) const {
    return m_trussService.getTrussView(handle);
}

core::Truss& TrussAnalysisFacade::getTrussMutable(application::TrussHandle handle) {
    return m_trussService.getTrussMutable(handle);
}

application::Result<core::validation::ValidationResult>
TrussAnalysisFacade::validateTruss(application::TrussHandle handle) {
    return m_trussService.validateTruss(handle);
}

application::Result<core::NodeId> TrussAnalysisFacade::addNode(application::TrussHandle handle,
                                                               const core::Point2D& position,
                                                               core::SupportType supportType) {
    return m_trussService.addNode(handle, position, supportType);
}

application::Result<core::MemberId>
TrussAnalysisFacade::addMember(application::TrussHandle handle,
                               core::NodeId startNodeId,
                               core::NodeId endNodeId,
                               const application::MaterialSpec& material,
                               const application::SectionSpec& section) {
    return m_trussService.addMember(handle, startNodeId, endNodeId, material, section);
}

application::Result<bool> TrussAnalysisFacade::removeNode(application::TrussHandle handle,
                                                          core::NodeId nodeId) {
    return m_trussService.removeNode(handle, nodeId);
}

application::Result<bool> TrussAnalysisFacade::removeMember(application::TrussHandle handle,
                                                            core::MemberId memberId) {
    return m_trussService.removeMember(handle, memberId);
}

application::Result<bool> TrussAnalysisFacade::setNodeSupport(application::TrussHandle handle,
                                                              core::NodeId nodeId,
                                                              core::SupportType supportType) {
    return m_trussService.setNodeSupport(handle, nodeId, supportType);
}

application::Result<bool> TrussAnalysisFacade::applyNodeLoad(application::TrussHandle handle,
                                                             core::NodeId nodeId,
                                                             const core::Force2D& force) {
    return m_trussService.applyNodeLoad(handle, nodeId, force);
}

application::Result<bool> TrussAnalysisFacade::clearNodeLoad(application::TrussHandle handle,
                                                             core::NodeId nodeId) {
    return m_trussService.clearNodeLoad(handle, nodeId);
}

application::Result<bool>
TrussAnalysisFacade::updateNode(application::TrussHandle handle,
                                core::NodeId nodeId,
                                const application::NodeUpdateSpec& update) {
    return m_trussService.updateNode(handle, nodeId, update);
}

application::Result<bool>
TrussAnalysisFacade::updateMember(application::TrussHandle handle,
                                  core::MemberId memberId,
                                  const application::MemberUpdateSpec& update) {
    return m_trussService.updateMember(handle, memberId, update);
}

// ============================================================
// IAnalysisService Operations (delegate to m_analysisService)
// ============================================================

application::Result<application::ResultsHandle>
TrussAnalysisFacade::analyze(const core::Truss& truss,
                             const core::analysis::AnalysisOptions& options) {
    return m_analysisService.analyze(truss, options);
}

const core::interfaces::IAnalysisResultsView&
TrussAnalysisFacade::getResultsView(application::ResultsHandle handle) const {
    return m_analysisService.getResultsView(handle);
}

application::Result<bool>
TrussAnalysisFacade::exportResults(application::ResultsHandle handle,
                                   truss::ExportFormat format,
                                   const std::filesystem::path& filepath,
                                   const core::Truss& truss,
                                   const infrastructure::export_::ExportOptions& options) {
    return m_analysisService.exportResults(handle, format, filepath, truss, options);
}

application::Result<bool>
TrussAnalysisFacade::exportResults(application::ResultsHandle handle,
                                   const std::filesystem::path& filepath,
                                   const core::Truss& truss,
                                   const infrastructure::export_::ExportOptions& options) {
    return m_analysisService.exportResults(handle, filepath, truss, options);
}

bool TrussAnalysisFacade::clearResults(application::ResultsHandle handle) {
    return m_analysisService.clearResults(handle);
}

// ============================================================
// High-Level Workflow Methods
// ============================================================

AnalysisWorkflowResult
TrussAnalysisFacade::analyzeFromFile(const std::filesystem::path& filepath,
                                     const core::analysis::AnalysisOptions& options) {
    // Step 1: Load truss from file
    auto loadResult = loadTruss(filepath);
    if (!loadResult) {
        return AnalysisWorkflowResult::Failure("Failed to load truss: " + loadResult.errorMessage);
    }
    auto trussHandle = loadResult.value;

    // Step 2: Validate truss structure
    auto validationResult = validateTruss(trussHandle);
    if (!validationResult || !validationResult.value.isValid()) {
        std::string error = validationResult ? formatValidationErrors(validationResult.value)
                                             : validationResult.errorMessage;
        clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Validation failed: " + error);
    }

    // Step 3: Perform analysis
    auto& truss = getTrussMutable(trussHandle);
    auto analysisResult = analyze(truss, options);
    if (!analysisResult) {
        clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Analysis failed: " + analysisResult.errorMessage);
    }

    return AnalysisWorkflowResult::Success(trussHandle, analysisResult.value);
}

AnalysisWorkflowResult
TrussAnalysisFacade::analyzeInteractive(TrussBuilder& builder,
                                        const core::analysis::AnalysisOptions& options) {
    // Step 1: Build truss from builder
    std::shared_ptr<core::Truss> trussPtr;
    try {
        trussPtr = builder.build();
    } catch (const std::exception& e) {
        return AnalysisWorkflowResult::Failure(std::string("Failed to build truss: ") + e.what());
    }

    // Step 2: Create truss handle
    auto createResult = createTruss("Interactive Truss");
    if (!createResult) {
        return AnalysisWorkflowResult::Failure("Failed to create truss: " +
                                               createResult.errorMessage);
    }
    auto trussHandle = createResult.value;

    // Copy built truss into managed truss
    auto& managedTruss = getTrussMutable(trussHandle);
    managedTruss = *trussPtr;

    // Step 3: Validate
    auto validationResult = validateTruss(trussHandle);
    if (!validationResult || !validationResult.value.isValid()) {
        std::string error = validationResult ? formatValidationErrors(validationResult.value)
                                             : validationResult.errorMessage;
        clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Validation failed: " + error);
    }

    // Step 4: Analyze
    auto analysisResult = analyze(managedTruss, options);
    if (!analysisResult) {
        clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Analysis failed: " + analysisResult.errorMessage);
    }

    return AnalysisWorkflowResult::Success(trussHandle, analysisResult.value);
}

core::validation::ValidationResult
TrussAnalysisFacade::validateFromFile(const std::filesystem::path& filepath) {
    // Load truss
    auto loadResult = loadTruss(filepath);
    if (!loadResult) {
        core::validation::ValidationResult result;
        core::validation::ValidationIssue issue(core::validation::ValidationSeverity::Fatal,
                                                "File Loading",
                                                "Failed to load truss file: " +
                                                    loadResult.errorMessage,
                                                "Check file path and format");
        result.addIssue(issue);
        return result;
    }

    auto trussHandle = loadResult.value;

    // Validate
    auto validationResult = validateTruss(trussHandle);

    // Cleanup and return
    clearTruss(trussHandle);

    if (validationResult) {
        return validationResult.value;
    }
    core::validation::ValidationResult result;
    core::validation::ValidationIssue issue(core::validation::ValidationSeverity::Fatal,
                                            "Validation",
                                            "Validation process failed: " +
                                                validationResult.errorMessage,
                                            "Internal error during validation");
    result.addIssue(issue);
    return result;
}

core::validation::ValidationResult TrussAnalysisFacade::validateBuilder(TrussBuilder& builder) {
    std::shared_ptr<core::Truss> trussPtr;
    try {
        trussPtr = builder.build();
    } catch (const std::exception& e) {
        core::validation::ValidationResult result;
        core::validation::ValidationIssue issue(core::validation::ValidationSeverity::Fatal,
                                                "Build Error",
                                                std::string("Failed to build truss: ") + e.what(),
                                                "Builder validation failed during construction");
        result.addIssue(issue);
        return result;
    }
    return core::validation::TrussValidator::validate(*trussPtr);
}

// ============================================================
// Simplified Export Methods (with auto-format detection)
// ============================================================

bool TrussAnalysisFacade::exportResults(application::ResultsHandle resultsHandle,
                                        const std::filesystem::path& filepath,
                                        const infrastructure::export_::ExportOptions& options) {
    auto format = infrastructure::export_::ExporterFactory::detectFormat(filepath);
    return exportResults(resultsHandle, format, filepath, options);
}

bool TrussAnalysisFacade::exportResults(application::ResultsHandle resultsHandle,
                                        truss::ExportFormat format,
                                        const std::filesystem::path& filepath,
                                        const infrastructure::export_::ExportOptions& options) {
    // Validate handles
    if (!m_analysisService.isValidResultsHandle(resultsHandle)) {
        return false;
    }
    if (m_lastTrussHandle == 0 || !m_trussService.isValidTrussHandle(m_lastTrussHandle)) {
        return false;
    }
    const auto& truss = m_trussService.getTrussMutable(m_lastTrussHandle);
    auto result = m_analysisService.exportResults(resultsHandle, format, filepath, truss, options);
    return result.success;
}

// ============================================================
// Resource Management
// ============================================================

void TrussAnalysisFacade::clearAll() {
    m_analysisService.clearAll();
    m_trussService.clearAll();
    m_lastTrussHandle = 0;
}

void TrussAnalysisFacade::clearWorkflow(application::TrussHandle trussHandle,
                                        application::ResultsHandle resultsHandle) {
    if (trussHandle != 0) {
        m_trussService.clearTruss(trussHandle);
        if (m_lastTrussHandle == trussHandle) {
            m_lastTrussHandle = 0;
        }
    }
    if (resultsHandle != 0) {
        m_analysisService.clearResults(resultsHandle);
    }
}

// ============================================================
// Handle Validation
// ============================================================

bool TrussAnalysisFacade::isValidTrussHandle(application::TrussHandle handle) const {
    return m_trussService.isValidTrussHandle(handle);
}

bool TrussAnalysisFacade::isValidResultsHandle(application::ResultsHandle handle) const {
    return m_analysisService.isValidResultsHandle(handle);
}

// ============================================================
// Helper Methods (Lower-level operations)
// ============================================================

application::TrussHandle TrussAnalysisFacade::createEmptyTruss(const std::string& name) {
    auto result = m_trussService.createTruss(name);
    if (result) {
        m_lastTrussHandle = result.value;
        return result.value;
    }
    return 0;
}

application::TrussHandle TrussAnalysisFacade::loadTrussOnly(const std::filesystem::path& filepath) {
    auto result = m_trussService.loadTruss(filepath);
    if (result) {
        m_lastTrussHandle = result.value;
        return result.value;
    }
    return 0;
}

application::ResultsHandle
TrussAnalysisFacade::analyzeOnly(application::TrussHandle trussHandle,
                                 const core::analysis::AnalysisOptions& options) {
    if (!m_trussService.isValidTrussHandle(trussHandle)) {
        return 0;
    }
    auto& truss = m_trussService.getTrussMutable(trussHandle);
    auto result = m_analysisService.analyze(truss, options);
    if (result) {
        return result.value;
    }
    return 0;
}

// ============================================================
// Private Helper Methods
// ============================================================

std::string TrussAnalysisFacade::formatValidationErrors(
    const core::validation::ValidationResult& result) const {
    std::ostringstream oss;
    oss << result.getSummary() << "\n";
    auto errors = result.getErrorMessages();
    if (!errors.empty()) {
        oss << "\nErrors:\n";
        for (const auto& error : errors) {
            oss << "  • " << error << "\n";
        }
    }
    auto warnings = result.getWarningMessages();
    if (!warnings.empty()) {
        oss << "\nWarnings:\n";
        for (size_t i = 0; i < std::min(warnings.size(), static_cast<size_t>(5)); ++i) {
            oss << "  • " << warnings.at(i) << "\n";
        }
        if (warnings.size() > 5) {
            oss << "  ... and " << (warnings.size() - 5) << " more warnings\n";
        }
    }
    return oss.str();
}

}  // namespace truss::interface
