/**
 * @file exporter_factory.cpp
 * @brief Factory for creating results exporters - Implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "exporter_factory.hpp"

#include "csv_exporter.hpp"
#include "html_exporter.hpp"
#include "json_exporter.hpp"
#include "latex_exporter.hpp"
#include "text_exporter.hpp"
#include "xml_exporter.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace truss::infrastructure::export_ {

std::unique_ptr<IResultsExporter> ExporterFactory::create(ExportFormat format) {
    switch (format) {
        case ExportFormat::CSV:
        case ExportFormat::TSV:
            return std::make_unique<CSVExporter>();

        case ExportFormat::JSON:
            return std::make_unique<JSONExporter>();

        case ExportFormat::XML:
            return std::make_unique<XMLExporter>();

        case ExportFormat::HTML:
            return std::make_unique<HTMLExporter>();

        case ExportFormat::TXT:
            return std::make_unique<TextExporter>();

        case ExportFormat::LaTeX:
            return std::make_unique<LaTeXExporter>();

        default:
            throw std::invalid_argument("Unsupported export format");
    }
}

ExportFormat ExporterFactory::detectFormat(const std::filesystem::path& filePath) {
    std::string extension = filePath.extension().string();

    // Convert to lowercase for case-insensitive comparison
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    if (extension == ".csv") {
        return ExportFormat::CSV;
    } else if (extension == ".tsv" || extension == ".tab") {
        return ExportFormat::TSV;
    } else if (extension == ".json") {
        return ExportFormat::JSON;
    } else if (extension == ".xml") {
        return ExportFormat::XML;
    } else if (extension == ".html" || extension == ".htm") {
        return ExportFormat::HTML;
    } else if (extension == ".txt") {
        return ExportFormat::TXT;
    } else if (extension == ".tex" || extension == ".latex") {
        return ExportFormat::LaTeX;
    } else {
        // Default to CSV if unknown
        return ExportFormat::CSV;
    }
}

std::string ExporterFactory::getExtension(ExportFormat format) {
    switch (format) {
        case ExportFormat::CSV:
            return ".csv";
        case ExportFormat::TSV:
            return ".tsv";
        case ExportFormat::JSON:
            return ".json";
        case ExportFormat::XML:
            return ".xml";
        case ExportFormat::HTML:
            return ".html";
        case ExportFormat::TXT:
            return ".txt";
        case ExportFormat::LaTeX:
            return ".tex";
        default:
            return ".csv";
    }
}

std::string ExporterFactory::getFormatName(ExportFormat format) {
    switch (format) {
        case ExportFormat::CSV:
            return "Comma-Separated Values";
        case ExportFormat::TSV:
            return "Tab-Separated Values";
        case ExportFormat::JSON:
            return "JavaScript Object Notation";
        case ExportFormat::XML:
            return "Extensible Markup Language";
        case ExportFormat::HTML:
            return "HyperText Markup Language";
        case ExportFormat::TXT:
            return "Plain Text";
        case ExportFormat::LaTeX:
            return "LaTeX Document";
        default:
            return "Unknown Format";
    }
}

}  // namespace truss::infrastructure::export_
