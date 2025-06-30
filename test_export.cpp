/**
 * @file test_export.cpp
 * @brief Test for the ResultsExporter functionality
 */

#include "AnalysisEngine.hpp"
#include "ResultsExporter.hpp"
#include "Truss.hpp"
#include <iostream>
#include <filesystem>

using namespace truss::core;

int main() {
    try {
        std::cout << "Testing Results Export Functionality...\n\n";
        
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

        std::cout << "Created truss with " << truss.getNodeCount() 
                  << " nodes and " << truss.getMemberCount() << " members\n";

        // Perform analysis
        AnalysisEngine engine;
        auto results = engine.analyze(truss);

        if (!results.converged) {
            std::cerr << "ERROR: Analysis did not converge\n";
            return 1;
        }

        std::cout << "Analysis completed successfully!\n";
        std::cout << "Max displacement: " << results.maxDisplacement << "\n";
        std::cout << "Max stress: " << results.maxStress << "\n\n";

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
            std::cout << "Exporting to " << fileName << "... ";
            
            if (exporter.exportResults(truss, results, fileName, format, options)) {
                std::cout << "✅ SUCCESS\n";
                
                // Check if file exists and has content
                if (std::filesystem::exists(fileName)) {
                    auto fileSize = std::filesystem::file_size(fileName);
                    std::cout << "   File size: " << fileSize << " bytes\n";
                    if (fileSize == 0) {
                        std::cout << "   ⚠️ WARNING: File is empty\n";
                    }
                } else {
                    std::cout << "   ❌ ERROR: File was not created\n";
                    allSuccessful = false;
                }
            } else {
                std::cout << "❌ FAILED\n";
                std::cout << "   Error: " << exporter.getLastError() << "\n";
                allSuccessful = false;
            }
        }

        std::cout << "\n=== Export Test Summary ===\n";
        if (allSuccessful) {
            std::cout << "✅ All export formats tested successfully!\n";
            std::cout << "Generated files:\n";
            for (const auto& [fileName, format] : testFormats) {
                if (std::filesystem::exists(fileName)) {
                    std::cout << "  - " << fileName << " (" 
                              << std::filesystem::file_size(fileName) << " bytes)\n";
                }
            }
        } else {
            std::cout << "❌ Some export tests failed!\n";
            return 1;
        }

        // Test format detection
        std::cout << "\n=== Testing Format Detection ===\n";
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
            std::cout << fileName << " -> ";
            if (detectedFormat == expectedFormat) {
                std::cout << "✅ Correct\n";
            } else {
                std::cout << "❌ Wrong (expected " << static_cast<int>(expectedFormat) 
                          << ", got " << static_cast<int>(detectedFormat) << ")\n";
                allSuccessful = false;
            }
        }

        // Test summary generation
        std::cout << "\n=== Testing Summary Generation ===\n";
        std::string summary = exporter.generateSummary(truss, results);
        std::cout << "Generated summary (" << summary.length() << " characters):\n";
        std::cout << summary << "\n";

        std::cout << "\n=== Final Result ===\n";
        if (allSuccessful) {
            std::cout << "🎉 ALL TESTS PASSED! Results export feature is working correctly.\n";
            return 0;
        } else {
            std::cout << "❌ SOME TESTS FAILED! Check the output above for details.\n";
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Export test failed: " << e.what() << "\n";
        return 1;
    }
}
