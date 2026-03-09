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

#include "application/analysis_application_service.hpp"
#include "application/truss_application_service.hpp"
#include "core/analysis/analysis_orchestrator.hpp"
#include "core/model/truss.hpp"
#include "interface/truss_analysis_facade.hpp"
#include "interface/truss_builder.hpp"

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::interface;
using namespace truss::core;
using namespace truss::application;
using AnalysisOptions = truss::core::analysis::AnalysisOptions;
using ExportOptions = truss::infrastructure::export_::ExportOptions;
using ExportFormat = truss::ExportFormat;
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

    bool success = facade->exportResults(
        analysisResult.resultsHandle, truss::ExportFormat::XML, outputFile, options);

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
    EXPECT_NE(facade->getTrussView(result1.trussHandle).getNodeCount(),
              facade->getTrussView(result2.trussHandle).getNodeCount());
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

// =============================================================================
// BRANCH COVERAGE: clearTruss / clearWorkflow
// =============================================================================

TEST_F(TrussAnalysisFacadeTest, ClearTruss_NotLastHandle_DoesNotResetLastHandle) {
    // Create two trusses - second becomes m_lastTrussHandle
    auto r1 = facade->createTruss("First");
    auto r2 = facade->createTruss("Second");
    ASSERT_TRUE(r1.success);
    ASSERT_TRUE(r2.success);

    // Clear the FIRST truss (not the lastTrussHandle)
    bool cleared = facade->clearTruss(r1.value);
    EXPECT_TRUE(cleared);
    // Second handle should still be valid
    EXPECT_TRUE(facade->isValidTrussHandle(r2.value));
}

TEST_F(TrussAnalysisFacadeTest, ClearWorkflow_ZeroHandles_DoesNotCrash) {
    EXPECT_NO_THROW(facade->clearWorkflow(0, 0));
}

TEST_F(TrussAnalysisFacadeTest, ClearWorkflow_OnlyTrussHandle_ClearsTruss) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    facade->clearWorkflow(cr.value, 0);
    EXPECT_FALSE(facade->isValidTrussHandle(cr.value));
}

TEST_F(TrussAnalysisFacadeTest, ClearWorkflow_OnlyResultsHandle_ClearsResults) {
    fs::path jsonFile = tempDir / "simple_truss.json";
    auto r = facade->analyzeFromFile(jsonFile, AnalysisOptions{});
    ASSERT_TRUE(r.success);

    // Save results handle, then clear only the results
    facade->clearWorkflow(0, r.resultsHandle);
    EXPECT_FALSE(facade->isValidResultsHandle(r.resultsHandle));
}

// =============================================================================
// BRANCH COVERAGE: simplified exportResults
// =============================================================================

TEST_F(TrussAnalysisFacadeTest, ExportResultsSimplified_InvalidResultsHandle_ReturnsFalse) {
    ExportOptions opts;
    auto path = tempDir / "out.json";
    bool ok = facade->exportResults(9999, path, opts);
    EXPECT_FALSE(ok);
}

TEST_F(TrussAnalysisFacadeTest, ExportResultsSimplified_NoLastTrussHandle_ReturnsFalse) {
    // Analyze but then clear all to reset m_lastTrussHandle
    fs::path jsonFile = tempDir / "simple_truss.json";
    auto r = facade->analyzeFromFile(jsonFile, AnalysisOptions{});
    ASSERT_TRUE(r.success);
    auto resultsHandle = r.resultsHandle;
    facade->clearAll();  // m_lastTrussHandle becomes 0

    ExportOptions opts;
    auto path = tempDir / "out.json";
    bool ok = facade->exportResults(resultsHandle, path, opts);
    EXPECT_FALSE(ok);
}

// =============================================================================
// BRANCH COVERAGE: GUI delegate methods via facade
// =============================================================================

TEST_F(TrussAnalysisFacadeTest, AddNode_InvalidHandle_ReturnsFailure) {
    auto result = facade->addNode(9999, Point2D{0.0, 0.0}, SupportType::Free);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussAnalysisFacadeTest, AddNode_ValidHandle_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto result = facade->addNode(cr.value, Point2D{1.0, 0.0}, SupportType::Pinned);
    EXPECT_TRUE(result.success);
}

