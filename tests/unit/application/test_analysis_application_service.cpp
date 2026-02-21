/**
 * @file test_analysis_application_service.cpp
 * @brief Unit tests for AnalysisApplicationService facade
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Test Coverage:
 * - Analysis execution (success and failure paths)
 * - Results access (view and mutable)
 * - Export operations (multiple formats)
 * - Handle management (multiple results, isolation)
 * - Error handling (Result<T> pattern, validation)
 * - Pre-analysis validation
 */

#include "../../../src/application/AnalysisApplicationService.hpp"
#include "../../../src/application/TrussApplicationService.hpp"
#include "../../../src/core/model/Member.hpp"
#include "../../../src/core/model/Node.hpp"
#include "../../../src/core/model/Truss.hpp"
#include "../../../src/infrastructure/export/exporter_factory.hpp"

#include <filesystem>
#include <gtest/gtest.h>

using namespace truss::application;
using namespace truss::core;
using namespace truss::core::analysis;
using namespace truss::infrastructure::export_;

/**
 * @brief Test fixture for AnalysisApplicationService
 */
class AnalysisApplicationServiceTest : public ::testing::Test {
protected:
    AnalysisApplicationService analysisService;
    std::filesystem::path tempDir;
    MaterialProperties material;
    SectionProperties section;

    void SetUp() override {
        // Create temporary directory for export tests (unique per fixture instance)
        auto uniqueName = "analysis_app_service_test-" +
                          std::to_string(
                              std::chrono::system_clock::now().time_since_epoch().count());
        tempDir = std::filesystem::temp_directory_path() / uniqueName;
        std::filesystem::create_directories(tempDir);

        // Default material properties
        material.youngModulus = 200e9;   // 200 GPa (steel)
        material.yieldStrength = 250e6;  // 250 MPa

        section.area = 0.01;  // 100 cm²
    }

    void TearDown() override {
        // Clean up temporary files
        if (std::filesystem::exists(tempDir)) {
            std::filesystem::remove_all(tempDir);
        }
    }

    /**
     * @brief Create a simple valid triangular truss
     */
    Truss createValidTriangularTruss() {
        Truss truss("Test Truss");

        auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Pinned);
        auto node2 = std::make_shared<Node>(2, Point2D{4.0, 0.0}, SupportType::RollerX);
        auto node3 = std::make_shared<Node>(3, Point2D{2.0, 3.0}, SupportType::Free);

        node3->setAppliedForce(Force2D{0.0, -10000.0});  // 10 kN downward

        truss.addNode(node1);
        truss.addNode(node2);
        truss.addNode(node3);

        auto member1 = std::make_shared<Member>(1, node1, node2, material, section);
        auto member2 = std::make_shared<Member>(2, node1, node3, material, section);
        auto member3 = std::make_shared<Member>(3, node2, node3, material, section);

        truss.addMember(member1);
        truss.addMember(member2);
        truss.addMember(member3);

        return truss;
    }

    /**
     * @brief Create an invalid truss (no nodes)
     */
    Truss createInvalidTruss() {
        Truss truss("Invalid Truss");
        return truss;
    }

    /**
     * @brief Create a truss with insufficient constraints
     */
    Truss createUnsupportedTruss() {
        Truss truss("Unsupported Truss");

        // All free nodes - no supports
        auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Free);
        auto node2 = std::make_shared<Node>(2, Point2D{4.0, 0.0}, SupportType::Free);
        auto node3 = std::make_shared<Node>(3, Point2D{2.0, 3.0}, SupportType::Free);

        node3->setAppliedForce(Force2D{0.0, -10000.0});

        truss.addNode(node1);
        truss.addNode(node2);
        truss.addNode(node3);

        auto member1 = std::make_shared<Member>(1, node1, node2, material, section);
        auto member2 = std::make_shared<Member>(2, node1, node3, material, section);
        auto member3 = std::make_shared<Member>(3, node2, node3, material, section);

        truss.addMember(member1);
        truss.addMember(member2);
        truss.addMember(member3);

        return truss;
    }
};

// ============================================================================
// ANALYSIS EXECUTION TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, Analyze_ValidTruss_Succeeds) {
    Truss truss = createValidTriangularTruss();

    auto result = analysisService.analyze(truss);

    ASSERT_TRUE(result.success) << "Analysis failed with error: " << result.errorMessage;
    EXPECT_GT(result.value, 0);  // Valid handle
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(AnalysisApplicationServiceTest, Analyze_WithDefaultOptions_Succeeds) {
    Truss truss = createValidTriangularTruss();

    auto result = analysisService.analyze(truss);

    ASSERT_TRUE(result.success);
}

