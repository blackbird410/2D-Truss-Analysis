/**
 * @file csv_exporter.hpp
 * @brief CSV format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "export_types.hpp"
#include "exporter.hpp"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::IAnalysisResultsView;
using core::interfaces::ITrussView;

/**
 * @brief CSV/TSV format exporter
 *
 * Exports analysis results to comma-separated values (CSV) or tab-separated
 * values (TSV) format. The delimiter is controlled by ExportOptions.
 */
class CSVExporter : public IResultsExporter {
public:
    /**
     * @brief Construct a CSV exporter
     */
    CSVExporter() = default;

    /**
     * @brief Virtual destructor
     */
    ~CSVExporter() override = default;

    /**
     * @brief Export analysis results to CSV file
     * @param truss Read-only view of the analyzed truss structure
     * @param results Read-only view of analysis results
     * @param filePath Output file path
     * @param options Export options (delimiter, precision, sections to include)
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
    std::string getLastError() const override { return m_lastError; }

    /**
     * @brief Get the export format
     * @return ExportFormat::CSV or ExportFormat::TSV depending on delimiter
     */
    ExportFormat getFormat() const noexcept override {
        return ExportFormat::CSV;  // Default, actual format depends on options
    }

private:
    std::string m_lastError;  ///< Last error message

    // Helper methods
    std::string formatNumber(Real value, const ExportOptions& options) const;
    std::string formatTimestamp() const;

    // Section writers
    void
    writeGeometrySection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    void
    writePropertiesSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    void writeLoadsSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    void writeDisplacementsSection(std::ostream& os,
                                   const ITrussView& truss,
                                   const IAnalysisResultsView& results,
                                   const ExportOptions& options);
    void writeMemberForcesSection(std::ostream& os,
                                  const ITrussView& truss,
                                  const IAnalysisResultsView& results,
                                  const ExportOptions& options);
    void writeReactionsSection(std::ostream& os,
                               const ITrussView& truss,
                               const IAnalysisResultsView& results,
                               const ExportOptions& options);
    void writeMetadataSection(std::ostream& os,
                              const IAnalysisResultsView& results,
                              const ExportOptions& options);
};

}  // namespace truss::infrastructure::export_
