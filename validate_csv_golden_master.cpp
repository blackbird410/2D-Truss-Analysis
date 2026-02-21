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
#include "src/infrastructure/logging/logger_factory.hpp"
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
bool compareFiles(const std::string& file1,
                  const std::string& file2,
                  truss::infrastructure::logging::ILogger& logger,
                  bool ignoreTimestamps = true) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);
    
    if (!f1.is_open() || !f2.is_open()) {
        logger.error("Cannot open files for comparison");
        return false;
    }
    
    std::string line1, line2;
    int lineNum = 0;
    int differences = 0;
    
    while (std::getline(f1, line1) && std::getline(f2, line2)) {
        lineNum++;
        
        // Skip timestamp lines if requested
        if (ignoreTimestamps && line1.find("Generated:") != std::string::npos) {
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
    
    // Check if one file has more lines than the other
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

    logger->info("Golden Master CSV Validation");
    logger->info("=============================");
    
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
    
    // Step 3: Export with CSVExporter
    logger->info("Step 3: Exporting with CSVExporter...");
    
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
    std::string goldenPath = "tests/fixtures/export_golden/golden_master.csv";
    
    if (!fs::exists(goldenPath)) {
        logger->error(std::string("Golden master file not found: ") + goldenPath);
        return 1;
    }
    
    bool identical = compareFiles(goldenPath, outputPath, *logger, true);
    
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
