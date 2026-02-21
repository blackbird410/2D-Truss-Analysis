/**
 * @file test_csv_exporter.cpp
 * @brief Unit tests for CSV exporter
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "core/analysis/AnalysisOrchestrator.hpp"
#include "core/analysis/SolverFactory.hpp"
#include "core/model/Truss.hpp"
#include "infrastructure/export/csv_exporter.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

/**
 * @brief Test fixture for CSV exporter tests
 */
class CSVExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        exporter = std::make_unique<CSVExporter>();
        testOutputDir = "test_output";
        fs::create_directories(testOutputDir);
    }

    void TearDown() override {
        // Clean up test output files
        if (fs::exists(testOutputDir)) {
            fs::remove_all(testOutputDir);
        }
    }

    /**
     * @brief Create a simple triangle truss for testing
     */
    std::unique_ptr<Truss> createSimpleTriangleTruss() {
        auto truss = std::make_unique<Truss>("Test Triangle Truss");

        // Create nodes
        auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);  // Left support
        auto node2 = truss->addNode(
            4.0, 0.0, SupportType::RollerX);                       // Right support (Y constrained)
        auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);  // Top node

        // Create members
        truss->addMember(node1, node2);  // Bottom horizontal
        truss->addMember(node1, node3);  // Left diagonal
        truss->addMember(node2, node3);  // Right diagonal

        // Apply load at top node (15 kN downward)
        node3->setAppliedForce(0.0, -15000.0);

        return truss;
    }

    /**
     * @brief Run analysis on truss
     */
    AnalysisResults analyzeAndGetResults(Truss& truss) {
        auto solver = SolverFactory::createDirectSolver();
        AnalysisOrchestrator orchestrator(std::move(solver),
                                          std::make_unique<validation::TrussValidator>());
        return orchestrator.analyze(truss);
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
     * @brief Check if file contains text
     */
    bool fileContains(const std::string& path, const std::string& text) {
        std::string content = readFile(path);
        return content.find(text) != std::string::npos;
    }

    std::unique_ptr<CSVExporter> exporter;
    std::string testOutputDir;
};

/**
 * @brief Test basic CSV export functionality
 */
TEST_F(CSVExporterTest, BasicExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/basic_export.csv";
    ExportOptions options;

    bool success = exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(success) << "Export should succeed";
    EXPECT_TRUE(fs::exists(outputPath)) << "Output file should exist";
    EXPECT_GT(fs::file_size(outputPath), 0) << "Output file should not be empty";
}

/**
 * @brief Test CSV header generation
 */
TEST_F(CSVExporterTest, HeaderGeneration) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/header_test.csv";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "2D Truss Analysis Results Export"));
    EXPECT_TRUE(fileContains(outputPath, "Generated:"));
    EXPECT_TRUE(fileContains(outputPath, "Project: Test Triangle Truss"));
}

/**
 * @brief Test geometry section export
 */
TEST_F(CSVExporterTest, GeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/geometry_test.csv";
    ExportOptions options;
    options.includeGeometry = true;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    EXPECT_TRUE(fileContains(outputPath, "# GEOMETRY"));
    EXPECT_TRUE(fileContains(outputPath, "Node ID,X,Y,Support Type"));
    EXPECT_TRUE(fileContains(outputPath, "Member ID,Start Node,End Node,Length"));

    // Check node data
    EXPECT_TRUE(fileContains(outputPath, "1,0.000000,0.000000,1"));  // Node 1: Pinned (type 1)
    EXPECT_TRUE(fileContains(outputPath, "2,4.000000,0.000000,2"));  // Node 2: RollerX (type 2)
    EXPECT_TRUE(fileContains(outputPath, "3,2.000000,3.000000,0"));  // Node 3: Free (type 0)

    // Check member data
    EXPECT_TRUE(fileContains(outputPath, "1,1,2,4.000000"));  // Member 1: horizontal
}

/**
 * @brief Test displacements section export
 */
TEST_F(CSVExporterTest, DisplacementsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/displacements_test.csv";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = true;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "# NODAL DISPLACEMENTS"));
    EXPECT_TRUE(fileContains(outputPath, "DOF,Displacement"));

    // Should have displacements for all DOFs
    std::string content = readFile(outputPath);
    EXPECT_GT(content.find("0,"), 0) << "Should have DOF 0";
}