TEST_F(AnalysisApplicationServiceTest, Analyze_WithCustomOptions_Succeeds) {
    Truss truss = createValidTriangularTruss();

    AnalysisOptions options;
    options.useDirectSolver = true;
    options.convergenceTolerance = 1e-8;
    options.maxIterations = 1000;

    auto result = analysisService.analyze(truss, options);

    ASSERT_TRUE(result.success);
}

TEST_F(AnalysisApplicationServiceTest, Analyze_EmptyTruss_ReturnsFailure) {
    Truss emptyTruss = createInvalidTruss();

    auto result = analysisService.analyze(emptyTruss);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(AnalysisApplicationServiceTest, Analyze_UnsupportedTruss_ReturnsFailure) {
    Truss unsupportedTruss = createUnsupportedTruss();

    auto result = analysisService.analyze(unsupportedTruss);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(AnalysisApplicationServiceTest, Analyze_MultipleAnalyses_ReturnsUniqueHandles) {
    Truss truss1 = createValidTriangularTruss();
    Truss truss2 = createValidTriangularTruss();

    auto result1 = analysisService.analyze(truss1);
    auto result2 = analysisService.analyze(truss2);

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
    EXPECT_NE(result1.value, result2.value);  // Unique handles
}

TEST_F(AnalysisApplicationServiceTest, Analyze_CreatesResultsHandle) {
    Truss truss = createValidTriangularTruss();

    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    // Should be able to access results
    EXPECT_NO_THROW(analysisService.getResultsView(result.value));
}

// ============================================================================
// RESULTS ACCESS TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, GetResultsView_ValidHandle_ReturnsView) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);

    // Verify view contains results
    auto displacements = view.getDisplacements();
    EXPECT_GT(displacements.size(), 0);
}

TEST_F(AnalysisApplicationServiceTest, GetResultsView_InvalidHandle_ThrowsException) {
    EXPECT_THROW(analysisService.getResultsView(99999), std::invalid_argument);
}

TEST_F(AnalysisApplicationServiceTest, GetResultsView_ContainsDisplacements) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);

    auto displacements = view.getDisplacements();
    // 3 nodes * 2 DOF = 6 displacement values
    EXPECT_EQ(displacements.size(), 6);
}

TEST_F(AnalysisApplicationServiceTest, GetResultsView_ContainsMemberForces) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);

    auto forces = view.getMemberForces();
    // 3 members = 3 force values
    EXPECT_EQ(forces.size(), 3);
}

TEST_F(AnalysisApplicationServiceTest, GetResultsView_ProvidesMaxDisplacement) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);

    Real maxDisp = view.getMaxDisplacement();
    EXPECT_GT(maxDisp, 0.0);  // Should have some displacement
}

TEST_F(AnalysisApplicationServiceTest, GetResultsView_ProvidesMaxStress) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);

    Real maxStress = view.getMaxStress();
    EXPECT_GT(std::abs(maxStress), 0.0);  // Should have some stress
}

TEST_F(AnalysisApplicationServiceTest, GetResults_ValidHandle_ReturnsMutableReference) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    auto& results = analysisService.getResults(result.value);

    // Verify mutable access
    auto& displacements = results.displacements;
    EXPECT_GT(displacements.size(), 0);
}

TEST_F(AnalysisApplicationServiceTest, GetResults_InvalidHandle_ThrowsException) {
    EXPECT_THROW(analysisService.getResults(99999), std::invalid_argument);
}

TEST_F(AnalysisApplicationServiceTest, GetResults_ModificationsPersist) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    // Modify results
    auto& results = analysisService.getResults(result.value);
    Real originalMax = results.maxDisplacement;
    results.maxDisplacement = 999.0;

    // Verify modification persisted
    const auto& view = analysisService.getResultsView(result.value);
    EXPECT_DOUBLE_EQ(view.getMaxDisplacement(), 999.0);

    // Restore original
    results.maxDisplacement = originalMax;
}

