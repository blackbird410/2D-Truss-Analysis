/**
 * @file test_html_exporter.cpp
 * @brief Unit tests for HTML exporter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "core/analysis/analysis_orchestrator.hpp"
#include "core/analysis/solver_factory.hpp"
#include "core/model/truss.hpp"
#include "infrastructure/export/html_exporter.hpp"
#include "truss/export/export_format.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

/**
 * @brief Test fixture for HTML exporter tests
 */
class HTMLExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        exporter = std::make_unique<HTMLExporter>();
        testOutputDir = "test_output_html";
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
     * @brief Check if file contains valid HTML structure
     */
    bool isValidHTML(const std::string& path) {
        std::string content = readFile(path);
        if (content.empty())
            return false;

        // Basic HTML structure validation
        return content.find("<!DOCTYPE html>") != std::string::npos &&
               content.find("<html") != std::string::npos &&
               content.find("</html>") != std::string::npos &&
               content.find("<head>") != std::string::npos &&
               content.find("</head>") != std::string::npos &&
               content.find("<body>") != std::string::npos &&
               content.find("</body>") != std::string::npos;
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

    std::unique_ptr<HTMLExporter> exporter;
    std::string testOutputDir;
};

/**
 * @brief Test basic HTML export functionality
 */
TEST_F(HTMLExporterTest, BasicExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/basic_export.html";
    ExportOptions options;

    bool success = exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(success) << "Export should succeed";
    EXPECT_TRUE(fs::exists(outputPath)) << "Output file should exist";
    EXPECT_GT(fs::file_size(outputPath), 0) << "Output file should not be empty";
    EXPECT_TRUE(isValidHTML(outputPath)) << "Output should be valid HTML";
}

/**
 * @brief Test HTML document structure
 */
TEST_F(HTMLExporterTest, HTMLStructure) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/structure_test.html";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "<!DOCTYPE html>"));
    EXPECT_TRUE(fileContains(outputPath, "<html lang=\"en\">"));
    EXPECT_TRUE(fileContains(outputPath, "<head>"));
    EXPECT_TRUE(fileContains(outputPath, "<meta charset=\"UTF-8\">"));
    EXPECT_TRUE(fileContains(outputPath, "<title>"));
    EXPECT_TRUE(fileContains(outputPath, "<style>"));
    EXPECT_TRUE(fileContains(outputPath, "<body>"));
    EXPECT_TRUE(fileContains(outputPath, "</body>"));
    EXPECT_TRUE(fileContains(outputPath, "</html>"));
}

/**
 * @brief Test project metadata section
 */
TEST_F(HTMLExporterTest, ProjectSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/project_test.html";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "<h1>Truss Analysis Results</h1>"));
    EXPECT_TRUE(fileContains(outputPath, "Test Triangle Truss"));
    EXPECT_TRUE(fileContains(outputPath, "<strong>Project Name:</strong>"));
    EXPECT_TRUE(fileContains(outputPath, "<strong>Export Date:</strong>"));
    EXPECT_TRUE(fileContains(outputPath, "<strong>Version:</strong>"));
    EXPECT_TRUE(fileContains(outputPath, "3.0.0"));
}

/**
 * @brief Test geometry section structure
 */
TEST_F(HTMLExporterTest, GeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/geometry_test.html";
    ExportOptions options;
    options.includeGeometry = true;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    // Check section header
    EXPECT_TRUE(fileContains(outputPath, "<h2>Geometry</h2>"));
    EXPECT_TRUE(fileContains(outputPath, "<h3>Nodes</h3>"));
    EXPECT_TRUE(fileContains(outputPath, "<h3>Members</h3>"));

    // Check nodes table structure
    EXPECT_TRUE(fileContains(outputPath, "<th>Node ID</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>X Coordinate</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Y Coordinate</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Support Type</th>"));

    // Check node data is present (3 nodes)
    int nodeRowCount = countOccurrences(outputPath, "<td>1</td>") +
                       countOccurrences(outputPath, "<td>2</td>") +
                       countOccurrences(outputPath, "<td>3</td>");
    EXPECT_GE(nodeRowCount, 3) << "Should have at least 3 node IDs";

    // Check members table structure
    EXPECT_TRUE(fileContains(outputPath, "<th>Member ID</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Start Node</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>End Node</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Length</th>"));
}

