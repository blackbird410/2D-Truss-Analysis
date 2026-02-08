/**
 * @file json_exporter.cpp
 * @brief JSON format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "json_exporter.hpp"
#include "src/core/Logger.hpp"
#include <fstream>
#include <ctime>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::Truss;
using core::analysis::AnalysisResults;

bool JSONExporter::exportResults(const Truss& truss,
                                 const AnalysisResults& results,
                                 const std::filesystem::path& filePath,
                                 const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }
    
    try {
        file << "{\n";
        
        // Project metadata (always included)
        writeProjectSection(file, truss, options);
        
        // Track if we need commas between sections
        bool needsComma = true;
        
        // Conditional sections
        if (options.includeGeometry) {
            writeGeometrySection(file, truss, options, needsComma);
        }
        
        if (options.includeProperties) {
            writePropertiesSection(file, truss, options, needsComma);
        }
        
        if (options.includeLoads) {
            writeLoadsSection(file, truss, options, needsComma);
        }
        
        if (options.includeDisplacements && results.displacements.size() > 0) {
            writeDisplacementsSection(file, results, options, needsComma);
        }
        
        if (options.includeMemberForces && !results.memberForces.empty()) {
            writeMemberForcesSection(file, results, options, needsComma);
        }
        
        if (options.includeReactions && !results.reactions.empty()) {
            writeReactionsSection(file, results, options, needsComma);
        }
        
        if (options.includeMetadata) {
            writeMetadataSection(file, results, options, needsComma);
        }
        
        file << "}\n";
        
        file.close();
        core::Logger::info("Results exported to JSON: " + filePath.string());
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        core::Logger::error("JSON export failed: " + m_lastError);
        return false;
    }
}

std::string JSONExporter::formatNumber(Real value, const ExportOptions& options) const {
    std::stringstream ss;
    if (options.useScientificNotation) {
        ss << std::scientific;
    } else {
        ss << std::fixed;
    }
    ss << std::setprecision(options.precision) << value;
    return ss.str();
}

std::string JSONExporter::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string JSONExporter::escapeString(const std::string& str) const {
    std::string result;
    result.reserve(str.length());
    
    for (char c : str) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b";  break;
            case '\f': result += "\\f";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Control characters
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
        }
    }
    
    return result;
}

void JSONExporter::writeProjectSection(std::ostream& os, const Truss& truss,
                                       const ExportOptions& options) {
    os << "  \"project\": {\n";
    os << "    \"name\": \"" << escapeString(truss.getName()) << "\",\n";
    os << "    \"exportTime\": \"" << formatTimestamp() << "\",\n";
    os << "    \"version\": \"2.2.0\"\n";
    os << "  }";
}

void JSONExporter::writeGeometrySection(std::ostream& os, const Truss& truss,
                                       const ExportOptions& options, bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"geometry\": {\n";
    os << "    \"nodes\": [\n";
    
    const auto& nodes = truss.getNodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        os << "      {\n";
        os << "        \"id\": " << node->getId() << ",\n";
        os << "        \"x\": " << formatNumber(node->getX(), options) << ",\n";
        os << "        \"y\": " << formatNumber(node->getY(), options) << ",\n";
        os << "        \"supportType\": \"" << static_cast<int>(node->getSupportType()) << "\"\n";
        os << "      }" << (i < nodes.size() - 1 ? "," : "") << "\n";
    }
    
    os << "    ],\n";
    os << "    \"members\": [\n";
    
    const auto& members = truss.getMembers();
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        os << "      {\n";
        os << "        \"id\": " << member->getId() << ",\n";
        os << "        \"startNode\": " << member->getStartNode()->getId() << ",\n";
        os << "        \"endNode\": " << member->getEndNode()->getId() << ",\n";
        os << "        \"length\": " << formatNumber(member->getLength(), options) << "\n";
        os << "      }" << (i < members.size() - 1 ? "," : "") << "\n";
    }
    
    os << "    ]\n";
    os << "  }";
    
    needsComma = true;
}

void JSONExporter::writePropertiesSection(std::ostream& os, const Truss& truss,
                                         const ExportOptions& options, bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"properties\": {\n";
    os << "    \"members\": [\n";
    
    const auto& members = truss.getMembers();
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        const auto& material = member->getMaterial();
        const auto& section = member->getSection();
        
        os << "      {\n";
        os << "        \"memberId\": " << member->getId() << ",\n";
        os << "        \"material\": \"" << escapeString(material.name) << "\",\n";
        os << "        \"youngModulus\": " << formatNumber(material.youngModulus, options) << ",\n";
        os << "        \"density\": " << formatNumber(material.density, options) << ",\n";
        os << "        \"area\": " << formatNumber(section.area, options) << ",\n";
        os << "        \"section\": \"" << escapeString(section.designation) << "\"\n";
        os << "      }" << (i < members.size() - 1 ? "," : "") << "\n";
    }
    
    os << "    ]\n";
    os << "  }";
    
    needsComma = true;
}

void JSONExporter::writeLoadsSection(std::ostream& os, const Truss& truss,
                                    const ExportOptions& options, bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"loads\": {\n";
    os << "    \"nodalForces\": [\n";
    
    // Collect nodes with non-zero forces
    std::vector<std::shared_ptr<const core::Node>> loadedNodes;
    for (const auto& node : truss.getNodes()) {
        const auto& force = node->getAppliedForce();
        if (force.fx != 0.0 || force.fy != 0.0) {
            loadedNodes.push_back(node);
        }
    }
    
    for (size_t i = 0; i < loadedNodes.size(); ++i) {
        const auto& node = loadedNodes[i];
        const auto& force = node->getAppliedForce();
        
        os << "      {\n";
        os << "        \"nodeId\": " << node->getId() << ",\n";
        os << "        \"fx\": " << formatNumber(force.fx, options) << ",\n";
        os << "        \"fy\": " << formatNumber(force.fy, options) << "\n";
        os << "      }" << (i < loadedNodes.size() - 1 ? "," : "") << "\n";
    }
    
    os << "    ]\n";
    os << "  }";
    
    needsComma = true;
}

void JSONExporter::writeDisplacementsSection(std::ostream& os,
                                            const AnalysisResults& results,
                                            const ExportOptions& options,
                                            bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"displacements\": {\n";
    os << "    \"values\": [";
    
    for (size_t i = 0; i < results.displacements.size(); ++i) {
        os << formatNumber(results.displacements[i], options);
        if (i < results.displacements.size() - 1) {
            os << ", ";
        }
    }
    
    os << "],\n";
    os << "    \"maxDisplacement\": " << formatNumber(results.maxDisplacement, options) << "\n";
    os << "  }";
    
    needsComma = true;
}

void JSONExporter::writeMemberForcesSection(std::ostream& os,
                                           const AnalysisResults& results,
                                           const ExportOptions& options,
                                           bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"memberForces\": {\n";
    os << "    \"values\": [";
    
    for (size_t i = 0; i < results.memberForces.size(); ++i) {
        os << formatNumber(results.memberForces[i], options);
        if (i < results.memberForces.size() - 1) {
            os << ", ";
        }
    }
    
    os << "]\n";
    os << "  }";
    
    needsComma = true;
}

void JSONExporter::writeReactionsSection(std::ostream& os,
                                        const AnalysisResults& results,
                                        const ExportOptions& options,
                                        bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"reactions\": {\n";
    os << "    \"values\": [";
    
    for (size_t i = 0; i < results.reactions.size(); ++i) {
        os << formatNumber(results.reactions[i], options);
        if (i < results.reactions.size() - 1) {
            os << ", ";
        }
    }
    
    os << "]\n";
    os << "  }";
    
    needsComma = true;
}

void JSONExporter::writeMetadataSection(std::ostream& os,
                                       const AnalysisResults& results,
                                       const ExportOptions& options,
                                       bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }
    
    os << "  \"analysis\": {\n";
    os << "    \"converged\": " << (results.converged ? "true" : "false") << ",\n";
    os << "    \"iterations\": " << results.iterations << ",\n";
    os << "    \"totalDofs\": " << results.totalDofs << ",\n";
    os << "    \"freeDofs\": " << results.freeDofs << ",\n";
    os << "    \"maxStress\": " << formatNumber(results.maxStress, options) << "\n";
    os << "  }\n";
    
    needsComma = true;
}

} // namespace truss::infrastructure::export_
