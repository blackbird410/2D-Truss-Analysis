/**
 * @file latex_exporter.cpp
 * @brief LaTeX format results exporter implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "latex_exporter.hpp"
#include <fstream>
#include <ctime>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::ITrussView;
using core::interfaces::IAnalysisResultsView;
using core::interfaces::NodeView;
using core::interfaces::MemberView;

bool LaTeXExporter::exportResults(const ITrussView& truss,
                                 const IAnalysisResultsView& results,
                                 const std::filesystem::path& filePath,
                                 const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }
    
    try {
        // LaTeX document structure
        writePreamble(file, truss);
        file << "\n\\begin{document}\n\n";
        
        // Title and project metadata (always included)
        writeProjectSection(file, truss, options);
        
        // Conditional sections (following ExportOptions)
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
        
        file << "\n\\end{document}\n";
        
        file.close();
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        return false;
    }
}

std::string LaTeXExporter::formatNumber(Real value, 
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

std::string LaTeXExporter::formatTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string LaTeXExporter::escapeLatex(const std::string& text) const {
    std::string result;
    result.reserve(text.size() * 1.2); // Pre-allocate
    
    for (char c : text) {
        switch (c) {
            case '\\': result += "\\textbackslash{}"; break;
            case '{':  result += "\\{"; break;
            case '}':  result += "\\}"; break;
            case '$':  result += "\\$"; break;
            case '&':  result += "\\&"; break;
            case '%':  result += "\\%"; break;
            case '#':  result += "\\#"; break;
            case '_':  result += "\\_"; break;
            case '~':  result += "\\textasciitilde{}"; break;
            case '^':  result += "\\textasciicircum{}"; break;
            default:   result += c;
        }
    }
    return result;
}

void LaTeXExporter::writePreamble(std::ostream& os, const ITrussView& truss) {
    os << "\\documentclass[11pt,a4paper]{article}\n";
    os << "\\usepackage[utf8]{inputenc}\n";
    os << "\\usepackage{amsmath}\n";
    os << "\\usepackage{booktabs}\n";
    os << "\\usepackage{longtable}\n";
    os << "\\usepackage{geometry}\n";
    os << "\\geometry{margin=1in}\n\n";
    os << "\\title{2D Truss Analysis Results\\\\";
    os << escapeLatex(truss.getName()) << "}\n";
    os << "\\author{Civil Engineering Software Solutions}\n";
    os << "\\date{" << formatTimestamp() << "}\n";
}

void LaTeXExporter::writeClosing(std::ostream& os) {
    os << "\\end{document}\n";
}

void LaTeXExporter::writeProjectSection(std::ostream& os, 
                                       const ITrussView& truss, 
                                       const ExportOptions& /*options*/) {
    os << "\\maketitle\n\n";
    os << "\\section{Project Metadata}\n\n";
    os << "\\begin{itemize}\n";
    os << "  \\item \\textbf{Project Name:} " << escapeLatex(truss.getName()) << "\n";
    os << "  \\item \\textbf{Generated:} " << formatTimestamp() << "\n";
    os << "  \\item \\textbf{Software:} 2D Truss Analysis v3.0.0\n";
    os << "  \\item \\textbf{Nodes:} " << truss.getNodeViews().size() << "\n";
    os << "  \\item \\textbf{Members:} " << truss.getMemberViews().size() << "\n";
    os << "\\end{itemize}\n\n";
}

void LaTeXExporter::writeGeometrySection(std::ostream& os,
                                        const ITrussView& truss,
                                        const ExportOptions& options) {
    os << "\\section{Geometry}\n\n";
    
    // Nodes table
    os << "\\subsection{Nodes}\n\n";
    os << "\\begin{longtable}{cccc}\n";
    os << "\\toprule\n";
    os << "Node ID & X (m) & Y (m) & Support Type \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{4}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "Node ID & X (m) & Y (m) & Support Type \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{4}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (const auto& node : truss.getNodeViews()) {
        os << node.id << " & "
           << formatNumber(node.x, options) << " & "
           << formatNumber(node.y, options) << " & "
           << static_cast<int>(node.support) << " \\\\\n";
    }
    
    os << "\\end{longtable}\n\n";
    
    // Members table
    os << "\\subsection{Members}\n\n";
    os << "\\begin{longtable}{cccc}\n";
    os << "\\toprule\n";
    os << "Member ID & Start Node & End Node & Length (m) \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{4}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "Member ID & Start Node & End Node & Length (m) \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{4}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (const auto& member : truss.getMemberViews()) {
        os << member.id << " & "
           << member.startNodeId << " & "
           << member.endNodeId << " & "
           << formatNumber(member.length, options) << " \\\\\n";
    }
    
    os << "\\end{longtable}\n\n";
}

