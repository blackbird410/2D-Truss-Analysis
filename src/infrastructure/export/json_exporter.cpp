/**
 * @file json_exporter.cpp
 * @brief Exports analysis results to JSON format.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "json_exporter.hpp"

#include "utilities/string_utils.hpp"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iterator>

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
        file << "    \"name\": \"" << truss::utils::string::escapeJson(truss.getName()) << "\",\n";
        file << "    \"exportTime\": \"" << truss::utils::string::formatTimestamp() << "\",\n";
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
        os << "        \"x\": "
           << truss::utils::string::formatReal(
                  node.x, options.precision, options.useScientificNotation)
           << ",\n";
        os << "        \"y\": "
           << truss::utils::string::formatReal(
                  node.y, options.precision, options.useScientificNotation)
           << ",\n";
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
        os << "        \"length\": "
           << truss::utils::string::formatReal(
                  member.length, options.precision, options.useScientificNotation)
           << "\n";
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
        os << "        \"youngModulus\": "
           << truss::utils::string::formatReal(
                  member.youngModulus, options.precision, options.useScientificNotation)
           << ",\n";
        os << "        \"yieldStrength\": "
           << truss::utils::string::formatReal(
                  member.yieldStrength, options.precision, options.useScientificNotation)
           << ",\n";
        os << "        \"density\": "
           << truss::utils::string::formatReal(
                  member.density, options.precision, options.useScientificNotation)
           << ",\n";
        os << "        \"area\": "
           << truss::utils::string::formatReal(
                  member.area, options.precision, options.useScientificNotation)
           << "\n";
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
    const auto& nodes = truss.getNodeViews();
    loadedNodes.reserve(nodes.size());
    std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(loadedNodes), [](const auto& node) {
        return node.fx != 0.0 || node.fy != 0.0;
    });

    for (size_t i = 0; i < loadedNodes.size(); ++i) {
        const auto& node = loadedNodes[i];

        os << "      {\n";
        os << "        \"nodeId\": " << node.id << ",\n";
        os << "        \"fx\": "
           << truss::utils::string::formatReal(
                  node.fx, options.precision, options.useScientificNotation)
           << ",\n";
        os << "        \"fy\": "
           << truss::utils::string::formatReal(
                  node.fy, options.precision, options.useScientificNotation)
           << "\n";
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
        os << truss::utils::string::formatReal(
            results.getDisplacements()[i], options.precision, options.useScientificNotation);
        if (i < results.getDisplacements().size() - 1) {
            os << ", ";
        }
    }

    os << "],\n";
    os << "    \"maxDisplacement\": "
       << truss::utils::string::formatReal(
              results.getMaxDisplacement(), options.precision, options.useScientificNotation)
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
        os << truss::utils::string::formatReal(
            results.getMemberForces()[i], options.precision, options.useScientificNotation);
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
        os << truss::utils::string::formatReal(
            results.getReactions()[i], options.precision, options.useScientificNotation);
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
    os << "    \"maxStress\": "
       << truss::utils::string::formatReal(
              results.getMaxStress(), options.precision, options.useScientificNotation)
       << "\n";
    os << "  }\n";

    needsComma = true;
}

}  // namespace truss::infrastructure::export_
