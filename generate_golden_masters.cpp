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
#include "src/infrastructure/logging/logger_factory.hpp"
#include <filesystem>
#include <iomanip>
#include <sstream>

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
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerX);   // Right support (Fx=0)
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
    auto logger = truss::infrastructure::logging::LoggerFactory::createConsoleLogger(
        truss::infrastructure::logging::LogLevel::Info,
        true
    );

    try {
        logger->info("=============================================================");
        logger->info("  Golden Master Export File Generator - Phase 3");
        logger->info("  2D Truss Analysis v3.0.0");
        logger->info("=============================================================");
        
        // Determine output directory
        std::string outputDir = "tests/fixtures/export_golden";
        if (argc > 1) {
            outputDir = argv[1];
        }
        
        // Ensure output directory exists
        std::filesystem::create_directories(outputDir);
        logger->info(std::string("Output directory: ") + outputDir);
        
        // Create test truss
        logger->info("Step 1: Creating test truss structure...");
        Truss truss = createSimpleTriangleTruss();
        {
            std::ostringstream oss;
            oss << "Nodes: " << truss.getNodeCount();
            logger->info(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "Members: " << truss.getMemberCount();
            logger->info(oss.str());
        }
        logger->info("Applied loads: 1 (15 kN downward at top node)");
        logger->info("Supports: Pinned (left) + RollerX (right)");

        // Perform analysis
        logger->info("Step 2: Running structural analysis...");
        auto solver = SolverFactory::createDirectSolver();
        AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
        auto results = orchestrator.analyze(truss);

        if (!results.converged) {
            logger->error("Analysis did not converge");
            return 1;
        }

        logger->info("Analysis converged successfully!");
        {
            std::ostringstream oss;
            oss << "Max displacement: " << std::scientific << std::setprecision(6)
                << results.maxDisplacement << " m";
            logger->info(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "Max stress: " << results.maxStress << " Pa";
            logger->info(oss.str());
        }

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
        logger->info("Step 3: Generating golden master files...");
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
            logger->info(std::string("[") + description + "]");
            logger->info(std::string("Generating: ") + fullPath + "...");
            
            if (exporter.exportResults(truss, results, fullPath, format, options)) {
                if (std::filesystem::exists(fullPath)) {
                    auto fileSize = std::filesystem::file_size(fullPath);
                    totalBytes += fileSize;
                    {
                        std::ostringstream oss;
                        oss << "SUCCESS (" << fileSize << " bytes)";
                        logger->info(oss.str());
                    }
                    
                    if (fileSize == 0) {
                        logger->warn("File is empty");
                        allSuccessful = false;
                    }
                } else {
                    logger->error("FAILED (file not created)");
                    allSuccessful = false;
                }
            } else {
                logger->error(std::string("FAILED: ") + exporter.getLastError());
                allSuccessful = false;
            }
        }

        // Summary
        logger->info("=============================================================");
        logger->info("  Golden Master Generation Summary");
        logger->info("=============================================================");
        
        if (allSuccessful) {
            logger->info("Status: ALL FORMATS SUCCESSFUL");
            {
                std::ostringstream oss;
                oss << "Files generated: " << exportFormats.size();
                logger->info(oss.str());
            }
            {
                std::ostringstream oss;
                oss << "Total size: " << totalBytes << " bytes";
                logger->info(oss.str());
            }
            logger->info(std::string("Location: ") + std::filesystem::absolute(outputDir).string());
            
            logger->info("Golden master files:");
            for (const auto& [fileName, format, description] : exportFormats) {
                std::string fullPath = outputDir + "/" + fileName;
                if (std::filesystem::exists(fullPath)) {
                    std::ostringstream oss;
                    oss << "- " << std::setw(25) << std::left << fileName
                        << " (" << std::setw(8) << std::right
                        << std::filesystem::file_size(fullPath) << " bytes)";
                    logger->info(oss.str());
                }
            }
            
            logger->info("Next steps:");
            logger->info("  1. Review generated files to ensure correctness");
            logger->info("  2. Commit golden masters to repository");
            logger->info("  3. Use these files to validate new exporter implementations");
            logger->info("  4. Byte-compare new outputs with golden masters");
            
            return 0;
        } else {
            logger->error("Status: SOME FORMATS FAILED");
            logger->error("Check error messages above for details.");
            return 1;
        }

    } catch (const std::exception& e) {
        logger->critical(std::string("EXCEPTION: ") + e.what());
        return 1;
    }
}