void LaTeXExporter::writePropertiesSection(std::ostream& os,
                                          const ITrussView& truss,
                                          const ExportOptions& options) {
    os << "\\section{Material and Section Properties}\n\n";
    os << "\\begin{longtable}{cccccc}\n";
    os << "\\toprule\n";
    os << "Member ID & Material & E (Pa) & Density (kg/m³) & Area (m²) & Section \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{6}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "Member ID & Material & E (Pa) & Density (kg/m³) & Area (m²) & Section \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{6}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (const auto& member : truss.getMemberViews()) {
        os << member.id << " & "
           << formatNumber(member.youngModulus, options) << " & "
           << formatNumber(member.yieldStrength, options) << " & "
           << formatNumber(member.density, options) << " & "
           << formatNumber(member.area, options) << " \\\\\n";
    }
    
    os << "\\end{longtable}\n\n";
}

void LaTeXExporter::writeLoadsSection(std::ostream& os,
                                     const ITrussView& truss,
                                     const ExportOptions& options) {
    os << "\\section{Applied Loads}\n\n";
    os << "\\begin{longtable}{ccc}\n";
    os << "\\toprule\n";
    os << "Node ID & Fx (N) & Fy (N) \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{3}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "Node ID & Fx (N) & Fy (N) \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{3}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (const auto& node : truss.getNodeViews()) {
        // Only export nodes with non-zero forces
        if (node.fx != 0.0 || node.fy != 0.0) {
            os << node.id << " & "
               << formatNumber(node.fx, options) << " & "
               << formatNumber(node.fy, options) << " \\\\\n";
        }
    }
    
    os << "\\end{longtable}\n\n";
}

void LaTeXExporter::writeDisplacementsSection(std::ostream& os,
                                             const IAnalysisResultsView& results,
                                             const ExportOptions& options) {
    os << "\\section{Nodal Displacements}\n\n";
    os << "\\begin{longtable}{cc}\n";
    os << "\\toprule\n";
    os << "DOF & Displacement (m) \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{2}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "DOF & Displacement (m) \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{2}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (size_t i = 0; i < results.getDisplacements().size(); ++i) {
        os << i << " & " << formatNumber(results.getDisplacements()[i], options) << " \\\\\n";
    }
    
    os << "\\end{longtable}\n\n";
    os << "\\textbf{Maximum Displacement:} " 
       << formatNumber(results.getMaxDisplacement(), options) << " m\n\n";
}

void LaTeXExporter::writeMemberForcesSection(std::ostream& os,
                                            const IAnalysisResultsView& results,
                                            const ExportOptions& options) {
    os << "\\section{Member Forces}\n\n";
    os << "\\begin{longtable}{ccc}\n";
    os << "\\toprule\n";
    os << "Member ID & Axial Force (N) & Type \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{3}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "Member ID & Axial Force (N) & Type \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{3}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (size_t i = 0; i < results.getMemberForces().size(); ++i) {
        Real force = results.getMemberForces()[i];
        std::string type = (force > 0) ? "Tension" : "Compression";
        os << (i + 1) << " & "
           << formatNumber(force, options) << " & "
           << type << " \\\\\n";
    }
    
    os << "\\end{longtable}\n\n";
}

void LaTeXExporter::writeReactionsSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options) {
    os << "\\section{Support Reactions}\n\n";
    os << "\\begin{longtable}{cc}\n";
    os << "\\toprule\n";
    os << "DOF & Reaction Force (N) \\\\\n";
    os << "\\midrule\n";
    os << "\\endfirsthead\n\n";
    os << "\\multicolumn{2}{c}{{\\tablename\\ \\thetable{} -- continued from previous page}} \\\\\n";
    os << "\\toprule\n";
    os << "DOF & Reaction Force (N) \\\\\n";
    os << "\\midrule\n";
    os << "\\endhead\n\n";
    os << "\\midrule\n";
    os << "\\multicolumn{2}{r}{{Continued on next page}} \\\\\n";
    os << "\\endfoot\n\n";
    os << "\\bottomrule\n";
    os << "\\endlastfoot\n\n";
    
    for (size_t i = 0; i < results.getReactions().size(); ++i) {
        os << i << " & " << formatNumber(results.getReactions()[i], options) << " \\\\\n";
    }
    
    os << "\\end{longtable}\n\n";
}

void LaTeXExporter::writeMetadataSection(std::ostream& os,
                                        const IAnalysisResultsView& results,
                                        const ExportOptions& options) {
    os << "\\section{Analysis Metadata}\n\n";
    os << "\\begin{tabular}{ll}\n";
    os << "\\toprule\n";
    os << "Property & Value \\\\\n";
    os << "\\midrule\n";
    os << "Converged & " << (results.hasConverged() ? "Yes" : "No") << " \\\\\n";
    os << "Iterations & " << results.getIterations() << " \\\\\n";
    os << "Total DOFs & " << results.getTotalDofs() << " \\\\\n";
    os << "Free DOFs & " << results.getFreeDofs() << " \\\\\n";
    os << "Max Displacement & " << formatNumber(results.getMaxDisplacement(), options) << " m \\\\\n";
    os << "Max Stress & " << formatNumber(results.getMaxStress(), options) << " Pa \\\\\n";
    os << "\\bottomrule\n";
    os << "\\end{tabular}\n\n";
}

} // namespace truss::infrastructure::export_
