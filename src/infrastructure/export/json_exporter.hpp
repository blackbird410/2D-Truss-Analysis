/**
 * @file json_exporter.hpp
 * @brief JSON format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "exporter.hpp"
#include "export_types.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::ITrussView;
using core::interfaces::IAnalysisResultsView;

/**
 * @brief JSON format exporter
 * 
 * Exports analysis results to JavaScript Object Notation (JSON) format.
 * Provides structured, machine-readable output suitable for web applications
 * and data processing pipelines.
 */
class JSONExporter : public IResultsExporter {
public:
    /**
     * @brief Construct a JSON exporter
     */
    JSONExporter() = default;
    
    /**
     * @brief Virtual destructor
     */
    ~JSONExporter() override = default;
    
    /**
     * @brief Export analysis results to JSON file
     * @param truss Read-only view of the analyzed truss structure
     * @param results Read-only view of analysis results
     * @param filePath Output file path
     * @param options Export options (precision, sections to include)
     * @return true if export successful, false otherwise
     */
    bool exportResults(const ITrussView& truss,
                      const IAnalysisResultsView& results,
                      const std::filesystem::path& filePath,
                      const ExportOptions& options = ExportOptions{}) override;
    
    /**
     * @brief Get the last error message
     * @return Error message string
     */
    std::string getLastError() const override {
        return m_lastError;
    }
    
    /**
     * @brief Get the export format
     * @return ExportFormat::JSON
     */
    ExportFormat getFormat() const noexcept override {
        return ExportFormat::JSON;
    }

private:
    std::string m_lastError; ///< Last error message
    
    // Helper methods
    std::string formatNumber(Real value, const ExportOptions& options) const;
    std::string formatTimestamp() const;
    std::string escapeString(const std::string& str) const;
    
    // Section writers
    void writeProjectSection(std::ostream& os, const ITrussView& truss,
                            const ExportOptions& options);
    void writeGeometrySection(std::ostream& os, const ITrussView& truss,
                             const ExportOptions& options, bool& needsComma);
    void writePropertiesSection(std::ostream& os, const ITrussView& truss,
                               const ExportOptions& options, bool& needsComma);
    void writeLoadsSection(std::ostream& os, const ITrussView& truss,
                          const ExportOptions& options, bool& needsComma);
    void writeDisplacementsSection(std::ostream& os, const IAnalysisResultsView& results,
                                  const ExportOptions& options, bool& needsComma);
    void writeMemberForcesSection(std::ostream& os, const IAnalysisResultsView& results,
                                 const ExportOptions& options, bool& needsComma);
    void writeReactionsSection(std::ostream& os, const IAnalysisResultsView& results,
                              const ExportOptions& options, bool& needsComma);
    void writeMetadataSection(std::ostream& os, const IAnalysisResultsView& results,
                             const ExportOptions& options, bool& needsComma);
};

} // namespace truss::infrastructure::export_
