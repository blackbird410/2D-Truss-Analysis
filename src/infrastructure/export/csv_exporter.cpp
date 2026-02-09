/**
 * @file csv_exporter.cpp
 * @brief CSV format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "csv_exporter.hpp"
#include <fstream>
#include <ctime>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::Truss;
using core::analysis::AnalysisResults;

bool CSVExporter::exportResults(const Truss& truss,
                                const AnalysisResults& results,
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

std::string CSVExporter::formatNumber(Real value, 
                                     const ExportOptions& options) const {
    std::stringstream ss;
    if (options.useScientificNotation) {
        ss << std::scientific;
    } else {
        ss << std::fixed;
    }
    ss << std::setprecision(options.precision) << value;
    return ss.str();
}

std::string CSVExporter::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void CSVExporter::writeGeometrySection(std::ostream& os,
                                      const Truss& truss,
                                      const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    // Nodes
    os << "Node ID" << delim << "X" << delim << "Y" << delim << "Support Type" << std::endl;
    for (const auto& node : truss.getNodes()) {
        os << node->getId() << delim
           << formatNumber(node->getX(), options) << delim
           << formatNumber(node->getY(), options) << delim
           << static_cast<int>(node->getSupportType()) << std::endl;
    }
    
    os << std::endl;
    
    // Members
    os << "Member ID" << delim << "Start Node" << delim << "End Node" << delim << "Length" << std::endl;
    for (const auto& member : truss.getMembers()) {
        os << member->getId() << delim
           << member->getStartNode()->getId() << delim
           << member->getEndNode()->getId() << delim
           << formatNumber(member->getLength(), options) << std::endl;
    }
}

void CSVExporter::writePropertiesSection(std::ostream& os,
                                        const Truss& truss,
                                        const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "Member ID" << delim << "Material" << delim << "E (Pa)" << delim 
       << "Density (kg/m³)" << delim << "Area (m²)" << delim << "Section" << std::endl;
    
    for (const auto& member : truss.getMembers()) {
        const auto& material = member->getMaterial();
        const auto& section = member->getSection();
        os << member->getId() << delim
           << material.name << delim
           << formatNumber(material.youngModulus, options) << delim
           << formatNumber(material.density, options) << delim
           << formatNumber(section.area, options) << delim
           << section.designation << std::endl;
    }
}

void CSVExporter::writeLoadsSection(std::ostream& os,
                                   const Truss& truss,
                                   const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "Node ID" << delim << "Fx (N)" << delim << "Fy (N)" << std::endl;
    
    for (const auto& node : truss.getNodes()) {
        const auto& force = node->getAppliedForce();
        // Only export nodes with non-zero forces
        if (force.fx != 0.0 || force.fy != 0.0) {
            os << node->getId() << delim
               << formatNumber(force.fx, options) << delim
               << formatNumber(force.fy, options) << std::endl;
        }
    }
}

void CSVExporter::writeDisplacementsSection(std::ostream& os,
                                           const Truss& /*truss*/,
                                           const AnalysisResults& results,
                                           const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "DOF" << delim << "Displacement" << std::endl;
    for (size_t i = 0; i < results.displacements.size(); ++i) {
        os << i << delim << formatNumber(results.displacements[i], options) << std::endl;
    }
}

void CSVExporter::writeMemberForcesSection(std::ostream& os,
                                          const Truss& truss,
                                          const AnalysisResults& results,
                                          const ExportOptions& options) {
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

void CSVExporter::writeReactionsSection(std::ostream& os,
                                       const Truss& /*truss*/,
                                       const AnalysisResults& results,
                                       const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "DOF" << delim << "Reaction Force" << std::endl;
    for (size_t i = 0; i < results.reactions.size(); ++i) {
        os << i << delim << formatNumber(results.reactions[i], options) << std::endl;
    }
}

void CSVExporter::writeMetadataSection(std::ostream& os,
                                      const AnalysisResults& results,
                                      const ExportOptions& options) {
    const std::string& delim = options.delimiter;
    
    os << "Property" << delim << "Value" << std::endl;
    os << "Converged" << delim << (results.converged ? "Yes" : "No") << std::endl;
    os << "Iterations" << delim << results.iterations << std::endl;
    os << "Total DOFs" << delim << results.totalDofs << std::endl;
    os << "Free DOFs" << delim << results.freeDofs << std::endl;
    os << "Max Displacement" << delim << formatNumber(results.maxDisplacement, options) << std::endl;
    os << "Max Stress" << delim << formatNumber(results.maxStress, options) << std::endl;
}

} // namespace truss::infrastructure::export_