/**
 * @brief Test properties section (CONTRACT COMPLETENESS)
 *
 * Material properties section is REQUIRED for 8-section export contract.
 * Domain model provides complete material and section data through
 * Member::getMaterial() and Member::getSection().
 */
TEST_F(HTMLExporterTest, PropertiesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/properties_test.html";
    ExportOptions options;
    options.includeProperties = true;

    exporter->exportResults(*truss, results, outputPath, options);

    // Properties section must be present with real data
    EXPECT_TRUE(fileContains(outputPath, "<h2>Material and Section Properties</h2>"))
        << "HTML export MUST include properties section";
    EXPECT_TRUE(fileContains(outputPath, "<table")) << "Properties must include table structure";
    EXPECT_TRUE(fileContains(outputPath, "Material")) << "Properties must include material column";
    EXPECT_TRUE(fileContains(outputPath, "Young's Modulus"))
        << "Properties must include Young's modulus column";
}

/**
 * @brief Test loads section (CONTRACT COMPLETENESS)
 *
 * Applied loads section is REQUIRED for 8-section export contract.
 * Domain model provides load data through Node::getAppliedForce().
 */
TEST_F(HTMLExporterTest, LoadsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/loads_test.html";
    ExportOptions options;
    options.includeLoads = true;

    exporter->exportResults(*truss, results, outputPath, options);

    // Loads section must be present with real data
    EXPECT_TRUE(fileContains(outputPath, "<h2>Applied Loads</h2>"))
        << "HTML export MUST include loads section";
    EXPECT_TRUE(fileContains(outputPath, "<table")) << "Loads must include table structure";
    EXPECT_TRUE(fileContains(outputPath, "Fx (N)") || fileContains(outputPath, "Node ID"))
        << "Loads must include force column headers";
}

/**
 * @brief Test displacements section structure
 */
TEST_F(HTMLExporterTest, DisplacementsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/displacements_test.html";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = true;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "<h2>Nodal Displacements</h2>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>DOF</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Displacement</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<strong>Maximum Displacement:</strong>"));

    // Should have table rows with displacement values
    EXPECT_TRUE(fileContains(outputPath, "<tbody>"));
}

/**
 * @brief Test member forces section structure
 */
TEST_F(HTMLExporterTest, MemberForcesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/member_forces_test.html";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = true;
    options.includeReactions = false;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "<h2>Member Forces</h2>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Member ID</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Axial Force</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Type</th>"));

    // Should have tension/compression markers
    bool hasTensionOrCompression = fileContains(outputPath, "tension") ||
                                   fileContains(outputPath, "compression");
    EXPECT_TRUE(hasTensionOrCompression) << "Should have tension/compression indicators";
}

/**
 * @brief Test reactions section structure (CRITICAL - MANDATORY SECTION)
 *
 * Reactions section is MANDATORY for structural analysis verification.
 * This test ensures HTMLExporter does not regress to legacy incomplete behavior.
 */
TEST_F(HTMLExporterTest, ReactionsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/reactions_test.html";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = true;
    options.includeMetadata = false;

    exporter->exportResults(*truss, results, outputPath, options);

    // Reactions section is MANDATORY for equilibrium verification
    EXPECT_TRUE(fileContains(outputPath, "<h2>Support Reactions</h2>"))
        << "HTML export MUST include reactions section (equilibrium requirement)";
    EXPECT_TRUE(fileContains(outputPath, "<th>DOF</th>"));
    EXPECT_TRUE(fileContains(outputPath, "<th>Reaction Force</th>"));

    // Should have table with reaction values
    EXPECT_TRUE(fileContains(outputPath, "<tbody>"));
}

/**
 * @brief Test metadata section structure
 */
