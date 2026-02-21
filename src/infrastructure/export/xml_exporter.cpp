/**
 * @file xml_exporter.cpp
 * @brief XML format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "xml_exporter.hpp"
#include <fstream>
#include <ctime>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::ITrussView;
using core::interfaces::IAnalysisResultsView;
using core::interfaces::NodeView;
using core::interfaces::MemberView;

bool XMLExporter::exportResults(const ITrussView& truss,
                                const IAnalysisResultsView& results,
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
        
        // Project metadata
        file << "  <Project>\n";
        file << "    <Name>" << escapeString(truss.getName()) << "</Name>\n";
        file << "    <ExportTime>" << formatTimestamp() << "</ExportTime>\n";
        file << "    <Version>3.0.0</Version>\n";
        file << "  </Project>\n";
        
        // Geometry section (conditional)
        if (options.includeGeometry) {
            writeGeometrySection(file, truss, options);
        }
        
        // Properties section (conditional)
        if (options.includeProperties) {
            writePropertiesSection(file, truss, options);
        }
        
        // Loads section (conditional)
        if (options.includeLoads) {
            writeLoadsSection(file, truss, options);
        }
        
        // Displacements section (conditional)
        if (options.includeDisplacements && results.getDisplacements().size() > 0) {
            writeDisplacementsSection(file, results, options);
        }
        
        // Member forces section (conditional)
        if (options.includeMemberForces && !results.getMemberForces().empty()) {
            writeMemberForcesSection(file, results, options);
        }
        
        // Reactions section (conditional)
        if (options.includeReactions && !results.getReactions().empty()) {
            writeReactionsSection(file, results, options);
        }
        
        // Metadata section (conditional)
        if (options.includeMetadata) {
            writeMetadataSection(file, results, options);
        }
        
        file << "</TrussAnalysisResults>\n";
        
        file.close();
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
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

void XMLExporter::writeGeometrySection(std::ostream& os, const ITrussView& truss,
                                        const ExportOptions& options) {
    os << "  <Geometry>\n";
    
    // Nodes
    os << "    <Nodes>\n";
    for (const auto& node : truss.getNodeViews()) {
        os << "      <Node id=\"" << node.id << "\">\n";
        os << "        <X>" << formatNumber(node.x, options) << "</X>\n";
        os << "        <Y>" << formatNumber(node.y, options) << "</Y>\n";
        os << "        <SupportType>" << static_cast<int>(node.support) << "</SupportType>\n";
        os << "      </Node>\n";
    }
    os << "    </Nodes>\n";
    
    // Members
    os << "    <Members>\n";
    for (const auto& member : truss.getMemberViews()) {
        os << "      <Member id=\"" << member.id << "\">\n";
        os << "        <StartNode>" << member.startNodeId << "</StartNode>\n";
        os << "        <EndNode>" << member.endNodeId << "</EndNode>\n";
        os << "        <Length>" << formatNumber(member.length, options) << "</Length>\n";
        os << "      </Member>\n";
    }
    os << "    </Members>\n";
    
    os << "  </Geometry>\n";
}

void XMLExporter::writePropertiesSection(std::ostream& os, const ITrussView& truss,
                                        const ExportOptions& options) {
    os << "  <Properties>\n";
    os << "    <Members>\n";
    
    for (const auto& member : truss.getMemberViews()) {
        os << "      <Member id=\"" << member.id << "\">\n";
        os << "        <YoungModulus>" << formatNumber(member.youngModulus, options) << "</YoungModulus>\n";
        os << "        <YieldStrength>" << formatNumber(member.yieldStrength, options) << "</YieldStrength>\n";
        os << "        <Density>" << formatNumber(member.density, options) << "</Density>\n";
        os << "        <Area>" << formatNumber(member.area, options) << "</Area>\n";
        os << "      </Member>\n";
    }
    
    os << "    </Members>\n";
    os << "  </Properties>\n";
}

void XMLExporter::writeLoadsSection(std::ostream& os, const ITrussView& truss,
                                   const ExportOptions& options) {
    os << "  <Loads>\n";
    os << "    <NodalForces>\n";
    
    for (const auto& node : truss.getNodeViews()) {
        // Only export nodes with non-zero forces
        if (node.fx != 0.0 || node.fy != 0.0) {
            os << "      <Force nodeId=\"" << node.id << "\">\n";
            os << "        <Fx>" << formatNumber(node.fx, options) << "</Fx>\n";
            os << "        <Fy>" << formatNumber(node.fy, options) << "</Fy>\n";
            os << "      </Force>\n";
        }
    }
    
    os << "    </NodalForces>\n";
    os << "  </Loads>\n";
}

void XMLExporter::writeDisplacementsSection(std::ostream& os,
                                           const IAnalysisResultsView& results,
                                           const ExportOptions& options) {
    os << "  <Displacements>\n";
    os << "    <Values>\n";
    
    for (size_t i = 0; i < results.getDisplacements().size(); ++i) {
        os << "      <Displacement dof=\"" << i << "\">";
        os << formatNumber(results.getDisplacements()[i], options);
        os << "</Displacement>\n";
    }
    
    os << "    </Values>\n";
    os << "    <MaxDisplacement>" << formatNumber(results.getMaxDisplacement(), options) 
       << "</MaxDisplacement>\n";
    os << "  </Displacements>\n";
}

void XMLExporter::writeMemberForcesSection(std::ostream& os,
                                          const IAnalysisResultsView& results,
                                          const ExportOptions& options) {
    os << "  <MemberForces>\n";
    os << "    <Values>\n";
    
    for (size_t i = 0; i < results.getMemberForces().size(); ++i) {
        Real force = results.getMemberForces()[i];
        std::string type = (force > 0) ? "Tension" : "Compression";
        os << "      <Force memberId=\"" << (i + 1) << "\" type=\"" << type << "\">";
        os << formatNumber(force, options);
        os << "</Force>\n";
    }
    
    os << "    </Values>\n";
    os << "  </MemberForces>\n";
}

void XMLExporter::writeReactionsSection(std::ostream& os,
                                       const IAnalysisResultsView& results,
                                       const ExportOptions& options) {
    os << "  <Reactions>\n";
    os << "    <Values>\n";
    
    for (size_t i = 0; i < results.getReactions().size(); ++i) {
        os << "      <Reaction dof=\"" << i << "\">";
        os << formatNumber(results.getReactions()[i], options);
        os << "</Reaction>\n";
    }
    
    os << "    </Values>\n";
    os << "  </Reactions>\n";
}

void XMLExporter::writeMetadataSection(std::ostream& os,
                                      const IAnalysisResultsView& results,
                                      const ExportOptions& options) {
    os << "  <Analysis>\n";
    os << "    <Converged>" << (results.hasConverged() ? "true" : "false") << "</Converged>\n";
    os << "    <Iterations>" << results.getIterations() << "</Iterations>\n";
    os << "    <TotalDofs>" << results.getTotalDofs() << "</TotalDofs>\n";
    os << "    <FreeDofs>" << results.getFreeDofs() << "</FreeDofs>\n";
    os << "    <MaxStress>" << formatNumber(results.getMaxStress(), options) << "</MaxStress>\n";
    os << "  </Analysis>\n";
}

} // namespace truss::infrastructure::export_
