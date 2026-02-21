/**
 * @file exporter_factory.hpp
 * @brief Factory for creating results exporters
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-07
 */

#pragma once

#include "export_types.hpp"
#include "exporter.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace truss::infrastructure::export_ {

/**
 * @brief Factory for creating concrete exporter instances
 *
 * Design Pattern: Factory Method
 * - Encapsulates exporter instantiation logic
 * - Client code depends only on IResultsExporter interface
 * - Adding new formats requires modifying only this factory
 *
 * Usage:
 * @code
 *   auto format = ExporterFactory::detectFormat("results.csv");
 *   auto exporter = ExporterFactory::create(format);
 *   exporter->exportResults(truss, results, "results.csv");
 * @endcode
 */
class ExporterFactory {
public:
    /**
     * @brief Create an exporter for the specified format
     *
     * @param format Export format
     * @return Unique pointer to concrete exporter implementation
     * @throws std::invalid_argument if format is not supported
     */
    static std::unique_ptr<IResultsExporter> create(ExportFormat format);

    /**
     * @brief Detect export format from file extension
     *
     * Supported extensions:
     * - .csv → CSV
     * - .tsv, .tab → TSV
     * - .json → JSON
     * - .xml → XML
     * - .txt → TXT
     * - .tex, .latex → LaTeX
     * - .html, .htm → HTML
     *
     * @param filePath File path (or just filename with extension)
     * @return Detected format (defaults to CSV if unknown)
     */
    static ExportFormat detectFormat(const std::filesystem::path& filePath);

    /**
     * @brief Get file extension for a format
     *
     * @param format Export format
     * @return Recommended file extension (e.g., ".csv", ".json")
     */
    static std::string getExtension(ExportFormat format);

    /**
     * @brief Get human-readable format name
     *
     * @param format Export format
     * @return Format name (e.g., "Comma-Separated Values", "JSON")
     */
    static std::string getFormatName(ExportFormat format);

private:
    // Factory is a utility class, no instantiation
    ExporterFactory() = delete;
};

}  // namespace truss::infrastructure::export_