// ============================================================================
// EXPORT TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, ExportResults_CSV_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    auto filepath = tempDir / "results.csv";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::CSV, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_JSON_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    auto filepath = tempDir / "results.json";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::JSON, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_XML_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    auto filepath = tempDir / "results.xml";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::XML, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_HTML_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    auto filepath = tempDir / "results.html";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::HTML, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_Text_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    auto filepath = tempDir / "results.txt";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::TXT, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_LaTeX_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    auto filepath = tempDir / "results.tex";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::LaTeX, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_AutoDetectFormat_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    // Export without specifying format (should auto-detect from extension)
    auto filepath = tempDir / "results.csv";
    auto exportResult = analysisService.exportResults(analysisResult.value, filepath, truss);

    EXPECT_TRUE(exportResult.success);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_WithOptions_Succeeds) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    ExportOptions options;
    options.includeReactions = true;
    options.includeDisplacements = true;
    options.includeMemberForces = true;

    auto filepath = tempDir / "results.csv";
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::CSV, filepath, truss, options);

    EXPECT_TRUE(exportResult.success);
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_InvalidHandle_ReturnsFailure) {
    Truss truss = createValidTriangularTruss();

    auto filepath = tempDir / "results.csv";
    auto exportResult = analysisService.exportResults(99999,  // Invalid handle
                                                      ExportFormat::CSV,
                                                      filepath,
                                                      truss);

    EXPECT_FALSE(exportResult.success);
    EXPECT_FALSE(exportResult.errorMessage.empty());
}

TEST_F(AnalysisApplicationServiceTest, ExportResults_InvalidPath_ReturnsFailure) {
    Truss truss = createValidTriangularTruss();
    auto analysisResult = analysisService.analyze(truss);
    ASSERT_TRUE(analysisResult.success);

    // Invalid path
    auto filepath = std::filesystem::path("/invalid/nonexistent/path/results.csv");
    auto exportResult = analysisService.exportResults(
        analysisResult.value, ExportFormat::CSV, filepath, truss);

    EXPECT_FALSE(exportResult.success);
    EXPECT_FALSE(exportResult.errorMessage.empty());
}

// ============================================================================
// HANDLE MANAGEMENT TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, ClearResults_ValidHandle_ReturnsTrue) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    bool cleared = analysisService.clearResults(result.value);

    EXPECT_TRUE(cleared);
}

TEST_F(AnalysisApplicationServiceTest, ClearResults_InvalidHandle_ReturnsFalse) {
    bool cleared = analysisService.clearResults(99999);

    EXPECT_FALSE(cleared);
}

TEST_F(AnalysisApplicationServiceTest, GetResultsView_AfterClear_ThrowsException) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    analysisService.clearResults(result.value);

    EXPECT_THROW(analysisService.getResultsView(result.value), std::invalid_argument);
}

TEST_F(AnalysisApplicationServiceTest, ClearAll_RemovesAllResults) {
    Truss truss = createValidTriangularTruss();
    auto result1 = analysisService.analyze(truss);
    auto result2 = analysisService.analyze(truss);

    analysisService.clearAll();

    EXPECT_THROW(analysisService.getResultsView(result1.value), std::invalid_argument);
    EXPECT_THROW(analysisService.getResultsView(result2.value), std::invalid_argument);
}

TEST_F(AnalysisApplicationServiceTest, MultipleResults_IndependentLifecycles) {
    Truss truss = createValidTriangularTruss();

    auto handle1 = analysisService.analyze(truss).value;
    auto handle2 = analysisService.analyze(truss).value;
    auto handle3 = analysisService.analyze(truss).value;

    // Clear handle2
    analysisService.clearResults(handle2);

    // handle1 and handle3 should still be valid
    EXPECT_NO_THROW(analysisService.getResultsView(handle1));
    EXPECT_NO_THROW(analysisService.getResultsView(handle3));

    // handle2 should be invalid
    EXPECT_THROW(analysisService.getResultsView(handle2), std::invalid_argument);
}

TEST_F(AnalysisApplicationServiceTest, ResultsIsolation_MultipleHandles) {
    Truss truss = createValidTriangularTruss();

    auto handle1 = analysisService.analyze(truss).value;
    auto handle2 = analysisService.analyze(truss).value;

    // Modify results via handle1
    auto& results1 = analysisService.getResults(handle1);
    results1.maxDisplacement = 999.0;

    // Results via handle2 should be unchanged
    const auto& view2 = analysisService.getResultsView(handle2);
    EXPECT_NE(view2.getMaxDisplacement(), 999.0);
}

// ============================================================================
// VALIDATION INTEGRATION TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, Analyze_InvalidTruss_FailsValidation) {
    Truss invalidTruss = createInvalidTruss();

    auto result = analysisService.analyze(invalidTruss);

    EXPECT_FALSE(result.success);
    // Error message should mention validation failure
    EXPECT_NE(result.errorMessage.find("validat"), std::string::npos);
}

TEST_F(AnalysisApplicationServiceTest, Analyze_UnsupportedTruss_FailsValidation) {
    Truss unsupportedTruss = createUnsupportedTruss();

    auto result = analysisService.analyze(unsupportedTruss);

    EXPECT_FALSE(result.success);
}

