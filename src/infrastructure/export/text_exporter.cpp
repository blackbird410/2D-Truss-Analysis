/**
 * @file text_exporter.cpp
 * @brief Plain text format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "text_exporter.hpp"
#include "src/core/Logger.hpp"
#include <fstream>
#include <ctime>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::Truss;
using core::analysis::AnalysisResults;

bool TextExporter::exportResults(const Truss& truss,
                                const AnalysisResults& results,
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
        
        // Project metadata (always included)
        writeProjectSection(file, truss, options);
        
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
        
        if (options.includeDisplacements && results.displacements.size() > 0) {
            writeDisplacementsSection(file, results, options);
        }
        
        if (options.includeMemberForces && !results.memberForces.empty()) {
            writeMemberForcesSection(file, results, options);
        }
        
        if (options.includeReactions && !results.reactions.empty()) {
            writeReactionsSection(file, results, options);
        }
        
        if (options.includeMetadata) {
            writeMetadataSection(file, results, options);
        }
        
        // Document footer
        writeSeparator(file);
        file << "End of Report\n";
        
        file.close();
        core::Logger::info("Results exported to text: " + filePath.string());
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        core::Logger::error("Text export failed: " + m_lastError);
        return false;
    }
}

std::string TextExporter::formatNumber(Real value, 
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

std::string TextExporter::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void TextExporter::writeSeparator(std::ostream& os, int width) const {
    os << std::string(width, '=') << "\n";
}

void TextExporter::writeSectionHeader(std::ostream& os, const std::string& title) const {
    os << "\n";
    writeSeparator(os);
    os << title << "\n";
    writeSeparator(os);
}

void TextExporter::writeHeader(std::ostream& os, const Truss& truss) {
    writeSeparator(os);
    os << "2D TRUSS ANALYSIS RESULTS\n";
    os << truss.getName() << "\n";
    writeSeparator(os);
    os << "Generated: " << formatTimestamp() << "\n";
    os << "Version: 3.0.0\n";
    writeSeparator(os);
}

void TextExporter::writeProjectSection(std::ostream& os, 
                                      const Truss& truss,
                                      const ExportOptions& /*options*/) {
    writeSectionHeader(os, "PROJECT METADATA");
    
    os << "  Project Name:    " << truss.getName() << "\n";
    os << "  Generated:       " << formatTimestamp() << "\n";
    os << "  Software:        2D Truss Analysis v3.0.0\n";
    os << "  Number of Nodes: " << truss.getNodeCount() << "\n";
    os << "  Number of Members: " << truss.getMemberCount() << "\n";
}

void TextExporter::writeGeometrySection(std::ostream& os,
                                       const Truss& truss,
                                       const ExportOptions& options) {
    writeSectionHeader(os, "GEOMETRY");
    
    // Nodes subsection
    os << "\nNodes:\n";
    os << "  " << std::left << std::setw(8) << "Node ID"
       << std::setw(15) << "X (m)"
       << std::setw(15) << "Y (m)"
       << std::setw(15) << "Support Type" << "\n";
    os << "  " << std::string(53, '-') << "\n";
    
    for (const auto& node : truss.getNodes()) {
        std::string supportType;
        switch (node->getSupportType()) {
            case core::SupportType::Free: supportType = "Free"; break;
            case core::SupportType::Pinned: supportType = "Pinned"; break;
            case core::SupportType::PinnedX: supportType = "Pinned-X"; break;
            case core::SupportType::PinnedY: supportType = "Pinned-Y"; break;
            case core::SupportType::RollerX: supportType = "Roller-X"; break;
            case core::SupportType::RollerY: supportType = "Roller-Y"; break;
            default: supportType = "Unknown"; break;
        }
        
        os << "  " << std::left << std::setw(8) << node->getId()
           << std::setw(15) << formatNumber(node->getX(), options)
           << std::setw(15) << formatNumber(node->getY(), options)
           << std::setw(15) << supportType << "\n";
    }
    
    // Members subsection
    os << "\nMembers:\n";
    os << "  " << std::left << std::setw(10) << "Member ID"
       << std::setw(12) << "Start Node"
       << std::setw(12) << "End Node"
       << std::setw(15) << "Length (m)" << "\n";
    os << "  " << std::string(49, '-') << "\n";
    
    for (const auto& member : truss.getMembers()) {
        os << "  " << std::left << std::setw(10) << member->getId()
           << std::setw(12) << member->getStartNode()->getId()
           << std::setw(12) << member->getEndNode()->getId()
           << std::setw(15) << formatNumber(member->getLength(), options) << "\n";
    }
}

