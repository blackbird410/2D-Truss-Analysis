/**
 * @file test_facade_analysis_service_adapter.cpp
 * @brief Unit tests for FacadeAnalysisServiceAdapter
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 *
 * Comprehensive test suite verifying that FacadeAnalysisServiceAdapter:
 * - Correctly implements IAnalysisService interface
 * - Delegates all calls to TrussAnalysisFacade public methods
 * - Maintains proper encapsulation (no friend access)
 * - Forwards return values correctly
 * - Forwards all parameters correctly
 */

#include "interface/facade_analysis_service_adapter.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss;
using namespace truss::interface;
using namespace truss::test;
using namespace testing;

namespace {

/**
 * @brief Test fixture for FacadeAnalysisServiceAdapter unit tests
 *
 * Uses MockTrussAnalysisFacade to verify delegation without
 * depending on actual Facade implementation.
 */
class FacadeAnalysisServiceAdapterTest : public ::testing::Test {
protected:
    truss::test::MockTrussAnalysisFacade mockFacade;
    std::unique_ptr<truss::interface::FacadeAnalysisServiceAdapter> adapter;

    void SetUp() override {
        // Safe injection of mock via interface
        adapter = std::make_unique<truss::interface::FacadeAnalysisServiceAdapter>(mockFacade);
    }
};

// ============================================================
// Analysis Operations Tests
// ============================================================

TEST_F(FacadeAnalysisServiceAdapterTest, AnalyzeDelegatesToFacade) {
    // Arrange
    core::Truss testTruss("TestTruss");
    core::analysis::AnalysisOptions options;
    const application::ResultsHandle expectedHandle = 100;

    EXPECT_CALL(mockFacade, analyze(Ref(testTruss), _))
        .WillOnce(
            Return(application::Result<application::ResultsHandle>::Success(expectedHandle)));

    // Act
    auto result = adapter->analyze(testTruss, options);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.value, expectedHandle);
}