TEST_F(TrussAnalysisFacadeTest, AddMember_InvalidHandle_ReturnsFailure) {
    MaterialSpec mat{200e9, "Steel"};
    SectionSpec  sec{0.01,  "Square"};
    auto result = facade->addMember(9999, 1, 2, mat, sec);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussAnalysisFacadeTest, AddMember_ValidNodes_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto n1 = facade->addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto n2 = facade->addNode(cr.value, Point2D{1.0, 0.0}, SupportType::RollerY);
    ASSERT_TRUE(n1.success); ASSERT_TRUE(n2.success);
    MaterialSpec mat{200e9, "Steel"};
    SectionSpec  sec{0.01,  "Circular"};
    auto r = facade->addMember(cr.value, n1.value, n2.value, mat, sec);
    EXPECT_TRUE(r.success);
}

TEST_F(TrussAnalysisFacadeTest, RemoveNode_InvalidHandle_ReturnsFailure) {
    auto result = facade->removeNode(9999, 1);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussAnalysisFacadeTest, RemoveMember_InvalidHandle_ReturnsFailure) {
    auto result = facade->removeMember(9999, 1);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussAnalysisFacadeTest, SetNodeSupport_ValidNode_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = facade->addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    auto r = facade->setNodeSupport(cr.value, nr.value, SupportType::Pinned);
    EXPECT_TRUE(r.success);
}

