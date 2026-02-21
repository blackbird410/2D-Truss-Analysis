/**
 * @file text_exporter.hpp
 * @brief Plain text format results exporter implementation
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
 * @brief Plain text format exporter
 *
 * Exports analysis results to human-readable plain text format.
 * Produces formatted output with headers, separators, and aligned columns.
 *
 * CRITICAL: This implementation follows the corrected 8-section export contract.
 * ALL 8 sections MUST be included to maintain semantic equivalence with
 * CSV, JSON, XML, HTML, and LaTeX exporters. Text is just another presentation layer.
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
 * Text Output Features:
 * - Clear section headers with separator lines
 * - Column-aligned tabular data
 * - Box-drawing characters for visual structure
 * - Readable without special tools or parsing
 * - Fixed-width formatting for easy viewing in terminals
 */
class TextExporter : public IResultsExporter {
public:
    /**
     * @brief Construct a text exporter
     */
    TextExporter() = default;

    /**
     * @brief Virtual destructor
     */
    ~TextExporter() override = default;

    /**
     * @brief Export analysis results to text file
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
     * @brief Get the format this exporter handles
     * @return ExportFormat::TXT
     */
    ExportFormat getFormat() const override { return ExportFormat::TXT; }

private:
    /**
     * @brief Write document header with project title and timestamp
     * @param os Output stream
     * @param truss Truss structure
     */
    static void writeHeader(std::ostream& os, const ITrussView& truss);

    /**
     * @brief Write geometry section with nodes and members (Section 2)
     * @param os Output stream
     * @param truss Truss structure
     * @param options Export options
     */
    static void
    writeGeometrySection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);

    /**
     * @brief Write material properties section (Section 3 - placeholder)
     * @param os Output stream
     * @param truss Truss structure
     * @param options Export options
     */
    static void
    writePropertiesSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);

    /**
     * @brief Write applied loads section (Section 4 - placeholder)
     * @param os Output stream
     * @param truss Truss structure
     * @param options Export options
     */
    static void writeLoadsSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);

    /**
     * @brief Write nodal displacements section (Section 5)
     * @param os Output stream
     * @param results Analysis results
     * @param options Export options
     */
    static void writeDisplacementsSection(std::ostream& os,
                                          const IAnalysisResultsView& results,
                                          const ExportOptions& options);

    /**
     * @brief Write member forces section (Section 6)
     * @param os Output stream
     * @param results Analysis results
     * @param options Export options
     */
    static void writeMemberForcesSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options);

    /**
     * @brief Write support reactions section (Section 7 - MANDATORY)
     * @param os Output stream
     * @param results Analysis results
     * @param options Export options
     */
    static void writeReactionsSection(std::ostream& os,
                                      const IAnalysisResultsView& results,
                                      const ExportOptions& options);

    /**
     * @brief Write analysis metadata section (Section 8)
     * @param os Output stream
     * @param results Analysis results
     * @param options Export options
     */
    static void writeMetadataSection(std::ostream& os,
                                     const IAnalysisResultsView& results,
                                     const ExportOptions& options);

    /**
     * @brief Format a number according to export options
     * @param value Floating-point value
     * @param options Export options
     * @return Formatted string
     */
    static std::string formatNumber(Real value, const ExportOptions& options);

    /**
     * @brief Format current timestamp
     * @return Formatted timestamp string
     */
    static std::string formatTimestamp();

    /**
     * @brief Write a section separator line
     * @param os Output stream
     * @param width Line width
     */
    static void writeSeparator(std::ostream& os, int width = 78);

    /**
     * @brief Write a section header
     * @param os Output stream
     * @param title Section title
     */
    static void writeSectionHeader(std::ostream& os, const std::string& title);

    std::string m_lastError;  ///< Last error message
};

}  // namespace truss::infrastructure::export_
