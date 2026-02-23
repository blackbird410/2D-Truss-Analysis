/**
 * @file test_json_exporter.cpp
 * @brief Unit tests for JSON exporter
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "core/analysis/analysis_orchestrator.hpp"
#include "core/analysis/solver_factory.hpp"
#include "core/model/truss.hpp"
#include "infrastructure/export/json_exporter.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

/**
 * @brief Test fixture for JSON exporter tests
 */
class JSONExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        exporter = std::make_unique<JSONExporter>();
        testOutputDir = "test_output_json";
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

    /**
     * @brief Check if file contains valid JSON structure
     */
    bool isValidJSON(const std::string& path) {
        std::string content = readFile(path);
        if (content.empty())
            return false;

        // Basic JSON structure validation
        int braceCount = 0;
        int bracketCount = 0;
        bool inString = false;
        char prevChar = '\0';

        for (char c : content) {
            if (c == '"' && prevChar != '\\') {
                inString = !inString;
            }
            if (!inString) {
                if (c == '{')
                    braceCount++;
                if (c == '}')
                    braceCount--;
                if (c == '[')
                    bracketCount++;
                if (c == ']')
                    bracketCount--;
            }
            prevChar = c;
        }

        return braceCount == 0 && bracketCount == 0;
    }

    /**
     * @brief Count occurrences of a string in file
     */
    int countOccurrences(const std::string& path, const std::string& text) {
        std::string content = readFile(path);
        int count = 0;
        size_t pos = 0;
        while ((pos = content.find(text, pos)) != std::string::npos) {
            count++;
            pos += text.length();
        }
        return count;
    }

    std::unique_ptr<JSONExporter> exporter;
    std::string testOutputDir;
};

/**
 * @brief Test basic JSON export functionality
 */
TEST_F(JSONExporterTest, BasicExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/basic_export.json";
    ExportOptions options;

    bool success = exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(success) << "Export should succeed";
    EXPECT_TRUE(fs::exists(outputPath)) << "Output file should exist";
    EXPECT_GT(fs::file_size(outputPath), 0) << "Output file should not be empty";
    EXPECT_TRUE(isValidJSON(outputPath)) << "Output should be valid JSON";
}

/**
 * @brief Test JSON project section
 */
TEST_F(JSONExporterTest, ProjectSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/project_test.json";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "\"project\""));
    EXPECT_TRUE(fileContains(outputPath, "\"name\": \"Test Triangle Truss\""));
    EXPECT_TRUE(fileContains(outputPath, "\"exportTime\""));
    EXPECT_TRUE(fileContains(outputPath, "\"version\": \"3.0.0\""));
}

/**
 * @brief Test geometry section structure
 */
TEST_F(JSONExporterTest, GeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/geometry_test.json";
    ExportOptions options;
    options.includeGeometry = true;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Check structure
    EXPECT_TRUE(fileContains(outputPath, "\"geometry\""));
    EXPECT_TRUE(fileContains(outputPath, "\"nodes\""));
    EXPECT_TRUE(fileContains(outputPath, "\"members\""));

    // Check node data
    EXPECT_TRUE(fileContains(outputPath, "\"id\": 1"));
    EXPECT_TRUE(fileContains(outputPath, "\"x\": 0.000000"));
    EXPECT_TRUE(fileContains(outputPath, "\"y\": 0.000000"));
    EXPECT_TRUE(fileContains(outputPath, "\"supportType\": \"1\""));  // Pinned

    EXPECT_TRUE(fileContains(outputPath, "\"id\": 2"));
    EXPECT_TRUE(fileContains(outputPath, "\"x\": 4.000000"));
    EXPECT_TRUE(fileContains(outputPath, "\"supportType\": \"2\""));  // RollerX

    EXPECT_TRUE(fileContains(outputPath, "\"id\": 3"));
    EXPECT_TRUE(fileContains(outputPath, "\"y\": 3.000000"));
    EXPECT_TRUE(fileContains(outputPath, "\"supportType\": \"0\""));  // Free

    // Check member data
    EXPECT_TRUE(fileContains(outputPath, "\"startNode\": 1"));
    EXPECT_TRUE(fileContains(outputPath, "\"endNode\": 2"));
    EXPECT_TRUE(fileContains(outputPath, "\"length\": 4.000000"));
}

