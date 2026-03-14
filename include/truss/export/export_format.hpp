/**
 * @file export_format.hpp
 * @brief Export format specifications for analysis results
 */

#pragma once

#include <memory>
#include <string>

namespace truss {

// Forward declarations
class AnalysisResults;

/**
 * @enum ExportFormat
 * @brief Supported output formats for analysis results
 */
enum class ExportFormat {
    CSV,   ///< Comma-separated values
    TSV,   ///< Tab-separated values
    JSON,  ///< JSON format
    XML,   ///< XML format
    HTML,  ///< HTML report
    TXT,   ///< Plain text
    LaTeX  ///< LaTeX for academic papers
};

/**
 * @class Exporter
 * @brief Interface for exporting analysis results in various formats
 */
class Exporter {
public:
    virtual ~Exporter() = default;

    /**
     * Export analysis results to a file
     * @param results The analysis results to export
     * @param filename Output file path
     * @throws std::exception if export fails
     */
    virtual void export_results(const AnalysisResults& results, const std::string& filename) = 0;
};

/**
 * Factory function to create an exporter for the specified format
 * @param format The desired output format
 * @return Unique pointer to an Exporter instance
 */
std::unique_ptr<Exporter> createExporter(ExportFormat format);

}  // namespace truss
