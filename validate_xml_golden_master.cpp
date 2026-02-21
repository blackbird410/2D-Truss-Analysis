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
#include "src/infrastructure/logging/logger_factory.hpp"
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
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerX);
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
bool compareXMLFiles(const std::string& file1,
                     const std::string& file2,
                     truss::infrastructure::logging::ILogger& logger) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);
    
    if (!f1.is_open() || !f2.is_open()) {
        logger.error("Cannot open files for comparison");
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
            std::ostringstream oss;
            oss << "Line " << lineNum << ": Skipping timestamp comparison";
            logger.info(oss.str());
            continue;
        }
        
        if (line1 != line2) {
            differences++;
            {
                std::ostringstream oss;
                oss << "Line " << lineNum << " differs:";
                logger.warn(oss.str());
            }
            logger.warn(std::string("Golden: ") + line1);
            logger.warn(std::string("Output: ") + line2);
            
            if (differences > 10) {
                logger.warn("More than 10 differences, stopping comparison");
                return false;
            }
        }
    }
    
    // Check if one file has more lines
    if (std::getline(f1, line1) || std::getline(f2, line2)) {
        logger.warn("Files have different number of lines");
        return false;
    }
    
    if (differences == 0) {
        logger.info("Files are identical (excluding timestamps)");
        return true;
    } else {
        std::ostringstream oss;
        oss << "Files differ in " << differences << " line(s)";
        logger.warn(oss.str());
        return false;
    }
}

int main() {
    auto logger = truss::infrastructure::logging::LoggerFactory::createConsoleLogger(
        truss::infrastructure::logging::LogLevel::Info,
        true
    );

    logger->info("Golden Master XML Validation");
    logger->info("============================");
    
    // Step 1: Create test truss
    logger->info("Step 1: Creating test truss...");
    auto truss = createSimpleTriangleTruss();
    {
        std::ostringstream oss;
        oss << "Nodes: " << truss->getNodeCount();
        logger->info(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "Members: " << truss->getMemberCount();
        logger->info(oss.str());
    }
    
    // Step 2: Run analysis
    logger->info("Step 2: Running structural analysis...");
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
    auto results = orchestrator.analyze(*truss);
    
    if (!results.converged) {
        logger->error("Analysis did not converge!");
        return 1;
    }
    
    logger->info("Analysis converged successfully!");
    {
        std::ostringstream oss;
        oss << "Max displacement: " << results.maxDisplacement << " m";
        logger->info(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "Max stress: " << results.maxStress << " Pa";
        logger->info(oss.str());
    }
    
    // Step 3: Export with XMLExporter
    logger->info("Step 3: Exporting with XMLExporter...");
    
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
        logger->error(std::string("Export failed: ") + exporter.getLastError());
        return 1;
    }
    
    logger->info(std::string("Export successful: ") + outputPath);
    {
        std::ostringstream oss;
        oss << "File size: " << fs::file_size(outputPath) << " bytes";
        logger->info(oss.str());
    }
    
    // Step 4: Compare with golden master
    logger->info("Step 4: Comparing with golden master...");
    std::string goldenPath = "tests/fixtures/export_golden/golden_master.xml";
    
    if (!fs::exists(goldenPath)) {
        logger->error(std::string("Golden master file not found: ") + goldenPath);
        return 1;
    }
    
    bool identical = compareXMLFiles(goldenPath, outputPath, *logger);
    
    if (identical) {
        logger->info("Validation Result: PASSED (output matches golden master)");
        
        // Clean up test file
        fs::remove(outputPath);
        return 0;
    } else {
        logger->error("Validation Result: FAILED (output differs from golden master)");
        logger->warn(std::string("Test file preserved for inspection: ") + outputPath);
        logger->warn(std::string("Golden master: ") + goldenPath);
        logger->info(std::string("Run diff for detailed comparison: diff ") + goldenPath + " " + outputPath);
        return 1;
    }
}