/**
 * @brief Test displacements section structure
 */
TEST_F(JSONExporterTest, DisplacementsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/displacements_test.json";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = true;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "\"displacements\""));
    EXPECT_TRUE(fileContains(outputPath, "\"values\""));
    EXPECT_TRUE(fileContains(outputPath, "\"maxDisplacement\""));

    // Should have array of displacement values
    std::string content = readFile(outputPath);
    EXPECT_TRUE(content.find("\"values\": [") != std::string::npos);
}

/**
 * @brief Test member forces section structure
 */
TEST_F(JSONExporterTest, MemberForcesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/forces_test.json";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = true;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "\"memberForces\""));
    EXPECT_TRUE(fileContains(outputPath, "\"values\""));

    // Should have array of force values
    std::string content = readFile(outputPath);
    EXPECT_TRUE(content.find("\"values\": [") != std::string::npos);
}

/**
 * @brief Test metadata (analysis) section structure
 */
TEST_F(JSONExporterTest, MetadataSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/metadata_test.json";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = true;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "\"analysis\""));
    EXPECT_TRUE(fileContains(outputPath, "\"converged\""));
    EXPECT_TRUE(fileContains(outputPath, "\"iterations\""));
    EXPECT_TRUE(fileContains(outputPath, "\"totalDofs\""));
    EXPECT_TRUE(fileContains(outputPath, "\"freeDofs\""));
    EXPECT_TRUE(fileContains(outputPath, "\"maxStress\""));

    // Verify boolean value (not string)
    std::string content = readFile(outputPath);
    EXPECT_TRUE(content.find("\"converged\": true") != std::string::npos ||
                content.find("\"converged\": false") != std::string::npos)
        << "Converged should be boolean, not string";
}

/**
 * @brief Test that reactions section is NOT included (legacy behavior)
 */
/**
 * @brief Test reactions section (CORRECTNESS FIX)
 * Legacy behavior omitted reactions - this was incorrect.
 * Reactions data is mandatory for semantic equivalence with CSV.
 */
TEST_F(JSONExporterTest, ReactionsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/reactions_test.json";
    ExportOptions options;
    options.includeReactions = true;

    exporter->exportResults(*truss, results, outputPath, options);

    // JSON export MUST include reactions (correctness requirement)
    EXPECT_TRUE(fileContains(outputPath, "\"reactions\""))
        << "JSON export MUST include reactions section for data completeness";
    EXPECT_TRUE(fileContains(outputPath, "\"values\""))
        << "Reactions section must contain values array";
}

/**
 * @brief Test properties section (CONTRACT COMPLETENESS)
 *
 * Material properties section is REQUIRED for 8-section export contract.
 * Domain model provides complete material and section data through
 * ITrussView::getMemberViews().
 */
TEST_F(JSONExporterTest, PropertiesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/properties_test.json";
    ExportOptions options;
    options.includeProperties = true;

    exporter->exportResults(*truss, results, outputPath, options);

    // Properties section must be present with real data
    EXPECT_TRUE(fileContains(outputPath, "\"properties\""))
        << "JSON export MUST include properties section";
    EXPECT_TRUE(fileContains(outputPath, "\"members\"")) << "Properties must include members array";
    // Material properties are directly in member objects (flat format)
    EXPECT_TRUE(fileContains(outputPath, "\"youngModulus\""))
        << "Properties must include Young's modulus";
    EXPECT_TRUE(fileContains(outputPath, "\"yieldStrength\""))
        << "Properties must include yield strength";
    EXPECT_TRUE(fileContains(outputPath, "\"density\"")) << "Properties must include density";
    EXPECT_TRUE(fileContains(outputPath, "\"area\""))
        << "Properties must include cross-sectional area";
}

