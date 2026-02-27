/**
 * @file truss_analysis_facade.cpp
 * @brief Implementation of TrussAnalysisFacade unified workflow API
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 */

#include "truss_analysis_facade.hpp"

#include <sstream>

namespace truss::interface {

TrussAnalysisFacade::TrussAnalysisFacade() = default;

// ============================================================
// Complete Workflow Methods
// ============================================================

AnalysisWorkflowResult TrussAnalysisFacade::analyzeFromFile(
    const std::filesystem::path& filepath,
    const core::analysis::AnalysisOptions& options) {
    
    // Step 1: Load truss from file
    auto loadResult = m_trussService.loadTruss(filepath);
    if (!loadResult) {
        return AnalysisWorkflowResult::Failure("Load failed: " + loadResult.errorMessage);
    }
    application::TrussHandle trussHandle = loadResult.value;
    m_lastTrussHandle = trussHandle;

    // Step 2: Validate truss structure
    auto validationResult = m_trussService.validateTruss(trussHandle);
    if (!validationResult) {
        m_trussService.clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Validation failed: " +
                                               validationResult.errorMessage);
    }

    if (!validationResult.value.isValid()) {
        std::string errorMsg = "Truss validation failed:\n" +
                               formatValidationErrors(validationResult.value);
        m_trussService.clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure(errorMsg);
    }

    // Step 3: Perform structural analysis
    auto& truss = m_trussService.getTrussMutable(trussHandle);
    auto analysisResult = m_analysisService.analyze(truss, options);
    if (!analysisResult) {
        m_trussService.clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Analysis failed: " +
                                               analysisResult.errorMessage);
    }

    // Success: return both handles
    return AnalysisWorkflowResult::Success(trussHandle, analysisResult.value);
}

AnalysisWorkflowResult TrussAnalysisFacade::analyzeInteractive(
    TrussBuilder& builder,
    const core::analysis::AnalysisOptions& options) {
    
    // Step 1: Build truss from builder
    std::shared_ptr<core::Truss> trussPtr;
    try {
        trussPtr = builder.build();
    } catch (const std::exception& e) {
        return AnalysisWorkflowResult::Failure(std::string("Build failed: ") + e.what());
    }

    // Step 2: Create truss handle via application service
    auto createResult = m_trussService.createTruss(trussPtr->getName());
    if (!createResult) {
        return AnalysisWorkflowResult::Failure("Create truss failed: " +
                                               createResult.errorMessage);
    }
    application::TrussHandle trussHandle = createResult.value;
    m_lastTrussHandle = trussHandle;

    // Copy built truss data into managed truss
    auto& managedTruss = m_trussService.getTrussMutable(trussHandle);
    managedTruss = *trussPtr;  // Copy assignment

    // Step 3: Validate truss structure
    auto validationResult = m_trussService.validateTruss(trussHandle);
    if (!validationResult || !validationResult.value.isValid()) {
        std::string errorMsg = "Truss validation failed:\n";
        if (validationResult) {
            errorMsg += formatValidationErrors(validationResult.value);
        } else {
            errorMsg += validationResult.errorMessage;
        }
        m_trussService.clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure(errorMsg);
    }

    // Step 4: Perform structural analysis
    auto analysisResult = m_analysisService.analyze(managedTruss, options);
    if (!analysisResult) {
        m_trussService.clearTruss(trussHandle);
        return AnalysisWorkflowResult::Failure("Analysis failed: " +
                                               analysisResult.errorMessage);
    }

    // Success: return both handles
    return AnalysisWorkflowResult::Success(trussHandle, analysisResult.value);
}

core::validation::ValidationResult
TrussAnalysisFacade::validateFromFile(const std::filesystem::path& filepath) {
    // Load truss
    auto loadResult = m_trussService.loadTruss(filepath);
    if (!loadResult) {
        // Return validation result with error
        core::validation::ValidationResult result;
        core::validation::ValidationIssue issue(
            core::validation::ValidationSeverity::Fatal,
            "File I/O Error",
            loadResult.errorMessage,
            "Failed to load truss from file"
        );
        result.addIssue(issue);
        return result;
    }

    // Validate and cleanup
    auto validationResult = m_trussService.validateTruss(loadResult.value);
    m_trussService.clearTruss(loadResult.value);

    if (!validationResult) {
        core::validation::ValidationResult result;
        core::validation::ValidationIssue issue(
            core::validation::ValidationSeverity::Fatal,
            "Validation Error",
            validationResult.errorMessage,
            "Validation service failed unexpectedly"
        );
        result.addIssue(issue);
        return result;
    }

    return validationResult.value;
}

