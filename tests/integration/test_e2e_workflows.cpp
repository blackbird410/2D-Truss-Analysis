/**
 * @file test_e2e_workflows.cpp
 * @brief End-to-end integration tests for complete application workflows
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-21
 */

#include <gtest/gtest.h>
#include <filesystem>
#include "core/model/Truss.hpp"
#include "core/validation/TrussValidator.hpp"
#include "core/assembly/TrussAssembler.hpp"
#include "application/AnalysisApplicationService.hpp"
#include "infrastructure/io/fileio_factory.hpp"
#include "infrastructure/export/exporter_factory.hpp"

using namespace truss::core;
using namespace truss::core::validation;
using namespace truss::core::assembly;
using namespace truss::application;
using namespace truss::infrastructure::io;
using namespace truss::infrastructure::export_;

class E2EWorkflowTest : public ::testing::Test {
protected:
    void TearDown() override {
        cleanupTestFiles();
    }
    
    void cleanupTestFiles() {
        const std::vector<std::string> testFiles = {
            "e2e_test.json", "e2e_roundtrip.json", "e2e_export.csv",
            "e2e_export.xml", "e2e_complex.json"
        };
        
        for (const auto& file : testFiles) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }
};

// ============================================================================
// Test 8.5.1: Domain Model CRUD Operations
// ============================================================================

TEST_F(E2EWorkflowTest, CompleteDomainCRUDWorkflow) {
    // CREATE
    Truss truss("CRUD Test");
    
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    auto m1 = truss.addMember(n1, n2);
    auto m2 = truss.addMember(n1, n3);
    auto m3 = truss.addMember(n2, n3);
    
    truss.applyForce(n3->getId(), Force2D(0.0, -10000.0));
    
    // READ
    EXPECT_EQ(truss.getNodeCount(), 3);
    EXPECT_EQ(truss.getMemberCount(), 3);
    
    // UPDATE
    auto n4 = truss.addNode(6.0, 3.0, SupportType::Free);
    auto m4 = truss.addMember(n2, n4);
    
    EXPECT_EQ(truss.getNodeCount(), 4);
    EXPECT_EQ(truss.getMemberCount(), 4);
    
    // DELETE
    bool removed = truss.removeMember(m4->getId());
    EXPECT_TRUE(removed);
    EXPECT_EQ(truss.getMemberCount(), 3);
}

// ============================================================================
// Test 8.5.2: Save-Load-Analyze Workflow
// ============================================================================

TEST_F(E2EWorkflowTest, SaveLoadAnalyzeWorkflow) {
    std::filesystem::path testFile = "e2e_test.json";
    
    // SAVE: Create and persist truss
    {
        Truss truss("Warren Truss");
        
        auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
        auto n2 = truss.addNode(2.0, 2.0, SupportType::Free);
        auto n3 = truss.addNode(4.0, 0.0, SupportType::Free);
        auto n4 = truss.addNode(6.0, 2.0, SupportType::Free);
        auto n5 = truss.addNode(8.0, 0.0, SupportType::RollerX);  // Changed to RollerX
        
        truss.addMember(n1, n3);
        truss.addMember(n3, n5);
        truss.addMember(n1, n2);
        truss.addMember(n2, n3);
        truss.addMember(n3, n4);
        truss.addMember(n4, n5);
        truss.addMember(n2, n4);
        
        truss.applyForce(n2->getId(), Force2D(0.0, -5000.0));
        truss.applyForce(n4->getId(), Force2D(0.0, -5000.0));
        
        auto dto = TrussAssembler::createDTO(truss);
        auto writer = FileIOFactory::createWriter(testFile);
        FileIOOptions options;
        options.prettyPrint = true;
        bool writeSuccess = writer->write(dto, testFile, options);
        EXPECT_TRUE(writeSuccess);
    }
    
    // LOAD: Read back and verify
    {
        ASSERT_TRUE(std::filesystem::exists(testFile));
        
        auto reader = FileIOFactory::createReader(testFile);
        auto dto = reader->read(testFile);  // Returns TrussDTO directly
        
        auto loadedTruss = TrussAssembler::assembleTruss(dto);
        
        EXPECT_EQ(loadedTruss->getName(), "Warren Truss");
        EXPECT_EQ(loadedTruss->getNodeCount(), 5);
        EXPECT_EQ(loadedTruss->getMemberCount(), 7);
        
        size_t loadedNodes = 0;
        for (const auto& node : loadedTruss->getNodes()) {
            if (node->hasAppliedForce()) {
                loadedNodes++;
            }
        }
        EXPECT_EQ(loadedNodes, 2);
    }
    
    // ANALYZE: Perform structural analysis
    {
        auto reader = FileIOFactory::createReader(testFile);
        auto dto = reader->read(testFile);
        auto loadedTruss = TrussAssembler::assembleTruss(dto);
        
        AnalysisApplicationService analysisService;
        auto analysisResult = analysisService.analyze(*loadedTruss);
        ASSERT_TRUE(analysisResult.success) << "Analysis failed: " << analysisResult.errorMessage;
        
        // Results are available through getResultsView()
        const auto& resultsView = analysisService.getResultsView(analysisResult.value);
        EXPECT_TRUE(resultsView.hasConverged());
        EXPECT_GT(resultsView.getTotalDofs(), 0);
    }
}

