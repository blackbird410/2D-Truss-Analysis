/**
 * @file validate_xml_golden_master.cpp
 * @brief Validate XMLExporter output against golden master
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "src/infrastructure/export/xml_exporter.hpp"
#include "src/infrastructure/export/export_types.hpp"
#include "src/core/model/Truss.hpp"
#include "src/core/analysis/AnalysisOrchestrator.hpp"
#include "src/core/analysis/SolverFactory.hpp"
#include "src/core/Logger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

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
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);
    
    // Create members
    truss->addMember(node1, node2);
    truss->addMember(node1, node3);
    truss->addMember(node2, node3);
    
    // Apply load at top node (15 kN downward)
    node3->setAppliedForce(0.0, -15000.0);
    
    return truss;
}

/**
 * @brief Compare two XML files, ignoring timestamp differences
 */
bool compareXMLFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);
    
    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "Cannot open files for comparison" << std::endl;
        return false;
    }
    
    std::string line1, line2;
    int lineNum = 0;
    int differences = 0;
    
    // Regex to match timestamp lines
    std::regex timestampRegex(R"(\s*<ExportTime>[^<]+</ExportTime>)");
    
    while (std::getline(f1, line1) && std::getline(f2, line2)) {
        lineNum++;
        
        // Skip timestamp comparison
        if (std::regex_search(line1, timestampRegex) && 
            std::regex_search(line2, timestampRegex)) {
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
    
    // Check if one file has more lines
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
    std::cout << "Golden Master XML Validation" << std::endl;
    std::cout << "============================" << std::endl << std::endl;
    
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
    
    // Step 3: Export with XMLExporter
    std::cout << "Step 3: Exporting with XMLExporter..." << std::endl;
    
    XMLExporter exporter;
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
    
    std::string outputPath = "test_golden_validation.xml";
    bool success = exporter.exportResults(*truss, results, outputPath, options);
    
    if (!success) {
        std::cerr << "ERROR: Export failed: " << exporter.getLastError() << std::endl;
        return 1;
    }
    
    std::cout << "  Export successful: " << outputPath << std::endl;
    std::cout << "  File size: " << fs::file_size(outputPath) << " bytes" << std::endl << std::endl;
    
    // Step 4: Compare with golden master
    std::cout << "Step 4: Comparing with golden master..." << std::endl;
    std::string goldenPath = "tests/fixtures/export_golden/golden_master.xml";
    
    if (!fs::exists(goldenPath)) {
        std::cerr << "ERROR: Golden master file not found: " << goldenPath << std::endl;
        return 1;
    }
    
    bool identical = compareXMLFiles(goldenPath, outputPath);
    
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