TEST_F(AnalysisApplicationServiceTest, Analyze_NoLoads_SucceedsWithZeroResults) {
    Truss truss("No Loads");

    auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto node2 = std::make_shared<Node>(2, Point2D{4.0, 0.0}, SupportType::RollerX);
    auto node3 = std::make_shared<Node>(3, Point2D{2.0, 3.0}, SupportType::Free);

    // No loads applied

    truss.addNode(node1);
    truss.addNode(node2);
    truss.addNode(node3);

    auto member1 = std::make_shared<Member>(1, node1, node2, material, section);
    auto member2 = std::make_shared<Member>(2, node1, node3, material, section);
    auto member3 = std::make_shared<Member>(3, node2, node3, material, section);

    truss.addMember(member1);
    truss.addMember(member2);
    truss.addMember(member3);

    auto result = analysisService.analyze(truss);

    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);
    EXPECT_DOUBLE_EQ(view.getMaxDisplacement(), 0.0);
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, ResultType_SuccessCase_HasCorrectFlags) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result);  // operator bool()
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(AnalysisApplicationServiceTest, ResultType_FailureCase_HasCorrectFlags) {
    Truss invalidTruss = createInvalidTruss();
    auto result = analysisService.analyze(invalidTruss);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result);  // operator bool()
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(AnalysisApplicationServiceTest, ErrorMessages_AreDescriptive) {
    Truss invalidTruss = createInvalidTruss();
    auto result = analysisService.analyze(invalidTruss);

    EXPECT_FALSE(result.success);
    // Error message should contain useful information
    EXPECT_GT(result.errorMessage.length(), 10);
}

TEST_F(AnalysisApplicationServiceTest, ExceptionSafety_NoMemoryLeaks) {
    // Test that failed operations don't leak memory
    // (Verified by running with Valgrind or ASAN)

    for (int i = 0; i < 100; ++i) {
        Truss invalidTruss = createInvalidTruss();
        auto result = analysisService.analyze(invalidTruss);
        EXPECT_FALSE(result.success);
    }

    // If no crashes or ASAN errors, exception safety is maintained
    SUCCEED();
}

// ============================================================================
// INTERFACE ABSTRACTION TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, IAnalysisResultsView_ProvidesReadOnlyAccess) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);

    const auto& view = analysisService.getResultsView(result.value);

    // Type check: should return IAnalysisResultsView interface
    const truss::core::interfaces::IAnalysisResultsView& interface = view;
    (void)interface;  // Suppress unused warning

    // Should be able to read data
    EXPECT_GT(view.getDisplacements().size(), 0);
    EXPECT_GT(view.getMemberForces().size(), 0);
    EXPECT_GT(view.getMaxDisplacement(), 0.0);
}

// ============================================================================
// MOVE SEMANTICS TESTS
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, MoveConstructor_TransfersOwnership) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);
    auto handle = result.value;

    // Move service
    AnalysisApplicationService movedService = std::move(analysisService);

    // Moved-to service should have the results
    EXPECT_NO_THROW(movedService.getResultsView(handle));
}

TEST_F(AnalysisApplicationServiceTest, MoveAssignment_TransfersOwnership) {
    Truss truss = createValidTriangularTruss();
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);
    auto handle = result.value;

    // Create another service and move-assign
    AnalysisApplicationService otherService;
    otherService = std::move(analysisService);

    // Moved-to service should have the results
    EXPECT_NO_THROW(otherService.getResultsView(handle));
}

// ============================================================================
// REGRESSION TESTS (Numerical Equivalence)
// ============================================================================

TEST_F(AnalysisApplicationServiceTest, NumericalEquivalence_SimpleTruss) {
    // This test verifies that AnalysisApplicationService produces
    // results numerically equivalent to direct AnalysisOrchestrator usage

    Truss truss = createValidTriangularTruss();

    // Analysis via Application service
    auto result = analysisService.analyze(truss);
    ASSERT_TRUE(result.success);
    const auto& view = analysisService.getResultsView(result.value);

    // Analysis via direct Domain orchestrator
    auto solver = std::make_unique<DirectSolver>();
    auto validator = std::make_unique<validation::TrussValidator>();
    AnalysisOrchestrator orchestrator(std::move(solver), std::move(validator));
    auto directResults = orchestrator.analyze(truss);

    // Compare results
    auto appDisplacements = view.getDisplacements();
    auto directDisplacements = directResults.displacements;

    ASSERT_EQ(appDisplacements.size(), directDisplacements.size());

    for (size_t i = 0; i < appDisplacements.size(); ++i) {
        EXPECT_NEAR(appDisplacements[i], directDisplacements[i], 1e-6)
            << "Displacement mismatch at index " << i;
    }

    EXPECT_NEAR(view.getMaxDisplacement(), directResults.maxDisplacement, 1e-6);
}
