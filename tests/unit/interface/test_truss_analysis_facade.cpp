/**
 * @file test_truss_analysis_facade.cpp
 * @brief Unit tests for TrussAnalysisFacade interface class
 * @version 1.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 *
 * Comprehensive test suite for the TrussAnalysisFacade, covering:
 * - File-based analysis workflows (load → validate → analyze)
 * - Interactive builder-based workflows
 * - Validation without analysis
 * - Export with format auto-detection and explicit format
 * - Resource management (handles, cleanup)
 * - Error propagation and handling
 * - Workflow state management
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>

#include "interface/truss_analysis_facade.hpp"
#include "interface/truss_builder.hpp"
#include "core/model/truss.hpp"
#include "core/analysis/analysis_orchestrator.hpp"
#include "application/truss_application_service.hpp"
#include "application/analysis_application_service.hpp"

using namespace truss;
using namespace truss::interface;
using namespace truss::core;
using namespace truss::application;
using AnalysisOptions = truss::core::analysis::AnalysisOptions;
using ExportOptions = truss::infrastructure::export_::ExportOptions;
namespace fs = std::filesystem;

namespace {

/**
 * @brief Test fixture for TrussAnalysisFacade tests
 */
class TrussAnalysisFacadeTest : public ::testing::Test {
protected:
    std::unique_ptr<TrussAnalysisFacade> facade;
    fs::path testDataDir;
    fs::path tempDir;

    void SetUp() override {
        // Create facade
        facade = std::make_unique<TrussAnalysisFacade>();
        
        // Set up test directories
        testDataDir = fs::path(__FILE__).parent_path().parent_path().parent_path() / "fixtures";
        tempDir = fs::temp_directory_path() / "truss_facade_test";
        fs::create_directories(tempDir);
        
        // Create test JSON file
        createTestJsonFile();
    }

    void TearDown() override {
        // Cleanup temp directory
        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
        
        // Clear facade state
        if (facade) {
            facade->clearAll();
        }
    }

    /**
     * @brief Create a simple test JSON file
     */
    void createTestJsonFile() {
        fs::path jsonFile = tempDir / "simple_truss.json";
        std::ofstream file(jsonFile);
        file << R"({
            "metadata": {
                "version": "1.0",
                "description": "Simple two-node truss"
            },
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0, "support": "Pinned"},
                {"id": 2, "x": 1.0, "y": 0.0, "support": "RollerX"}
            ],
            "members": [
                {
                    "id": 1,
                    "startNode": 1,
                    "endNode": 2,
                    "material": {"name": "Steel", "youngsModulusPa": 200e9},
                    "section": {"profile": "Circular", "areaM2": 0.001}
                }
            ],
            "loads": []
        })";
        file.close();
    }

    /**
     * @brief Create an invalid JSON file
     */
    void createInvalidJsonFile() {
        fs::path jsonFile = tempDir / "invalid.json";
        std::ofstream file(jsonFile);
        file << "{ invalid json content";
        file.close();
    }

    /**
     * @brief Create a truss with validation errors
     */
    void createInvalidTrussFile() {
        fs::path jsonFile = tempDir / "invalid_truss.json";
        std::ofstream file(jsonFile);
        file << R"({
            "metadata": {"version": "1.0"},
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0, "support": "Free"}
            ],
            "members": [],
            "loads": []
        })";  // Invalid: insufficient nodes, no members, no supports
        file.close();
    }

    /**
     * @brief Build a simple valid truss using builder
     */
    TrussBuilder createSimpleBuilder() {
        TrussBuilder builder;
        builder.addNode(0.0, 0.0, SupportType::Pinned);
        builder.addNode(1.0, 0.0, SupportType::RollerX);
        builder.addMember(NodeId(1), NodeId(2));
        return builder;
    }

    /**
     * @brief Build a triangular truss with load
     */
    TrussBuilder createTriangularBuilder() {
        TrussBuilder builder;
        builder.addNode(0.0, 0.0, SupportType::Pinned);
        builder.addNode(2.0, 0.0, SupportType::Pinned);
        builder.addNode(1.0, 1.5, SupportType::Free);
        builder.addMember(NodeId(1), NodeId(2))
               .addMember(NodeId(2), NodeId(3))
               .addMember(NodeId(3), NodeId(1))
               .applyForce(NodeId(3), 0.0, -10000.0);
        return builder;
    }
};

