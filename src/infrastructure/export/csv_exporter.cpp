/**
 * @file csv_exporter.cpp
 * @brief Implements the CSV exporter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "csv_exporter.hpp"

#include <ctime>
#include <fstream>

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
        file << "# Generated: " << formatTimestamp() << std::endl;
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

std::string CSVExporter::formatNumber(Real value, const ExportOptions& options) {
    std::stringstream ss;
    if (options.useScientificNotation) {
        ss << std::scientific;
    } else {
        ss << std::fixed;
    }
    ss << std::setprecision(options.precision) << value;
    return ss.str();
}

std::string CSVExporter::formatTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void CSVExporter::writeGeometrySection(std::ostream& os,
                                       const ITrussView& truss,
                                       const ExportOptions& options) {
    const std::string& delim = options.delimiter;

    // Nodes
    os << "Node ID" << delim << "X" << delim << "Y" << delim << "Support Type" << std::endl;
    auto nodes = truss.getNodeViews();
    for (const auto& node : nodes) {
        os << node.id << delim << formatNumber(node.x, options) << delim
           << formatNumber(node.y, options) << delim << static_cast<int>(node.support) << std::endl;
    }

    os << std::endl;

    // Members
    os << "Member ID" << delim << "Start Node" << delim << "End Node" << delim << "Length"
       << std::endl;
    auto members = truss.getMemberViews();
    for (const auto& member : members) {
        os << member.id << delim << member.startNodeId << delim << member.endNodeId << delim
           << formatNumber(member.length, options) << std::endl;
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
        os << member.id << delim << formatNumber(member.youngModulus, options) << delim
           << formatNumber(member.density, options) << delim << formatNumber(member.area, options)
           << delim << formatNumber(member.yieldStrength, options) << std::endl;
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
            os << node.id << delim << formatNumber(node.fx, options) << delim
               << formatNumber(node.fy, options) << std::endl;
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
        os << i << delim << formatNumber(displacements[i], options) << std::endl;
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
        os << (i + 1) << delim << formatNumber(force, options) << delim << type << std::endl;
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
        os << i << delim << formatNumber(reactions[i], options) << std::endl;
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
    os << "Max Displacement" << delim << formatNumber(results.getMaxDisplacement(), options)
       << std::endl;
    os << "Max Stress" << delim << formatNumber(results.getMaxStress(), options) << std::endl;
}

}  // namespace truss::infrastructure::export_