/**
 * @brief Test TSV format (tab delimiter)
 */
TEST_F(CSVExporterTest, TSVFormat) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/tsv_test.tsv";
    ExportOptions options;
    options.delimiter = "\t";  // Tab delimiter for TSV

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Check for tab delimiters
    EXPECT_TRUE(content.find("\t") != std::string::npos) << "Should contain tab delimiters";
    EXPECT_TRUE(fileContains(outputPath, "Node ID\tX\tY\tSupport Type"));
}

/**
 * @brief Test precision option
 */
TEST_F(CSVExporterTest, PrecisionOption) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    // Test with 2 decimal places
    std::string outputPath2 = testOutputDir + "/precision_2.csv";
    ExportOptions options2;
    options2.precision = 2;
    exporter->exportResults(*truss, results, outputPath2, options2);

    std::string content2 = readFile(outputPath2);
    EXPECT_TRUE(content2.find("4.00") != std::string::npos) << "Should have 2 decimal places";

    // Test with 8 decimal places
    std::string outputPath8 = testOutputDir + "/precision_8.csv";
    ExportOptions options8;
    options8.precision = 8;
    exporter->exportResults(*truss, results, outputPath8, options8);

    std::string content8 = readFile(outputPath8);
    EXPECT_TRUE(content8.find("4.00000000") != std::string::npos) << "Should have 8 decimal places";
}

/**
 * @brief Test scientific notation option
 */
TEST_F(CSVExporterTest, ScientificNotation) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/scientific_test.csv";
    ExportOptions options;
    options.useScientificNotation = true;
    options.precision = 6;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Should contain scientific notation (e.g., "4.000000e+00")
    EXPECT_TRUE(content.find("e+") != std::string::npos || content.find("e-") != std::string::npos)
        << "Should contain scientific notation";
}

/**
 * @brief Test selective section export
 */
TEST_F(CSVExporterTest, SelectiveSections) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/selective_test.csv";
    ExportOptions options;
    options.includeGeometry = true;
    options.includeProperties = false;
    options.includeLoads = false;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Should have geometry
    EXPECT_TRUE(fileContains(outputPath, "# GEOMETRY"));

    // Should NOT have other sections
    EXPECT_FALSE(fileContains(outputPath, "# NODAL DISPLACEMENTS"));
    EXPECT_FALSE(fileContains(outputPath, "# MEMBER FORCES"));
    EXPECT_FALSE(fileContains(outputPath, "# ANALYSIS METADATA"));
}

/**
 * @brief Test error handling for invalid file path
 */
TEST_F(CSVExporterTest, InvalidFilePath) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string invalidPath = "/invalid/path/that/does/not/exist/output.csv";
    ExportOptions options;

    bool success = exporter->exportResults(*truss, results, invalidPath, options);

    EXPECT_FALSE(success) << "Export should fail for invalid path";
    EXPECT_FALSE(exporter->getLastError().empty()) << "Should have error message";
}

/**
 * @brief Test getFormat() method
 */
TEST_F(CSVExporterTest, GetFormat) {
    EXPECT_EQ(exporter->getFormat(), ExportFormat::CSV);
}

/**
 * @brief Test member forces section
 */
TEST_F(CSVExporterTest, MemberForcesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/forces_test.csv";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = true;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "# MEMBER FORCES"));
    EXPECT_TRUE(fileContains(outputPath, "Member ID,Axial Force,Type"));

    std::string content = readFile(outputPath);

    // Should have "Tension" or "Compression" labels
    bool hasTension = content.find("Tension") != std::string::npos;
    bool hasCompression = content.find("Compression") != std::string::npos;
    EXPECT_TRUE(hasTension || hasCompression) << "Should have force type labels";
}

/**
 * @brief Test metadata section
 */
TEST_F(CSVExporterTest, MetadataSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/metadata_test.csv";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = true;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "# ANALYSIS METADATA"));
    EXPECT_TRUE(fileContains(outputPath, "Property,Value"));
    EXPECT_TRUE(fileContains(outputPath, "Converged,"));
    EXPECT_TRUE(fileContains(outputPath, "Total DOFs,"));
}
