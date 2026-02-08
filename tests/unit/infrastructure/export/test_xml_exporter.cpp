/**
 * @file test_xml_exporter.cpp
 * @brief Unit tests for XML exporter
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include "src/infrastructure/export/xml_exporter.hpp"
#include "src/core/model/Truss.hpp"
#include "src/core/analysis/AnalysisOrchestrator.hpp"
#include "src/core/analysis/SolverFactory.hpp"
#include <fstream>
#include <filesystem>
#include <regex>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

/**
 * @brief Test fixture for XML exporter tests
 */
class XMLExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        exporter = std::make_unique<XMLExporter>();
        testOutputDir = "test_output_xml";
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
        auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);   // Left support
        auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerY);  // Right support
        auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);     // Top node
        
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
        AnalysisOrchestrator orchestrator(std::move(solver));
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
     * @brief Check if file contains valid XML structure
     */
    bool isValidXML(const std::string& path) {
        std::string content = readFile(path);
        if (content.empty()) return false;
        
        // Check XML declaration
        if (content.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") == std::string::npos) {
            return false;
        }
        
        // Basic XML tag balance check
        std::regex openTag("<([a-zA-Z][a-zA-Z0-9]*)");
        std::regex closeTag("</([a-zA-Z][a-zA-Z0-9]*)>");
        
        std::map<std::string, int> tagCounts;
        
        // Count opening tags
        auto openBegin = std::sregex_iterator(content.begin(), content.end(), openTag);
        auto openEnd = std::sregex_iterator();
        for (auto it = openBegin; it != openEnd; ++it) {
            std::string tag = (*it)[1];
            tagCounts[tag]++;
        }
        
        // Count closing tags
        auto closeBegin = std::sregex_iterator(content.begin(), content.end(), closeTag);
        auto closeEnd = std::sregex_iterator();
        for (auto it = closeBegin; it != closeEnd; ++it) {
            std::string tag = (*it)[1];
            tagCounts[tag]--;
        }
        
        // All counts should be zero (balanced)
        for (const auto& pair : tagCounts) {
            if (pair.second != 0) {
                return false;
            }
        }
        
        return true;
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
    
    std::unique_ptr<XMLExporter> exporter;
    std::string testOutputDir;
};

/**
 * @brief Test basic XML export functionality
 */
TEST_F(XMLExporterTest, BasicExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/basic_export.xml";
    ExportOptions options;
    
    bool success = exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(success) << "Export should succeed";
    EXPECT_TRUE(fs::exists(outputPath)) << "Output file should exist";
    EXPECT_GT(fs::file_size(outputPath), 0) << "Output file should not be empty";
    EXPECT_TRUE(isValidXML(outputPath)) << "Output should be valid XML";
}

/**
 * @brief Test XML declaration
 */
TEST_F(XMLExporterTest, XMLDeclaration) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/declaration_test.xml";
    ExportOptions options;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    std::string content = readFile(outputPath);
    EXPECT_TRUE(content.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") == 0) 
        << "File should start with XML declaration";
}

/**
 * @brief Test root element
 */
TEST_F(XMLExporterTest, RootElement) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/root_test.xml";
    ExportOptions options;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(fileContains(outputPath, "<TrussAnalysisResults>"));
    EXPECT_TRUE(fileContains(outputPath, "</TrussAnalysisResults>"));
}

/**
 * @brief Test project section structure
 */
TEST_F(XMLExporterTest, ProjectSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/project_test.xml";
    ExportOptions options;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(fileContains(outputPath, "<Project>"));
    EXPECT_TRUE(fileContains(outputPath, "</Project>"));
    EXPECT_TRUE(fileContains(outputPath, "<Name>Test Triangle Truss</Name>"));
    EXPECT_TRUE(fileContains(outputPath, "<ExportTime>"));
    EXPECT_TRUE(fileContains(outputPath, "</ExportTime>"));
    EXPECT_TRUE(fileContains(outputPath, "<Version>2.2.0</Version>"));
}

/**
 * @brief Test geometry section structure
 */
TEST_F(XMLExporterTest, GeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/geometry_test.xml";
    ExportOptions options;
    options.includeGeometry = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    // Check structure
    EXPECT_TRUE(fileContains(outputPath, "<Geometry>"));
    EXPECT_TRUE(fileContains(outputPath, "</Geometry>"));
    EXPECT_TRUE(fileContains(outputPath, "<Nodes>"));
    EXPECT_TRUE(fileContains(outputPath, "</Nodes>"));
    EXPECT_TRUE(fileContains(outputPath, "<Members>"));
    EXPECT_TRUE(fileContains(outputPath, "</Members>"));
}