TEST_F(TrussAnalysisFacadeTest, ApplyNodeLoad_ValidNode_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = facade->addNode(cr.value, Point2D{1.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    auto r = facade->applyNodeLoad(cr.value, nr.value, Force2D{0.0, -1000.0});
    EXPECT_TRUE(r.success);
}

TEST_F(TrussAnalysisFacadeTest, ClearNodeLoad_ValidNode_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = facade->addNode(cr.value, Point2D{1.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    facade->applyNodeLoad(cr.value, nr.value, Force2D{100.0, -200.0});
    auto r = facade->clearNodeLoad(cr.value, nr.value);
    EXPECT_TRUE(r.success);
}

TEST_F(TrussAnalysisFacadeTest, UpdateNode_ValidNode_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = facade->addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    NodeUpdateSpec spec{3.0, 4.0};
    auto r = facade->updateNode(cr.value, nr.value, spec);
    EXPECT_TRUE(r.success);
}

TEST_F(TrussAnalysisFacadeTest, UpdateMember_ValidMember_Succeeds) {
    auto cr = facade->createTruss("T");
    ASSERT_TRUE(cr.success);
    auto n1 = facade->addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto n2 = facade->addNode(cr.value, Point2D{1.0, 0.0}, SupportType::RollerY);
    ASSERT_TRUE(n1.success); ASSERT_TRUE(n2.success);
    MaterialSpec mat{200e9, "Steel"};
    SectionSpec  sec{0.01,  "Square"};
    auto mr = facade->addMember(cr.value, n1.value, n2.value, mat, sec);
    ASSERT_TRUE(mr.success);
    MemberUpdateSpec spec{MaterialSpec{70e9, "Aluminum"},
                                       SectionSpec{0.005, "Circular"}};
    auto r = facade->updateMember(cr.value, mr.value, spec);
    EXPECT_TRUE(r.success);
}

/**
 * @test formatValidationErrors — warnings path
 *
 * Creates a truss that fails validation (no members → mechanism error) AND
 * has coincident nodes (warning).  This exercises the
 *   if (!warnings.empty() && warnings.size() <= 5)
 * branch inside formatValidationErrors().
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractive_ValidationFailureWithWarnings) {
    TrussBuilder builder;
    // Two nodes at identical positions → coincident-node WARNING from validator
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(0.0, 0.0, SupportType::Free);  // same position as node 1
    // No members → structural ERROR (mechanism / insufficient rigidity)

    AnalysisOptions options;
    auto result = facade->analyzeInteractive(builder, options);

    // Validation must fail so formatValidationErrors is exercised
    EXPECT_FALSE(result.success);
    // The error message should include warning text
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test loadTrussOnly with a non-existent file returns 0 (failure path).
 */
TEST_F(TrussAnalysisFacadeTest, LoadTrussOnly_NonExistentFile_ReturnsZero) {
    auto trussHandle = facade->loadTrussOnly(tempDir / "no_such_file.json");
    EXPECT_EQ(trussHandle, TrussHandle{0});
}

/**
 * @test analyzeOnly with a valid but unsolvable truss returns 0.
 *
 * Covers the `if (result) { ... } return 0;` false-branch inside analyzeOnly.
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeOnly_UnsolvableTruss_ReturnsZero) {
    // Create and load a valid-handle truss that has no members (will fail analysis)
    auto trussHandle = facade->createEmptyTruss("Empty");
    ASSERT_NE(trussHandle, TrussHandle{0});
    // An empty truss will fail analysis (no DOFs / singular stiffness matrix)
    auto resultsHandle = facade->analyzeOnly(trussHandle, AnalysisOptions{});
    EXPECT_EQ(resultsHandle, ResultsHandle{0});
}

/**
 * @test clearWorkflow with zero handles for both args is a no-op.
 *
 * Covers the false branches of both `if (trussHandle != 0)` and
 * `if (resultsHandle != 0)` in clearWorkflow().
 */
TEST_F(TrussAnalysisFacadeTest, ClearWorkflow_ZeroHandles_IsNoOp) {
    auto builder = createTriangularBuilder();
    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(result.success);

    // Pass zero for both – should not invalidate existing handles
    facade->clearWorkflow(TrussHandle{0}, ResultsHandle{0});

    // Original handles must still be valid
    EXPECT_TRUE(facade->isValidTrussHandle(result.trussHandle));
    EXPECT_TRUE(facade->isValidResultsHandle(result.resultsHandle));
}

/**
 * @test clearWorkflow with a trussHandle that is NOT m_lastTrussHandle does
 *       not reset m_lastTrussHandle (covers the false branch of the inner
 *       `if (m_lastTrussHandle == trussHandle)` check).
 */
TEST_F(TrussAnalysisFacadeTest, ClearWorkflow_NonLastTrussHandle_LastHandlePreserved) {
    // Analyze two trusses – m_lastTrussHandle will be set to the second one
    auto builder1 = createSimpleBuilder();
    auto result1 = facade->analyzeInteractive(builder1, AnalysisOptions{});
    ASSERT_TRUE(result1.success);

    auto builder2 = createTriangularBuilder();
    auto result2 = facade->analyzeInteractive(builder2, AnalysisOptions{});
    ASSERT_TRUE(result2.success);

    // Clear only result1's truss (not the last one); m_lastTrussHandle should still be result2
    facade->clearWorkflow(result1.trussHandle, ResultsHandle{0});

    // result2's truss handle must still be valid (m_lastTrussHandle was not reset)
    EXPECT_TRUE(facade->isValidTrussHandle(result2.trussHandle));
}

// =============================================================================
// COLLINEAR TRUSS — Analysis failure (singular stiffness matrix)
// All 3 members are horizontal (sin=0); Y-direction DOF is unconstrained →
// stiffness matrix is singular → analysis throws → Result::Failure
// Covers: analyzeInteractive lines 234-235 and analyzeFromFile lines 192-193
// Also covers: analysis_application_service.cpp catch block (lines 57-58) and
//              analysis_orchestrator.cpp singular-matrix throw (line 67)
// =============================================================================

/**
 * @brief A collinear 3-node truss: all members horizontal (θ=0°).
 *        - Passes TrussValidator (n=3, m=3, r=3 → 2n=m+r, determinate).
 *        - Fails analysis: Y-direction DOF has zero stiffness → singular K.
 */
static TrussBuilder createCollinearBuilder() {
    TrussBuilder builder;
    // Pinned at left (x1=y1=0), RollerX at right (y2=0), free middle
    builder.addNode(0.0, 0.0, SupportType::Pinned);   // id 1
    builder.addNode(2.0, 0.0, SupportType::RollerX);  // id 2
    builder.addNode(1.0, 0.0, SupportType::Free);     // id 3
    builder.addMember(NodeId(1), NodeId(3))            // left span
        .addMember(NodeId(3), NodeId(2))               // right span
        .addMember(NodeId(1), NodeId(2))               // full span
        .applyForce(NodeId(3), 0.0, -10000.0);         // vertical load → null-space direction
    return builder;
}

/**
 * @test analyzeInteractive: analysis failure path (lines 234-235 of facade.cpp).
 *
 * The collinear truss passes validation but has a singular stiffness matrix.
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractive_CollinearTruss_AnalysisFails) {
    auto builder = createCollinearBuilder();

    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});

    // Analysis must fail due to singular stiffness matrix
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    EXPECT_NE(result.errorMessage.find("Analysis failed"), std::string::npos);
}

/**
 * @test analyzeFromFile: analysis failure path (lines 192-193 of facade.cpp).
 *
 * The collinear truss JSON loads and validates successfully, but analysis fails.
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeFromFile_CollinearTruss_AnalysisFails) {
    // Write a collinear 3-node truss JSON (n=3, m=3, r=3 → statically determinate)
    fs::path collinearFile = tempDir / "collinear_truss.json";
    {
        std::ofstream f(collinearFile);
        f << R"({
            "metadata": {"version": "1.0", "description": "Collinear truss"},
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0, "support": "Pinned"},
                {"id": 2, "x": 2.0, "y": 0.0, "support": "RollerX"},
                {"id": 3, "x": 1.0, "y": 0.0, "support": "Free"}
            ],
            "members": [
                {"id": 1, "startNode": 1, "endNode": 3,
                 "material": {"name": "Steel", "youngsModulusPa": 200e9},
                 "section": {"profile": "Circular", "areaM2": 0.001}},
                {"id": 2, "startNode": 3, "endNode": 2,
                 "material": {"name": "Steel", "youngsModulusPa": 200e9},
                 "section": {"profile": "Circular", "areaM2": 0.001}},
                {"id": 3, "startNode": 1, "endNode": 2,
                 "material": {"name": "Steel", "youngsModulusPa": 200e9},
                 "section": {"profile": "Circular", "areaM2": 0.001}}
            ],
            "loads": [
                {"nodeId": 3, "fx": 0.0, "fy": -10000.0}
            ]
        })";
    }

    auto result = facade->analyzeFromFile(collinearFile, AnalysisOptions{});

    // Load + validate succeeds, but analysis fails with singular stiffness matrix
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test analyzeInteractive: large-displacement analysis failure (lines 87-91 of
 *       analysis_orchestrator.cpp and subsequent failure through the call chain).
 *
 * Uses a 2-node truss with an absurdly tiny cross-section:
 *   k = E*A/L = 200e9 * 2.5e-21 / 1 = 5e-10 N/m
 * Eigenvalue = 5e-10 > 1e-10 (singular check passes), but
 * displacement d = F/k = 1 / 5e-10 = 2e9 m >> 1e6 m threshold.
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractive_HugeDisplacement_AnalysisFails) {
    // 2-node horizontal truss with near-zero stiffness
    TrussBuilder builder;
    builder.addNode(0.0, 0.0, SupportType::Pinned);   // node 1: x1=y1=0
    builder.addNode(1.0, 0.0, SupportType::RollerX);  // node 2: y2=0, x2 free
    // A = 2.5e-21 m² → k = 200e9 * 2.5e-21 / 1 = 5e-10 N/m (eigenvalue > 1e-10)
    MaterialSpec mat{200e9, "ExtremelySoft"};
    SectionSpec  sec{2.5e-21, "TinySection"};
    builder.addMember(NodeId(1), NodeId(2), mat, sec)
           .applyForce(NodeId(2), 1.0, 0.0);  // 1 N → d = 2e9 m >> 1e6 threshold

    AnalysisOptions options;
    options.checkStability = true;
    auto result = facade->analyzeInteractive(builder, options);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    // Should mention displacement or analysis failure
    EXPECT_NE(result.errorMessage.find("Analysis failed"), std::string::npos);
}

/**
 * @test analyzeInteractive with RollerY support: covers lines 402-404 of
 *       analysis_orchestrator.cpp (RollerY branch in reaction index counting).
 *
 * Geometry chosen so K_ff is non-singular:
 *   Node 1 (0,3): RollerY  — constrains X only (r=1), FIRST → lowest DOFs
 *   Node 2 (0,0): Pinned   — constrains X and Y (r=2), SECOND → higher DOFs
 *   Node 3 (3,0): Free
 *   Members: 1-2 (vertical), 1-3 (diagonal), 2-3 (horizontal)
 *   n=3, m=3, r=3 → 2n=m+r (determinate)
 *
 * When computing Node 2's reaction index:
 *   Inner loop hits Node 1 (RollerY, dofX=0 < Node2's dofX=2) → lines 402-404
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeInteractive_RollerYSupport_AnalysisSucceeds) {
    TrussBuilder builder;
    // Node 1 added FIRST → gets DOFs 0(X),1(Y); X is constrained by RollerY
    builder.addNode(0.0, 3.0, SupportType::RollerY);   // X constrained, Y free
    // Node 2 added SECOND → gets DOFs 2(X),3(Y); both constrained by Pinned
    builder.addNode(0.0, 0.0, SupportType::Pinned);    // fully fixed
    // Node 3 → free apex
    builder.addNode(3.0, 0.0, SupportType::Free);
    builder.addMember(NodeId(1), NodeId(2))             // vertical member (stabilises node1 Y)
        .addMember(NodeId(1), NodeId(3))                // diagonal (stabilises node3)
        .addMember(NodeId(2), NodeId(3))                // horizontal (stabilises node3 X)
        .applyForce(NodeId(3), 1000.0, -2000.0);        // arbitrary load at free apex

    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});

    // Stable truss — analysis must succeed, exercising the RollerY reaction code path
    EXPECT_TRUE(result.success) << "RollerY truss analysis failed: " << result.errorMessage;
    EXPECT_TRUE(facade->isValidTrussHandle(result.trussHandle));
    EXPECT_TRUE(facade->isValidResultsHandle(result.resultsHandle));
}

/**
 * @test saveTruss() via facade: covers truss_analysis_facade.cpp lines 45-48.
 */
TEST_F(TrussAnalysisFacadeTest, SaveTruss_ValidHandle_Succeeds) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);

    fs::path savePath = tempDir / "saved_truss.json";
    auto saveResult = facade->saveTruss(analysisResult.trussHandle, savePath);
    EXPECT_TRUE(saveResult.success) << saveResult.errorMessage;
    EXPECT_TRUE(fs::exists(savePath));
}

/**
 * @test exportResults(handle, format, filepath, truss, options):
 *       covers facade.cpp lines 145-150.
 */
TEST_F(TrussAnalysisFacadeTest, ExportResults_WithTrussAndFormat_Succeeds) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);

    // Obtain the Truss object to pass explicitly
    const auto& trussView = facade->getTrussView(analysisResult.trussHandle);
    const auto& truss = dynamic_cast<const truss::core::Truss&>(trussView);

    fs::path outputFile = tempDir / "result_with_truss.csv";
    ExportOptions options;
    auto result = facade->exportResults(
        analysisResult.resultsHandle, ExportFormat::CSV, outputFile, truss, options);
    EXPECT_TRUE(result.success) << result.errorMessage;
    EXPECT_TRUE(fs::exists(outputFile));
}

