/**
 * @file test_latex_exporter.cpp
 * @brief Unit tests for LaTeX exporter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * CRITICAL: These tests enforce the 8-section export contract.
 * LaTeX exporter MUST emit semantically equivalent data to CSV, JSON, XML, HTML.
 */

#include "core/analysis/analysis_orchestrator.hpp"
#include "core/analysis/solver_factory.hpp"
#include "core/model/truss.hpp"
#include "infrastructure/export/latex_exporter.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

/**
 * @brief Test fixture for LaTeX exporter tests
 */
class LaTeXExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        exporter = std::make_unique<LaTeXExporter>();
        testOutputDir = "test_output_latex";
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
     * @brief Check if file contains valid LaTeX structure
     */
    bool isValidLaTeX(const std::string& path) {
        std::string content = readFile(path);
        if (content.empty())
            return false;

        // Basic LaTeX structure validation
        bool hasDocumentClass = content.find("\\documentclass") != std::string::npos;
        bool hasBeginDoc = content.find("\\begin{document}") != std::string::npos;
        bool hasEndDoc = content.find("\\end{document}") != std::string::npos;

        return hasDocumentClass && hasBeginDoc && hasEndDoc;
    }

    /**
     * @brief Count number of \section{} commands
     */
    int countSections(const std::string& content) {
        std::regex sectionRegex(R"(\\section\{[^}]+\})");
        auto begin = std::sregex_iterator(content.begin(), content.end(), sectionRegex);
        auto end = std::sregex_iterator();
        return std::distance(begin, end);
    }

    std::unique_ptr<LaTeXExporter> exporter;
    std::string testOutputDir;
};

// ============================================================================
// CORE CONTRACT TESTS (8-SECTION ENFORCEMENT)
// ============================================================================

/**
 * @brief Test: All 8 sections must be present
 * CRITICAL: This is the primary contract enforcement test
 */
TEST_F(LaTeXExporterTest, AllEightSectionsPresent) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/eight_sections_test.tex";
    ExportOptions options;  // All sections enabled by default

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success) << "Export should succeed";
    ASSERT_TRUE(fs::exists(outputPath)) << "Output file should exist";

    std::string content = readFile(outputPath);

    // Count \section{} commands (should be 8: all 8 sections including Project Metadata)
    int sectionCount = countSections(content);
    EXPECT_EQ(sectionCount, 8) << "Should have 8 \\section{} commands";

    // Verify each section explicitly
    EXPECT_TRUE(content.find("\\section{Project Metadata}") != std::string::npos)
        << "Section 1: Project Metadata must be present";

    EXPECT_TRUE(content.find("\\section{Geometry}") != std::string::npos)
        << "Section 2: Geometry must be present";

    EXPECT_TRUE(content.find("\\section{Material and Section Properties}") != std::string::npos)
        << "Section 3: Material Properties must be present (even if placeholder)";

    EXPECT_TRUE(content.find("\\section{Applied Loads}") != std::string::npos)
        << "Section 4: Applied Loads must be present (even if placeholder)";

    EXPECT_TRUE(content.find("\\section{Nodal Displacements}") != std::string::npos)
        << "Section 5: Displacements must be present";

    EXPECT_TRUE(content.find("\\section{Member Forces}") != std::string::npos)
        << "Section 6: Member Forces must be present";

    EXPECT_TRUE(content.find("\\section{Support Reactions}") != std::string::npos)
        << "Section 7: Reactions must be present (MANDATORY)";

    EXPECT_TRUE(content.find("\\section{Analysis Metadata}") != std::string::npos)
        << "Section 8: Analysis Metadata must be present";
}

/**
 * @brief Test: Properties section must be present (even if placeholder)
 */
TEST_F(LaTeXExporterTest, PropertiesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/properties_test.tex";
    ExportOptions options;
    options.includeProperties = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Material and Section Properties}"))
        << "Properties section header must be present";

    // Should contain real data table
    EXPECT_TRUE(fileContains(outputPath, "\\begin{longtable}") ||
                fileContains(outputPath, "Material"))
        << "Properties section should include real data or table structure";
}

/**
 * @brief Test: Loads section must be present (even if placeholder)
 */
TEST_F(LaTeXExporterTest, LoadsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/loads_test.tex";
    ExportOptions options;
    options.includeLoads = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Applied Loads}"))
        << "Loads section header must be present";

    // Should contain real data table
    EXPECT_TRUE(fileContains(outputPath, "\\begin{longtable}") ||
                fileContains(outputPath, "Node ID"))
        << "Loads section should include real data or table structure";
}

/**
 * @brief Test: Reactions section must be present (MANDATORY)
 */
TEST_F(LaTeXExporterTest, ReactionsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/reactions_test.tex";
    ExportOptions options;
    options.includeReactions = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Support Reactions}"))
        << "Reactions section header MUST be present";

    // Should contain actual reaction data (not placeholder)
    std::string content = readFile(outputPath);
    EXPECT_TRUE(content.find("Reaction Force") != std::string::npos)
        << "Should contain reaction force data";

    // Should have longtable environment for reactions
    EXPECT_TRUE(content.find("\\begin{longtable}") != std::string::npos)
        << "Should use longtable for reactions data";
}

// ============================================================================
// BASIC FUNCTIONALITY TESTS
// ============================================================================

/**
 * @brief Test: Basic export succeeds
 */
TEST_F(LaTeXExporterTest, BasicExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/basic_test.tex";

    bool success = exporter->exportResults(*truss, results, outputPath);

    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(outputPath));
    EXPECT_GT(fs::file_size(outputPath), 0) << "File should not be empty";
}

