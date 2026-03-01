/**
 * @file xml_exporter.cpp
 * @brief Exports analysis results to XML format.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "xml_exporter.hpp"

#include <ctime>
#include <fstream>
#include "utilities/string_utils.hpp"

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::IAnalysisResultsView;
using core::interfaces::ITrussView;
using core::interfaces::MemberView;
using core::interfaces::NodeView;

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
        file << "    <Name>" << truss::utils::string::escapeXml(truss.getName()) << "</Name>\n";
        file << "    <ExportTime>" << truss::utils::string::formatTimestamp() << "</ExportTime>\n";
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

void XMLExporter::writeGeometrySection(std::ostream& os,
                                       const ITrussView& truss,
                                       const ExportOptions& options) {
    os << "  <Geometry>\n";

    // Nodes
    os << "    <Nodes>\n";
    for (const auto& node : truss.getNodeViews()) {
        os << "      <Node id=\"" << node.id << "\">\n";
        os << "        <X>" << truss::utils::string::formatReal(node.x, options.precision, options.useScientificNotation) << "</X>\n";
        os << "        <Y>" << truss::utils::string::formatReal(node.y, options.precision, options.useScientificNotation) << "</Y>\n";
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
        os << "        <Length>" << truss::utils::string::formatReal(member.length, options.precision, options.useScientificNotation) << "</Length>\n";
        os << "      </Member>\n";
    }
    os << "    </Members>\n";

    os << "  </Geometry>\n";
}

void XMLExporter::writePropertiesSection(std::ostream& os,
                                         const ITrussView& truss,
                                         const ExportOptions& options) {
    os << "  <Properties>\n";
    os << "    <Members>\n";

    for (const auto& member : truss.getMemberViews()) {
        os << "      <Member id=\"" << member.id << "\">\n";
        os << "        <YoungModulus>" << truss::utils::string::formatReal(member.youngModulus, options.precision, options.useScientificNotation)
           << "</YoungModulus>\n";
        os << "        <YieldStrength>" << truss::utils::string::formatReal(member.yieldStrength, options.precision, options.useScientificNotation)
           << "</YieldStrength>\n";
        os << "        <Density>" << truss::utils::string::formatReal(member.density, options.precision, options.useScientificNotation) << "</Density>\n";
        os << "        <Area>" << truss::utils::string::formatReal(member.area, options.precision, options.useScientificNotation) << "</Area>\n";
        os << "      </Member>\n";
    }

    os << "    </Members>\n";
    os << "  </Properties>\n";
}

void XMLExporter::writeLoadsSection(std::ostream& os,
                                    const ITrussView& truss,
                                    const ExportOptions& options) {
    os << "  <Loads>\n";
    os << "    <NodalForces>\n";

    for (const auto& node : truss.getNodeViews()) {
        // Only export nodes with non-zero forces
        if (node.fx != 0.0 || node.fy != 0.0) {
            os << "      <Force nodeId=\"" << node.id << "\">\n";
            os << "        <Fx>" << truss::utils::string::formatReal(node.fx, options.precision, options.useScientificNotation) << "</Fx>\n";
            os << "        <Fy>" << truss::utils::string::formatReal(node.fy, options.precision, options.useScientificNotation) << "</Fy>\n";
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
        os << truss::utils::string::formatReal(results.getDisplacements()[i], options.precision, options.useScientificNotation);
        os << "</Displacement>\n";
    }

    os << "    </Values>\n";
    os << "    <MaxDisplacement>" << truss::utils::string::formatReal(results.getMaxDisplacement(), options.precision, options.useScientificNotation)
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
        os << truss::utils::string::formatReal(force, options.precision, options.useScientificNotation);
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
        os << truss::utils::string::formatReal(results.getReactions()[i], options.precision, options.useScientificNotation);
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
    os << "    <MaxStress>" << truss::utils::string::formatReal(results.getMaxStress(), options.precision, options.useScientificNotation) << "</MaxStress>\n";
    os << "  </Analysis>\n";
}

}  // namespace truss::infrastructure::export_
