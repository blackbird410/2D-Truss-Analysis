/**
 * @file generate_corrected_golden_masters.cpp
 * @brief Generate CORRECTED golden master files with complete data
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-08
 * 
 * BREAKING CHANGE: This utility generates golden masters using the CORRECTED
 * Strategy pattern exporters with complete data (including reactions).
 * 
 * Legacy ResultsExporter had incomplete implementations - JSON omitted reactions,
 * XML omitted displacements, forces, reactions, and metadata.
 * 
 * CSVExporter is now the authoritative reference for data completeness.
 * All exporters MUST emit semantically equivalent data sets.
 */

#include "src/core/analysis/AnalysisOrchestrator.hpp"
#include "src/core/analysis/SolverFactory.hpp"
#include "src/core/model/Truss.hpp"
#include "src/infrastructure/export/csv_exporter.hpp"
#include "src/infrastructure/export/json_exporter.hpp"
#include "src/infrastructure/export/xml_exporter.hpp"
#include "src/infrastructure/export/html_exporter.hpp"
#include "src/infrastructure/export/latex_exporter.hpp"
#include "src/infrastructure/export/text_exporter.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <memory>

using namespace truss::core;
using namespace truss::core::analysis;
using namespace truss::infrastructure::export_;

/**
 * @brief Create the same simple triangle truss as original generator
 */
Truss createTestTruss() {
    Truss truss("Golden Master Test Truss");
    
    // Create nodes (same as original)
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);    // Left support
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerX);   // Right support
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);      // Top node
    
    // Add members to form triangle
    truss.addMember(node1, node2);  // Bottom chord
    truss.addMember(node1, node3);  // Left diagonal
    truss.addMember(node2, node3);  // Right diagonal
    
    // Apply downward load at apex
    truss.applyForce(node3->getId(), Force2D(0.0, -15000.0)); // 15 kN downward
    
    return truss;
}

