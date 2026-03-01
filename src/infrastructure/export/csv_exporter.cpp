/**
 * @file csv_exporter.cpp
 * @brief Exports analysis results to CSV format.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "csv_exporter.hpp"

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

bool CSVExporter::exportResults(const ITrussView& truss,
                                const IAnalysisResultsView& results,
                                const std::filesystem::path& filePath,
                                const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }

    try {
        // Write header
        file << "# 2D Truss Analysis Results Export" << std::endl;
        file << "# Generated: " << truss::utils::string::formatTimestamp() << std::endl;
        file << "# Project: " << truss.getName() << std::endl;
        file << "#" << std::endl;

        // Write sections based on options
        if (options.includeGeometry) {
            file << std::endl << "# GEOMETRY" << std::endl;
            writeGeometrySection(file, truss, options);
        }

        if (options.includeProperties) {
            file << std::endl << "# MATERIAL AND SECTION PROPERTIES" << std::endl;
            writePropertiesSection(file, truss, options);
        }

        if (options.includeLoads) {
            file << std::endl << "# APPLIED LOADS" << std::endl;
            writeLoadsSection(file, truss, options);
        }

        if (options.includeDisplacements) {
            file << std::endl << "# NODAL DISPLACEMENTS" << std::endl;
            writeDisplacementsSection(file, truss, results, options);
        }

        if (options.includeMemberForces) {
            file << std::endl << "# MEMBER FORCES" << std::endl;
            writeMemberForcesSection(file, truss, results, options);
        }

        if (options.includeReactions) {
            file << std::endl << "# SUPPORT REACTIONS" << std::endl;
            writeReactionsSection(file, truss, results, options);
        }

        if (options.includeMetadata) {
            file << std::endl << "# ANALYSIS METADATA" << std::endl;
            writeMetadataSection(file, results, options);
        }

        file.close();
        return true;

    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        return false;
    }
}

void CSVExporter::writeGeometrySection(std::ostream& os,
                                       const ITrussView& truss,
                                       const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    // Nodes
    os << "Node ID" << delim << "X" << delim << "Y" << delim << "Support Type" << std::endl;
    auto nodes = truss.getNodeViews();
    for (const auto& node : nodes) {
        os << node.id << delim << truss::utils::string::formatReal(node.x, options.precision, options.useScientificNotation) << delim
           << truss::utils::string::formatReal(node.y, options.precision, options.useScientificNotation) << delim << static_cast<int>(node.support) << std::endl;
    }

    os << std::endl;

    // Members
    os << "Member ID" << delim << "Start Node" << delim << "End Node" << delim << "Length"
       << std::endl;
    auto members = truss.getMemberViews();
    for (const auto& member : members) {
        os << member.id << delim << member.startNodeId << delim << member.endNodeId << delim
           << truss::utils::string::formatReal(member.length, options.precision, options.useScientificNotation) << std::endl;
    }
}

void CSVExporter::writePropertiesSection(std::ostream& os,
                                         const ITrussView& truss,
                                         const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    os << "Member ID" << delim << "E (Pa)" << delim << "Density (kg/m³)" << delim << "Area (m²)"
       << delim << "Yield Strength (Pa)" << std::endl;

    auto members = truss.getMemberViews();
    for (const auto& member : members) {
        os << member.id << delim << truss::utils::string::formatReal(member.youngModulus, options.precision, options.useScientificNotation) << delim
           << truss::utils::string::formatReal(member.density, options.precision, options.useScientificNotation) << delim << truss::utils::string::formatReal(member.area, options.precision, options.useScientificNotation)
           << delim << truss::utils::string::formatReal(member.yieldStrength, options.precision, options.useScientificNotation) << std::endl;
    }
}

void CSVExporter::writeLoadsSection(std::ostream& os,
                                    const ITrussView& truss,
                                    const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    os << "Node ID" << delim << "Fx (N)" << delim << "Fy (N)" << std::endl;

    auto nodes = truss.getNodeViews();
    for (const auto& node : nodes) {
        // Only export nodes with non-zero forces
        if (node.fx != 0.0 || node.fy != 0.0) {
            os << node.id << delim << truss::utils::string::formatReal(node.fx, options.precision, options.useScientificNotation) << delim
               << truss::utils::string::formatReal(node.fy, options.precision, options.useScientificNotation) << std::endl;
        }
    }
}

void CSVExporter::writeDisplacementsSection(std::ostream& os,
                                            const ITrussView& /*truss*/,
                                            const IAnalysisResultsView& results,
                                            const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    os << "DOF" << delim << "Displacement" << std::endl;
    const auto& displacements = results.getDisplacements();
    for (size_t i = 0; i < displacements.size(); ++i) {
        os << i << delim << truss::utils::string::formatReal(displacements[i], options.precision, options.useScientificNotation) << std::endl;
    }
}

void CSVExporter::writeMemberForcesSection(std::ostream& os,
                                           const ITrussView& truss,
                                           const IAnalysisResultsView& results,
                                           const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    os << "Member ID" << delim << "Axial Force" << delim << "Type" << std::endl;
    const auto& memberForces = results.getMemberForces();
    for (size_t i = 0; i < memberForces.size() && i < truss.getMemberCount(); ++i) {
        Real force = memberForces[i];
        std::string type = (force > 0) ? "Tension" : "Compression";
        os << (i + 1) << delim << truss::utils::string::formatReal(force, options.precision, options.useScientificNotation) << delim << type << std::endl;
    }
}

void CSVExporter::writeReactionsSection(std::ostream& os,
                                        const ITrussView& /*truss*/,
                                        const IAnalysisResultsView& results,
                                        const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    os << "DOF" << delim << "Reaction Force" << std::endl;
    const auto& reactions = results.getReactions();
    for (size_t i = 0; i < reactions.size(); ++i) {
        os << i << delim << truss::utils::string::formatReal(reactions[i], options.precision, options.useScientificNotation) << std::endl;
    }
}

void CSVExporter::writeMetadataSection(std::ostream& os,
                                       const IAnalysisResultsView& results,
                                       const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    os << "Property" << delim << "Value" << std::endl;
    os << "Converged" << delim << (results.hasConverged() ? "Yes" : "No") << std::endl;
    os << "Iterations" << delim << results.getIterations() << std::endl;
    os << "Total DOFs" << delim << results.getTotalDofs() << std::endl;
    os << "Free DOFs" << delim << results.getFreeDofs() << std::endl;
    os << "Max Displacement" << delim << truss::utils::string::formatReal(results.getMaxDisplacement(), options.precision, options.useScientificNotation)
       << std::endl;
    os << "Max Stress" << delim << truss::utils::string::formatReal(results.getMaxStress(), options.precision, options.useScientificNotation) << std::endl;
}

}  // namespace truss::infrastructure::export_