// =============================================================================
// FILE-BASED WORKFLOW TESTS
// =============================================================================

/**
 * @test Facade performs complete analysis from valid file
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeFromFileSuccess) {
    fs::path jsonFile = tempDir / "simple_truss.json";
    AnalysisOptions options;
    
    auto result = facade->analyzeFromFile(jsonFile, options);
    
    ASSERT_TRUE(result.success) << result.errorMessage;
    EXPECT_TRUE(result.errorMessage.empty());
    EXPECT_NE(result.trussHandle, TrussHandle{0});
    EXPECT_NE(result.resultsHandle, ResultsHandle{0});
}

/**
 * @test Facade fails gracefully when file doesn't exist
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeFromFileNotFound) {
    fs::path nonExistent = tempDir / "does_not_exist.json";
    AnalysisOptions options;
    
    auto result = facade->analyzeFromFile(nonExistent, options);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test Facade fails when loading invalid JSON
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeFromFileInvalidJson) {
    createInvalidJsonFile();
    fs::path jsonFile = tempDir / "invalid.json";
    AnalysisOptions options;
    
    auto result = facade->analyzeFromFile(jsonFile, options);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test Facade fails when truss validation fails
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeFromFileValidationFails) {
    createInvalidTrussFile();
    fs::path jsonFile = tempDir / "invalid_truss.json";
    AnalysisOptions options;
    
    auto result = facade->analyzeFromFile(jsonFile, options);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

// =============================================================================
// INTERACTIVE BUILDER WORKFLOW TESTS
// =============================================================================

/**
 * @test Facade analyzes truss from builder
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractiveSuccess) {
    auto builder = createTriangularBuilder();
    AnalysisOptions options;
    
    auto result = facade->analyzeInteractive(builder, options);
    
    ASSERT_TRUE(result.success) << result.errorMessage;
    EXPECT_TRUE(result.errorMessage.empty());
    EXPECT_NE(result.trussHandle, TrussHandle{0});
    EXPECT_NE(result.resultsHandle, ResultsHandle{0});
}

/**
 * @test Facade fails when builder produces invalid truss
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractiveInvalidBuilder) {
    TrussBuilder builder;
    builder.addNode(0.0, 0.0, SupportType::Free);  // Insufficient nodes/members
    AnalysisOptions options;
    
    auto result = facade->analyzeInteractive(builder, options);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test Facade reports validation errors for unstable truss
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractiveValidationFailureReports) {
    TrussBuilder builder;
    builder.addNode(0.0, 0.0, SupportType::Free);
    builder.addNode(1.0, 0.0, SupportType::Free);
    builder.addMember(NodeId(1), NodeId(2));

    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.errorMessage.find("Validation FAILED"), std::string::npos);
}

/**
 * @test Facade analyzes simple two-node truss from builder
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractiveSimpleTruss) {
    auto builder = createSimpleBuilder();
    AnalysisOptions options;
    
    auto result = facade->analyzeInteractive(builder, options);
    
    EXPECT_TRUE(result.success);
}

// =============================================================================
// VALIDATION-ONLY TESTS
// =============================================================================

/**
 * @test Facade validates valid file without analysis
 */
TEST_F(TrussAnalysisFacadeTest, ValidateFromFileSuccess) {
    fs::path jsonFile = tempDir / "simple_truss.json";
    
    auto result = facade->validateFromFile(jsonFile);
    
    EXPECT_TRUE(result.isValid());
    EXPECT_FALSE(result.hasErrors());
}

/**
 * @test Facade detects validation errors in file
 */
TEST_F(TrussAnalysisFacadeTest, ValidateFromFileDetectsErrors) {
    createInvalidTrussFile();
    fs::path jsonFile = tempDir / "invalid_truss.json";
    
    auto result = facade->validateFromFile(jsonFile);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors() || result.hasFatal());
}

/**
 * @test Facade validates builder without permanent storage
 */
TEST_F(TrussAnalysisFacadeTest, ValidateBuilderSuccess) {
    auto builder = createTriangularBuilder();
    
    auto result = facade->validateBuilder(builder);
    
    EXPECT_TRUE(result.isValid());
    EXPECT_FALSE(result.hasErrors());
}

/**
 * @test Facade detects errors in invalid builder
 */
TEST_F(TrussAnalysisFacadeTest, ValidateBuilderDetectsErrors) {
    TrussBuilder builder;
    builder.addNode(0.0, 0.0, SupportType::Free);  // Invalid configuration
    
    auto result = facade->validateBuilder(builder);
    
    EXPECT_FALSE(result.isValid());
}

