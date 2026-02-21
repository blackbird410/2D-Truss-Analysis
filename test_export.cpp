/**
 * @file test_export.cpp
 * @brief Test for the ResultsExporter functionality
 */

#include "AnalysisEngine.hpp"
#include "ResultsExporter.hpp"
#include "Truss.hpp"
#include "src/infrastructure/logging/logger_factory.hpp"
#include <filesystem>
#include <sstream>

using namespace truss::core;

int main() {
    auto logger = truss::infrastructure::logging::LoggerFactory::createConsoleLogger(
        truss::infrastructure::logging::LogLevel::Info,
        true
    );

    try {
        logger->info("Testing Results Export Functionality...");
        
        // Create a simple statically determinate truss system
        Truss truss("Export Test Truss");
        auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);    // Left support (2 constraints)
        auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);   // Right support (1 constraint)
        auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);      // Top node (0 constraints)

        // Add members to form a triangle
        truss.addMember(node1, node2);  // Bottom member
        truss.addMember(node1, node3);  // Left member  
        truss.addMember(node2, node3);  // Right member

        truss.applyForce(node3->getId(), Force2D(0.0, -15000.0)); // Apply downward load

        {
            std::ostringstream oss;
            oss << "Created truss with " << truss.getNodeCount()
                << " nodes and " << truss.getMemberCount() << " members";
            logger->info(oss.str());
        }

        // Perform analysis
        AnalysisEngine engine;
        auto results = engine.analyze(truss);

        if (!results.converged) {
            logger->error("Analysis did not converge");
            return 1;
        }

        logger->info("Analysis completed successfully!");
        {
            std::ostringstream oss;
            oss << "Max displacement: " << results.maxDisplacement;
            logger->info(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "Max stress: " << results.maxStress;
            logger->info(oss.str());
        }

        // Test exports to different formats
        ResultsExporter exporter;
        ExportOptions options;
        options.includeGeometry = true;
        options.includeDisplacements = true;
        options.includeMemberForces = true;
        options.includeReactions = true;
        options.includeMetadata = true;
        options.precision = 6;

        std::vector<std::pair<std::string, ExportFormat>> testFormats = {
            {"test_results.csv", ExportFormat::CSV},
            {"test_results.json", ExportFormat::JSON},
            {"test_results.txt", ExportFormat::TXT},
            {"test_results.xml", ExportFormat::XML},
            {"test_results.html", ExportFormat::HTML},
            {"test_results.tex", ExportFormat::LaTeX}
        };

        bool allSuccessful = true;
        for (const auto& [fileName, format] : testFormats) {
            logger->info(std::string("Exporting to ") + fileName + "...");
            
            if (exporter.exportResults(truss, results, fileName, format, options)) {
                logger->info("SUCCESS");
                
                // Check if file exists and has content
                if (std::filesystem::exists(fileName)) {
                    auto fileSize = std::filesystem::file_size(fileName);
                    {
                        std::ostringstream oss;
                        oss << "File size: " << fileSize << " bytes";
                        logger->info(oss.str());
                    }
                    if (fileSize == 0) {
                        logger->warn("File is empty");
                    }
                } else {
                    logger->error("File was not created");
                    allSuccessful = false;
                }
            } else {
                logger->error(std::string("Export failed: ") + exporter.getLastError());
                allSuccessful = false;
            }
        }

        logger->info("=== Export Test Summary ===");
        if (allSuccessful) {
            logger->info("All export formats tested successfully!");
            logger->info("Generated files:");
            for (const auto& [fileName, format] : testFormats) {
                if (std::filesystem::exists(fileName)) {
                    std::ostringstream oss;
                    oss << "- " << fileName << " (" 
                        << std::filesystem::file_size(fileName) << " bytes)";
                    logger->info(oss.str());
                }
            }
        } else {
            logger->error("Some export tests failed!");
            return 1;
        }

        // Test format detection
        logger->info("=== Testing Format Detection ===");
        std::vector<std::pair<std::string, ExportFormat>> formatTests = {
            {"file.csv", ExportFormat::CSV},
            {"file.tsv", ExportFormat::TSV},
            {"file.json", ExportFormat::JSON},
            {"file.xml", ExportFormat::XML},
            {"file.txt", ExportFormat::TXT},
            {"file.tex", ExportFormat::LaTeX},
            {"file.html", ExportFormat::HTML}
        };

        for (const auto& [fileName, expectedFormat] : formatTests) {
            auto detectedFormat = ResultsExporter::detectFormat(fileName);
            if (detectedFormat == expectedFormat) {
                logger->info(fileName + " -> Correct");
            } else {
                std::ostringstream oss;
                oss << fileName << " -> Wrong (expected " << static_cast<int>(expectedFormat)
                    << ", got " << static_cast<int>(detectedFormat) << ")";
                logger->error(oss.str());
                allSuccessful = false;
            }
        }

        // Test summary generation
        logger->info("=== Testing Summary Generation ===");
        std::string summary = exporter.generateSummary(truss, results);
        {
            std::ostringstream oss;
            oss << "Generated summary (" << summary.length() << " characters)";
            logger->info(oss.str());
        }
        logger->info(summary);

        logger->info("=== Final Result ===");
        if (allSuccessful) {
            logger->info("ALL TESTS PASSED! Results export feature is working correctly.");
            return 0;
        } else {
            logger->error("SOME TESTS FAILED! Check the output above for details.");
            return 1;
        }

    } catch (const std::exception& e) {
        logger->error(std::string("Export test failed: ") + e.what());
        return 1;
    }
}