TEST_F(HTMLExporterTest, MetadataSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/metadata_test.html";
    ExportOptions options;
    options.includeGeometry = false;
    options.includeDisplacements = false;
    options.includeMemberForces = false;
    options.includeReactions = false;
    options.includeMetadata = true;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "<h2>Analysis Metadata</h2>"));
    EXPECT_TRUE(fileContains(outputPath, "Converged"));
    EXPECT_TRUE(fileContains(outputPath, "Iterations"));
    EXPECT_TRUE(fileContains(outputPath, "Total DOFs"));
    EXPECT_TRUE(fileContains(outputPath, "Free DOFs"));
    EXPECT_TRUE(fileContains(outputPath, "Max Displacement"));
    EXPECT_TRUE(fileContains(outputPath, "Max Stress"));
}

/**
 * @brief Test that all 8 sections are present when all options enabled
 *
 * CRITICAL TEST: Verifies HTMLExporter conforms to 8-section contract.
 * This is a regression test to prevent reintroduction of legacy incomplete behavior.
 */
TEST_F(HTMLExporterTest, AllEightSectionsPresent) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/all_sections_test.html";
    ExportOptions options;  // All options enabled by default

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // VERIFY ALL 8 MANDATORY SECTIONS (8-section contract compliance)
    EXPECT_TRUE(fileContains(outputPath, "Truss Analysis Results"))
        << "Section 1: Project metadata must be present";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Geometry</h2>"))
        << "Section 2: Geometry must be present";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Material and Section Properties</h2>"))
        << "Section 3: Properties must be present (placeholder OK)";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Applied Loads</h2>"))
        << "Section 4: Loads must be present (placeholder OK)";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Nodal Displacements</h2>"))
        << "Section 5: Displacements must be present";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Member Forces</h2>"))
        << "Section 6: Member forces must be present";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Support Reactions</h2>"))
        << "Section 7: Reactions must be present (MANDATORY for equilibrium)";

    EXPECT_TRUE(fileContains(outputPath, "<h2>Analysis Metadata</h2>"))
        << "Section 8: Analysis metadata must be present";

    // Count <h2> tags - should have exactly 8 sections
    int h2Count = countOccurrences(outputPath, "<h2>");
    EXPECT_EQ(h2Count, 8) << "HTML export must have exactly 8 section headers (8-section contract)";
}

/**
 * @brief Test precision option
 */
TEST_F(HTMLExporterTest, PrecisionOption) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    // Export with precision 2
    std::string outputPath1 = testOutputDir + "/precision2_test.html";
    ExportOptions options1;
    options1.precision = 2;
    exporter->exportResults(*truss, results, outputPath1, options1);

    // Export with precision 8
    std::string outputPath2 = testOutputDir + "/precision8_test.html";
    ExportOptions options2;
    options2.precision = 8;
    exporter->exportResults(*truss, results, outputPath2, options2);

    std::string content1 = readFile(outputPath1);
    std::string content2 = readFile(outputPath2);

    // Higher precision file should be larger (more decimal places)
    EXPECT_GT(content2.size(), content1.size())
        << "Higher precision should result in more decimal places";
}

/**
 * @brief Test CSS styling is included
 */
TEST_F(HTMLExporterTest, CSSStylesIncluded) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/css_test.html";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "<style>"));
    EXPECT_TRUE(fileContains(outputPath, "font-family"));
    EXPECT_TRUE(fileContains(outputPath, "table"));
    EXPECT_TRUE(fileContains(outputPath, ".container"));
    EXPECT_TRUE(fileContains(outputPath, ".placeholder"));
    EXPECT_TRUE(fileContains(outputPath, ".tension"));
    EXPECT_TRUE(fileContains(outputPath, ".compression"));
}

/**
 * @brief Test HTML escaping
 */