// ============================================================================
// ============================================================================
// Test 8.5.3: Analysis and Export Workflow
// ============================================================================

TEST_F(E2EWorkflowTest, AnalysisAndExportWorkflow) {
    // Create and analyze a truss
    Truss truss("Export Test");
    
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerX);  // Changed to RollerX
    auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(n1, n2);
    truss.addMember(n1, n3);
    truss.addMember(n2, n3);
    truss.applyForce(n3->getId(), Force2D(0.0, -10000.0));
    
    // Analyze
    AnalysisApplicationService analysisService;
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success) << "Analysis failed: " << analysisResult.errorMessage;
    
    // Export to CSV
    {
        std::filesystem::path csvFile = "e2e_export.csv";
        auto exportResult = analysisService.exportResults(
            analysisResult.value,
            ExportFormat::CSV,
            csvFile,
            truss
        );
        EXPECT_TRUE(exportResult.success);
        EXPECT_TRUE(std::filesystem::exists(csvFile));
    }
    
    // Export to XML
    {
        std::filesystem::path xmlFile = "e2e_export.xml";
        auto exportResult = analysisService.exportResults(
            analysisResult.value,
            ExportFormat::XML,
            xmlFile,
            truss
        );
        EXPECT_TRUE(exportResult.success);
        EXPECT_TRUE(std::filesystem::exists(xmlFile));
    }
}

// ============================================================================
// Test 8.5.4: Round-Trip Persistence
// ============================================================================

TEST_F(E2EWorkflowTest, RoundTripPersistenceWorkflow) {
    std::filesystem::path testFile = "e2e_roundtrip.json";
    
    std::string originalName = "Round Trip Test";
    
    // SAVE
    {
        Truss truss(originalName);
        
        auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
        auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
        auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
        auto n4 = truss.addNode(6.0, 3.0, SupportType::Free);
        
        truss.addMember(n1, n2);
        truss.addMember(n1, n3);
        truss.addMember(n2, n3);
        truss.addMember(n2, n4);
        truss.addMember(n3, n4);
        
        truss.applyForce(n3->getId(), Force2D(-1000.0, -5000.0));
        truss.applyForce(n4->getId(), Force2D(1000.0, -3000.0));
        
        auto dto = TrussAssembler::createDTO(truss);
        auto writer = FileIOFactory::createWriter(testFile);
        FileIOOptions options;
        options.prettyPrint = true;
        bool writeSuccess = writer->write(dto, testFile, options);
        EXPECT_TRUE(writeSuccess);
    }
    
    // LOAD
    {
        auto reader = FileIOFactory::createReader(testFile);
        auto dto = reader->read(testFile);
        
        auto loadedTruss = TrussAssembler::assembleTruss(dto);
        
        EXPECT_EQ(loadedTruss->getName(), originalName);
        EXPECT_EQ(loadedTruss->getNodeCount(), 4);
        EXPECT_EQ(loadedTruss->getMemberCount(), 5);
        
        // Verify support types
        size_t pinnedCount = 0, rollerCount = 0, freeCount = 0;
        for (const auto& node : loadedTruss->getNodes()) {
            switch (node->getSupportType()) {
                case SupportType::Pinned: pinnedCount++; break;
                case SupportType::RollerY: rollerCount++; break;
                case SupportType::Free: freeCount++; break;
                default: break;
            }
        }
        EXPECT_EQ(pinnedCount, 1);
        EXPECT_EQ(rollerCount, 1);
        EXPECT_EQ(freeCount, 2);
        
        // Verify loads
        size_t nodesWithLoads = 0;
        for (const auto& node : loadedTruss->getNodes()) {
            if (node->hasAppliedForce()) {
                nodesWithLoads++;
            }
        }
        EXPECT_EQ(nodesWithLoads, 2);
    }
}

