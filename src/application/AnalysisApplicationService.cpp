/**
 * @file AnalysisApplicationService.cpp
 * @brief Implementation of analysis application service
 */

#include "AnalysisApplicationService.hpp"

#include "../core/validation/TrussValidator.hpp"

#include <stdexcept>

namespace truss::application {

AnalysisApplicationService::AnalysisApplicationService() = default;

Result<ResultsHandle>
AnalysisApplicationService::analyze(const core::Truss& truss,
                                    const core::analysis::AnalysisOptions& options) {
    try {
        // Phase 1: Validate truss structure
        core::validation::TrussValidator tempValidator;
        auto validationResult = tempValidator.validate(truss);
        if (!validationResult.isValid()) {
            std::string errorMsg = "Truss validation failed:\n";
            for (const auto& message : validationResult.getErrorMessages()) {
                errorMsg += "  - " + message + "\n";
            }
            return Result<ResultsHandle>::Failure(errorMsg);
        }

        // Phase 2: Create orchestrator with solver and validator
        // Note: AnalysisOrchestrator takes ownership of solver and validator
        // Select solver based on options.useDirectSolver
        std::unique_ptr<core::analysis::ILinearSolver> solver;
        if (options.useDirectSolver) {
            solver = std::make_unique<core::analysis::DirectSolver>();
        } else {
            solver = std::make_unique<core::analysis::IterativeSolver>();
        }
        auto validator = std::make_unique<core::validation::TrussValidator>();
        core::analysis::AnalysisOrchestrator orchestrator(
            std::move(solver), std::move(validator), options);

        // Phase 3: Run analysis on a local mutable copy
        core::Truss trussCopy = truss;
        auto results = orchestrator.analyze(trussCopy);

        // Phase 4: Store results and generate handle
        auto handle = generateHandle();
        m_results[handle] = std::make_shared<core::analysis::AnalysisResults>(std::move(results));

        return Result<ResultsHandle>::Success(handle);

    } catch (const std::exception& e) {
        return Result<ResultsHandle>::Failure(std::string("Analysis failed: ") + e.what());
    }
}

const core::interfaces::IAnalysisResultsView&
AnalysisApplicationService::getResultsView(ResultsHandle handle) const {
    auto it = m_results.find(handle);
    if (it == m_results.end()) {
        throw std::invalid_argument("Invalid results handle: " + std::to_string(handle));
    }

    // AnalysisResults implements IAnalysisResultsView interface
    return *it->second;
}

core::analysis::AnalysisResults& AnalysisApplicationService::getResults(ResultsHandle handle) {
    auto it = m_results.find(handle);
    if (it == m_results.end()) {
        throw std::invalid_argument("Invalid results handle: " + std::to_string(handle));
    }

    return *it->second;
}

Result<bool>
AnalysisApplicationService::exportResults(ResultsHandle handle,
                                          infrastructure::export_::ExportFormat format,
                                          const std::filesystem::path& filepath,
                                          const core::Truss& truss,
                                          const infrastructure::export_::ExportOptions& options) {
    try {
        // Validate handle
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid results handle: " + std::to_string(handle));
        }

        // Get results
        const auto& results = getResults(handle);

        // Create exporter for specified format
        auto exporter = infrastructure::export_::ExporterFactory::create(format);

        // Export results - check return value for file I/O errors
        bool exportSuccess = exporter->exportResults(truss, results, filepath, options);
        if (!exportSuccess) {
            std::string errorMsg = exporter->getLastError();
            if (errorMsg.empty()) {
                errorMsg = "Export failed for unknown reason";
            }
            return Result<bool>::Failure(errorMsg);
        }

        return Result<bool>::Success(true);

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Export failed: ") + e.what());
    }
}

Result<bool>
AnalysisApplicationService::exportResults(ResultsHandle handle,
                                          const std::filesystem::path& filepath,
                                          const core::Truss& truss,
                                          const infrastructure::export_::ExportOptions& options) {
    try {
        // Auto-detect format from file extension
        auto format = infrastructure::export_::ExporterFactory::detectFormat(filepath);

        return exportResults(handle, format, filepath, truss, options);

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Export failed: ") + e.what());
    }
}

bool AnalysisApplicationService::clearResults(ResultsHandle handle) {
    return m_results.erase(handle) > 0;
}

void AnalysisApplicationService::clearAll() {
    m_results.clear();
    m_nextHandle = 1;
}

bool AnalysisApplicationService::isValidHandle(ResultsHandle handle) const {
    return m_results.find(handle) != m_results.end();
}

}  // namespace truss::application