core::validation::ValidationResult TrussAnalysisFacade::validateBuilder(TrussBuilder& builder) {
    // Build truss temporarily
    std::shared_ptr<core::Truss> trussPtr;
    try {
        trussPtr = builder.build();
    } catch (const std::exception& e) {
        core::validation::ValidationResult result;
        core::validation::ValidationIssue issue(
            core::validation::ValidationSeverity::Fatal,
            "Build Error",
            std::string("Failed to build truss: ") + e.what(),
            "Builder validation failed during construction"
        );
        result.addIssue(issue);
        return result;
    }

    // Validate using validator directly
    core::validation::TrussValidator validator;
    return validator.validate(*trussPtr);
}

bool TrussAnalysisFacade::exportResults(
    application::ResultsHandle resultsHandle,
    const std::filesystem::path& filepath,
    const infrastructure::export_::ExportOptions& options) {
    
    // Auto-detect format from extension
    auto format = infrastructure::export_::ExporterFactory::detectFormat(filepath);
    return exportResults(resultsHandle, format, filepath, options);
}

bool TrussAnalysisFacade::exportResults(
    application::ResultsHandle resultsHandle,
    infrastructure::export_::ExportFormat format,
    const std::filesystem::path& filepath,
    const infrastructure::export_::ExportOptions& options) {
    
    // Validate handles
    if (!m_analysisService.isValidHandle(resultsHandle)) {
        return false;
    }
    if (m_lastTrussHandle == 0 || !m_trussService.isValidHandle(m_lastTrussHandle)) {
        return false;
    }

    // Get truss for export context
    const auto& truss = m_trussService.getTrussMutable(m_lastTrussHandle);

    // Export via analysis service
    auto result = m_analysisService.exportResults(resultsHandle, format, filepath, truss, options);
    return result.success;
}

// ============================================================
// Resource Access Methods
// ============================================================

const core::interfaces::ITrussView&
TrussAnalysisFacade::getTrussView(application::TrussHandle trussHandle) const {
    return m_trussService.getTrussView(trussHandle);
}

const core::interfaces::IAnalysisResultsView&
TrussAnalysisFacade::getResultsView(application::ResultsHandle resultsHandle) const {
    return m_analysisService.getResultsView(resultsHandle);
}

core::Truss& TrussAnalysisFacade::getTrussMutable(application::TrussHandle trussHandle) {
    return m_trussService.getTrussMutable(trussHandle);
}

// ============================================================
// Resource Management
// ============================================================

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

void TrussAnalysisFacade::clearAll() {
    m_trussService.clearAll();
    m_analysisService.clearAll();
    m_lastTrussHandle = 0;
}

bool TrussAnalysisFacade::isValidTrussHandle(application::TrussHandle handle) const {
    return m_trussService.isValidHandle(handle);
}

bool TrussAnalysisFacade::isValidResultsHandle(application::ResultsHandle handle) const {
    return m_analysisService.isValidHandle(handle);
}

// ============================================================
// Workflow Helpers
// ============================================================

application::TrussHandle TrussAnalysisFacade::createEmptyTruss(const std::string& name) {
    auto result = m_trussService.createTruss(name);
    if (result) {
        m_lastTrussHandle = result.value;
        return result.value;
    }
    return 0;
}

application::TrussHandle
TrussAnalysisFacade::loadTrussOnly(const std::filesystem::path& filepath) {
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
    if (!m_trussService.isValidHandle(trussHandle)) {
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
    if (!warnings.empty() && warnings.size() <= 5) {  // Only show first 5 warnings
        oss << "\nWarnings:\n";
        for (size_t i = 0; i < std::min(warnings.size(), size_t(5)); ++i) {
            oss << "  • " << warnings[i] << "\n";
        }
        if (warnings.size() > 5) {
            oss << "  ... and " << (warnings.size() - 5) << " more warnings\n";
        }
    }

    return oss.str();
}

}  // namespace truss::interface