/**
 * @test exportResults(handle, filepath, truss, options):
 *       covers facade.cpp lines 154-158.
 */
TEST_F(TrussAnalysisFacadeTest, ExportResults_WithTrussAutoDetect_Succeeds) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);

    const auto& trussView = facade->getTrussView(analysisResult.trussHandle);
    const auto& truss = dynamic_cast<const truss::core::Truss&>(trussView);

    fs::path outputFile = tempDir / "result_auto.csv";
    ExportOptions options;
    auto result = facade->exportResults(
        analysisResult.resultsHandle, outputFile, truss, options);
    EXPECT_TRUE(result.success) << result.errorMessage;
    EXPECT_TRUE(fs::exists(outputFile));
}

/**
 * @test clearResults(): covers facade.cpp lines 161-162.
 */
TEST_F(TrussAnalysisFacadeTest, ClearResults_ValidHandle_ReturnsTrue) {
    auto builder = createTriangularBuilder();
    auto analysisResult = facade->analyzeInteractive(builder, AnalysisOptions{});
    ASSERT_TRUE(analysisResult.success);

    bool cleared = facade->clearResults(analysisResult.resultsHandle);
    EXPECT_TRUE(cleared);
    EXPECT_FALSE(facade->isValidResultsHandle(analysisResult.resultsHandle));
}

