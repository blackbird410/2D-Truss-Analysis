/**
 * @file ResultsExporter.hpp
 * @brief Results export functionality for various formats
 * @author Civil Engineering Software Solutions
 * @version 2.2.0
 */

#pragma once

#include "Types.hpp"
#include "Truss.hpp"
#include "analysis/AnalysisOrchestrator.hpp"
#include <string>
#include <fstream>
#include <memory>

namespace truss::core {

// Use AnalysisResults from the analysis namespace
using analysis::AnalysisResults;

/**
 * @brief Export format options
 */
enum class ExportFormat {
    CSV,        ///< Comma-separated values
    TSV,        ///< Tab-separated values
    JSON,       ///< JSON format
    XML,        ///< XML format
    TXT,        ///< Plain text format
    LaTeX,      ///< LaTeX table format
    HTML        ///< HTML table format
};

/**
 * @brief Export section options
 */
struct ExportOptions {
    bool includeGeometry{true};         ///< Include node coordinates and member connectivity
    bool includeProperties{true};       ///< Include material and section properties
    bool includeLoads{true};           ///< Include applied loads
    bool includeDisplacements{true};   ///< Include nodal displacements
    bool includeMemberForces{true};    ///< Include member forces
    bool includeReactions{true};       ///< Include support reactions
    bool includeStresses{true};        ///< Include member stresses
    bool includeUtilization{true};     ///< Include utilization ratios
    bool includeMetadata{true};        ///< Include analysis metadata
    bool useScientificNotation{false}; ///< Use scientific notation for numbers
    int precision{6};                  ///< Number of decimal places
    std::string delimiter{","};        ///< Delimiter for CSV/TSV formats
};

/**
 * @brief Comprehensive results exporter
 */
class ResultsExporter {
public:
    /**
     * @brief Construct a results exporter
     */
    ResultsExporter() = default;
    
    /**
     * @brief Export analysis results to file
     * @param truss The analyzed truss structure
     * @param results Analysis results
     * @param fileName Output file name
     * @param format Export format
     * @param options Export options
     * @return true if export successful
     */
    bool exportResults(const Truss& truss, 
                      const AnalysisResults& results,
                      const std::string& fileName,
                      ExportFormat format = ExportFormat::CSV,
                      const ExportOptions& options = ExportOptions{});
    
    /**
     * @brief Export to CSV format
     */
    bool exportToCSV(const Truss& truss, const AnalysisResults& results,
                    const std::string& fileName, const ExportOptions& options);
    
    /**
     * @brief Export to JSON format
     */
    bool exportToJSON(const Truss& truss, const AnalysisResults& results,
                     const std::string& fileName, const ExportOptions& options);
    
    /**
     * @brief Export to XML format
     */
    bool exportToXML(const Truss& truss, const AnalysisResults& results,
                    const std::string& fileName, const ExportOptions& options);
    
    /**
     * @brief Export to plain text format
     */
    bool exportToText(const Truss& truss, const AnalysisResults& results,
                     const std::string& fileName, const ExportOptions& options);
    
    /**
     * @brief Export to LaTeX format
     */
    bool exportToLaTeX(const Truss& truss, const AnalysisResults& results,
                      const std::string& fileName, const ExportOptions& options);
    
    /**
     * @brief Export to HTML format
     */
    bool exportToHTML(const Truss& truss, const AnalysisResults& results,
                     const std::string& fileName, const ExportOptions& options);
    
    /**
     * @brief Get the last error message
     */
    const std::string& getLastError() const noexcept { return m_lastError; }
    
    /**
     * @brief Get supported file extensions for format
     */
    static std::vector<std::string> getSupportedExtensions(ExportFormat format);
    
    /**
     * @brief Detect format from file extension
     */
    static ExportFormat detectFormat(const std::string& fileName);
    
    /**
     * @brief Generate summary statistics
     */
    std::string generateSummary(const Truss& truss, const AnalysisResults& results);

private:
    std::string m_lastError;
    
    // Helper methods
    std::string formatNumber(Real value, const ExportOptions& options) const;
    std::string formatTimestamp() const;
    std::string escapeString(const std::string& str, ExportFormat format) const;
    
    // Section writers
    void writeGeometrySection(std::ostream& os, const Truss& truss, 
                            ExportFormat format, const ExportOptions& options);
    void writePropertiesSection(std::ostream& os, const Truss& truss,
                              ExportFormat format, const ExportOptions& options);
    void writeLoadsSection(std::ostream& os, const Truss& truss,
                          ExportFormat format, const ExportOptions& options);
    void writeDisplacementsSection(std::ostream& os, const Truss& truss, 
                                 const AnalysisResults& results,
                                 ExportFormat format, const ExportOptions& options);
    void writeMemberForcesSection(std::ostream& os, const Truss& truss,
                                const AnalysisResults& results,
                                ExportFormat format, const ExportOptions& options);
    void writeReactionsSection(std::ostream& os, const Truss& truss,
                             const AnalysisResults& results,
                             ExportFormat format, const ExportOptions& options);
    void writeMetadataSection(std::ostream& os, const AnalysisResults& results,
                            ExportFormat format, const ExportOptions& options);
};

// Type aliases
using ResultsExporterPtr = std::shared_ptr<ResultsExporter>;

} // namespace truss::core
