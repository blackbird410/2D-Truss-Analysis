/**
 * @file html_exporter.hpp
 * @brief HTML format results exporter implementation
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
 * @brief HTML format exporter
 *
 * Exports analysis results to HTML format with embedded CSS styling.
 * Produces a complete, self-contained HTML document suitable for viewing
 * in web browsers.
 *
 * CRITICAL: This implementation follows the corrected 8-section export contract.
 * ALL 8 sections MUST be included to maintain semantic equivalence with
 * CSV, JSON, and XML exporters. Legacy HTML behavior (incomplete sections)
 * has been intentionally replaced.
 *
 * 8-Section Contract (MANDATORY):
 * 1. Project metadata
 * 2. Geometry (nodes + members)
 * 3. Material properties (placeholder until domain model implements)
 * 4. Applied loads (placeholder until domain model implements)
 * 5. Displacements
 * 6. Member forces
 * 7. Reactions (MANDATORY for equilibrium verification)
 * 8. Analysis metadata
 */
class HTMLExporter : public IResultsExporter {
public:
    /**
     * @brief Construct an HTML exporter
     */
    HTMLExporter() = default;

    /**
     * @brief Virtual destructor
     */
    ~HTMLExporter() override = default;

    /**
     * @brief Export analysis results to HTML file
     * @param truss The analyzed truss structure
     * @param results Analysis results
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
    std::string getLastError() const override { return m_lastError; }

    /**
     * @brief Get the export format
     * @return ExportFormat::HTML
     */
    ExportFormat getFormat() const noexcept override { return ExportFormat::HTML; }

private:
    std::string m_lastError;  ///< Last error message

    // Helper methods
    static std::string formatNumber(Real value, const ExportOptions& options);
    static std::string formatTimestamp();
    static std::string escapeHtml(const std::string& text);

    // Document structure methods
    static void writeHeader(std::ostream& os, const ITrussView& truss);
    static void writeStyles(std::ostream& os);
    static void writeFooter(std::ostream& os);

    // Section writers (MUST implement all 8 sections)
    static void
    writeGeometrySection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void
    writePropertiesSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void
    writeLoadsSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void writeDisplacementsSection(std::ostream& os,
                                          const IAnalysisResultsView& results,
                                          const ExportOptions& options);
    static void writeMemberForcesSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options);
    static void writeReactionsSection(std::ostream& os,
                                      const IAnalysisResultsView& results,
                                      const ExportOptions& options);
    static void writeMetadataSection(std::ostream& os,
                                     const IAnalysisResultsView& results,
                                     const ExportOptions& options);
};

}  // namespace truss::infrastructure::export_