void TextExporter::writePropertiesSection(std::ostream& os,
                                         const Truss& /*truss*/,
                                         const ExportOptions& /*options*/) {
    writeSectionHeader(os, "MATERIAL AND SECTION PROPERTIES");
    os << "  [Not yet implemented in domain model]\n";
    os << "  This section will contain:\n";
    os << "    - Young's modulus\n";
    os << "    - Cross-sectional area\n";
    os << "    - Material type\n";
}

void TextExporter::writeLoadsSection(std::ostream& os,
                                    const Truss& /*truss*/,
                                    const ExportOptions& /*options*/) {
    writeSectionHeader(os, "APPLIED LOADS");
    os << "  [Not yet implemented in domain model]\n";
    os << "  This section will contain:\n";
    os << "    - Node ID\n";
    os << "    - Force X component\n";
    os << "    - Force Y component\n";
    os << "    - Load case\n";
}

void TextExporter::writeDisplacementsSection(std::ostream& os,
                                            const AnalysisResults& results,
                                            const ExportOptions& options) {
    writeSectionHeader(os, "NODAL DISPLACEMENTS");
    
    os << "  " << std::left << std::setw(10) << "DOF"
       << std::setw(20) << "Displacement (m)" << "\n";
    os << "  " << std::string(30, '-') << "\n";
    
    for (size_t i = 0; i < results.displacements.size(); ++i) {
        os << "  " << std::left << std::setw(10) << i
           << std::setw(20) << formatNumber(results.displacements[i], options) << "\n";
    }
    
    os << "\n  Maximum Displacement: " 
       << formatNumber(results.maxDisplacement, options) << " m\n";
}

void TextExporter::writeMemberForcesSection(std::ostream& os,
                                           const AnalysisResults& results,
                                           const ExportOptions& options) {
    writeSectionHeader(os, "MEMBER FORCES");
    
    os << "  " << std::left << std::setw(12) << "Member ID"
       << std::setw(20) << "Axial Force (N)"
       << std::setw(15) << "Type" << "\n";
    os << "  " << std::string(47, '-') << "\n";
    
    for (size_t i = 0; i < results.memberForces.size(); ++i) {
        Real force = results.memberForces[i];
        std::string type = (force > 0) ? "Tension" : "Compression";
        
        os << "  " << std::left << std::setw(12) << (i + 1)
           << std::setw(20) << formatNumber(force, options)
           << std::setw(15) << type << "\n";
    }
}

void TextExporter::writeReactionsSection(std::ostream& os,
                                        const AnalysisResults& results,
                                        const ExportOptions& options) {
    writeSectionHeader(os, "SUPPORT REACTIONS");
    
    os << "  " << std::left << std::setw(10) << "DOF"
       << std::setw(25) << "Reaction Force (N)" << "\n";
    os << "  " << std::string(35, '-') << "\n";
    
    for (size_t i = 0; i < results.reactions.size(); ++i) {
        os << "  " << std::left << std::setw(10) << i
           << std::setw(25) << formatNumber(results.reactions[i], options) << "\n";
    }
}

void TextExporter::writeMetadataSection(std::ostream& os,
                                       const AnalysisResults& results,
                                       const ExportOptions& options) {
    writeSectionHeader(os, "ANALYSIS METADATA");
    
    os << "  Converged:          " << (results.converged ? "Yes" : "No") << "\n";
    os << "  Iterations:         " << results.iterations << "\n";
    os << "  Total DOFs:         " << results.totalDofs << "\n";
    os << "  Free DOFs:          " << results.freeDofs << "\n";
    os << "  Max Displacement:   " << formatNumber(results.maxDisplacement, options) << " m\n";
    os << "  Max Stress:         " << formatNumber(results.maxStress, options) << " Pa\n";
}

} // namespace truss::infrastructure::export_