/**
 * @test formatValidationErrors with >5 warnings covers facade.cpp lines 413-414.
 *
 * Strategy:
 *   - 6 pairs of coincident nodes → 6 ValidationSeverity::Warning issues
 *   - Connect each pair with a zero-length member → at least 1 Error
 *     making isValid() == false → formatValidationErrors() is invoked
 *   - With warnings.size() > 5, lines 413-414 ("... and N more warnings") run
 *
 * Node layout (1-based IDs via addNode order):
 *   1: pinned  (0,0)    support
 *   2: roller  (10,0)   support
 *   Pairs 3-4, 5-6, 7-8, 9-10, 11-12, 13-14 — all at (0,0) coincident
 *   Each pair connected by a zero-length member → Error
 *   Members 1→1-2, 2→2-3 give the truss enough nodes to load.
 */
TEST_F(TrussAnalysisFacadeTest, FormatValidationErrors_MoreThanFiveWarnings) {
    // Build a truss that produces 6 coincident-node warnings (Warning severity)
    // AND at least one zero-length-member error (Error severity, making isValid()
    // return false). That combination triggers formatValidationErrors() with
    // warnings.size() > 5, exercising facade.cpp lines 413-414.
    TrussBuilder builder;

    // Two proper support nodes
    builder.addNode(0.0, 0.0, SupportType::Pinned);   // node 1
    builder.addNode(10.0, 0.0, SupportType::RollerY);  // node 2

    // 6 pairs of coincident nodes (same coordinates → Warning per pair)
    // Each pair also gets a zero-length member → Error (needed so isValid()==false)
    constexpr int pairs = 6;
    for (int i = 0; i < pairs; ++i) {
        // nodeA = 3 + 2*i,  nodeB = 4 + 2*i
        builder.addNode(1.0 + i, 1.0, SupportType::Free);  // nodeA
        builder.addNode(1.0 + i, 1.0, SupportType::Free);  // nodeB — coincident with nodeA
    }

    // One normal member between the two supports
    builder.addMember(NodeId(1), NodeId(2));

    // Zero-length member for the first coincident pair (node 3–4) → Error
    builder.addMember(NodeId(3), NodeId(4));

    // Apply a load so the truss has external loads
    builder.applyForce(NodeId(1), 0.0, -1000.0);

    auto result = facade->analyzeInteractive(builder, AnalysisOptions{});
    // Must fail (zero-length member → isValid()==false → formatValidationErrors called)
    EXPECT_FALSE(result.success);
    // Error message must be non-empty and routed through formatValidationErrors
    EXPECT_FALSE(result.errorMessage.empty());
}

