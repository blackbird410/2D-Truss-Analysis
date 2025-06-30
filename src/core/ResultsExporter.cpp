/**
 * @file ResultsExporter.cpp
 * @brief Implementation of results export functionality
 * @author Civil Engineering Software Solutions
 * @version 2.2.0
 */

#include "ResultsExporter.hpp"
#include "Logger.hpp"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <filesystem>

namespace truss::core {

bool ResultsExporter::exportResults(const Truss& truss, 
                                   const AnalysisResults& results,
                                   const std::string& fileName,
                                   ExportFormat format,
                                   const ExportOptions& options) {
    try {
        m_lastError.clear();
        
        // Detect format from extension if not explicitly specified
        if (format == ExportFormat::CSV && fileName.find('.') != std::string::npos) {
            format = detectFormat(fileName);
        }
        
        switch (format) {
            case ExportFormat::CSV:
            case ExportFormat::TSV:
                return exportToCSV(truss, results, fileName, options);
            case ExportFormat::JSON:
                return exportToJSON(truss, results, fileName, options);
            case ExportFormat::XML:
                return exportToXML(truss, results, fileName, options);
            case ExportFormat::TXT:
                return exportToText(truss, results, fileName, options);
            case ExportFormat::LaTeX:
                return exportToLaTeX(truss, results, fileName, options);
            case ExportFormat::HTML:
                return exportToHTML(truss, results, fileName, options);
            default:
                m_lastError = "Unsupported export format";
                return false;
        }
    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        Logger::error("Results export failed: " + m_lastError);
        return false;
    }
}

bool ResultsExporter::exportToCSV(const Truss& truss, const AnalysisResults& results,
                                 const std::string& fileName, const ExportOptions& options) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + fileName;
        return false;
    }
    
    // Write header
    file << "# 2D Truss Analysis Results Export" << std::endl;
    file << "# Generated: " << formatTimestamp() << std::endl;
    file << "# Project: " << truss.getName() << std::endl;
    file << "#" << std::endl;
    
    ExportFormat format = (options.delimiter == "\t") ? ExportFormat::TSV : ExportFormat::CSV;
    
    if (options.includeGeometry) {
        file << std::endl << "# GEOMETRY" << std::endl;
        writeGeometrySection(file, truss, format, options);
    }
    
    if (options.includeProperties) {
        file << std::endl << "# MATERIAL AND SECTION PROPERTIES" << std::endl;
        writePropertiesSection(file, truss, format, options);
    }
    
    if (options.includeLoads) {
        file << std::endl << "# APPLIED LOADS" << std::endl;
        writeLoadsSection(file, truss, format, options);
    }
    
    if (options.includeDisplacements) {
        file << std::endl << "# NODAL DISPLACEMENTS" << std::endl;
        writeDisplacementsSection(file, truss, results, format, options);
    }
    
    if (options.includeMemberForces) {
        file << std::endl << "# MEMBER FORCES" << std::endl;
        writeMemberForcesSection(file, truss, results, format, options);
    }
    
    if (options.includeReactions) {
        file << std::endl << "# SUPPORT REACTIONS" << std::endl;
        writeReactionsSection(file, truss, results, format, options);
    }
    
    if (options.includeMetadata) {
        file << std::endl << "# ANALYSIS METADATA" << std::endl;
        writeMetadataSection(file, results, format, options);
    }
    
    file.close();
    Logger::info("Results exported to CSV: " + fileName);
    return true;
}

