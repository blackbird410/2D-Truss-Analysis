/**
 * @file test_text_exporter.cpp
 * @brief Unit tests for Text exporter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * CRITICAL: These tests enforce the 8-section export contract.
 * Text exporter MUST emit semantically equivalent data to CSV, JSON, XML, HTML, LaTeX.
 */

#include "core/analysis/analysis_orchestrator.hpp"
#include "truss/export/export_format.hpp"
#include "core/analysis/solver_factory.hpp"
#include "core/model/truss.hpp"
#include "infrastructure/export/text_exporter.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

/**
 * @brief Test fixture for Text exporter tests
 */
class TextExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        exporter = std::make_unique<TextExporter>();
        testOutputDir = "test_output_text";
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
     * @brief Count occurrences of the 8 mandatory section headers
     */
    int countSections(const std::string& path) {
        std::string content = readFile(path);
        int count = 0;

        // Count each specific section header
        if (content.find("PROJECT METADATA") != std::string::npos)
            count++;
        if (content.find("GEOMETRY") != std::string::npos)
            count++;
        if (content.find("MATERIAL AND SECTION PROPERTIES") != std::string::npos)
            count++;
        if (content.find("APPLIED LOADS") != std::string::npos)
            count++;
        if (content.find("NODAL DISPLACEMENTS") != std::string::npos)
            count++;
        if (content.find("MEMBER FORCES") != std::string::npos)
            count++;
        if (content.find("SUPPORT REACTIONS") != std::string::npos)
            count++;
        if (content.find("ANALYSIS METADATA") != std::string::npos)
            count++;

        return count;
    }

    std::unique_ptr<TextExporter> exporter;
    std::string testOutputDir;
};

// ============================================================================
// CRITICAL CONTRACT TESTS - ENFORCE 8-SECTION REQUIREMENT
// ============================================================================

/**
 * @brief PRIMARY TEST: Verify all 8 sections are present
 *
 * This test is the primary enforcement of the 8-section export contract.
 * If this test fails, the exporter is non-compliant.
 */
TEST_F(TextExporterTest, AllEightSectionsPresent) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/all_sections.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    // Count sections in output
    int sectionCount = countSections(outputPath);
    EXPECT_EQ(sectionCount, 8) << "Should have 8 sections";

    // Explicitly verify each mandatory section by name
    EXPECT_TRUE(fileContains(outputPath, "PROJECT METADATA")) << "Missing PROJECT METADATA section";
    EXPECT_TRUE(fileContains(outputPath, "GEOMETRY")) << "Missing GEOMETRY section";
    EXPECT_TRUE(fileContains(outputPath, "MATERIAL AND SECTION PROPERTIES"))
        << "Missing PROPERTIES section";
    EXPECT_TRUE(fileContains(outputPath, "APPLIED LOADS")) << "Missing LOADS section";
    EXPECT_TRUE(fileContains(outputPath, "NODAL DISPLACEMENTS")) << "Missing DISPLACEMENTS section";
    EXPECT_TRUE(fileContains(outputPath, "MEMBER FORCES")) << "Missing FORCES section";
    EXPECT_TRUE(fileContains(outputPath, "SUPPORT REACTIONS")) << "Missing REACTIONS section";
    EXPECT_TRUE(fileContains(outputPath, "ANALYSIS METADATA")) << "Missing METADATA section";
}

/**
 * @brief Verify properties section contains real material data
 */
TEST_F(TextExporterTest, PropertiesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/properties.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "MATERIAL AND SECTION PROPERTIES"));
    EXPECT_TRUE(fileContains(outputPath, "Member ID"));
    EXPECT_TRUE(fileContains(outputPath, "Material"));
    EXPECT_TRUE(fileContains(outputPath, "E (Pa)"));
    EXPECT_TRUE(fileContains(outputPath, "Area (m"));
}

/**
 * @brief Verify loads section contains real applied force data
 */
TEST_F(TextExporterTest, LoadsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/loads.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "APPLIED LOADS"));
    EXPECT_TRUE(fileContains(outputPath, "Node ID"));
    EXPECT_TRUE(fileContains(outputPath, "Fx (N)"));
    EXPECT_TRUE(fileContains(outputPath, "Fy (N)"));
}

/**
 * @brief MANDATORY: Verify reactions section is present with data
 */
TEST_F(TextExporterTest, ReactionsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/reactions.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "SUPPORT REACTIONS"));
    EXPECT_TRUE(fileContains(outputPath, "DOF"));
    EXPECT_TRUE(fileContains(outputPath, "Reaction Force"));
}

// ============================================================================
// FUNCTIONAL TESTS
// ============================================================================

