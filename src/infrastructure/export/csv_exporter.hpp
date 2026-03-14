/**
 * @file csv_exporter.hpp
 * @brief CSV format results exporter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
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

    // Section writers
    static void
    writeGeometrySection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void
    writePropertiesSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void
    writeLoadsSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void writeDisplacementsSection(std::ostream& os,
                                          const ITrussView& truss,
                                          const IAnalysisResultsView& results,
                                          const ExportOptions& options);
    static void writeMemberForcesSection(std::ostream& os,
                                         const ITrussView& truss,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options);
    static void writeReactionsSection(std::ostream& os,
                                      const ITrussView& truss,
                                      const IAnalysisResultsView& results,
                                      const ExportOptions& options);
    static void writeMetadataSection(std::ostream& os,
                                     const IAnalysisResultsView& results,
                                     const ExportOptions& options);
};

}  // namespace truss::infrastructure::export_
