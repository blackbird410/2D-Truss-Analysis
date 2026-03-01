/**
 * @file text_exporter.cpp
 * @brief Exports analysis results to plain text format.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "text_exporter.hpp"

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

bool TextExporter::exportResults(const ITrussView& truss,
                                 const IAnalysisResultsView& results,
                                 const std::filesystem::path& filePath,
                                 const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }

    try {
        // Document header
        writeHeader(file, truss);

        // ALL 8 sections MUST be represented (placeholder if not implemented)
        if (options.includeGeometry) {
            writeGeometrySection(file, truss, options);
        }

        if (options.includeProperties) {
            writePropertiesSection(file, truss, options);
        }

        if (options.includeLoads) {
            writeLoadsSection(file, truss, options);
        }

        if (options.includeDisplacements && results.getDisplacements().size() > 0) {
            writeDisplacementsSection(file, results, options);
        }

        if (options.includeMemberForces && !results.getMemberForces().empty()) {
            writeMemberForcesSection(file, results, options);
        }

        if (options.includeReactions && !results.getReactions().empty()) {
            writeReactionsSection(file, results, options);
        }

        if (options.includeMetadata) {
            writeMetadataSection(file, results, options);
        }

        // Document footer
        writeSeparator(file);
        file << "End of Report\n";

        file.close();
        return true;

    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        return false;
    }
}

void TextExporter::writeSeparator(std::ostream& os, int width) {
    os << std::string(width, '=') << "\n";
}

void TextExporter::writeSectionHeader(std::ostream& os, const std::string& title) {
    os << "\n";
    writeSeparator(os);
    os << title << "\n";
    writeSeparator(os);
}

void TextExporter::writeHeader(std::ostream& os, const ITrussView& truss) {
    writeSeparator(os);
    os << "2D TRUSS ANALYSIS RESULTS\n";
    os << truss.getName() << "\n";
    writeSeparator(os);
    os << "Generated: " << truss::utils::string::formatTimestamp() << "\n";
    os << "Version: 3.0.0\n";
    writeSeparator(os);
    os << "\n";

    // Project metadata
    writeSectionHeader(os, "PROJECT METADATA");
    os << "  Project Name:    " << truss.getName() << "\n";
    os << "  Generated:       " << truss::utils::string::formatTimestamp() << "\n";
    os << "  Software:        2D Truss Analysis v3.0.0\n";
    os << "  Number of Nodes: " << truss.getNodeCount() << "\n";
    os << "  Number of Members: " << truss.getMemberCount() << "\n";
}

void TextExporter::writeGeometrySection(std::ostream& os,
                                        const ITrussView& truss,
                                        const ExportOptions& options) {
    writeSectionHeader(os, "GEOMETRY");

    // Nodes subsection
    os << "\nNodes:\n";
    os << "  " << std::left << std::setw(8) << "Node ID" << std::setw(15) << "X (m)"
       << std::setw(15) << "Y (m)" << std::setw(15) << "Support Type" << "\n";
    os << "  " << std::string(53, '-') << "\n";

    for (const auto& node : truss.getNodeViews()) {
        std::string supportType;
        switch (node.support) {
            case core::SupportType::Free:
                supportType = "Free";
                break;
            case core::SupportType::Pinned:
                supportType = "Pinned";
                break;
            case core::SupportType::RollerX:
                supportType = "Roller-X";
                break;
            case core::SupportType::RollerY:
                supportType = "Roller-Y";
                break;
            default:
                supportType = "Unknown";
                break;
        }

        os << "  " << std::left << std::setw(8) << node.id << std::setw(15)
           << truss::utils::string::formatReal(node.x, options.precision, options.useScientificNotation) << std::setw(15) << truss::utils::string::formatReal(node.y, options.precision, options.useScientificNotation)
           << std::setw(15) << supportType << "\n";
    }

    // Members subsection
    os << "\nMembers:\n";
    os << "  " << std::left << std::setw(10) << "Member ID" << std::setw(12) << "Start Node"
       << std::setw(12) << "End Node" << std::setw(15) << "Length (m)" << "\n";
    os << "  " << std::string(49, '-') << "\n";

    for (const auto& member : truss.getMemberViews()) {
        os << "  " << std::left << std::setw(10) << member.id << std::setw(12) << member.startNodeId
           << std::setw(12) << member.endNodeId << std::setw(15)
           << truss::utils::string::formatReal(member.length, options.precision, options.useScientificNotation) << "\n";
    }
}

void TextExporter::writePropertiesSection(std::ostream& os,
                                          const ITrussView& truss,
                                          const ExportOptions& options) {
    writeSectionHeader(os, "MATERIAL AND SECTION PROPERTIES");

    os << "\n  " << std::left << std::setw(10) << "Member ID" << std::setw(15) << "Material"
       << std::setw(15) << "E (Pa)" << std::setw(18) << "Yield Strength" << std::setw(15)
       << "Density" << std::setw(15) << "Area (m²)" << "\n";
    os << "  " << std::string(88, '-') << "\n";

    for (const auto& member : truss.getMemberViews()) {
        os << "  " << std::left << std::setw(10) << member.id << std::setw(15)
           << "Steel"  // TODO: Add material type to MemberView struct
           << std::setw(15) << truss::utils::string::formatReal(member.youngModulus, options.precision, options.useScientificNotation) << std::setw(18)
           << truss::utils::string::formatReal(member.yieldStrength, options.precision, options.useScientificNotation) << std::setw(15)
           << truss::utils::string::formatReal(member.density, options.precision, options.useScientificNotation) << std::setw(15)
           << truss::utils::string::formatReal(member.area, options.precision, options.useScientificNotation) << "\n";
    }
}

void TextExporter::writeLoadsSection(std::ostream& os,
                                     const ITrussView& truss,
                                     const ExportOptions& options) {
    writeSectionHeader(os, "APPLIED LOADS");

    os << "\n  " << std::left << std::setw(10) << "Node ID" << std::setw(20) << "Fx (N)"
       << std::setw(20) << "Fy (N)" << "\n";
    os << "  " << std::string(50, '-') << "\n";

    bool hasLoads = false;
    for (const auto& node : truss.getNodeViews()) {
        // Only export nodes with non-zero forces
        if (node.fx != 0.0 || node.fy != 0.0) {
            os << "  " << std::left << std::setw(10) << node.id << std::setw(20)
               << truss::utils::string::formatReal(node.fx, options.precision, options.useScientificNotation) << std::setw(20) << truss::utils::string::formatReal(node.fy, options.precision, options.useScientificNotation)
               << "\n";
            hasLoads = true;
        }
    }

    if (!hasLoads) {
        os << "  (No applied loads)\n";
    }
}

void TextExporter::writeDisplacementsSection(std::ostream& os,
                                             const IAnalysisResultsView& results,
                                             const ExportOptions& options) {
    writeSectionHeader(os, "NODAL DISPLACEMENTS");

    os << "  " << std::left << std::setw(10) << "DOF" << std::setw(20) << "Displacement (m)"
       << "\n";
    os << "  " << std::string(30, '-') << "\n";

    for (size_t i = 0; i < results.getDisplacements().size(); ++i) {
        os << "  " << std::left << std::setw(10) << i << std::setw(20)
           << truss::utils::string::formatReal(results.getDisplacements()[i], options.precision, options.useScientificNotation) << "\n";
    }

    os << "\n  Maximum Displacement: " << truss::utils::string::formatReal(results.getMaxDisplacement(), options.precision, options.useScientificNotation)
       << " m\n";
}

void TextExporter::writeMemberForcesSection(std::ostream& os,
                                            const IAnalysisResultsView& results,
                                            const ExportOptions& options) {
    writeSectionHeader(os, "MEMBER FORCES");

    os << "  " << std::left << std::setw(12) << "Member ID" << std::setw(20) << "Axial Force (N)"
       << std::setw(15) << "Type" << "\n";
    os << "  " << std::string(47, '-') << "\n";

    for (size_t i = 0; i < results.getMemberForces().size(); ++i) {
        Real force = results.getMemberForces()[i];
        std::string type = (force > 0) ? "Tension" : "Compression";

        os << "  " << std::left << std::setw(12) << (i + 1) << std::setw(20)
           << truss::utils::string::formatReal(force, options.precision, options.useScientificNotation) << std::setw(15) << type << "\n";
    }
}

void TextExporter::writeReactionsSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options) {
    writeSectionHeader(os, "SUPPORT REACTIONS");

    os << "  " << std::left << std::setw(10) << "DOF" << std::setw(25) << "Reaction Force (N)"
       << "\n";
    os << "  " << std::string(35, '-') << "\n";

    for (size_t i = 0; i < results.getReactions().size(); ++i) {
        os << "  " << std::left << std::setw(10) << i << std::setw(25)
           << truss::utils::string::formatReal(results.getReactions()[i], options.precision, options.useScientificNotation) << "\n";
    }
}

void TextExporter::writeMetadataSection(std::ostream& os,
                                        const IAnalysisResultsView& results,
                                        const ExportOptions& options) {
    writeSectionHeader(os, "ANALYSIS METADATA");

    os << "  Converged:          " << (results.hasConverged() ? "Yes" : "No") << "\n";
    os << "  Iterations:         " << results.getIterations() << "\n";
    os << "  Total DOFs:         " << results.getTotalDofs() << "\n";
    os << "  Free DOFs:          " << results.getFreeDofs() << "\n";
    os << "  Max Displacement:   " << truss::utils::string::formatReal(results.getMaxDisplacement(), options.precision, options.useScientificNotation) << " m\n";
    os << "  Max Stress:         " << truss::utils::string::formatReal(results.getMaxStress(), options.precision, options.useScientificNotation) << " Pa\n";
}

}  // namespace truss::infrastructure::export_