bool ResultsExporter::exportToJSON(const Truss& truss, const AnalysisResults& results,
                                  const std::string& fileName, const ExportOptions& options) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + fileName;
        return false;
    }
    
    file << "{\n";
    file << "  \"project\": {\n";
    file << "    \"name\": \"" << escapeString(truss.getName(), ExportFormat::JSON) << "\",\n";
    file << "    \"exportTime\": \"" << formatTimestamp() << "\",\n";
    file << "    \"version\": \"2.2.0\"\n";
    file << "  },\n";
    
    if (options.includeGeometry) {
        file << "  \"geometry\": {\n";
        file << "    \"nodes\": [\n";
        const auto& nodes = truss.getNodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& node = nodes[i];
            file << "      {\n";
            file << "        \"id\": " << node->getId() << ",\n";
            file << "        \"x\": " << formatNumber(node->getX(), options) << ",\n";
            file << "        \"y\": " << formatNumber(node->getY(), options) << ",\n";
            file << "        \"supportType\": \"" << static_cast<int>(node->getSupportType()) << "\"\n";
            file << "      }" << (i < nodes.size() - 1 ? "," : "") << "\n";
        }
        file << "    ],\n";
        
        file << "    \"members\": [\n";
        const auto& members = truss.getMembers();
        for (size_t i = 0; i < members.size(); ++i) {
            const auto& member = members[i];
            file << "      {\n";
            file << "        \"id\": " << member->getId() << ",\n";
            file << "        \"startNode\": " << member->getStartNode()->getId() << ",\n";
            file << "        \"endNode\": " << member->getEndNode()->getId() << ",\n";
            file << "        \"length\": " << formatNumber(member->getLength(), options) << "\n";
            file << "      }" << (i < members.size() - 1 ? "," : "") << "\n";
        }
        file << "    ]\n";
        file << "  },\n";
    }
    
    if (options.includeDisplacements && results.displacements.size() > 0) {
        file << "  \"displacements\": {\n";
        file << "    \"values\": [";
        for (size_t i = 0; i < results.displacements.size(); ++i) {
            file << formatNumber(results.displacements[i], options);
            if (i < results.displacements.size() - 1) file << ", ";
        }
        file << "],\n";
        file << "    \"maxDisplacement\": " << formatNumber(results.maxDisplacement, options) << "\n";
        file << "  },\n";
    }
    
    if (options.includeMemberForces && !results.memberForces.empty()) {
        file << "  \"memberForces\": {\n";
        file << "    \"values\": [";
        for (size_t i = 0; i < results.memberForces.size(); ++i) {
            file << formatNumber(results.memberForces[i], options);
            if (i < results.memberForces.size() - 1) file << ", ";
        }
        file << "]\n";
        file << "  },\n";
    }
    
    if (options.includeMetadata) {
        file << "  \"analysis\": {\n";
        file << "    \"converged\": " << (results.converged ? "true" : "false") << ",\n";
        file << "    \"iterations\": " << results.iterations << ",\n";
        file << "    \"totalDofs\": " << results.totalDofs << ",\n";
        file << "    \"freeDofs\": " << results.freeDofs << ",\n";
        file << "    \"maxStress\": " << formatNumber(results.maxStress, options) << "\n";
        file << "  }\n";
    }
    
    file << "}\n";
    
    file.close();
    Logger::info("Results exported to JSON: " + fileName);
    return true;
}