int main() {
    try {        
        std::cout << "=============================================================\n";
        std::cout << "  CORRECTED Golden Master Generator - Data Completeness Fix\n";
        std::cout << "  2D Truss Analysis v3.0.0\n";
        std::cout << "=============================================================\n\n";
        
        std::cout << "BREAKING CHANGE: Generating golden masters with COMPLETE data\n";
        std::cout << "  - JSON now includes reactions section\n";
        std::cout << "  - XML now includes displacements, forces, reactions, metadata\n";
        std::cout << "  - CSVExporter is authoritative reference\n\n";
        
        // Output directory (absolute path based on project root)
        std::filesystem::path outputDir = std::filesystem::path(__FILE__).parent_path()
            / "tests/fixtures/export_golden";
        std::filesystem::create_directories(outputDir);
        std::cout << "Output directory: " << outputDir.string() << "\n\n";
        
        // Step 1: Create test truss
        std::cout << "Step 1: Creating test truss structure...\n";
        Truss truss = createTestTruss();
        std::cout << "  Nodes: " << truss.getNodes().size() << "\n";
        std::cout << "  Members: " << truss.getMembers().size() << "\n";
        std::cout << "  Applied loads: 1 (15 kN downward at top node)\n";
        std::cout << "  Supports: Pinned (left) + RollerX (right)\n\n";
        
        // Step 2: Run analysis
        std::cout << "Step 2: Running structural analysis...\n";
        auto solver = SolverFactory::createSolver(SolverType::Direct);
        AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
        auto results = orchestrator.analyze(truss);
        
        if (results.converged) {
            std::cout << "  Analysis converged successfully!\n";
            std::cout << "  Max displacement: " << std::scientific << std::setprecision(6) 
                     << results.maxDisplacement << " m\n";
            std::cout << "  Max stress: " << std::scientific << std::setprecision(6)
                     << results.maxStress << " Pa\n\n";
        } else {
            std::cerr << "ERROR: Analysis failed to converge!\n";
            return 1;
        }
        
        // Export options
        ExportOptions options;
        options.includeGeometry = true;
        options.includeDisplacements = true;
        options.includeMemberForces = true;
        options.includeReactions = true;  // CRITICAL: Now included in all formats
        options.includeMetadata = true;
        options.precision = 6;
        
        // Step 3: Generate golden masters using CORRECTED exporters
        std::cout << "Step 3: Generating CORRECTED golden master files...\n";
        
        struct ExportConfig {
            std::string fileName;
            std::unique_ptr<IResultsExporter> exporter;
            std::string description;
        };
        
        std::vector<ExportConfig> exporters;
        exporters.push_back({"golden_master.csv", std::make_unique<CSVExporter>(), 
                            "CSV (Authoritative Reference)"});
        exporters.push_back({"golden_master.json", std::make_unique<JSONExporter>(), 
                            "JSON (NOW INCLUDES REACTIONS)"});
        exporters.push_back({"golden_master.xml", std::make_unique<XMLExporter>(), 
                            "XML (NOW COMPLETE - 8 SECTIONS)"});
        exporters.push_back({"golden_master.html", std::make_unique<HTMLExporter>(), 
                            "HTML (8-SECTION COMPLIANT)"});
        exporters.push_back({"golden_master.tex", std::make_unique<LaTeXExporter>(), 
                            "LaTeX (8-SECTION COMPLIANT)"});
        exporters.push_back({"golden_master.txt", std::make_unique<TextExporter>(), 
                            "Text (8-SECTION COMPLIANT)"});
        
        bool allSuccessful = true;
        size_t totalBytes = 0;
        std::vector<std::pair<std::string, size_t>> fileDetails;
        
        for (auto& config : exporters) {
            std::string fullPath = (outputDir / config.fileName).string();
            std::cout << "\n  [" << config.description << "]\n";
            std::cout << "  Generating: " << fullPath << "... ";
            
            if (config.exporter->exportResults(truss, results, fullPath, options)) {
                if (std::filesystem::exists(fullPath)) {
                    auto fileSize = std::filesystem::file_size(fullPath);
                    totalBytes += fileSize;
                    fileDetails.push_back({config.fileName, fileSize});
                    std::cout << "✓ SUCCESS (" << fileSize << " bytes)\n";
                    
                    if (fileSize == 0) {
                        std::cout << "    ⚠️  WARNING: File is empty!\n";
                        allSuccessful = false;
                    }
                } else {
                    std::cout << "✗ FAILED (file not created)\n";
                    allSuccessful = false;
                }
            } else {
                std::cout << "✗ FAILED\n";
                std::cout << "  Error: " << config.exporter->getLastError() << "\n";
                allSuccessful = false;
            }
        }
        
        // Summary
        std::cout << "\n=============================================================\n";
        std::cout << "  CORRECTED Golden Master Generation Summary\n";
        std::cout << "=============================================================\n";
        
        if (allSuccessful) {
            std::cout << "✓ Status: ALL FORMATS SUCCESSFUL\n";
            std::cout << "  Files generated: " << exporters.size() << "\n";
            std::cout << "  Total size: " << totalBytes << " bytes\n";
            std::cout << "  Location: \"" << std::filesystem::absolute(outputDir).string() << "\"\n\n";
            
            std::cout << "CORRECTED golden master files:\n";
            for (const auto& [file, size] : fileDetails) {
                std::cout << "  ✓ " << std::setw(28) << std::left << file 
                         << "(" << std::setw(8) << std::right << size << " bytes)\n";
            }
            
            std::cout << "\nBREAKING CHANGE DOCUMENTATION:\n";
            std::cout << "  - JSON file now includes \"reactions\" section (was omitted)\n";
            std::cout << "  - XML file now includes <Displacements>, <MemberForces>, <Reactions>, <Analysis>\n";
            std::cout << "  - Legacy golden masters backed up to: legacy_incomplete/\n\n";
            
            std::cout << "Next steps:\n";
            std::cout << "  1. Review generated files to verify completeness\n";
            std::cout << "  2. Run test suites to validate against new golden masters\n";
            std::cout << "  3. Update BREAKING_CHANGES.md documentation\n";
            std::cout << "  4. Commit corrected golden masters to repository\n";
            
            return 0;
        } else {
            std::cerr << "✗ Status: SOME FORMATS FAILED\n";
            std::cerr << "  Check error messages above for details\n";
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }
}