/**
 * @brief Test node elements
 */
TEST_F(XMLExporterTest, NodeElements) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/nodes_test.xml";
    ExportOptions options;
    options.includeGeometry = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    // Check node attributes and elements
    EXPECT_TRUE(fileContains(outputPath, "<Node id=\"1\">"));
    EXPECT_TRUE(fileContains(outputPath, "<Node id=\"2\">"));
    EXPECT_TRUE(fileContains(outputPath, "<Node id=\"3\">"));
    EXPECT_TRUE(fileContains(outputPath, "<X>"));
    EXPECT_TRUE(fileContains(outputPath, "</X>"));
    EXPECT_TRUE(fileContains(outputPath, "<Y>"));
    EXPECT_TRUE(fileContains(outputPath, "</Y>"));
    EXPECT_TRUE(fileContains(outputPath, "<SupportType>"));
    EXPECT_TRUE(fileContains(outputPath, "</SupportType>"));
}

/**
 * @brief Test member elements
 */
TEST_F(XMLExporterTest, MemberElements) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/members_test.xml";
    ExportOptions options;
    options.includeGeometry = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    // Check member attributes and elements
    EXPECT_TRUE(fileContains(outputPath, "<Member id=\"1\">"));
    EXPECT_TRUE(fileContains(outputPath, "<Member id=\"2\">"));
    EXPECT_TRUE(fileContains(outputPath, "<Member id=\"3\">"));
    EXPECT_TRUE(fileContains(outputPath, "<StartNode>"));
    EXPECT_TRUE(fileContains(outputPath, "</StartNode>"));
    EXPECT_TRUE(fileContains(outputPath, "<EndNode>"));
    EXPECT_TRUE(fileContains(outputPath, "</EndNode>"));
    EXPECT_TRUE(fileContains(outputPath, "<Length>"));
    EXPECT_TRUE(fileContains(outputPath, "</Length>"));
}

/**
 * @brief Test geometry section can be excluded
 */
TEST_F(XMLExporterTest, NoGeometrySection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/no_geometry_test.xml";
    ExportOptions options;
    options.includeGeometry = false;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_FALSE(fileContains(outputPath, "<Geometry>")) 
        << "Geometry section should not be present when includeGeometry=false";
}

/**
 * @brief Test displacements section (CORRECTNESS FIX)
 * Legacy XML omitted this - now corrected for data completeness.
 */
TEST_F(XMLExporterTest, DisplacementsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/displacements_test.xml";
    ExportOptions options;
    options.includeDisplacements = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(fileContains(outputPath, "<Displacements>"));
    EXPECT_TRUE(fileContains(outputPath, "</Displacements>"));
    EXPECT_TRUE(fileContains(outputPath, "<Values>"));
    EXPECT_TRUE(fileContains(outputPath, "<MaxDisplacement>"));
}

/**
 * @brief Test member forces section (CORRECTNESS FIX)
 * Legacy XML omitted this - now corrected for data completeness.
 */
TEST_F(XMLExporterTest, MemberForcesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/member_forces_test.xml";
    ExportOptions options;
    options.includeMemberForces = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(fileContains(outputPath, "<MemberForces>"));
    EXPECT_TRUE(fileContains(outputPath, "</MemberForces>"));
    EXPECT_TRUE(fileContains(outputPath, "<Force memberId="));
}

/**
 * @brief Test reactions section (CORRECTNESS FIX)
 * Legacy XML omitted reactions - this was incorrect.
 * Reactions data is mandatory for semantic equivalence with CSV.
 */
TEST_F(XMLExporterTest, ReactionsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/reactions_test.xml";
    ExportOptions options;
    options.includeReactions = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(fileContains(outputPath, "<Reactions>"));
    EXPECT_TRUE(fileContains(outputPath, "</Reactions>"));
    EXPECT_TRUE(fileContains(outputPath, "<Reaction dof="));
}

/**
 * @brief Test properties section (CONTRACT COMPLETENESS)
 * 
 * Material properties section is REQUIRED for 8-section export contract,
 * even though domain model does not yet implement this feature.
 * Placeholder ensures forward compatibility and explicit contract definition.
 */