// =============================================================================
// EXPORT FUNCTIONALITY TESTS
// =============================================================================

/**
 * @test Facade exports results with auto-detected format (JSON)
 */
TEST_F(TrussAnalysisFacadeTest, ExportResultsAutoDetectJson) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);
    
    fs::path outputFile = tempDir / "results.json";
    ExportOptions options;
    
    bool success = facade->exportResults(analysisResult.resultsHandle, outputFile, options);
    
    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(outputFile));
}

/**
 * @test Facade exports results with auto-detected format (CSV)
 */
TEST_F(TrussAnalysisFacadeTest, ExportResultsAutoDetectCsv) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);
    
    fs::path outputFile = tempDir / "results.csv";
    ExportOptions options;
    
    bool success = facade->exportResults(analysisResult.resultsHandle, outputFile, options);
    
    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(outputFile));
}

/**
 * @test Facade exports results with explicit format
 */
TEST_F(TrussAnalysisFacadeTest, ExportResultsExplicitFormat) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);
    
    fs::path outputFile = tempDir / "results.xml";
    ExportOptions options;
    
    bool success = facade->exportResults(analysisResult.resultsHandle, 
                                        infrastructure::export_::ExportFormat::XML,
                                        outputFile, 
                                        options);
    
    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists(outputFile));
}

/**
 * @test Facade fails export with invalid results handle
 */
TEST_F(TrussAnalysisFacadeTest, ExportResultsInvalidHandle) {
    fs::path outputFile = tempDir / "results.json";
    ExportOptions options;
    
    bool success = facade->exportResults(ResultsHandle{999}, outputFile, options);
    
    EXPECT_FALSE(success);
}

/**
 * @test Facade uses last truss handle for export convenience
 */
TEST_F(TrussAnalysisFacadeTest, ExportResultsUsesLastHandle) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);
    
    fs::path outputFile = tempDir / "results.json";
    
    // Should use last results handle automatically
    bool success = facade->exportResults(analysisResult.resultsHandle, outputFile);
    
    EXPECT_TRUE(success);
}

/**
 * @test Facade export fails when last truss handle is invalid
 */
TEST_F(TrussAnalysisFacadeTest, ExportResultsFailsWithoutTrussContext) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);

    facade->clearWorkflow(analysisResult.trussHandle, ResultsHandle{0});

    fs::path outputFile = tempDir / "results.json";
    bool success = facade->exportResults(analysisResult.resultsHandle, outputFile);

    EXPECT_FALSE(success);
}

// =============================================================================
// RESOURCE ACCESS TESTS
// =============================================================================

/**
 * @test Facade provides read-only truss view
 */
TEST_F(TrussAnalysisFacadeTest, GetTrussView) {
    auto builder = createSimpleBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success) << result.errorMessage;
    
    const auto& trussView = facade->getTrussView(result.trussHandle);
    
    EXPECT_EQ(trussView.getNodeCount(), 2);
    EXPECT_EQ(trussView.getMemberCount(), 1);
}

/**
 * @test Facade provides read-only results view
 */
TEST_F(TrussAnalysisFacadeTest, GetResultsView) {
    auto builder = createTriangularBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success) << result.errorMessage;
    
    const auto& resultsView = facade->getResultsView(result.resultsHandle);
    
    // Should have displacement and force results
    auto displacements = resultsView.getDisplacements();
    EXPECT_GT(displacements.size(), 0);
}

/**
 * @test Facade provides mutable truss access for advanced users
 */
TEST_F(TrussAnalysisFacadeTest, GetTrussMutable) {
    auto builder = createSimpleBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success);
    
    auto& mutableTruss = facade->getTrussMutable(result.trussHandle);
    
    // Should be able to modify
    size_t originalNodeCount = mutableTruss.getNodeCount();
    EXPECT_GT(originalNodeCount, 0);
}

// =============================================================================
// LIFECYCLE MANAGEMENT TESTS
// =============================================================================

/**
 * @test Facade clears workflow state selectively
 */
TEST_F(TrussAnalysisFacadeTest, ClearWorkflow) {
    auto builder = createTriangularBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success);
    
    facade->clearWorkflow(result.trussHandle, result.resultsHandle);
    
    // After clearing workflow, handles should be invalid
    EXPECT_THROW(facade->getTrussView(result.trussHandle), std::exception);
}

