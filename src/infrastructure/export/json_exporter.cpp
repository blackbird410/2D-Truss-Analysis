/**
 * @file json_exporter.cpp
 * @brief JSON format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "json_exporter.hpp"

#include <ctime>
#include <fstream>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::IAnalysisResultsView;
using core::interfaces::ITrussView;
using core::interfaces::MemberView;
using core::interfaces::NodeView;

bool JSONExporter::exportResults(const ITrussView& truss,
                                 const IAnalysisResultsView& results,
                                 const std::filesystem::path& filePath,
                                 const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }

    try {
        file << "{\n";

        // Project metadata
        file << "  \"project\": {\n";
        file << "    \"name\": \"" << escapeString(truss.getName()) << "\",\n";
        file << "    \"exportTime\": \"" << formatTimestamp() << "\",\n";
        file << "    \"version\": \"3.0.0\"\n";
        file << "  }";

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

        if (options.includeDisplacements && results.getDisplacements().size() > 0) {
            writeDisplacementsSection(file, results, options, needsComma);
        }

        if (options.includeMemberForces && !results.getMemberForces().empty()) {
            writeMemberForcesSection(file, results, options, needsComma);
        }

        if (options.includeReactions && !results.getReactions().empty()) {
            writeReactionsSection(file, results, options, needsComma);
        }

        if (options.includeMetadata) {
            writeMetadataSection(file, results, options, needsComma);
        }

        file << "}\n";

        file.close();
        return true;

    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
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
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Control characters
                    std::ostringstream oss;
                    oss << "\\u" << std::setw(4) << std::setfill('0') << std::hex
                        << std::nouppercase << static_cast<int>(static_cast<unsigned char>(c));
                    result += oss.str();
                } else {
                    result += c;
                }
        }
    }

    return result;
}

void JSONExporter::writeGeometrySection(std::ostream& os,
                                        const ITrussView& truss,
                                        const ExportOptions& options,
                                        bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"geometry\": {\n";
    os << "    \"nodes\": [\n";

    const auto& nodes = truss.getNodeViews();
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        os << "      {\n";
        os << "        \"id\": " << node.id << ",\n";
        os << "        \"x\": " << formatNumber(node.x, options) << ",\n";
        os << "        \"y\": " << formatNumber(node.y, options) << ",\n";
        os << "        \"supportType\": \"" << static_cast<int>(node.support) << "\"\n";
        os << "      }" << (i < nodes.size() - 1 ? "," : "") << "\n";
    }

    os << "    ],\n";
    os << "    \"members\": [\n";

    const auto& members = truss.getMemberViews();
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        os << "      {\n";
        os << "        \"id\": " << member.id << ",\n";
        os << "        \"startNode\": " << member.startNodeId << ",\n";
        os << "        \"endNode\": " << member.endNodeId << ",\n";
        os << "        \"length\": " << formatNumber(member.length, options) << "\n";
        os << "      }" << (i < members.size() - 1 ? "," : "") << "\n";
    }

    os << "    ]\n";
    os << "  }";

    needsComma = true;
}

void JSONExporter::writePropertiesSection(std::ostream& os,
                                          const ITrussView& truss,
                                          const ExportOptions& options,
                                          bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"properties\": {\n";
    os << "    \"members\": [\n";

    const auto& members = truss.getMemberViews();
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];

        os << "      {\n";
        os << "        \"memberId\": " << member.id << ",\n";
        os << "        \"youngModulus\": " << formatNumber(member.youngModulus, options) << ",\n";
        os << "        \"yieldStrength\": " << formatNumber(member.yieldStrength, options) << ",\n";
        os << "        \"density\": " << formatNumber(member.density, options) << ",\n";
        os << "        \"area\": " << formatNumber(member.area, options) << "\n";
        os << "      }" << (i < members.size() - 1 ? "," : "") << "\n";
    }

    os << "    ]\n";
    os << "  }";

    needsComma = true;
}

void JSONExporter::writeLoadsSection(std::ostream& os,
                                     const ITrussView& truss,
                                     const ExportOptions& options,
                                     bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"loads\": {\n";
    os << "    \"nodalForces\": [\n";

    // Collect nodes with non-zero forces
    std::vector<NodeView> loadedNodes;
    for (const auto& node : truss.getNodeViews()) {
        if (node.fx != 0.0 || node.fy != 0.0) {
            loadedNodes.push_back(node);
        }
    }

    for (size_t i = 0; i < loadedNodes.size(); ++i) {
        const auto& node = loadedNodes[i];

        os << "      {\n";
        os << "        \"nodeId\": " << node.id << ",\n";
        os << "        \"fx\": " << formatNumber(node.fx, options) << ",\n";
        os << "        \"fy\": " << formatNumber(node.fy, options) << "\n";
        os << "      }" << (i < loadedNodes.size() - 1 ? "," : "") << "\n";
    }

    os << "    ]\n";
    os << "  }";

    needsComma = true;
}

void JSONExporter::writeDisplacementsSection(std::ostream& os,
                                             const IAnalysisResultsView& results,
                                             const ExportOptions& options,
                                             bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"displacements\": {\n";
    os << "    \"values\": [";

    for (size_t i = 0; i < results.getDisplacements().size(); ++i) {
        os << formatNumber(results.getDisplacements()[i], options);
        if (i < results.getDisplacements().size() - 1) {
            os << ", ";
        }
    }

    os << "],\n";
    os << "    \"maxDisplacement\": " << formatNumber(results.getMaxDisplacement(), options)
       << "\n";
    os << "  }";

    needsComma = true;
}

void JSONExporter::writeMemberForcesSection(std::ostream& os,
                                            const IAnalysisResultsView& results,
                                            const ExportOptions& options,
                                            bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"memberForces\": {\n";
    os << "    \"values\": [";

    for (size_t i = 0; i < results.getMemberForces().size(); ++i) {
        os << formatNumber(results.getMemberForces()[i], options);
        if (i < results.getMemberForces().size() - 1) {
            os << ", ";
        }
    }

    os << "]\n";
    os << "  }";

    needsComma = true;
}

void JSONExporter::writeReactionsSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options,
                                         bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"reactions\": {\n";
    os << "    \"values\": [";

    for (size_t i = 0; i < results.getReactions().size(); ++i) {
        os << formatNumber(results.getReactions()[i], options);
        if (i < results.getReactions().size() - 1) {
            os << ", ";
        }
    }

    os << "]\n";
    os << "  }";

    needsComma = true;
}

void JSONExporter::writeMetadataSection(std::ostream& os,
                                        const IAnalysisResultsView& results,
                                        const ExportOptions& options,
                                        bool& needsComma) {
    if (needsComma) {
        os << ",\n";
    }

    os << "  \"analysis\": {\n";
    os << "    \"converged\": " << (results.hasConverged() ? "true" : "false") << ",\n";
    os << "    \"iterations\": " << results.getIterations() << ",\n";
    os << "    \"totalDofs\": " << results.getTotalDofs() << ",\n";
    os << "    \"freeDofs\": " << results.getFreeDofs() << ",\n";
    os << "    \"maxStress\": " << formatNumber(results.getMaxStress(), options) << "\n";
    os << "  }\n";

    needsComma = true;
}

}  // namespace truss::infrastructure::export_
