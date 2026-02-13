/**
 * @file xml_exporter.hpp
 * @brief XML format results exporter implementation
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
    std::string getLastError() const override {
        return m_lastError;
    }
    
    /**
     * @brief Get the export format
     * @return ExportFormat::XML
     */
    ExportFormat getFormat() const noexcept override {
        return ExportFormat::XML;
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
                             const ExportOptions& options);
    void writePropertiesSection(std::ostream& os, const ITrussView& truss,
                               const ExportOptions& options);
    void writeLoadsSection(std::ostream& os, const ITrussView& truss,
                          const ExportOptions& options);
    void writeDisplacementsSection(std::ostream& os, const IAnalysisResultsView& results,
                                  const ExportOptions& options);
    void writeMemberForcesSection(std::ostream& os, const IAnalysisResultsView& results,
                                 const ExportOptions& options);
    void writeReactionsSection(std::ostream& os, const IAnalysisResultsView& results,
                              const ExportOptions& options);
    void writeMetadataSection(std::ostream& os, const IAnalysisResultsView& results,
                             const ExportOptions& options);
};

} // namespace truss::infrastructure::export_
