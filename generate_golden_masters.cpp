/**
 * @file generate_golden_masters.cpp
 * @brief Generate golden master export files for Phase 3 validation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-07
 * 
 * This utility generates reference export files using the current (legacy)
 * ResultsExporter implementation. These files serve as "golden masters" for
 * validating the new Strategy pattern exporters in Phase 3.
 */

#include "src/core/analysis/AnalysisOrchestrator.hpp"
#include "src/core/analysis/SolverFactory.hpp"
#include "src/core/ResultsExporter.hpp"
#include "src/core/model/Truss.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>

using namespace truss::core;
using namespace truss::core::analysis;

/**
 * @brief Create a simple statically determinate truss for testing
 */
Truss createSimpleTriangleTruss() {
    Truss truss("Golden Master Test Truss");
    
    // Create a simple triangle truss (statically determinate)
    // 3 nodes, 3 members, 3 constraints = 2*3 = 6 equations
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);    // Left support (Fx=0, Fy=0)
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);   // Right support (Fy=0)
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);      // Top node (free)

    // Add members to form a triangle
    truss.addMember(node1, node2);  // Bottom horizontal member
    truss.addMember(node1, node3);  // Left diagonal member  
    truss.addMember(node2, node3);  // Right diagonal member

    // Apply downward load at top node
    truss.applyForce(node3->getId(), Force2D(0.0, -15000.0)); // 15 kN downward

    return truss;
}

int main(int argc, char* argv[]) {
    try {
        std::cout << "=============================================================\n";
        std::cout << "  Golden Master Export File Generator - Phase 3\n";
        std::cout << "  2D Truss Analysis v3.0.0\n";
        std::cout << "=============================================================\n\n";
        
        // Determine output directory
        std::string outputDir = "tests/fixtures/export_golden";
        if (argc > 1) {
            outputDir = argv[1];
        }
        
        // Ensure output directory exists
        std::filesystem::create_directories(outputDir);
        std::cout << "Output directory: " << outputDir << "\n\n";
        
        // Create test truss
        std::cout << "Step 1: Creating test truss structure...\n";
        Truss truss = createSimpleTriangleTruss();
        std::cout << "  Nodes: " << truss.getNodeCount() << "\n";
        std::cout << "  Members: " << truss.getMemberCount() << "\n";
        std::cout << "  Applied loads: 1 (15 kN downward at top node)\n";
        std::cout << "  Supports: Pinned (left) + RollerY (right)\n\n";

        // Perform analysis
        std::cout << "Step 2: Running structural analysis...\n";
        auto solver = SolverFactory::createDirectSolver();
        AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
        auto results = orchestrator.analyze(truss);

        if (!results.converged) {
            std::cerr << "ERROR: Analysis did not converge\n";
            return 1;
        }

        std::cout << "  Analysis converged successfully!\n";
        std::cout << "  Max displacement: " << std::scientific << std::setprecision(6) 
                  << results.maxDisplacement << " m\n";
        std::cout << "  Max stress: " << results.maxStress << " Pa\n\n";

        // Configure export options
        ExportOptions options;
        options.includeGeometry = true;
        options.includeProperties = true;
        options.includeLoads = true;
        options.includeDisplacements = true;
        options.includeMemberForces = true;
        options.includeReactions = true;
        options.includeStresses = true;
        options.includeUtilization = true;
        options.includeMetadata = true;
        options.useScientificNotation = false;
        options.precision = 6;

        // Define export formats
        std::cout << "Step 3: Generating golden master files...\n";
        std::vector<std::tuple<std::string, ExportFormat, std::string>> exportFormats = {
            {"golden_master.csv",   ExportFormat::CSV,    "CSV (Comma-Separated Values)"},
            {"golden_master.json",  ExportFormat::JSON,   "JSON (JavaScript Object Notation)"},
            {"golden_master.xml",   ExportFormat::XML,    "XML (Extensible Markup Language)"},
            {"golden_master.html",  ExportFormat::HTML,   "HTML (HyperText Markup Language)"},
            {"golden_master.tex",   ExportFormat::LaTeX,  "LaTeX (Typesetting Format)"},
            {"golden_master.txt",   ExportFormat::TXT,    "TXT (Plain Text)"}
        };

        ResultsExporter exporter;
        bool allSuccessful = true;
        size_t totalBytes = 0;

        for (const auto& [fileName, format, description] : exportFormats) {
            std::string fullPath = outputDir + "/" + fileName;
            std::cout << "\n  [" << description << "]\n";
            std::cout << "  Generating: " << fullPath << "... ";
            
            if (exporter.exportResults(truss, results, fullPath, format, options)) {
                if (std::filesystem::exists(fullPath)) {
                    auto fileSize = std::filesystem::file_size(fullPath);
                    totalBytes += fileSize;
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
                std::cout << "    Error: " << exporter.getLastError() << "\n";
                allSuccessful = false;
            }
        }

        // Summary
        std::cout << "\n=============================================================\n";
        std::cout << "  Golden Master Generation Summary\n";
        std::cout << "=============================================================\n";
        
        if (allSuccessful) {
            std::cout << "✓ Status: ALL FORMATS SUCCESSFUL\n";
            std::cout << "  Files generated: " << exportFormats.size() << "\n";
            std::cout << "  Total size: " << totalBytes << " bytes\n";
            std::cout << "  Location: " << std::filesystem::absolute(outputDir) << "\n\n";
            
            std::cout << "Golden master files:\n";
            for (const auto& [fileName, format, description] : exportFormats) {
                std::string fullPath = outputDir + "/" + fileName;
                if (std::filesystem::exists(fullPath)) {
                    std::cout << "  ✓ " << std::setw(25) << std::left << fileName 
                              << " (" << std::setw(8) << std::right 
                              << std::filesystem::file_size(fullPath) << " bytes)\n";
                }
            }
            
            std::cout << "\nNext steps:\n";
            std::cout << "  1. Review generated files to ensure correctness\n";
            std::cout << "  2. Commit golden masters to repository\n";
            std::cout << "  3. Use these files to validate new exporter implementations\n";
            std::cout << "  4. Byte-compare new outputs with golden masters\n\n";
            
            return 0;
        } else {
            std::cout << "✗ Status: SOME FORMATS FAILED\n";
            std::cout << "  Check error messages above for details.\n\n";
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "\n✗ EXCEPTION: " << e.what() << "\n";
        return 1;
    }
}