/**
 * @brief Test basic export functionality
 */
TEST_F(TextExporterTest, BasicExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/basic_export.txt";
    EXPECT_TRUE(exporter->exportResults(*truss, results, outputPath));

    // Verify file was created
    EXPECT_TRUE(fs::exists(outputPath));

    // Verify file is not empty
    EXPECT_GT(fs::file_size(outputPath), 0);
}

/**
 * @brief Test project metadata section
 */
TEST_F(TextExporterTest, ProjectMetadata) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/project.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "Test Triangle Truss"));
    EXPECT_TRUE(fileContains(outputPath, "PROJECT METADATA"));
    EXPECT_TRUE(fileContains(outputPath, "Number of Nodes"));
    EXPECT_TRUE(fileContains(outputPath, "Number of Members"));
}

/**
 * @brief Test geometry section content
 */
TEST_F(TextExporterTest, GeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/geometry.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "GEOMETRY"));
    EXPECT_TRUE(fileContains(outputPath, "Nodes:"));
    EXPECT_TRUE(fileContains(outputPath, "Members:"));
    EXPECT_TRUE(fileContains(outputPath, "Node ID"));
    EXPECT_TRUE(fileContains(outputPath, "Member ID"));
    EXPECT_TRUE(fileContains(outputPath, "Support Type"));
}

/**
 * @brief Test displacements section
 */
TEST_F(TextExporterTest, DisplacementsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/displacements.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "NODAL DISPLACEMENTS"));
    EXPECT_TRUE(fileContains(outputPath, "DOF"));
    EXPECT_TRUE(fileContains(outputPath, "Displacement"));
    EXPECT_TRUE(fileContains(outputPath, "Maximum Displacement"));
}

/**
 * @brief Test member forces section
 */
TEST_F(TextExporterTest, MemberForcesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/forces.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "MEMBER FORCES"));
    EXPECT_TRUE(fileContains(outputPath, "Axial Force"));
    EXPECT_TRUE(fileContains(outputPath, "Type"));
    // Should have both tension and compression
    std::string content = readFile(outputPath);
    EXPECT_TRUE(content.find("Tension") != std::string::npos ||
                content.find("Compression") != std::string::npos);
}

/**
 * @brief Test metadata section
 */
TEST_F(TextExporterTest, MetadataSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/metadata.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    EXPECT_TRUE(fileContains(outputPath, "ANALYSIS METADATA"));
    EXPECT_TRUE(fileContains(outputPath, "Converged"));
    EXPECT_TRUE(fileContains(outputPath, "Iterations"));
    EXPECT_TRUE(fileContains(outputPath, "Total DOFs"));
}

/**
 * @brief Test number formatting with different precision
 */
TEST_F(TextExporterTest, NumberFormatting) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    ExportOptions options;
    options.precision = 3;

    std::string outputPath = testOutputDir + "/precision.txt";
    EXPECT_TRUE(exporter->exportResults(*truss, results, outputPath, options));

    // Just verify file was created successfully with custom precision
    EXPECT_TRUE(fs::exists(outputPath));
}

/**
 * @brief Test format identification
 */
TEST_F(TextExporterTest, FormatIdentification) {
    EXPECT_EQ(exporter->getFormat(), truss::ExportFormat::TXT);
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

/**
 * @brief Test handling of invalid file path
 */
TEST_F(TextExporterTest, InvalidFilePath) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string invalidPath = "/nonexistent/directory/output.txt";
    EXPECT_FALSE(exporter->exportResults(*truss, results, invalidPath));
    EXPECT_FALSE(exporter->getLastError().empty());
}

/**
 * @brief Test handling of empty truss
 */
TEST_F(TextExporterTest, EmptyTruss) {
    auto truss = std::make_unique<Truss>("Empty Truss");
    AnalysisResults results;

    std::string outputPath = testOutputDir + "/empty.txt";
    // Should still export successfully with empty sections
    EXPECT_TRUE(exporter->exportResults(*truss, results, outputPath));
}

/**
 * @brief Test document structure
 */
TEST_F(TextExporterTest, DocumentStructure) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/structure.txt";
    ASSERT_TRUE(exporter->exportResults(*truss, results, outputPath));

    std::string content = readFile(outputPath);

    // Check for document header
    EXPECT_TRUE(content.find("2D TRUSS ANALYSIS RESULTS") != std::string::npos);
    EXPECT_TRUE(content.find("Generated:") != std::string::npos);

    // Check for separators (===)
    EXPECT_TRUE(content.find("===") != std::string::npos);

    // Check for document footer
    EXPECT_TRUE(content.find("End of Report") != std::string::npos);
}
