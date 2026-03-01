/**
 * @file html_exporter.cpp
 * @brief Exports analysis results to HTML format with styling.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "html_exporter.hpp"

#include "utilities/string_utils.hpp"

#include <ctime>
#include <fstream>

namespace truss::infrastructure::export_ {

// Import types from core namespace
using core::Real;
using core::interfaces::IAnalysisResultsView;
using core::interfaces::ITrussView;
using core::interfaces::MemberView;
using core::interfaces::NodeView;

bool HTMLExporter::exportResults(const ITrussView& truss,
                                 const IAnalysisResultsView& results,
                                 const std::filesystem::path& filePath,
                                 const ExportOptions& options) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file for writing: " + filePath.string();
        return false;
    }

    try {
        // HTML document structure
        writeHeader(file, truss);
        writeStyles(file);

        file << "<body>\n";
        file << "<div class=\"container\">\n";

        // Project metadata
        file << "<h1>Truss Analysis Results</h1>\n";
        file << "<h2>Project Metadata</h2>\n";
        file << "<div class=\"metadata\">\n";
        file << "  <p><strong>Project Name:</strong> "
             << truss::utils::string::escapeHtml(truss.getName()) << "</p>\n";
        file << "  <p><strong>Export Date:</strong> " << truss::utils::string::formatTimestamp()
             << "</p>\n";
        file << "  <p><strong>Version:</strong> 3.0.0</p>\n";
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

        file << "</div>\n";  // container
        writeFooter(file);
        file << "</body>\n</html>\n";

        file.close();
        return true;

    } catch (const std::exception& e) {
        m_lastError = "Export failed: " + std::string(e.what());
        return false;
    }
}

void HTMLExporter::writeHeader(std::ostream& os, const ITrussView& truss) {
    os << "<!DOCTYPE html>\n";
    os << "<html lang=\"en\">\n";
    os << "<head>\n";
    os << "  <meta charset=\"UTF-8\">\n";
    os << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    os << "  <title>Truss Analysis Results - " << truss::utils::string::escapeHtml(truss.getName())
       << "</title>\n";
}

void HTMLExporter::writeStyles(std::ostream& os) {
    os << R"(  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      line-height: 1.6;
      color: #333;
      max-width: 1200px;
      margin: 0 auto;
      padding: 20px;
      background-color: #f5f5f5;
    }
    .container {
      background-color: white;
      padding: 30px;
      border-radius: 8px;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
    h1 {
      color: #2c3e50;
      border-bottom: 3px solid #3498db;
      padding-bottom: 10px;
      margin-bottom: 30px;
    }
    h2 {
      color: #34495e;
      margin-top: 30px;
      margin-bottom: 15px;
      border-left: 4px solid #3498db;
      padding-left: 15px;
    }
    .metadata {
      background-color: #ecf0f1;
      padding: 15px;
      border-radius: 5px;
      margin-bottom: 20px;
    }
    .metadata p {
      margin: 5px 0;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin: 20px 0;
      box-shadow: 0 1px 3px rgba(0,0,0,0.1);
    }
    th {
      background-color: #3498db;
      color: white;
      padding: 12px;
      text-align: left;
      font-weight: 600;
    }
    td {
      padding: 10px 12px;
      border-bottom: 1px solid #ddd;
    }
    tr:nth-child(even) {
      background-color: #f9f9f9;
    }
    tr:hover {
      background-color: #f5f5f5;
    }
    .placeholder {
      background-color: #fff3cd;
      border-left: 4px solid #ffc107;
      padding: 15px;
      margin: 15px 0;
      border-radius: 4px;
    }
    .placeholder p {
      margin: 0;
      color: #856404;
    }
    .footer {
      margin-top: 40px;
      padding-top: 20px;
      border-top: 1px solid #ddd;
      text-align: center;
      color: #7f8c8d;
      font-size: 0.9em;
    }
    .number {
      font-family: 'Courier New', monospace;
      text-align: right;
    }
    .tension {
      color: #e74c3c;
      font-weight: 600;
    }
    .compression {
      color: #3498db;
      font-weight: 600;
    }
  </style>
)";
    os << "</head>\n";
}

void HTMLExporter::writeFooter(std::ostream& os) {
    os << "<div class=\"footer\">\n";
    os << "  <p>Generated by 2D Truss Analysis v3.0.0</p>\n";
    os << "  <p>Export Time: " << truss::utils::string::formatTimestamp() << "</p>\n";
    os << "</div>\n";
}

void HTMLExporter::writeGeometrySection(std::ostream& os,
                                        const ITrussView& truss,
                                        const ExportOptions& options) {
    os << "<h2>Geometry</h2>\n";

    // Nodes table
    os << "<h3>Nodes</h3>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>Node ID</th>\n";
    os << "      <th>X Coordinate</th>\n";
    os << "      <th>Y Coordinate</th>\n";
    os << "      <th>Support Type</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (const auto& node : truss.getNodeViews()) {
        os << "    <tr>\n";
        os << "      <td>" << node.id << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  node.x, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  node.y, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "      <td>" << static_cast<int>(node.support) << "</td>\n";
        os << "    </tr>\n";
    }

    os << "  </tbody>\n";
    os << "</table>\n";

    // Members table
    os << "<h3>Members</h3>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>Member ID</th>\n";
    os << "      <th>Start Node</th>\n";
    os << "      <th>End Node</th>\n";
    os << "      <th>Length</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (const auto& member : truss.getMemberViews()) {
        os << "    <tr>\n";
        os << "      <td>" << member.id << "</td>\n";
        os << "      <td>" << member.startNodeId << "</td>\n";
        os << "      <td>" << member.endNodeId << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  member.length, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "    </tr>\n";
    }

    os << "  </tbody>\n";
    os << "</table>\n";
}

void HTMLExporter::writePropertiesSection(std::ostream& os,
                                          const ITrussView& truss,
                                          const ExportOptions& options) {
    os << "<h2>Material and Section Properties</h2>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>Member ID</th>\n";
    os << "      <th>Young's Modulus (Pa)</th>\n";
    os << "      <th>Yield Strength (Pa)</th>\n";
    os << "      <th>Density (kg/m³)</th>\n";
    os << "      <th>Area (m²)</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (const auto& member : truss.getMemberViews()) {
        os << "    <tr>\n";
        os << "      <td>" << member.id << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  member.youngModulus, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  member.yieldStrength, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  member.density, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  member.area, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "    </tr>\n";
    }

    os << "  </tbody>\n";
    os << "</table>\n";
}

void HTMLExporter::writeLoadsSection(std::ostream& os,
                                     const ITrussView& truss,
                                     const ExportOptions& options) {
    os << "<h2>Applied Loads</h2>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>Node ID</th>\n";
    os << "      <th>Fx (N)</th>\n";
    os << "      <th>Fy (N)</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (const auto& node : truss.getNodeViews()) {
        // Only export nodes with non-zero forces
        if (node.fx != 0.0 || node.fy != 0.0) {
            os << "    <tr>\n";
            os << "      <td>" << node.id << "</td>\n";
            os << "      <td class=\"number\">"
               << truss::utils::string::formatReal(
                      node.fx, options.precision, options.useScientificNotation)
               << "</td>\n";
            os << "      <td class=\"number\">"
               << truss::utils::string::formatReal(
                      node.fy, options.precision, options.useScientificNotation)
               << "</td>\n";
            os << "    </tr>\n";
        }
    }

    os << "  </tbody>\n";
    os << "</table>\n";
}

void HTMLExporter::writeDisplacementsSection(std::ostream& os,
                                             const IAnalysisResultsView& results,
                                             const ExportOptions& options) {
    os << "<h2>Nodal Displacements</h2>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>DOF</th>\n";
    os << "      <th>Displacement</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (size_t i = 0; i < results.getDisplacements().size(); ++i) {
        os << "    <tr>\n";
        os << "      <td>" << i << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  results.getDisplacements()[i], options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "    </tr>\n";
    }

    os << "  </tbody>\n";
    os << "</table>\n";

    os << "<p><strong>Maximum Displacement:</strong> <span class=\"number\">"
       << truss::utils::string::formatReal(
              results.getMaxDisplacement(), options.precision, options.useScientificNotation)
       << "</span></p>\n";
}

void HTMLExporter::writeMemberForcesSection(std::ostream& os,
                                            const IAnalysisResultsView& results,
                                            const ExportOptions& options) {
    os << "<h2>Member Forces</h2>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>Member ID</th>\n";
    os << "      <th>Axial Force</th>\n";
    os << "      <th>Type</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (size_t i = 0; i < results.getMemberForces().size(); ++i) {
        Real force = results.getMemberForces()[i];
        std::string type = (force > 0) ? "Tension" : "Compression";
        std::string cssClass = (force > 0) ? "tension" : "compression";

        os << "    <tr>\n";
        os << "      <td>" << (i + 1) << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  force, options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "      <td class=\"" << cssClass << "\">" << type << "</td>\n";
        os << "    </tr>\n";
    }

    os << "  </tbody>\n";
    os << "</table>\n";
}

void HTMLExporter::writeReactionsSection(std::ostream& os,
                                         const IAnalysisResultsView& results,
                                         const ExportOptions& options) {
    os << "<h2>Support Reactions</h2>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>DOF</th>\n";
    os << "      <th>Reaction Force</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    for (size_t i = 0; i < results.getReactions().size(); ++i) {
        os << "    <tr>\n";
        os << "      <td>" << i << "</td>\n";
        os << "      <td class=\"number\">"
           << truss::utils::string::formatReal(
                  results.getReactions()[i], options.precision, options.useScientificNotation)
           << "</td>\n";
        os << "    </tr>\n";
    }

    os << "  </tbody>\n";
    os << "</table>\n";
}

void HTMLExporter::writeMetadataSection(std::ostream& os,
                                        const IAnalysisResultsView& results,
                                        const ExportOptions& options) {
    os << "<h2>Analysis Metadata</h2>\n";
    os << "<table>\n";
    os << "  <thead>\n";
    os << "    <tr>\n";
    os << "      <th>Property</th>\n";
    os << "      <th>Value</th>\n";
    os << "    </tr>\n";
    os << "  </thead>\n";
    os << "  <tbody>\n";

    os << "    <tr>\n";
    os << "      <td>Converged</td>\n";
    os << "      <td>" << (results.hasConverged() ? "Yes" : "No") << "</td>\n";
    os << "    </tr>\n";

    os << "    <tr>\n";
    os << "      <td>Iterations</td>\n";
    os << "      <td>" << results.getIterations() << "</td>\n";
    os << "    </tr>\n";

    os << "    <tr>\n";
    os << "      <td>Total DOFs</td>\n";
    os << "      <td>" << results.getTotalDofs() << "</td>\n";
    os << "    </tr>\n";

    os << "    <tr>\n";
    os << "      <td>Free DOFs</td>\n";
    os << "      <td>" << results.getFreeDofs() << "</td>\n";
    os << "    </tr>\n";

    os << "    <tr>\n";
    os << "      <td>Max Displacement</td>\n";
    os << "      <td class=\"number\">"
       << truss::utils::string::formatReal(
              results.getMaxDisplacement(), options.precision, options.useScientificNotation)
       << "</td>\n";
    os << "    </tr>\n";

    os << "    <tr>\n";
    os << "      <td>Max Stress</td>\n";
    os << "      <td class=\"number\">"
       << truss::utils::string::formatReal(
              results.getMaxStress(), options.precision, options.useScientificNotation)
       << "</td>\n";
    os << "    </tr>\n";

    os << "  </tbody>\n";
    os << "</table>\n";
}

}  // namespace truss::infrastructure::export_
