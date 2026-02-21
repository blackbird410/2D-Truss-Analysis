/**
 * @file latex_exporter.hpp
 * @brief LaTeX format results exporter implementation
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
 * @brief LaTeX format exporter
 *
 * Exports analysis results to LaTeX format suitable for inclusion in
 * technical documents. Produces a complete LaTeX document with tabular
 * environments for structured data.
 *
 * CRITICAL: This implementation follows the corrected 8-section export contract.
 * ALL 8 sections MUST be included to maintain semantic equivalence with
 * CSV, JSON, XML, and HTML exporters. LaTeX is just another presentation layer.
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
 *
 * LaTeX Output Structure:
 * - Complete document with \documentclass, \begin{document}, \end{document}
 * - Tabular environments for data tables
 * - Section headings with \section{}
 * - Proper escaping of special LaTeX characters
 * - Floating-point values formatted according to ExportOptions
 */
class LaTeXExporter : public IResultsExporter {
public:
    /**
     * @brief Construct a LaTeX exporter
     */
    LaTeXExporter() = default;

    /**
     * @brief Virtual destructor
     */
    ~LaTeXExporter() override = default;

    /**
     * @brief Export analysis results to LaTeX file
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
     * @return Error message (empty if no error)
     */
    std::string getLastError() const override { return m_lastError; }

    /**
     * @brief Get the export format
     * @return ExportFormat::LaTeX
     */
    ExportFormat getFormat() const override { return ExportFormat::LaTeX; }

private:
    std::string m_lastError;

    /**
     * @brief Write LaTeX document preamble
     */
    static void writePreamble(std::ostream& os, const ITrussView& truss);

    /**
     * @brief Write document closing
     */
    [[maybe_unused]] static void writeClosing(std::ostream& os);

    /**
     * @brief Write geometry section (nodes + members)
     */
    static void
    writeGeometrySection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);

    /**
     * @brief Write material properties section
     */
    static void
    writePropertiesSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);

    /**
     * @brief Write applied loads section
     */
    static void writeLoadsSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);

    /**
     * @brief Write displacements section
     */
    static void writeDisplacementsSection(std::ostream& os,
                                          const IAnalysisResultsView& results,
                                          const ExportOptions& options);

    /**
     * @brief Write member forces section
     */
    static void writeMemberForcesSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options);

    /**
     * @brief Write reactions section
     */
    static void writeReactionsSection(std::ostream& os,
                                      const IAnalysisResultsView& results,
                                      const ExportOptions& options);

    /**
     * @brief Write analysis metadata section
     */
    static void writeMetadataSection(std::ostream& os,
                                     const IAnalysisResultsView& results,
                                     const ExportOptions& options);

    /**
     * @brief Format a number according to export options
     * @param value The value to format
     * @param options Export options (precision, scientific notation)
     * @return Formatted string
     */
    static std::string formatNumber(Real value, const ExportOptions& options);

    /**
     * @brief Format timestamp for LaTeX
     * @return Formatted timestamp string
     */
    static std::string formatTimestamp();

    /**
     * @brief Escape special LaTeX characters
     * @param text Input text
     * @return Escaped text safe for LaTeX
     */
    static std::string escapeLatex(const std::string& text);
};

}  // namespace truss::infrastructure::export_