bool ResultsExporter::exportToText(const Truss& truss, const AnalysisResults& results,
                                  const std::string& fileName, const ExportOptions& options) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + fileName;
        return false;
    }
    
    file << "================================================================================\n";
    file << "                        2D TRUSS ANALYSIS RESULTS\n";
    file << "================================================================================\n\n";
    file << "Project Name: " << truss.getName() << "\n";
    file << "Export Time:  " << formatTimestamp() << "\n";
    file << "Software Version: 2.2.0\n\n";
    
    file << generateSummary(truss, results) << "\n\n";
    
    if (options.includeGeometry) {
        file << "GEOMETRY:\n";
        file << "─────────\n";
        file << "Nodes (" << truss.getNodeCount() << "):\n";
        file << std::setw(8) << "ID" << std::setw(15) << "X" << std::setw(15) << "Y" 
             << std::setw(12) << "Support" << "\n";
        file << std::string(50, '-') << "\n";
        
        for (const auto& node : truss.getNodes()) {
            file << std::setw(8) << node->getId()
                 << std::setw(15) << formatNumber(node->getX(), options)
                 << std::setw(15) << formatNumber(node->getY(), options)
                 << std::setw(12) << static_cast<int>(node->getSupportType()) << "\n";
        }
        file << "\n";
        
        file << "Members (" << truss.getMemberCount() << "):\n";
        file << std::setw(8) << "ID" << std::setw(10) << "Start" << std::setw(10) << "End" 
             << std::setw(15) << "Length" << "\n";
        file << std::string(43, '-') << "\n";
        
        for (const auto& member : truss.getMembers()) {
            file << std::setw(8) << member->getId()
                 << std::setw(10) << member->getStartNode()->getId()
                 << std::setw(10) << member->getEndNode()->getId()
                 << std::setw(15) << formatNumber(member->getLength(), options) << "\n";
        }
        file << "\n";
    }
    
    if (options.includeDisplacements && results.displacements.size() > 0) {
        file << "NODAL DISPLACEMENTS:\n";
        file << "───────────────────\n";
        file << "Maximum displacement: " << formatNumber(results.maxDisplacement, options) << "\n\n";
    }
    
    if (options.includeMemberForces && !results.memberForces.empty()) {
        file << "MEMBER FORCES:\n";
        file << "─────────────\n";
        file << std::setw(8) << "Member" << std::setw(15) << "Force" << std::setw(12) << "Type" << "\n";
        file << std::string(35, '-') << "\n";
        
        for (size_t i = 0; i < results.memberForces.size() && i < truss.getMemberCount(); ++i) {
            Real force = results.memberForces[i];
            std::string type = (force > 0) ? "Tension" : "Compression";
            file << std::setw(8) << (i + 1)
                 << std::setw(15) << formatNumber(std::abs(force), options)
                 << std::setw(12) << type << "\n";
        }
        file << "\n";
    }
    
    file.close();
    Logger::info("Results exported to text: " + fileName);
    return true;
}

bool ResultsExporter::exportToXML(const Truss& truss, const AnalysisResults& /*results*/,
                                 const std::string& fileName, const ExportOptions& options) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + fileName;
        return false;
    }
    
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<TrussAnalysisResults>\n";
    file << "  <Project>\n";
    file << "    <Name>" << escapeString(truss.getName(), ExportFormat::XML) << "</Name>\n";
    file << "    <ExportTime>" << formatTimestamp() << "</ExportTime>\n";
    file << "    <Version>2.2.0</Version>\n";
    file << "  </Project>\n";
    
    if (options.includeGeometry) {
        file << "  <Geometry>\n";
        file << "    <Nodes>\n";
        for (const auto& node : truss.getNodes()) {
            file << "      <Node id=\"" << node->getId() << "\">\n";
            file << "        <X>" << formatNumber(node->getX(), options) << "</X>\n";
            file << "        <Y>" << formatNumber(node->getY(), options) << "</Y>\n";
            file << "        <SupportType>" << static_cast<int>(node->getSupportType()) << "</SupportType>\n";
            file << "      </Node>\n";
        }
        file << "    </Nodes>\n";
        file << "    <Members>\n";
        for (const auto& member : truss.getMembers()) {
            file << "      <Member id=\"" << member->getId() << "\">\n";
            file << "        <StartNode>" << member->getStartNode()->getId() << "</StartNode>\n";
            file << "        <EndNode>" << member->getEndNode()->getId() << "</EndNode>\n";
            file << "        <Length>" << formatNumber(member->getLength(), options) << "</Length>\n";
            file << "      </Member>\n";
        }
        file << "    </Members>\n";
        file << "  </Geometry>\n";
    }
    
    file << "</TrussAnalysisResults>\n";
    
    file.close();
    Logger::info("Results exported to XML: " + fileName);
    return true;
}