TEST_F(XMLExporterTest, PropertiesSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/properties_test.xml";
    ExportOptions options;
    options.includeProperties = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    // Properties section must be present (8-section contract requirement)
    EXPECT_TRUE(fileContains(outputPath, "<Properties>"))
        << "XML export MUST include properties section for contract completeness";
    EXPECT_TRUE(fileContains(outputPath, "</Properties>"))
        << "Properties section must have closing tag";
    EXPECT_TRUE(fileContains(outputPath, "<Comment>"))
        << "Properties placeholder must contain explanatory comment";
}

/**
 * @brief Test loads section (CONTRACT COMPLETENESS)
 * 
 * Applied loads section is REQUIRED for 8-section export contract,
 * even though domain model does not yet implement this feature.
 * Placeholder ensures forward compatibility and explicit contract definition.
 */
TEST_F(XMLExporterTest, LoadsSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/loads_test.xml";
    ExportOptions options;
    options.includeLoads = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    // Loads section must be present (8-section contract requirement)
    EXPECT_TRUE(fileContains(outputPath, "<Loads>"))
        << "XML export MUST include loads section for contract completeness";
    EXPECT_TRUE(fileContains(outputPath, "</Loads>"))
        << "Loads section must have closing tag";
    EXPECT_TRUE(fileContains(outputPath, "<Comment>"))
        << "Loads placeholder must contain explanatory comment";
}

/**
 * @brief Test metadata/analysis section (CORRECTNESS FIX)
 * Legacy XML omitted this - now corrected for data completeness.
 */
TEST_F(XMLExporterTest, MetadataSection) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/metadata_test.xml";
    ExportOptions options;
    options.includeMetadata = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(fileContains(outputPath, "<Analysis>"));
    EXPECT_TRUE(fileContains(outputPath, "</Analysis>"));
    EXPECT_TRUE(fileContains(outputPath, "<Converged>"));
    EXPECT_TRUE(fileContains(outputPath, "<Iterations>"));
}

/**
 * @brief Test precision option
 */
TEST_F(XMLExporterTest, PrecisionOption) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    // Test with 2 decimal places
    std::string outputPath1 = testOutputDir + "/precision_2.xml";
    ExportOptions options1;
    options1.includeGeometry = true;
    options1.precision = 2;
    exporter->exportResults(*truss, results, outputPath1, options1);
    
    // Test with 8 decimal places
    std::string outputPath2 = testOutputDir + "/precision_8.xml";
    ExportOptions options2;
    options2.includeGeometry = true;
    options2.precision = 8;
    exporter->exportResults(*truss, results, outputPath2, options2);
    
    std::string content1 = readFile(outputPath1);
    std::string content2 = readFile(outputPath2);
    
    // Content should differ due to precision
    EXPECT_NE(content1, content2);
    
    // Check precision in output (look for decimal places in coordinates)
    EXPECT_TRUE(fileContains(outputPath1, "<X>0.00</X>"));
    EXPECT_TRUE(fileContains(outputPath2, "<X>0.00000000</X>"));
}

/**
 * @brief Test scientific notation option
 */
TEST_F(XMLExporterTest, ScientificNotation) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/scientific_test.xml";
    ExportOptions options;
    options.includeGeometry = true;
    options.useScientificNotation = true;
    options.precision = 6;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    std::string content = readFile(outputPath);
    
    // Check for scientific notation (e+ or e-)
    EXPECT_TRUE(content.find("e+") != std::string::npos || 
                content.find("e-") != std::string::npos)
        << "Output should contain scientific notation";
}

/**
 * @brief Test XML string escaping
 */
TEST_F(XMLExporterTest, StringEscaping) {
    auto truss = std::make_unique<Truss>("Test <Truss> & \"Quote\" 'Apos'");
    
    // Create stable triangle structure
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);
    
    truss->addMember(node1, node2);
    truss->addMember(node1, node3);
    truss->addMember(node2, node3);
    
    node3->setAppliedForce(0.0, -15000.0);
    
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/escaping_test.xml";
    ExportOptions options;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    std::string content = readFile(outputPath);
    
    // Check that special characters are escaped
    EXPECT_TRUE(content.find("&lt;") != std::string::npos) << "< should be escaped";
    EXPECT_TRUE(content.find("&gt;") != std::string::npos) << "> should be escaped";
    EXPECT_TRUE(content.find("&amp;") != std::string::npos) << "& should be escaped";
    EXPECT_TRUE(content.find("&quot;") != std::string::npos) << "\" should be escaped";
    EXPECT_TRUE(content.find("&apos;") != std::string::npos) << "' should be escaped";
}