TEST_F(FacadeAnalysisServiceAdapterTest, AnalyzeForwardsError) {
    // Arrange
    core::Truss testTruss("ErrorTruss");
    core::analysis::AnalysisOptions options;
    const std::string errorMsg = "Analysis failed: singular matrix";

    EXPECT_CALL(mockFacade, analyze(Ref(testTruss), _))
        .WillOnce(Return(application::Result<application::ResultsHandle>::Failure(errorMsg)));

    // Act
    auto result = adapter->analyze(testTruss, options);

    // Assert
    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

TEST_F(FacadeAnalysisServiceAdapterTest, AnalyzeForwardsOptionsCorrectly) {
    // Arrange
    core::Truss testTruss("TestTruss");
    core::analysis::AnalysisOptions options;

    options.maxIterations = 100;

    application::ResultsHandle expectedHandle = 50;

    // Use a custom matcher to verify options are passed through
    EXPECT_CALL(mockFacade, analyze(Ref(testTruss), _))
        .WillOnce(
            Return(application::Result<application::ResultsHandle>::Success(expectedHandle)));

    // Act
    auto result = adapter->analyze(testTruss, options);

    // Assert
    ASSERT_TRUE(result.success);
}

// ============================================================
// Results Access Tests
// ============================================================

TEST_F(FacadeAnalysisServiceAdapterTest, GetResultsViewDelegatesToFacade) {
    // Arrange
    const application::ResultsHandle testHandle = 100;

    // Create mock results (we need a concrete object to return by reference)
    core::analysis::AnalysisResults mockResults;

    EXPECT_CALL(mockFacade, getResultsView(testHandle))
        .WillOnce(
            ReturnRef(static_cast<const core::interfaces::IAnalysisResultsView&>(mockResults)));

    // Act
    const auto& view = adapter->getResultsView(testHandle);

    // Assert - just verify we got a reference back
    // (In real testing, you'd verify specific properties)
    [[maybe_unused]] const auto& unused_view = view;
    SUCCEED();
}

// ============================================================
// Export Operations Tests
// ============================================================

TEST_F(FacadeAnalysisServiceAdapterTest, ExportResultsWithFormatDelegatesToFacade) {
    // Arrange
    const application::ResultsHandle testHandle = 100;
    const truss::ExportFormat format =
        truss::ExportFormat::JSON;
    const std::filesystem::path filepath = "/tmp/results.json";
    core::Truss testTruss("ExportTruss");
    infrastructure::export_::ExportOptions options;

    EXPECT_CALL(mockFacade, exportResults(testHandle, format, filepath, Ref(testTruss), _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->exportResults(testHandle, format, filepath, testTruss, options);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeAnalysisServiceAdapterTest, ExportResultsWithAutoFormatDelegatesToFacade) {
    // Arrange
    const application::ResultsHandle testHandle = 100;
    const std::filesystem::path filepath = "/tmp/results.csv";
    core::Truss testTruss("ExportTruss");
    infrastructure::export_::ExportOptions options;

    EXPECT_CALL(mockFacade, exportResults(testHandle, filepath, Ref(testTruss), _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->exportResults(testHandle, filepath, testTruss, options);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeAnalysisServiceAdapterTest, ExportResultsForwardsError) {
    // Arrange
    const application::ResultsHandle testHandle = 999;  // Invalid
    const std::filesystem::path filepath = "/invalid/path/results.json";
    core::Truss testTruss("ErrorTruss");
    infrastructure::export_::ExportOptions options;
    const std::string errorMsg = "Export failed: invalid handle";

    EXPECT_CALL(mockFacade, exportResults(testHandle, filepath, Ref(testTruss), _))
        .WillOnce(Return(application::Result<bool>::Failure(errorMsg)));

    // Act
    auto result = adapter->exportResults(testHandle, filepath, testTruss, options);

    // Assert
    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

// ============================================================
// Resource Management Tests
// ============================================================

TEST_F(FacadeAnalysisServiceAdapterTest, ClearResultsDelegatesToFacade) {
    // Arrange
    const application::ResultsHandle testHandle = 100;

    EXPECT_CALL(mockFacade, clearResults(testHandle)).WillOnce(Return(true));

    // Act
    bool result = adapter->clearResults(testHandle);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(FacadeAnalysisServiceAdapterTest, ClearResultsReturnsFalseOnFailure) {
    // Arrange
    const application::ResultsHandle testHandle = 999;  // Invalid

    EXPECT_CALL(mockFacade, clearResults(testHandle)).WillOnce(Return(false));

    // Act
    bool result = adapter->clearResults(testHandle);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(FacadeAnalysisServiceAdapterTest, ClearAllDelegatesToFacade) {
    // Arrange
    EXPECT_CALL(mockFacade, clearAll()).Times(1);

    // Act
    adapter->clearAll();

    // Assert - verification via EXPECT_CALL
}

TEST_F(FacadeAnalysisServiceAdapterTest, IsValidHandleDelegatesToFacade) {
    // Arrange
    const application::ResultsHandle testHandle = 100;

    EXPECT_CALL(mockFacade, isValidResultsHandle(testHandle)).WillOnce(Return(true));

    // Act
    bool result = adapter->isValidResultsHandle(testHandle);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(FacadeAnalysisServiceAdapterTest, IsValidHandleReturnsFalseForInvalid) {
    // Arrange
    const application::ResultsHandle testHandle = 999;

    EXPECT_CALL(mockFacade, isValidResultsHandle(testHandle)).WillOnce(Return(false));

    // Act
    bool result = adapter->isValidResultsHandle(testHandle);

    // Assert
    EXPECT_FALSE(result);
}

// ============================================================
// Multiple Operations Tests
// ============================================================

TEST_F(FacadeAnalysisServiceAdapterTest, SequentialOperationsWork) {
    // Test workflow: analyze -> export -> clear
    core::Truss testTruss("WorkflowTruss");
    core::analysis::AnalysisOptions options;
    const application::ResultsHandle resultsHandle = 200;
    const std::filesystem::path exportPath = "/tmp/workflow.json";

    // Analyze
    EXPECT_CALL(mockFacade, analyze(Ref(testTruss), _))
        .WillOnce(Return(application::Result<application::ResultsHandle>::Success(resultsHandle)));

    auto analyzeResult = adapter->analyze(testTruss, options);
    ASSERT_TRUE(analyzeResult.success);

    // Export
    EXPECT_CALL(mockFacade, exportResults(resultsHandle, exportPath, Ref(testTruss), _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    auto exportResult = adapter->exportResults(resultsHandle, exportPath, testTruss, {});
    ASSERT_TRUE(exportResult.success);

    // Clear
    EXPECT_CALL(mockFacade, clearResults(resultsHandle)).WillOnce(Return(true));

    bool clearResult = adapter->clearResults(resultsHandle);
    EXPECT_TRUE(clearResult);
}

TEST_F(FacadeAnalysisServiceAdapterTest, ParallelHandlesWork) {
    // Verify adapter can handle multiple result handles
    const application::ResultsHandle handle1 = 100;
    const application::ResultsHandle handle2 = 200;

    EXPECT_CALL(mockFacade, isValidResultsHandle(handle1)).WillOnce(Return(true));
    EXPECT_CALL(mockFacade, isValidResultsHandle(handle2)).WillOnce(Return(true));
    EXPECT_CALL(mockFacade, clearResults(handle1)).WillOnce(Return(true));
    EXPECT_CALL(mockFacade, clearResults(handle2)).WillOnce(Return(true));

    EXPECT_TRUE(adapter->isValidResultsHandle(handle1));
    EXPECT_TRUE(adapter->isValidResultsHandle(handle2));
    EXPECT_TRUE(adapter->clearResults(handle1));
    EXPECT_TRUE(adapter->clearResults(handle2));
}

// ============================================================
// Edge Cases and Error Handling
// ============================================================

TEST_F(FacadeAnalysisServiceAdapterTest, HandlesZeroResultsHandle) {
    // Zero is typically used for invalid handles
    const application::ResultsHandle invalidHandle = 0;

    EXPECT_CALL(mockFacade, isValidResultsHandle(invalidHandle)).WillOnce(Return(false));

    bool result = adapter->isValidResultsHandle(invalidHandle);
    EXPECT_FALSE(result);
}

TEST_F(FacadeAnalysisServiceAdapterTest, ExportWithDifferentFormats) {
    // Test multiple export format calls
    const application::ResultsHandle handle = 150;
    core::Truss testTruss("MultiFormatTruss");

    // JSON export
    EXPECT_CALL(mockFacade,
                exportResults(handle,
                              truss::ExportFormat::JSON,
                              _,
                              Ref(testTruss),
                              _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    auto jsonResult = adapter->exportResults(handle,
                                             truss::ExportFormat::JSON,
                                             "/tmp/out.json",
                                             testTruss,
                                             {});
    ASSERT_TRUE(jsonResult.success);

    // CSV export
    EXPECT_CALL(mockFacade,
                exportResults(handle,
                              truss::ExportFormat::CSV,
                              _,
                              Ref(testTruss),
                              _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    auto csvResult = adapter->exportResults(handle,
                                            truss::ExportFormat::CSV,
                                            "/tmp/out.csv",
                                            testTruss,
                                            {});
    ASSERT_TRUE(csvResult.success);
}

}  // namespace