bool ResultsExporter::exportToLaTeX(const Truss& truss, const AnalysisResults& results,
                                   const std::string& fileName, const ExportOptions& options) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + fileName;
        return false;
    }
    
    file << "\\documentclass{article}\n";
    file << "\\usepackage{booktabs}\n";
    file << "\\usepackage{geometry}\n";
    file << "\\geometry{margin=1in}\n";
    file << "\\title{2D Truss Analysis Results}\n";
    file << "\\author{Civil Engineering Software Solutions}\n";
    file << "\\date{" << formatTimestamp() << "}\n\n";
    file << "\\begin{document}\n";
    file << "\\maketitle\n\n";
    
    file << "\\section{Project Information}\n";
    file << "\\begin{itemize}\n";
    file << "\\item Project Name: " << escapeString(truss.getName(), ExportFormat::LaTeX) << "\n";
    file << "\\item Total Nodes: " << truss.getNodeCount() << "\n";
    file << "\\item Total Members: " << truss.getMemberCount() << "\n";
    file << "\\item Analysis Status: " << (results.converged ? "Converged" : "Did not converge") << "\n";
    file << "\\end{itemize}\n\n";
    
    if (options.includeGeometry) {
        file << "\\section{Geometry}\n";
        file << "\\subsection{Nodes}\n";
        file << "\\begin{table}[h]\n";
        file << "\\centering\n";
        file << "\\begin{tabular}{cccc}\n";
        file << "\\toprule\n";
        file << "Node ID & X Coordinate & Y Coordinate & Support Type \\\\\n";
        file << "\\midrule\n";
        
        for (const auto& node : truss.getNodes()) {
            file << node->getId() << " & "
                 << formatNumber(node->getX(), options) << " & "
                 << formatNumber(node->getY(), options) << " & "
                 << static_cast<int>(node->getSupportType()) << " \\\\\n";
        }
        
        file << "\\bottomrule\n";
        file << "\\end{tabular}\n";
        file << "\\caption{Node Coordinates and Support Types}\n";
        file << "\\end{table}\n\n";
    }
    
    file << "\\end{document}\n";
    
    file.close();
    Logger::info("Results exported to LaTeX: " + fileName);
    return true;
}

bool ResultsExporter::exportToHTML(const Truss& truss, const AnalysisResults& results,
                                  const std::string& fileName, const ExportOptions& options) {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + fileName;
        return false;
    }
    
    file << "<!DOCTYPE html>\n";
    file << "<html>\n<head>\n";
    file << "<title>2D Truss Analysis Results</title>\n";
    file << "<style>\n";
    file << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    file << "table { border-collapse: collapse; width: 100%; margin: 10px 0; }\n";
    file << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    file << "th { background-color: #f2f2f2; }\n";
    file << ".summary { background-color: #e7f3ff; padding: 15px; border-radius: 5px; }\n";
    file << "</style>\n";
    file << "</head>\n<body>\n";
    
    file << "<h1>2D Truss Analysis Results</h1>\n";
    file << "<div class=\"summary\">\n";
    file << "<h2>Project Summary</h2>\n";
    file << "<p><strong>Project Name:</strong> " << escapeString(truss.getName(), ExportFormat::HTML) << "</p>\n";
    file << "<p><strong>Export Time:</strong> " << formatTimestamp() << "</p>\n";
    file << "<p><strong>Total Nodes:</strong> " << truss.getNodeCount() << "</p>\n";
    file << "<p><strong>Total Members:</strong> " << truss.getMemberCount() << "</p>\n";
    file << "<p><strong>Analysis Status:</strong> " << (results.converged ? "Converged" : "Did not converge") << "</p>\n";
    file << "</div>\n";
    
    if (options.includeGeometry) {
        file << "<h2>Geometry</h2>\n";
        file << "<h3>Nodes</h3>\n";
        file << "<table>\n";
        file << "<tr><th>Node ID</th><th>X Coordinate</th><th>Y Coordinate</th><th>Support Type</th></tr>\n";
        
        for (const auto& node : truss.getNodes()) {
            file << "<tr>";
            file << "<td>" << node->getId() << "</td>";
            file << "<td>" << formatNumber(node->getX(), options) << "</td>";
            file << "<td>" << formatNumber(node->getY(), options) << "</td>";
            file << "<td>" << static_cast<int>(node->getSupportType()) << "</td>";
            file << "</tr>\n";
        }
        file << "</table>\n";
    }
    
    file << "</body>\n</html>\n";
    
    file.close();
    Logger::info("Results exported to HTML: " + fileName);
    return true;
}

