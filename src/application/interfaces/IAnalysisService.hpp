/**
 * @file IAnalysisService.hpp
 * @brief Abstract interface for analysis application services
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This interface defines the contract for structural analysis operations,
 * enabling dependency injection and unit testing of GUI controllers.
 *
 * Design Pattern: Dependency Inversion Principle (depend on abstractions)
 * Architecture: Application Layer Interface
 */

#pragma once

#include "../../core/analysis/AnalysisOrchestrator.hpp"
#include "../../core/interfaces/IAnalysisResultsView.hpp"
#include "../../core/model/Truss.hpp"
#include "../../infrastructure/export/exporter_factory.hpp"
#include "../Result.hpp"

#include <cstddef>
#include <filesystem>

namespace truss::application {

/**
 * @brief Handle type for managing analysis results
 */
using ResultsHandle = size_t;

/**
 * @brief Abstract interface for analysis application services
 *
 * Enables polymorphic dependency injection for controllers,
 * allowing mock implementations for unit testing.
 *
 * All methods return Result<T> for consistent error handling.
 */
class IAnalysisService {
public:
    virtual ~IAnalysisService() = default;

    /**
     * @brief Perform structural analysis on a truss
     *
     * @param truss The truss to analyze
     * @param options Analysis configuration options
     * @return Result<ResultsHandle> Handle to results on success, error on failure
     */
    virtual Result<ResultsHandle>
    analyze(const core::Truss& truss,
            const core::analysis::AnalysisOptions& options = core::analysis::AnalysisOptions{}) = 0;

    /**
     * @brief Get read-only view of analysis results
     *
     * @param handle Handle to results
     * @return const IAnalysisResultsView& Read-only view of results
     * @throws std::out_of_range if handle is invalid
     */
    virtual const core::interfaces::IAnalysisResultsView&
    getResultsView(ResultsHandle handle) const = 0;

    /**
     * @brief Export analysis results to file with explicit format
     *
     * @param handle Handle to results
     * @param format Explicit export format
     * @param filepath Export file path
     * @param truss Truss model (for context/metadata)
     * @param options Export options
     * @return Result<bool> true on success, error on failure
     */
    virtual Result<bool> exportResults(ResultsHandle handle,
                                       infrastructure::export_::ExportFormat format,
                                       const std::filesystem::path& filepath,
                                       const core::Truss& truss,
                                       const infrastructure::export_::ExportOptions& options =
                                           infrastructure::export_::ExportOptions{}) = 0;

    /**
     * @brief Export analysis results to file (auto-detect format from extension)
     *
     * @param handle Handle to results
     * @param filepath Export file path (extension determines format)
     * @param truss Truss model (for context/metadata)
     * @param options Export options
     * @return Result<bool> true on success, error on failure
     */
    virtual Result<bool> exportResults(ResultsHandle handle,
                                       const std::filesystem::path& filepath,
                                       const core::Truss& truss,
                                       const infrastructure::export_::ExportOptions& options =
                                           infrastructure::export_::ExportOptions{}) = 0;

    /**
     * @brief Clear specific analysis results
     *
     * @param handle Handle to results
     * @return true if results existed and were cleared
     */
    virtual bool clearResults(ResultsHandle handle) = 0;

    /**
     * @brief Clear all stored analysis results
     */
    virtual void clearAll() = 0;

    /**
     * @brief Check if results handle is valid
     *
     * @param handle Handle to check
     * @return true if handle is valid
     */
    virtual bool isValidHandle(ResultsHandle handle) const = 0;
};

}  // namespace truss::application