/**
 * @test Facade clears all managed resources
 */
TEST_F(TrussAnalysisFacadeTest, ClearAll) {
    auto builder = createTriangularBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success);
    
    facade->clearAll();
    
    // After clearing all, handles should be invalid
    EXPECT_THROW(facade->getTrussView(result.trussHandle), std::exception);
    EXPECT_THROW(facade->getResultsView(result.resultsHandle), std::exception);
}

/**
 * @test Facade manages multiple trusses independently
 */
TEST_F(TrussAnalysisFacadeTest, ManageMultipleTrusses) {
    auto builder1 = createSimpleBuilder();
    auto result1 = facade->analyzeInteractive(builder1, AnalysisOptions{});
    ASSERT_TRUE(result1.success) << result1.errorMessage;
    
    auto builder2 = createTriangularBuilder();
    auto result2 = facade->analyzeInteractive(builder2, AnalysisOptions{});
    ASSERT_TRUE(result2.success);
    
    // Both should be accessible
    EXPECT_NO_THROW(facade->getTrussView(result1.trussHandle));
    EXPECT_NO_THROW(facade->getTrussView(result2.trussHandle));
    
    // Should have different node counts
    EXPECT_NE(
        facade->getTrussView(result1.trussHandle).getNodeCount(),
        facade->getTrussView(result2.trussHandle).getNodeCount()
    );
}

/**
 * @test Facade validates handle state
 */
TEST_F(TrussAnalysisFacadeTest, HandleValidation) {
    auto builder = createSimpleBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success);

    EXPECT_TRUE(facade->isValidTrussHandle(result.trussHandle));
    EXPECT_TRUE(facade->isValidResultsHandle(result.resultsHandle));
    EXPECT_FALSE(facade->isValidTrussHandle(TrussHandle{0}));
    EXPECT_FALSE(facade->isValidResultsHandle(ResultsHandle{0}));
}

// =============================================================================
// LOWER-LEVEL HELPER TESTS
// =============================================================================

/**
 * @test Facade creates empty truss
 */
TEST_F(TrussAnalysisFacadeTest, CreateEmptyTruss) {
    auto handle = facade->createEmptyTruss("TestTruss");
    
    EXPECT_NE(handle, TrussHandle{0});
    
    const auto& truss = facade->getTrussView(handle);
    EXPECT_EQ(truss.getNodeCount(), 0);
}

/**
 * @test Facade loads truss without analysis
 */
TEST_F(TrussAnalysisFacadeTest, LoadTrussOnly) {
    fs::path jsonFile = tempDir / "simple_truss.json";
    
    auto trussHandle = facade->loadTrussOnly(jsonFile);
    
    EXPECT_NE(trussHandle, TrussHandle{0});
}

/**
 * @test Facade performs analysis only on existing truss
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeOnly) {
    fs::path jsonFile = tempDir / "simple_truss.json";
    auto trussHandle = facade->loadTrussOnly(jsonFile);
    ASSERT_NE(trussHandle, TrussHandle{0});
    
    AnalysisOptions options;
    auto resultsHandle = facade->analyzeOnly(trussHandle, options);
    
    EXPECT_NE(resultsHandle, ResultsHandle{0});
}

/**
 * @test Facade returns 0 for analysis on invalid handle
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeOnlyInvalidHandle) {
    auto resultsHandle = facade->analyzeOnly(TrussHandle{999}, AnalysisOptions{});
    EXPECT_EQ(resultsHandle, ResultsHandle{0});
}

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

/**
 * @test Facade propagates errors from application services
 */
TEST_F(TrussAnalysisFacadeTest, ErrorPropagationFromServices) {
    TrussBuilder invalidBuilder;
    invalidBuilder.addNode(0.0, 0.0, SupportType::Free);
    
    auto result = facade->analyzeInteractive(invalidBuilder, AnalysisOptions{});
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test Facade provides structured validation error messages
 */
TEST_F(TrussAnalysisFacadeTest, StructuredValidationErrors) {
    createInvalidTrussFile();
    fs::path jsonFile = tempDir / "invalid_truss.json";
    
    auto result = facade->validateFromFile(jsonFile);
    
    EXPECT_FALSE(result.isValid());
    auto errors = result.getErrorMessages();
    EXPECT_GT(errors.size(), 0);
}

}  // namespace