std::vector<std::string> ResultsExporter::getSupportedExtensions(ExportFormat format) {
    switch (format) {
        case ExportFormat::CSV: return {".csv"};
        case ExportFormat::TSV: return {".tsv", ".tab"};
        case ExportFormat::JSON: return {".json"};
        case ExportFormat::XML: return {".xml"};
        case ExportFormat::TXT: return {".txt"};
        case ExportFormat::LaTeX: return {".tex", ".latex"};
        case ExportFormat::HTML: return {".html", ".htm"};
        default: return {};
    }
}

ExportFormat ResultsExporter::detectFormat(const std::string& fileName) {
    auto pos = fileName.find_last_of('.');
    if (pos == std::string::npos) return ExportFormat::CSV;
    
    std::string ext = fileName.substr(pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".csv") return ExportFormat::CSV;
    if (ext == ".tsv" || ext == ".tab") return ExportFormat::TSV;
    if (ext == ".json") return ExportFormat::JSON;
    if (ext == ".xml") return ExportFormat::XML;
    if (ext == ".txt") return ExportFormat::TXT;
    if (ext == ".tex" || ext == ".latex") return ExportFormat::LaTeX;
    if (ext == ".html" || ext == ".htm") return ExportFormat::HTML;
    
    return ExportFormat::CSV; // Default
}

std::string ResultsExporter::generateSummary(const Truss& truss, const AnalysisResults& results) {
    std::stringstream ss;
    ss << "ANALYSIS SUMMARY:\n";
    ss << "─────────────────\n";
    ss << "Structure Statistics:\n";
    ss << "  • Total Nodes: " << truss.getNodeCount() << "\n";
    ss << "  • Total Members: " << truss.getMemberCount() << "\n";
    ss << "  • Total DOFs: " << results.totalDofs << "\n";
    ss << "  • Free DOFs: " << results.freeDofs << "\n";
    ss << "  • Constrained DOFs: " << results.constrainedDofs << "\n\n";
    
    ss << "Analysis Results:\n";
    ss << "  • Convergence: " << (results.converged ? "YES" : "NO") << "\n";
    ss << "  • Iterations: " << results.iterations << "\n";
    ss << "  • Maximum Displacement: " << results.maxDisplacement << "\n";
    ss << "  • Maximum Stress: " << results.maxStress << "\n";
    
    return ss.str();
}

std::string ResultsExporter::formatNumber(Real value, const ExportOptions& options) const {
    std::stringstream ss;
    if (options.useScientificNotation) {
        ss << std::scientific;
    } else {
        ss << std::fixed;
    }
    ss << std::setprecision(options.precision) << value;
    return ss.str();
}

std::string ResultsExporter::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string ResultsExporter::escapeString(const std::string& str, ExportFormat format) const {
    std::string result = str;
    switch (format) {
        case ExportFormat::JSON:
            // Escape quotes and backslashes for JSON
            for (size_t pos = 0; pos < result.length(); ++pos) {
                if (result[pos] == '"' || result[pos] == '\\') {
                    result.insert(pos, "\\");
                    ++pos;
                }
            }
            break;
        case ExportFormat::XML:
        case ExportFormat::HTML:
            // Escape XML/HTML entities
            for (size_t pos = 0; pos < result.length(); ++pos) {
                if (result[pos] == '<') {
                    result.replace(pos, 1, "&lt;");
                    pos += 3;
                } else if (result[pos] == '>') {
                    result.replace(pos, 1, "&gt;");
                    pos += 3;
                } else if (result[pos] == '&') {
                    result.replace(pos, 1, "&amp;");
                    pos += 4;
                }
            }
            break;
        case ExportFormat::LaTeX:
            // Escape LaTeX special characters
            for (size_t pos = 0; pos < result.length(); ++pos) {
                if (result[pos] == '\\' || result[pos] == '{' || result[pos] == '}' ||
                    result[pos] == '$' || result[pos] == '&' || result[pos] == '%' ||
                    result[pos] == '#' || result[pos] == '^' || result[pos] == '_') {
                    result.insert(pos, "\\");
                    ++pos;
                }
            }
            break;
        default:
            break;
    }
    return result;
}

