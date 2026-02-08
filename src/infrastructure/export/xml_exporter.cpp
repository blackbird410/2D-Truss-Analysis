/**
 * @file xml_exporter.cpp
 * @brief XML format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "xml_exporter.hpp"
#include "src/core/Logger.hpp"
#include <fstream>
#include <ctime>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::Truss;
using core::analysis::AnalysisResults;

bool XMLExporter::exportResults(const Truss& truss,
                                const AnalysisResults& results,
                                const std::filesystem::path& filePath,
                                const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }
    
    try {
        // XML declaration
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<TrussAnalysisResults>\n";
        
        // Project metadata (always included)
        writeProjectSection(file, truss, options);
        
        // Geometry section (conditional)
        if (options.includeGeometry) {
            writeGeometrySection(file, truss, options);
        }
        
        // Note: Legacy XML export only includes Project and Geometry sections
        // This matches the golden master behavior
        
        file << "</TrussAnalysisResults>\n";
        
        file.close();
        core::Logger::info("Results exported to XML: " + filePath.string());
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        core::Logger::error("XML export failed: " + m_lastError);
        return false;
    }
}

std::string XMLExporter::formatNumber(Real value, const ExportOptions& options) const {
    std::stringstream ss;
    if (options.useScientificNotation) {
        ss << std::scientific;
    } else {
        ss << std::fixed;
    }
    ss << std::setprecision(options.precision) << value;
    return ss.str();
}

std::string XMLExporter::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string XMLExporter::escapeString(const std::string& str) const {
    std::string result;
    result.reserve(str.length());
    
    for (char c : str) {
        switch (c) {
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '&':  result += "&amp;";  break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default:
                if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') {
                    // Control characters (except tab, newline, carriage return)
                    std::ostringstream oss;
                    oss << "&#" << static_cast<int>(static_cast<unsigned char>(c)) << ";";
                    result += oss.str();
                } else {
                    result += c;
                }
        }
    }
    return result;
}

void XMLExporter::writeProjectSection(std::ostream& os, const Truss& truss,
                                       const ExportOptions& options) {
    os << "  <Project>\n";
    os << "    <Name>" << escapeString(truss.getName()) << "</Name>\n";
    os << "    <ExportTime>" << formatTimestamp() << "</ExportTime>\n";
    os << "    <Version>2.2.0</Version>\n";
    os << "  </Project>\n";
}

void XMLExporter::writeGeometrySection(std::ostream& os, const Truss& truss,
                                        const ExportOptions& options) {
    os << "  <Geometry>\n";
    
    // Nodes
    os << "    <Nodes>\n";
    for (const auto& node : truss.getNodes()) {
        os << "      <Node id=\"" << node->getId() << "\">\n";
        os << "        <X>" << formatNumber(node->getX(), options) << "</X>\n";
        os << "        <Y>" << formatNumber(node->getY(), options) << "</Y>\n";
        os << "        <SupportType>" << static_cast<int>(node->getSupportType()) << "</SupportType>\n";
        os << "      </Node>\n";
    }
    os << "    </Nodes>\n";
    
    // Members
    os << "    <Members>\n";
    for (const auto& member : truss.getMembers()) {
        os << "      <Member id=\"" << member->getId() << "\">\n";
        os << "        <StartNode>" << member->getStartNode()->getId() << "</StartNode>\n";
        os << "        <EndNode>" << member->getEndNode()->getId() << "</EndNode>\n";
        os << "        <Length>" << formatNumber(member->getLength(), options) << "</Length>\n";
        os << "      </Member>\n";
    }
    os << "    </Members>\n";
    
    os << "  </Geometry>\n";
}

} // namespace truss::infrastructure::export_
