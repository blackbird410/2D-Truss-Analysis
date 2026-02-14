/**
 * @file exporter.hpp
 * @brief Results exporter interface (Strategy pattern)
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-07
 */

#pragma once

#include "export_types.hpp"
#include "../../core/interfaces/ITrussView.hpp"
#include "../../core/interfaces/IAnalysisResultsView.hpp"
#include <string>
#include <filesystem>

namespace truss::infrastructure::export_ {

// Import view interfaces from core
using core::interfaces::ITrussView;
using core::interfaces::IAnalysisResultsView;

/**
 * @brief Abstract interface for results exporters
 * 
 * Defines the contract for all concrete exporter implementations.
 * Each exporter is responsible for a single format (CSV, JSON, XML, etc.).
 * 
 * Design Pattern: Strategy
 * - Context: ExporterFactory
 * - Strategy: IResultsExporter (this interface)
 * - Concrete Strategies: CSVExporter, JSONExporter, etc.
 * 
 * Architecture: Depends on abstractions (ITrussView, IAnalysisResultsView),
 * not concrete Domain types (Truss, AnalysisResults). This enforces DIP.
 */
class IResultsExporter {
public:
    virtual ~IResultsExporter() = default;
    
    /**
     * @brief Export analysis results to file
     * 
     * @param truss Read-only view of the analyzed truss structure
     * @param results Read-only view of analysis results
     * @param filePath Output file path
     * @param options Export options (what to include, formatting)
     * @return true if export successful, false otherwise
     * 
     * @throws std::runtime_error if file cannot be written
     * @throws std::invalid_argument if truss or results are invalid
     */
    virtual bool exportResults(
        const ITrussView& truss,
        const IAnalysisResultsView& results,
        const std::filesystem::path& filePath,
        const ExportOptions& options = ExportOptions{}
    ) = 0;
    
    /**
     * @brief Get the last error message
     * 
     * @return Error message string (empty if no error)
     */
    virtual std::string getLastError() const = 0;
    
    /**
     * @brief Get the format this exporter handles
     * 
     * @return Export format enum value
     */
    virtual ExportFormat getFormat() const = 0;
};

} // namespace truss::infrastructure::export_