/**
 * @brief Test loads section (CONTRACT COMPLETENESS)
 *
 * Applied loads section is REQUIRED for 8-section export contract.
 * Domain model provides load data through Node::getAppliedForce().
 */
TEST_F(JSONExporterTest, LoadsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/loads_test.json";
    ExportOptions options;
    options.includeLoads = true;

    exporter->exportResults(*truss, results, outputPath, options);

    // Loads section must be present with real data
    EXPECT_TRUE(fileContains(outputPath, "\"loads\"")) << "JSON export MUST include loads section";
    EXPECT_TRUE(fileContains(outputPath, "\"nodalForces\""))
        << "Loads must include nodal forces array";
}

/**
 * @brief Test precision option
 */
TEST_F(JSONExporterTest, PrecisionOption) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    // Export with precision 2
    std::string outputPath1 = testOutputDir + "/precision2_test.json";
    ExportOptions options1;
    options1.precision = 2;
    exporter->exportResults(*truss, results, outputPath1, options1);

    // Export with precision 8
    std::string outputPath2 = testOutputDir + "/precision8_test.json";
    ExportOptions options2;
    options2.precision = 8;
    exporter->exportResults(*truss, results, outputPath2, options2);

    std::string content1 = readFile(outputPath1);
    std::string content2 = readFile(outputPath2);

    // Higher precision file should be larger (more decimal places)
    EXPECT_GT(content2.size(), content1.size())
        << "Higher precision should result in more decimal places";

    // Check for precision in numbers
    // Precision 2: should have values like "4.00"
    // Precision 8: should have values like "4.00000000"
    std::regex precisionRegex2(R"(\d+\.\d{2}[^\d])");
    std::regex precisionRegex8(R"(\d+\.\d{8})");

    EXPECT_TRUE(std::regex_search(content1, precisionRegex2))
        << "Should find 2-decimal precision values";
    EXPECT_TRUE(std::regex_search(content2, precisionRegex8))
        << "Should find 8-decimal precision values";
}

/**
 * @brief Test scientific notation option
 */
TEST_F(JSONExporterTest, ScientificNotation) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/scientific_test.json";
    ExportOptions options;
    options.useScientificNotation = true;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Should contain scientific notation (e+ or e-)
    EXPECT_TRUE(content.find("e+") != std::string::npos || content.find("e-") != std::string::npos)
        << "Should contain scientific notation";
}

/**
 * @brief Test selective section export
 */
TEST_F(JSONExporterTest, SelectiveSections) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/selective_test.json";
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

    // Should have project (always included) and geometry
    EXPECT_TRUE(fileContains(outputPath, "\"project\""));
    EXPECT_TRUE(fileContains(outputPath, "\"geometry\""));

    // Should NOT have other sections
    EXPECT_FALSE(fileContains(outputPath, "\"displacements\""));
    EXPECT_FALSE(fileContains(outputPath, "\"memberForces\""));
    EXPECT_FALSE(fileContains(outputPath, "\"analysis\""));
}

/**
 * @brief Test JSON string escaping
 */
TEST_F(JSONExporterTest, StringEscaping) {
    // Create truss with special characters in name
    auto truss = std::make_unique<Truss>("Test\"Truss\\With\nSpecial\tChars");

    // Create stable truss structure (3 nodes, 3 members forming triangle)
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerX);
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);

    truss->addMember(node1, node2);  // Bottom horizontal
    truss->addMember(node1, node3);  // Left diagonal
    truss->addMember(node2, node3);  // Right diagonal

    // Apply load to make it determinate
    node3->setAppliedForce(0.0, -15000.0);

    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/escaping_test.json";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Check that special characters are properly escaped
    EXPECT_TRUE(content.find("\\\"") != std::string::npos) << "Quotes should be escaped";
    EXPECT_TRUE(content.find("\\\\") != std::string::npos) << "Backslashes should be escaped";
    EXPECT_TRUE(content.find("\\n") != std::string::npos) << "Newlines should be escaped";
    EXPECT_TRUE(content.find("\\t") != std::string::npos) << "Tabs should be escaped";

    // Should still be valid JSON
    EXPECT_TRUE(isValidJSON(outputPath))
        << "Output with escaped characters should still be valid JSON";
}