/**
 * @brief Test proper indentation
 */
TEST_F(XMLExporterTest, ProperIndentation) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/indentation_test.xml";
    ExportOptions options;
    options.includeGeometry = true;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    std::string content = readFile(outputPath);
    
    // Check indentation patterns (2 spaces per level)
    EXPECT_TRUE(content.find("  <Project>") != std::string::npos);
    EXPECT_TRUE(content.find("    <Name>") != std::string::npos);
    EXPECT_TRUE(content.find("  <Geometry>") != std::string::npos);
    EXPECT_TRUE(content.find("    <Nodes>") != std::string::npos);
    EXPECT_TRUE(content.find("      <Node id=") != std::string::npos);
    EXPECT_TRUE(content.find("        <X>") != std::string::npos);
}

/**
 * @brief Test invalid file path handling
 */
TEST_F(XMLExporterTest, InvalidFilePath) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string invalidPath = "/nonexistent/directory/output.xml";
    ExportOptions options;
    
    bool success = exporter->exportResults(*truss, results, invalidPath, options);
    
    EXPECT_FALSE(success) << "Export should fail with invalid path";
    EXPECT_FALSE(exporter->getLastError().empty()) << "Error message should be set";
}

/**
 * @brief Test getFormat method
 */
TEST_F(XMLExporterTest, GetFormat) {
    EXPECT_EQ(exporter->getFormat(), ExportFormat::XML);
}

/**
 * @brief Test complete export with all sections
 */
TEST_F(XMLExporterTest, CompleteExport) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/complete_export.xml";
    ExportOptions options;
    options.includeGeometry = true;
    options.precision = 6;
    
    bool success = exporter->exportResults(*truss, results, outputPath, options);
    
    EXPECT_TRUE(success);
    EXPECT_TRUE(isValidXML(outputPath));
    
    // Verify all major sections
    EXPECT_TRUE(fileContains(outputPath, "<Project>"));
    EXPECT_TRUE(fileContains(outputPath, "<Geometry>"));
    EXPECT_TRUE(fileContains(outputPath, "<Nodes>"));
    EXPECT_TRUE(fileContains(outputPath, "<Members>"));
}

/**
 * @brief Test golden master equivalence
 */
TEST_F(XMLExporterTest, GoldenMasterEquivalence) {
    // Create truss with exact golden master name
    auto truss = std::make_unique<Truss>("Golden Master Test Truss");
    
    auto node1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss->addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss->addNode(2.0, 3.0, SupportType::Free);
    
    truss->addMember(node1, node2);
    truss->addMember(node1, node3);
    truss->addMember(node2, node3);
    
    node3->setAppliedForce(0.0, -15000.0);
    
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/golden_master_test.xml";
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
    
    // Read both files
    std::ifstream generatedFile(outputPath);
    std::ifstream goldenFile("tests/fixtures/export_golden/golden_master.xml");
    
    if (goldenFile.is_open() && generatedFile.is_open()) {
        std::string generatedLine, goldenLine;
        std::regex timestampRegex(R"(\s*<ExportTime>[^<]+</ExportTime>)");
        
        bool filesMatch = true;
        
        while (std::getline(goldenFile, goldenLine) && 
               std::getline(generatedFile, generatedLine)) {
            
            // Skip timestamp comparison
            if (std::regex_search(goldenLine, timestampRegex) && 
                std::regex_search(generatedLine, timestampRegex)) {
                continue;
            }
            
            if (goldenLine != generatedLine) {
                filesMatch = false;
                break;
            }
        }
        
        // Check if one file has more lines
        if (std::getline(goldenFile, goldenLine) || 
            std::getline(generatedFile, generatedLine)) {
            filesMatch = false;
        }
        
        EXPECT_TRUE(filesMatch) << "Generated XML should match golden master (excluding timestamp)";
    }
}

/**
 * @brief Test proper closing of root element
 */
TEST_F(XMLExporterTest, ProperClosingElement) {
    auto truss = createSimpleTriangleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    std::string outputPath = testOutputDir + "/closing_test.xml";
    ExportOptions options;
    
    exporter->exportResults(*truss, results, outputPath, options);
    
    std::string content = readFile(outputPath);
    
    // Check that file ends with proper closing
    EXPECT_TRUE(content.find("</TrussAnalysisResults>\n") != std::string::npos)
        << "File should end with closing root element and newline";
}