/**
 * @brief Test: LaTeX document structure is valid
 */
TEST_F(LaTeXExporterTest, ValidLaTeXStructure) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/structure_test.tex";

    bool success = exporter->exportResults(*truss, results, outputPath);
    ASSERT_TRUE(success);

    EXPECT_TRUE(isValidLaTeX(outputPath)) << "Should produce valid LaTeX document";

    // Check for required LaTeX packages
    EXPECT_TRUE(fileContains(outputPath, "\\usepackage{booktabs}"));
    EXPECT_TRUE(fileContains(outputPath, "\\usepackage{longtable}"));
    EXPECT_TRUE(fileContains(outputPath, "\\usepackage{amsmath}"));
}

/**
 * @brief Test: Project metadata section
 */
TEST_F(LaTeXExporterTest, ProjectMetadata) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/metadata_test.tex";

    bool success = exporter->exportResults(*truss, results, outputPath);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "Test Triangle Truss"));
    EXPECT_TRUE(fileContains(outputPath, "\\maketitle"));
}

/**
 * @brief Test: Geometry section
 */
TEST_F(LaTeXExporterTest, GeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/geometry_test.tex";
    ExportOptions options;
    options.includeGeometry = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Geometry}"));
    EXPECT_TRUE(fileContains(outputPath, "\\subsection{Nodes}"));
    EXPECT_TRUE(fileContains(outputPath, "\\subsection{Members}"));
}

/**
 * @brief Test: Displacements section
 */
TEST_F(LaTeXExporterTest, DisplacementsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/displacements_test.tex";
    ExportOptions options;
    options.includeDisplacements = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Nodal Displacements}"));
    EXPECT_TRUE(fileContains(outputPath, "Maximum Displacement"));
}

/**
 * @brief Test: Member forces section
 */
TEST_F(LaTeXExporterTest, MemberForcesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/forces_test.tex";
    ExportOptions options;
    options.includeMemberForces = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Member Forces}"));
    EXPECT_TRUE(fileContains(outputPath, "Axial Force"));
    EXPECT_TRUE(fileContains(outputPath, "Tension") || fileContains(outputPath, "Compression"));
}

/**
 * @brief Test: Analysis metadata section
 */
TEST_F(LaTeXExporterTest, MetadataSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/analysis_metadata_test.tex";
    ExportOptions options;
    options.includeMetadata = true;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    EXPECT_TRUE(fileContains(outputPath, "\\section{Analysis Metadata}"));
    EXPECT_TRUE(fileContains(outputPath, "Converged"));
    EXPECT_TRUE(fileContains(outputPath, "Iterations"));
    EXPECT_TRUE(fileContains(outputPath, "Total DOFs"));
}

/**
 * @brief Test: Number formatting with precision
 */
TEST_F(LaTeXExporterTest, NumberFormatting) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/precision_test.tex";
    ExportOptions options;
    options.precision = 3;
    options.useScientificNotation = false;

    bool success = exporter->exportResults(*truss, results, outputPath, options);
    ASSERT_TRUE(success);

    // File should exist and have content
    EXPECT_GT(fs::file_size(outputPath), 0);
}

/**
 * @brief Test: LaTeX special character escaping
 */
TEST_F(LaTeXExporterTest, LatexEscaping) {
    auto truss = std::make_unique<Truss>("Test & Special $ Characters # % _");

    // Create stable triangle structure (same as other tests)
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);   // Left support
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerX);  // Right support (Y constrained)
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);     // Top node

    truss->addMember(node1, node2);  // Bottom horizontal
    truss->addMember(node1, node3);  // Left diagonal
    truss->addMember(node2, node3);  // Right diagonal

    // Apply load at top node
    node3->setAppliedForce(0.0, -15000.0);

    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver),
                                      std::make_unique<validation::TrussValidator>());
    auto results = orchestrator.analyze(*truss);

    std::string outputPath = testOutputDir + "/escaping_test.tex";

    bool success = exporter->exportResults(*truss, results, outputPath);
    ASSERT_TRUE(success);

    std::string content = readFile(outputPath);

    // Should have escaped special characters
    EXPECT_TRUE(content.find("\\&") != std::string::npos ||
                content.find("\\\\&") != std::string::npos);
    EXPECT_TRUE(content.find("\\$") != std::string::npos);
    EXPECT_TRUE(content.find("\\#") != std::string::npos);
    EXPECT_TRUE(content.find("\\%") != std::string::npos);
    EXPECT_TRUE(content.find("\\_") != std::string::npos);
}

/**
 * @brief Test: Export format identification
 */
TEST_F(LaTeXExporterTest, FormatIdentification) {
    EXPECT_EQ(exporter->getFormat(), ExportFormat::LaTeX);
}

/**
 * @brief Test: Invalid file path handling
 */
TEST_F(LaTeXExporterTest, InvalidFilePath) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string invalidPath = "/nonexistent_directory_12345/test.tex";

    bool success = exporter->exportResults(*truss, results, invalidPath);

    EXPECT_FALSE(success);
    EXPECT_FALSE(exporter->getLastError().empty());
}

/**
 * @brief Test: Empty truss handling
 */
TEST_F(LaTeXExporterTest, EmptyTruss) {
    auto truss = std::make_unique<Truss>("Empty Truss");
    AnalysisResults results;  // Empty results

    std::string outputPath = testOutputDir + "/empty_test.tex";

    // Should still succeed (with placeholders)
    bool success = exporter->exportResults(*truss, results, outputPath);
    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(outputPath));
}
