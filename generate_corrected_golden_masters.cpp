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
#include "src/infrastructure/logging/logger_factory.hpp"
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>

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
    auto logger = truss::infrastructure::logging::LoggerFactory::createConsoleLogger(
        truss::infrastructure::logging::LogLevel::Info,
        true
    );

    try {        
        logger->info("=============================================================");
        logger->info("  CORRECTED Golden Master Generator - Data Completeness Fix");
        logger->info("  2D Truss Analysis v3.0.0");
        logger->info("=============================================================");
        
        logger->info("BREAKING CHANGE: Generating golden masters with COMPLETE data");
        logger->info("  - JSON now includes reactions section");
        logger->info("  - XML now includes displacements, forces, reactions, metadata");
        logger->info("  - CSVExporter is authoritative reference");
        
        // Output directory (absolute path based on project root)
        std::filesystem::path outputDir = std::filesystem::path(__FILE__).parent_path()
            / "tests/fixtures/export_golden";
        std::filesystem::create_directories(outputDir);
        logger->info(std::string("Output directory: ") + outputDir.string());
        
        // Step 1: Create test truss
        logger->info("Step 1: Creating test truss structure...");
        Truss truss = createTestTruss();
        {
            std::ostringstream oss;
            oss << "Nodes: " << truss.getNodes().size();
            logger->info(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "Members: " << truss.getMembers().size();
            logger->info(oss.str());
        }
        logger->info("Applied loads: 1 (15 kN downward at top node)");
        logger->info("Supports: Pinned (left) + RollerX (right)");
        
        // Step 2: Run analysis
        logger->info("Step 2: Running structural analysis...");
        auto solver = SolverFactory::createSolver(SolverType::Direct);
        AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
        auto results = orchestrator.analyze(truss);
        
        if (results.converged) {
            logger->info("Analysis converged successfully!");
            {
                std::ostringstream oss;
                oss << "Max displacement: " << std::scientific << std::setprecision(6)
                    << results.maxDisplacement << " m";
                logger->info(oss.str());
            }
            {
                std::ostringstream oss;
                oss << "Max stress: " << std::scientific << std::setprecision(6)
                    << results.maxStress << " Pa";
                logger->info(oss.str());
            }
        } else {
            logger->error("Analysis failed to converge!");
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
        logger->info("Step 3: Generating CORRECTED golden master files...");
        
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
            logger->info(std::string("[") + config.description + "]");
            logger->info(std::string("Generating: ") + fullPath + "...");
            
            if (config.exporter->exportResults(truss, results, fullPath, options)) {
                if (std::filesystem::exists(fullPath)) {
                    auto fileSize = std::filesystem::file_size(fullPath);
                    totalBytes += fileSize;
                    fileDetails.push_back({config.fileName, fileSize});
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
                logger->error(std::string("FAILED: ") + config.exporter->getLastError());
                allSuccessful = false;
            }
        }
        
        // Summary
        logger->info("=============================================================");
        logger->info("  CORRECTED Golden Master Generation Summary");
        logger->info("=============================================================");
        
        if (allSuccessful) {
            logger->info("Status: ALL FORMATS SUCCESSFUL");
            {
                std::ostringstream oss;
                oss << "Files generated: " << exporters.size();
                logger->info(oss.str());
            }
            {
                std::ostringstream oss;
                oss << "Total size: " << totalBytes << " bytes";
                logger->info(oss.str());
            }
            logger->info(std::string("Location: \"") + std::filesystem::absolute(outputDir).string() + "\"");
            
            logger->info("CORRECTED golden master files:");
            for (const auto& [file, size] : fileDetails) {
                std::ostringstream oss;
                oss << "- " << std::setw(28) << std::left << file
                    << "(" << std::setw(8) << std::right << size << " bytes)";
                logger->info(oss.str());
            }
            
            logger->info("BREAKING CHANGE DOCUMENTATION:");
            logger->info("  - JSON file now includes \"reactions\" section (was omitted)");
            logger->info("  - XML file now includes <Displacements>, <MemberForces>, <Reactions>, <Analysis>");
            logger->info("  - Legacy golden masters backed up to: legacy_incomplete/");
            
            logger->info("Next steps:");
            logger->info("  1. Review generated files to verify completeness");
            logger->info("  2. Run test suites to validate against new golden masters");
            logger->info("  3. Update BREAKING_CHANGES.md documentation");
            logger->info("  4. Commit corrected golden masters to repository");
            
            return 0;
        } else {
            logger->error("Status: SOME FORMATS FAILED");
            logger->error("Check error messages above for details");
            return 1;
        }
        
    } catch (const std::exception& e) {
        logger->critical(std::string("FATAL ERROR: ") + e.what());
        return 1;
    }
}