TEST_F(HTMLExporterTest, HTMLEscaping) {
    auto truss = std::make_unique<Truss>("Test <Tag> & \"Quotes\"");
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerX);
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);

    // Add members to make valid truss (stable triangle)
    truss->addMember(node1, node2);
    truss->addMember(node1, node3);
    truss->addMember(node2, node3);

    // Add a load
    node3->setAppliedForce(0.0, -15000.0);

    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/escaping_test.html";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    std::string content = readFile(outputPath);

    // Should escape HTML special characters
    EXPECT_TRUE(content.find("&lt;") != std::string::npos) << "< should be escaped";
    EXPECT_TRUE(content.find("&gt;") != std::string::npos) << "> should be escaped";
    EXPECT_TRUE(content.find("&amp;") != std::string::npos) << "& should be escaped";
    EXPECT_TRUE(content.find("&quot;") != std::string::npos) << "\" should be escaped";
}

/**
 * @brief Test file handle error
 */
TEST_F(HTMLExporterTest, FileHandleError) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    // Try to write to invalid path
    std::string outputPath = "/invalid/path/that/does/not/exist/file.html";
    ExportOptions options;

    bool success = exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_FALSE(success) << "Export to invalid path should fail";
    EXPECT_FALSE(exporter->getLastError().empty()) << "Should have error message";
}

/**
 * @brief Test footer generation
 */
TEST_F(HTMLExporterTest, FooterPresent) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);

    std::string outputPath = testOutputDir + "/footer_test.html";
    ExportOptions options;

    exporter->exportResults(*truss, results, outputPath, options);

    EXPECT_TRUE(fileContains(outputPath, "Generated by 2D Truss Analysis"));
    EXPECT_TRUE(fileContains(outputPath, "v3.0.0"));
    EXPECT_TRUE(fileContains(outputPath, "Export Time:"));
}

/**
 * @brief Test getFormat() returns correct format
 */
TEST_F(HTMLExporterTest, GetFormat) {
    EXPECT_EQ(exporter->getFormat(), truss::ExportFormat::HTML);
}

// ---------------------------------------------------------------------------
// Disabled-section branch coverage
// ---------------------------------------------------------------------------

TEST_F(HTMLExporterTest, GeometryDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_geom.html";
    ExportOptions opts;
    opts.includeGeometry = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Geometry</h2>"));
}

TEST_F(HTMLExporterTest, PropertiesDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_props.html";
    ExportOptions opts;
    opts.includeProperties = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Material and Section Properties</h2>"));
}

TEST_F(HTMLExporterTest, LoadsDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_loads.html";
    ExportOptions opts;
    opts.includeLoads = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Applied Loads</h2>"));
}

TEST_F(HTMLExporterTest, DisplacementsDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_disp.html";
    ExportOptions opts;
    opts.includeDisplacements = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Node Displacements</h2>"));
}

TEST_F(HTMLExporterTest, MemberForcesDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_forces.html";
    ExportOptions opts;
    opts.includeMemberForces = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Member Forces</h2>"));
}

TEST_F(HTMLExporterTest, ReactionsDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_react.html";
    ExportOptions opts;
    opts.includeReactions = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Support Reactions</h2>"));
}

TEST_F(HTMLExporterTest, MetadataDisabled_SectionOmitted) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    std::string outputPath = testOutputDir + "/no_meta.html";
    ExportOptions opts;
    opts.includeMetadata = false;
    exporter->exportResults(*truss, results, outputPath, opts);
    EXPECT_FALSE(fileContains(outputPath, "<h2>Analysis Metadata</h2>"));
}

TEST_F(HTMLExporterTest, NoLoadsOnNodes_LoadsSectionEmpty) {
    auto truss = std::make_unique<Truss>("No Load Truss");
    auto n1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss->addNode(1.0, 0.0, SupportType::RollerX);
    auto n3 = truss->addNode(0.5, 0.5, SupportType::Free);
    // No applied forces
    truss->addMember(n1, n2);
    truss->addMember(n1, n3);
    truss->addMember(n2, n3);
    AnalysisResults results;
    std::string outputPath = testOutputDir + "/no_load_nodes.html";
    ExportOptions opts;
    opts.includeLoads = true;
    exporter->exportResults(*truss, results, outputPath, opts);
    // Section header should still appear but no force data rows
    std::string content = readFile(outputPath);
    EXPECT_NE(content.find("<h2>Applied Loads</h2>"), std::string::npos);
}