// ============================================================================
// Test 8.5.5: Validation-Analysis Integration
// ============================================================================

TEST_F(E2EWorkflowTest, ValidationAnalysisIntegration) {
    TrussValidator validator;
    AnalysisApplicationService analysisService;
    
    // Valid truss should analyze successfully
    {
        Truss truss("Valid Structure");
        auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
        auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerX);  // Changed to RollerX
        auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
        
        truss.addMember(n1, n2);
        truss.addMember(n1, n3);
        truss.addMember(n2, n3);
        truss.applyForce(n3->getId(), Force2D(0.0, -10000.0));
        
        auto validationResult = validator.validate(truss);
        EXPECT_TRUE(validationResult.isValid());
        
        auto analysisResult = analysisService.analyze(truss);
        ASSERT_TRUE(analysisResult.success) << "Analysis failed: " << analysisResult.errorMessage;
        
        const auto& resultsView = analysisService.getResultsView(analysisResult.value);
        EXPECT_TRUE(resultsView.hasConverged());
        EXPECT_GT(resultsView.getTotalDofs(), 0);
    }
    
    // Invalid truss should fail validation
    {
        Truss truss("Invalid Structure");
        auto n1 = truss.addNode(0.0, 0.0, SupportType::Free);
        auto n2 = truss.addNode(4.0, 0.0, SupportType::Free);
        truss.addMember(n1, n2);
        
        auto validationResult = validator.validate(truss);
        EXPECT_FALSE(validationResult.isValid());
    }
}

// ============================================================================
// Test 8.5.6: Complex Multi-Step Workflow
// ============================================================================

TEST_F(E2EWorkflowTest, ComplexMultiStepWorkflow) {
    std::filesystem::path saveFile = "e2e_complex.json";
    
    // STEP 1: Create truss
    Truss truss("Complex Workflow");
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerX);  // Changed to RollerX
    auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(n1, n2);
    truss.addMember(n1, n3);
    truss.addMember(n2, n3);
    truss.applyForce(n3->getId(), Force2D(0.0, -10000.0));
    
    // STEP 2: Validate
    TrussValidator validator;
    auto validation = validator.validate(truss);
    EXPECT_TRUE(validation.isValid());
    
    // STEP 3: Analyze
    AnalysisApplicationService analysisService;
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success) << "Analysis failed: " << analysisResult.errorMessage;
    
    const auto& resultsView = analysisService.getResultsView(analysisResult.value);
    EXPECT_TRUE(resultsView.hasConverged());
    EXPECT_GT(resultsView.getTotalDofs(), 0);
    
    // STEP 4: Save
    auto dto = TrussAssembler::createDTO(truss);
    auto writer = FileIOFactory::createWriter(saveFile);
    FileIOOptions options;
    options.prettyPrint = true;
    bool writeSuccess = writer->write(dto, saveFile, options);
    EXPECT_TRUE(writeSuccess);
    EXPECT_TRUE(std::filesystem::exists(saveFile));
    
    // STEP 5: Export analysis results
    std::filesystem::path csvFile = "e2e_complex.csv";
    auto exportResult = analysisService.exportResults(
        analysisResult.value,
        ExportFormat::CSV,
        csvFile,
        truss
    );
    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(csvFile));
    
    // STEP 6: Reload and verify
    auto reader = FileIOFactory::createReader(saveFile);
    auto dto2 = reader->read(saveFile);
    
    auto reloadedTruss = TrussAssembler::assembleTruss(dto2);
    EXPECT_EQ(reloadedTruss->getName(), "Complex Workflow");
    EXPECT_EQ(reloadedTruss->getNodeCount(), 3);
}

// ============================================================================
// Test 8.5.7: Error Recovery Workflow
// ============================================================================

TEST_F(E2EWorkflowTest, ErrorRecoveryWorkflow) {
    // TEST: Load non-existent file (should throw exception)
    {
        auto reader = FileIOFactory::createReader("nonexistent.json");
        EXPECT_THROW(reader->read("nonexistent.json"), std::exception);
    }
    
    // TEST: Invalid domain operations
    {
        Truss truss("Error Test");
        
        // Remove non-existent member
        bool removed = truss.removeMember(999);
        EXPECT_FALSE(removed);
        
        // Remove non-existent node
        removed = truss.removeNode(999);
        EXPECT_FALSE(removed);
    }
}
