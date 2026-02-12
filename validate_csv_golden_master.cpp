/**
 * @file validate_csv_golden_master.cpp
 * @brief Validate CSVExporter output against golden master
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "src/infrastructure/export/csv_exporter.hpp"
#include "src/infrastructure/export/export_types.hpp"
#include "src/core/model/Truss.hpp"
#include "src/core/analysis/AnalysisOrchestrator.hpp"
#include "src/core/analysis/SolverFactory.hpp"
#include "src/core/Logger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;
namespace fs = std::filesystem;

/**
 * @brief Create the same simple triangle truss as golden master generator
 */
std::unique_ptr<Truss> createSimpleTriangleTruss() {
    auto truss = std::make_unique<Truss>("Golden Master Test Truss");
    
    // Create nodes (same as golden master)
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);   // Left support
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerY);  // Right support
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);     // Top node
    
    // Create members
    truss->addMember(node1, node2);  // Horizontal
    truss->addMember(node1, node3);  // Left diagonal
    truss->addMember(node2, node3);  // Right diagonal
    
    // Apply load at top node (15 kN downward)
    node3->setAppliedForce(0.0, -15000.0);
    
    return truss;
}

/**
 * @brief Read file contents
 */
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief Compare two files line by line, ignoring timestamp differences
 */
bool compareFiles(const std::string& file1, const std::string& file2, bool ignoreTimestamps = true) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);
    
    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "Cannot open files for comparison" << std::endl;
        return false;
    }
    
    std::string line1, line2;
    int lineNum = 0;
    int differences = 0;
    
    while (std::getline(f1, line1) && std::getline(f2, line2)) {
        lineNum++;
        
        // Skip timestamp lines if requested
        if (ignoreTimestamps && line1.find("Generated:") != std::string::npos) {
            std::cout << "  Line " << lineNum << ": Skipping timestamp comparison" << std::endl;
            continue;
        }
        
        if (line1 != line2) {
            differences++;
            std::cout << "  Line " << lineNum << " differs:" << std::endl;
            std::cout << "    Golden: " << line1 << std::endl;
            std::cout << "    Output: " << line2 << std::endl;
            
            if (differences > 10) {
                std::cout << "  ... (more than 10 differences, stopping comparison)" << std::endl;
                return false;
            }
        }
    }
    
    // Check if one file has more lines than the other
    if (std::getline(f1, line1) || std::getline(f2, line2)) {
        std::cout << "  Files have different number of lines" << std::endl;
        return false;
    }
    
    if (differences == 0) {
        std::cout << "  ✓ Files are identical (excluding timestamps)" << std::endl;
        return true;
    } else {
        std::cout << "  ✗ Files differ in " << differences << " line(s)" << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Golden Master CSV Validation" << std::endl;
    std::cout << "=============================" << std::endl << std::endl;
    
    // Step 1: Create test truss
    std::cout << "Step 1: Creating test truss..." << std::endl;
    auto truss = createSimpleTriangleTruss();
    std::cout << "  Nodes: " << truss->getNodeCount() << std::endl;
    std::cout << "  Members: " << truss->getMemberCount() << std::endl << std::endl;
    
    // Step 2: Run analysis
    std::cout << "Step 2: Running structural analysis..." << std::endl;
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
    auto results = orchestrator.analyze(*truss);
    
    if (!results.converged) {
        std::cerr << "ERROR: Analysis did not converge!" << std::endl;
        return 1;
    }
    
    std::cout << "  Analysis converged successfully!" << std::endl;
    std::cout << "  Max displacement: " << results.maxDisplacement << " m" << std::endl;
    std::cout << "  Max stress: " << results.maxStress << " Pa" << std::endl << std::endl;
    
    // Step 3: Export with CSVExporter
    std::cout << "Step 3: Exporting with CSVExporter..." << std::endl;
    
    CSVExporter exporter;
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
    options.delimiter = ",";
    
    std::string outputPath = "test_golden_validation.csv";
    bool success = exporter.exportResults(*truss, results, outputPath, options);
    
    if (!success) {
        std::cerr << "ERROR: Export failed: " << exporter.getLastError() << std::endl;
        return 1;
    }
    
    std::cout << "  Export successful: " << outputPath << std::endl;
    std::cout << "  File size: " << fs::file_size(outputPath) << " bytes" << std::endl << std::endl;
    
    // Step 4: Compare with golden master
    std::cout << "Step 4: Comparing with golden master..." << std::endl;
    std::string goldenPath = "tests/fixtures/export_golden/golden_master.csv";
    
    if (!fs::exists(goldenPath)) {
        std::cerr << "ERROR: Golden master file not found: " << goldenPath << std::endl;
        return 1;
    }
    
    bool identical = compareFiles(goldenPath, outputPath, true);
    
    std::cout << std::endl;
    std::cout << "Validation Result: ";
    if (identical) {
        std::cout << "✓ PASSED (output matches golden master)" << std::endl;
        
        // Clean up test file
        fs::remove(outputPath);
        return 0;
    } else {
        std::cout << "✗ FAILED (output differs from golden master)" << std::endl;
        std::cout << "  Test file preserved for inspection: " << outputPath << std::endl;
        std::cout << "  Golden master: " << goldenPath << std::endl;
        std::cout << std::endl;
        std::cout << "Run diff for detailed comparison:" << std::endl;
        std::cout << "  diff " << goldenPath << " " << outputPath << std::endl;
        return 1;
    }
}
