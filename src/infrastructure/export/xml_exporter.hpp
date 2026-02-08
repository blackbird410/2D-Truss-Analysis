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
using core::Truss;
using core::analysis::AnalysisResults;

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
    bool exportResults(const Truss& truss,
                      const AnalysisResults& results,
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
    void writeProjectSection(std::ostream& os, const Truss& truss,
                            const ExportOptions& options);
    void writeGeometrySection(std::ostream& os, const Truss& truss,
                             const ExportOptions& options);
    // Note: Legacy XML export only includes Project and Geometry sections
};

} // namespace truss::infrastructure::export_