// Section writers implementation (simplified for space)
void ResultsExporter::writeGeometrySection(std::ostream& os, const Truss& truss, 
                                          ExportFormat /*format*/, const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "Node ID" << delim << "X" << delim << "Y" << delim << "Support Type" << std::endl;
    for (const auto& node : truss.getNodes()) {
        os << node->getId() << delim 
           << formatNumber(node->getX(), options) << delim
           << formatNumber(node->getY(), options) << delim
           << static_cast<int>(node->getSupportType()) << std::endl;
    }
    
    os << std::endl;
    os << "Member ID" << delim << "Start Node" << delim << "End Node" << delim << "Length" << std::endl;
    for (const auto& member : truss.getMembers()) {
        os << member->getId() << delim
           << member->getStartNode()->getId() << delim
           << member->getEndNode()->getId() << delim
           << formatNumber(member->getLength(), options) << std::endl;
    }
}

void ResultsExporter::writePropertiesSection(std::ostream& os, const Truss& /*truss*/,
                                            ExportFormat /*format*/, const ExportOptions& /*options*/) {
    // Implementation for material properties
    os << "# Material properties section not yet implemented" << std::endl;
}

void ResultsExporter::writeLoadsSection(std::ostream& os, const Truss& /*truss*/,
                                       ExportFormat /*format*/, const ExportOptions& /*options*/) {
    // Implementation for applied loads
    os << "# Applied loads section not yet implemented" << std::endl;
}

void ResultsExporter::writeDisplacementsSection(std::ostream& os, const Truss& /*truss*/, 
                                               const AnalysisResults& results,
                                               ExportFormat /*format*/, const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "DOF" << delim << "Displacement" << std::endl;
    for (size_t i = 0; i < results.displacements.size(); ++i) {
        os << i << delim << formatNumber(results.displacements[i], options) << std::endl;
    }
}

void ResultsExporter::writeMemberForcesSection(std::ostream& os, const Truss& truss,
                                              const AnalysisResults& results,
                                              ExportFormat /*format*/, const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "Member ID" << delim << "Axial Force" << delim << "Type" << std::endl;
    for (size_t i = 0; i < results.memberForces.size() && i < truss.getMemberCount(); ++i) {
        Real force = results.memberForces[i];
        std::string type = (force > 0) ? "Tension" : "Compression";
        os << (i + 1) << delim 
           << formatNumber(force, options) << delim 
           << type << std::endl;
    }
}

void ResultsExporter::writeReactionsSection(std::ostream& os, const Truss& /*truss*/,
                                           const AnalysisResults& results,
                                           ExportFormat /*format*/, const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "DOF" << delim << "Reaction Force" << std::endl;
    for (size_t i = 0; i < results.reactions.size(); ++i) {
        os << i << delim << formatNumber(results.reactions[i], options) << std::endl;
    }
}

void ResultsExporter::writeMetadataSection(std::ostream& os, const AnalysisResults& results,
                                          ExportFormat /*format*/, const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "Property" << delim << "Value" << std::endl;
    os << "Converged" << delim << (results.converged ? "Yes" : "No") << std::endl;
    os << "Iterations" << delim << results.iterations << std::endl;
    os << "Total DOFs" << delim << results.totalDofs << std::endl;
    os << "Free DOFs" << delim << results.freeDofs << std::endl;
    os << "Max Displacement" << delim << formatNumber(results.maxDisplacement, options) << std::endl;
    os << "Max Stress" << delim << formatNumber(results.maxStress, options) << std::endl;
}

} // namespace truss::core