/**
 * @brief Test proper JSON indentation (2 spaces)
 */
TEST_F(JSONExporterTest, ProperIndentation) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/indentation_test.json";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Check for 2-space indentation pattern
    EXPECT_TRUE(content.find("{\n  \"project\"") != std::string::npos)
        << "Should have 2-space indentation for top-level keys";
    EXPECT_TRUE(content.find("  \"geometry\": {\n    \"nodes\"") != std::string::npos)
        << "Should have 4-space indentation for nested keys";
}

/**
 * @brief Test error handling for invalid file path
 */
TEST_F(JSONExporterTest, InvalidFilePath) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string invalidPath = "/invalid/path/that/does/not/exist/output.json";
    ExportOptions options;

    bool success = exporter->exportResults(*truss, results, invalidPath, options);

    EXPECT_FALSE(success) << "Export should fail for invalid path";
    EXPECT_FALSE(exporter->getLastError().empty()) << "Should have error message";
}

/**
 * @brief Test getFormat() method
 */
TEST_F(JSONExporterTest, GetFormat) {
    EXPECT_EQ(exporter->getFormat(), ExportFormat::JSON);
}

/**
 * @brief Test complete export with all sections
 */
TEST_F(JSONExporterTest, CompleteExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/complete_export.json";
    ExportOptions options;
    options.includeGeometry = true;
    options.includeDisplacements = true;
    options.includeMemberForces = true;
    options.includeMetadata = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(success);
    EXPECT_TRUE(isValidJSON(outputPath)) << "Complete export should produce valid JSON";

    // Verify all requested sections are present
    EXPECT_TRUE(fileContains(outputPath, "\"project\""));
    EXPECT_TRUE(fileContains(outputPath, "\"geometry\""));
    EXPECT_TRUE(fileContains(outputPath, "\"displacements\""));
    EXPECT_TRUE(fileContains(outputPath, "\"memberForces\""));
    EXPECT_TRUE(fileContains(outputPath, "\"analysis\""));
}

/**
 * @brief Test golden master equivalence (excluding timestamp)
 */
TEST_F(JSONExporterTest, GoldenMasterEquivalence) {
    // Create identical truss to golden master
    auto truss = std::make_unique<Truss>("Golden Master Test Truss");

    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerX);
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);

    truss->addMember(node1, node2);
    truss->addMember(node1, node3);
    truss->addMember(node2, node3);

    node3->setAppliedForce(0.0, -15000.0);

    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/golden_comparison.json";
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

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    EXPECT_TRUE(success);

    std::string goldenPath = "tests/fixtures/export_golden/golden_master.json";

    if (fs::exists(goldenPath)) {
        std::string generatedContent = readFile(outputPath);
        std::string goldenContent = readFile(goldenPath);

        // Remove timestamp lines for comparison
        std::regex timestampRegex(R"(\s*"exportTime":\s*"[^"]+",?\n)");
        generatedContent = std::regex_replace(generatedContent, timestampRegex, "");
        goldenContent = std::regex_replace(goldenContent, timestampRegex, "");

        // Compare structure (allowing for minor whitespace differences)
        EXPECT_EQ(generatedContent, goldenContent)
            << "Generated JSON should match golden master (excluding timestamp)";
    } else {
        GTEST_SKIP() << "Golden master file not found: " << goldenPath;
    }
}

/**
 * @brief Test that closing braces are properly formatted
 */
TEST_F(JSONExporterTest, ProperClosingBraces) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/closing_braces_test.json";
    ExportOptions options;
    options.includeMetadata = true;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Should end with "}\n" not "}}" or missing newline
    EXPECT_TRUE(content.size() >= 2);
    EXPECT_EQ(content[content.size() - 2], '}');
    EXPECT_EQ(content[content.size() - 1], '\n');

    // Should have proper section closing
    EXPECT_TRUE(content.find("  }\n}") != std::string::npos)
        << "Should have proper closing brace format (2 spaces, brace, newline, closing brace)";
}