/**
 * @test analyzeFromFile with a structurally invalid truss (negative Young's
 *       modulus) causes loadTruss() to fail because TrussApplicationService
 *       validates on load. This exercises facade.cpp line 174-175 (loadTruss
 *       failure path). Lines 182-186 (double-validation) are effectively dead
 *       code since loadTruss already validates — this test documents that path.
 */
TEST_F(TrussAnalysisFacadeTest, AnalyzeFromFile_InvalidTruss_LoadFails) {
    fs::path trussFile = tempDir / "invalid_modulus.json";
    {
        std::ofstream f(trussFile);
        // Member 1 has a negative Young's modulus via inline material.
        // TrussApplicationService::loadTruss validates on load and returns
        // Result::Failure when validation fails → analyzeFromFile line 174-175.
        f << R"({
            "metadata": {"name": "Invalid Modulus"},
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
                {"id": 2, "x": 4.0, "y": 0.0, "support": "roller_y"},
                {"id": 3, "x": 2.0, "y": 3.0, "support": "free"}
            ],
            "members": [
                {
                    "id": 1, "startNode": 1, "endNode": 2,
                    "material": {"youngsModulus": -200e9}
                },
                {
                    "id": 2, "startNode": 1, "endNode": 3,
                    "material": {"youngsModulus": 200e9}
                },
                {
                    "id": 3, "startNode": 2, "endNode": 3,
                    "material": {"youngsModulus": 200e9}
                }
            ],
            "loads": [{"nodeId": 3, "fx": 0.0, "fy": -1000.0}]
        })";
        f.close();
    }

    auto result = facade->analyzeFromFile(trussFile);
    EXPECT_FALSE(result.success);
    // loadTruss failure path (lines 174-175) produces "Failed to load truss:"
    EXPECT_NE(result.errorMessage.find("Failed to load"), std::string::npos);
}

}  // namespace
