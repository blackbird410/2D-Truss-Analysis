/**
 * @file xml_exporter.hpp
 * @brief XML format results exporter implementation
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
 * @brief XML format exporter
 *
 * Exports analysis results to Extensible Markup Language (XML) format.
 * Provides hierarchical, machine-readable output suitable for data exchange
 * and integration with other engineering software.
 */
class XMLExporter : public IResultsExporter {
public:
    /**
     * @brief Construct an XML exporter
     */
    XMLExporter() = default;

    /**
     * @brief Virtual destructor
     */
    ~XMLExporter() override = default;

    /**
     * @brief Export analysis results to XML file
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
     * @return ExportFormat::XML
     */
    ExportFormat getFormat() const noexcept override { return ExportFormat::XML; }

private:
    std::string m_lastError;  ///< Last error message

    // Helper methods
    static std::string formatNumber(Real value, const ExportOptions& options);
    static std::string formatTimestamp();
    static std::string escapeString(const std::string& str);

    // Section writers
    static void
    writeGeometrySection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void
    writePropertiesSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
    static void writeLoadsSection(std::ostream& os, const ITrussView& truss, const ExportOptions& options);
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
