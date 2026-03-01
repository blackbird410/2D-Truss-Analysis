/**
 * @file test_facade_adapters_integration.cpp
 * @brief Integration tests for Facade adapters with real TrussAnalysisFacade
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 *
 * Comprehensive integration test suite verifying:
 * - FacadeTrussServiceAdapter works with real TrussAnalysisFacade
 * - FacadeAnalysisServiceAdapter works with real TrussAnalysisFacade
 * - Adapters maintain correct behavior in real workflows
 * - End-to-end workflows through adapter interfaces
 * - SOLID principles are maintained (encapsulation, no friend access)
 */

#include "interface/facade_analysis_service_adapter.hpp"
#include "interface/facade_truss_service_adapter.hpp"
#include "interface/truss_analysis_facade.hpp"

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss;
using namespace truss::interface;
using namespace testing;
namespace fs = std::filesystem;

namespace {

/**
 * @brief Integration test fixture for Facade adapters
 *
 * Tests adapters with real TrussAnalysisFacade to verify
 * end-to-end functionality and correct delegation behavior.
 */
class FacadeAdaptersIntegrationTest : public ::testing::Test {
protected:
    std::unique_ptr<truss::interface::TrussAnalysisFacade> facade;
    std::unique_ptr<truss::interface::FacadeTrussServiceAdapter> trussAdapter;
    std::unique_ptr<truss::interface::FacadeAnalysisServiceAdapter> analysisAdapter;

    fs::path tempDir;
    fs::path testJsonFile;

    void SetUp() override {
        // Create real facade
        facade = std::make_unique<truss::interface::TrussAnalysisFacade>();

        // Create adapters backed by real facade
        trussAdapter = std::make_unique<truss::interface::FacadeTrussServiceAdapter>(*facade);
        analysisAdapter = std::make_unique<truss::interface::FacadeAnalysisServiceAdapter>(*facade);

        // Set up test directory
        tempDir = fs::temp_directory_path() / "facade_adapters_integration_test";
        fs::create_directories(tempDir);

        // Create test JSON file
        testJsonFile = tempDir / "test_truss.json";
        createTestJsonFile();
    }

    void TearDown() override {
        // Clean up
        if (facade) {
            facade->clearAll();
        }

        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
    }

    /**
     * @brief Create a simple valid truss JSON file for testing
     */
    void createTestJsonFile() {
        std::ofstream file(testJsonFile);
        file << R"({
  "name": "Simple Test Truss",
  "nodes": [
    {"id": 1, "x": 0.0, "y": 0.0, "support": "Fixed"},
    {"id": 2, "x": 4.0, "y": 0.0, "support": "Roller"},
    {"id": 3, "x": 2.0, "y": 3.0, "support": "Free"}
  ],
  "members": [
    {
      "id": 1,
      "startNode": 1,
      "endNode": 3,
      "material": {
        "youngModulus": 200e9,
        "poissonRatio": 0.3,
        "density": 7850.0
      },
      "section": {
        "area": 0.01
      }
    },
    {
      "id": 2,
      "startNode": 2,
      "endNode": 3,
      "material": {
        "youngModulus": 200e9,
        "poissonRatio": 0.3,
        "density": 7850.0
      },
      "section": {
        "area": 0.01
      }
    }
  ],
  "loads": [
    {"nodeId": 3, "fx": 0.0, "fy": -10000.0}
  ]
})";
        file.close();
    }
};

// ============================================================
// Truss Adapter Integration Tests
// ============================================================

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterCreateTrussWorks) {
    // Act
    auto result = trussAdapter->createTruss("IntegrationTestTruss");

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_GT(result.value, 0u);

    // Verify handle is valid
    EXPECT_TRUE(trussAdapter->isValidTrussHandle(result.value));
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterLoadTrussWorks) {
    // Act
    auto result = trussAdapter->loadTruss(testJsonFile);

    // Assert
    ASSERT_TRUE(result.success);
    application::TrussHandle handle = result.value;
    EXPECT_GT(handle, 0u);

    // Verify we can access the loaded truss
    const auto& trussView = trussAdapter->getTrussView(handle);
    EXPECT_EQ(trussView.getName(), "Simple Test Truss");
    EXPECT_EQ(trussView.getNodeCount(), 3u);
    EXPECT_EQ(trussView.getMemberCount(), 2u);
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterSaveTrussWorks) {
    // Arrange - create a truss
    auto createResult = trussAdapter->createTruss("SaveTest");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle handle = createResult.value;

    // Add some geometry
    auto node1 = trussAdapter->addNode(handle, {0.0, 0.0}, core::SupportType::Pinned);
    auto node2 = trussAdapter->addNode(handle, {1.0, 0.0}, core::SupportType::Free);
    ASSERT_TRUE(node1.success);
    ASSERT_TRUE(node2.success);

    // Act - save the truss
    fs::path savePath = tempDir / "saved_truss.json";
    auto saveResult = trussAdapter->saveTruss(handle, savePath);

    // Assert
    ASSERT_TRUE(saveResult.success);
    EXPECT_TRUE(fs::exists(savePath));
    EXPECT_GT(fs::file_size(savePath), 0u);
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterValidateTrussWorks) {
    // Arrange - load test truss
    auto loadResult = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(loadResult.success);
    application::TrussHandle handle = loadResult.value;

    // Act
    auto validateResult = trussAdapter->validateTruss(handle);

    // Assert
    ASSERT_TRUE(validateResult.success);
    EXPECT_TRUE(validateResult.value.isValid());
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterAddNodeWorks) {
    // Arrange
    auto createResult = trussAdapter->createTruss("NodeTest");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle handle = createResult.value;

    // Act
    auto nodeResult = trussAdapter->addNode(handle, {2.0, 3.0}, core::SupportType::Pinned);

    // Assert
    ASSERT_TRUE(nodeResult.success);
    EXPECT_GT(nodeResult.value, 0u);

    // Verify node was added
    const auto& view = trussAdapter->getTrussView(handle);
    EXPECT_EQ(view.getNodeCount(), 1u);
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterAddMemberWorks) {
    // Arrange - create truss with two nodes
    auto createResult = trussAdapter->createTruss("MemberTest");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle handle = createResult.value;

    auto node1 = trussAdapter->addNode(handle, {0.0, 0.0}, core::SupportType::Pinned);
    auto node2 = trussAdapter->addNode(handle, {1.0, 0.0}, core::SupportType::Pinned);
    ASSERT_TRUE(node1.success);
    ASSERT_TRUE(node2.success);

    // Act
    application::MaterialSpec material{200e9, "Steel"};
    application::SectionSpec section{0.01, "Square"};
    auto memberResult =
        trussAdapter->addMember(handle, node1.value, node2.value, material, section);

    // Assert
    ASSERT_TRUE(memberResult.success);
    EXPECT_GT(memberResult.value, 0u);

    // Verify member was added
    const auto& view = trussAdapter->getTrussView(handle);
    EXPECT_EQ(view.getMemberCount(), 1u);
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterApplyLoadWorks) {
    // Arrange
    auto createResult = trussAdapter->createTruss("LoadTest");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle handle = createResult.value;

    auto nodeResult = trussAdapter->addNode(handle, {0.0, 0.0}, core::SupportType::Free);
    ASSERT_TRUE(nodeResult.success);
    core::NodeId nodeId = nodeResult.value;

    // Act
    core::Force2D force{1000.0, -500.0};
    auto loadResult = trussAdapter->applyNodeLoad(handle, nodeId, force);

    // Assert
    ASSERT_TRUE(loadResult.success);
    EXPECT_TRUE(loadResult.value);
}

TEST_F(FacadeAdaptersIntegrationTest, TrussAdapterClearTrussWorks) {
    // Arrange
    auto createResult = trussAdapter->createTruss("ClearTest");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle handle = createResult.value;

    // Act
    bool cleared = trussAdapter->clearTruss(handle);

    // Assert
    EXPECT_TRUE(cleared);
    EXPECT_FALSE(trussAdapter->isValidHandle(handle));
}

// ============================================================
// Analysis Adapter Integration Tests
// ============================================================

TEST_F(FacadeAdaptersIntegrationTest, AnalysisAdapterAnalyzeWorks) {
    // Arrange - load and get truss
    auto loadResult = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(loadResult.success);
    application::TrussHandle trussHandle = loadResult.value;

    auto& truss = trussAdapter->getTrussMutable(trussHandle);

    // Act
    core::analysis::AnalysisOptions options;
    auto analyzeResult = analysisAdapter->analyze(truss, options);

    // Assert
    ASSERT_TRUE(analyzeResult.success);
    application::ResultsHandle resultsHandle = analyzeResult.value;
    EXPECT_GT(resultsHandle, 0u);

    // Verify results are valid
    EXPECT_TRUE(analysisAdapter->isValidHandle(resultsHandle));
}

TEST_F(FacadeAdaptersIntegrationTest, AnalysisAdapterGetResultsViewWorks) {
    // Arrange - perform analysis
    auto loadResult = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(loadResult.success);
    auto& truss = trussAdapter->getTrussMutable(loadResult.value);

    auto analyzeResult = analysisAdapter->analyze(truss);
    ASSERT_TRUE(analyzeResult.success);
    application::ResultsHandle resultsHandle = analyzeResult.value;

    // Act
    const auto& resultsView = analysisAdapter->getResultsView(resultsHandle);

    // Assert - verify we can access results
    EXPECT_GT(resultsView.getDisplacements().size(), 0u);
    EXPECT_GT(resultsView.getMemberForces().size(), 0u);
}

TEST_F(FacadeAdaptersIntegrationTest, AnalysisAdapterExportResultsWorks) {
    // Arrange - perform analysis
    auto loadResult = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(loadResult.success);
    application::TrussHandle trussHandle = loadResult.value;
    auto& truss = trussAdapter->getTrussMutable(trussHandle);

    auto analyzeResult = analysisAdapter->analyze(truss);
    ASSERT_TRUE(analyzeResult.success);
    application::ResultsHandle resultsHandle = analyzeResult.value;

    // Act - export with auto-detection
    fs::path exportPath = tempDir / "results.json";
    infrastructure::export_::ExportOptions options;
    auto exportResult = analysisAdapter->exportResults(resultsHandle, exportPath, truss, options);

    // Assert
    ASSERT_TRUE(exportResult.success);
    EXPECT_TRUE(fs::exists(exportPath));
    EXPECT_GT(fs::file_size(exportPath), 0u);
}

TEST_F(FacadeAdaptersIntegrationTest, AnalysisAdapterExportWithExplicitFormatWorks) {
    // Arrange - perform analysis
    auto loadResult = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(loadResult.success);
    auto& truss = trussAdapter->getTrussMutable(loadResult.value);

    auto analyzeResult = analysisAdapter->analyze(truss);
    ASSERT_TRUE(analyzeResult.success);
    application::ResultsHandle resultsHandle = analyzeResult.value;

    // Act - export CSV explicitly
    fs::path exportPath = tempDir / "results.csv";
    auto exportResult = analysisAdapter->exportResults(resultsHandle,
                                                       truss::ExportFormat::CSV,
                                                       exportPath,
                                                       truss,
                                                       {});

    // Assert
    ASSERT_TRUE(exportResult.success);
    EXPECT_TRUE(fs::exists(exportPath));
}

TEST_F(FacadeAdaptersIntegrationTest, AnalysisAdapterClearResultsWorks) {
    // Arrange - perform analysis
    auto loadResult = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(loadResult.success);
    auto& truss = trussAdapter->getTrussMutable(loadResult.value);

    auto analyzeResult = analysisAdapter->analyze(truss);
    ASSERT_TRUE(analyzeResult.success);
    application::ResultsHandle resultsHandle = analyzeResult.value;

    // Act
    bool cleared = analysisAdapter->clearResults(resultsHandle);

    // Assert
    EXPECT_TRUE(cleared);
    EXPECT_FALSE(analysisAdapter->isValidResultsHandle(resultsHandle));
}

// ============================================================
// End-to-End Workflow Tests
// ============================================================

TEST_F(FacadeAdaptersIntegrationTest, CompleteWorkflowThroughAdapters) {
    // This test verifies a complete workflow using ONLY the adapter interfaces
    // demonstrating that adapters provide full functionality

    // Step 1: Create truss through adapter
    auto createResult = trussAdapter->createTruss("E2E Workflow Truss");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle trussHandle = createResult.value;

    // Step 2: Build geometry through adapter
    auto node1 = trussAdapter->addNode(trussHandle, {0.0, 0.0}, core::SupportType::Pinned);
    auto node2 = trussAdapter->addNode(trussHandle, {4.0, 0.0}, core::SupportType::RollerY);
    auto node3 = trussAdapter->addNode(trussHandle, {2.0, 3.0}, core::SupportType::Free);
    ASSERT_TRUE(node1.success);
    ASSERT_TRUE(node2.success);
    ASSERT_TRUE(node3.success);

    application::MaterialSpec steel{200e9, "Steel"};
    application::SectionSpec section{0.01, "Square"};

    auto member1 =
        trussAdapter->addMember(trussHandle, node1.value, node3.value, steel, section);
    auto member2 =
        trussAdapter->addMember(trussHandle, node2.value, node3.value, steel, section);
    ASSERT_TRUE(member1.success);
    ASSERT_TRUE(member2.success);

    // Step 3: Apply loads through adapter
    core::Force2D downwardLoad{0.0, -10000.0};
    auto loadResult = trussAdapter->applyNodeLoad(trussHandle, node3.value, downwardLoad);
    ASSERT_TRUE(loadResult.success);

    // Step 4: Validate through adapter
    auto validateResult = trussAdapter->validateTruss(trussHandle);
    ASSERT_TRUE(validateResult.success);
    EXPECT_TRUE(validateResult.value.isValid());

    // Step 5: Analyze through adapter
    auto& truss = trussAdapter->getTrussMutable(trussHandle);
    auto analyzeResult = analysisAdapter->analyze(truss);
    ASSERT_TRUE(analyzeResult.success);
    application::ResultsHandle resultsHandle = analyzeResult.value;

    // Step 6: Access results through adapter
    const auto& results = analysisAdapter->getResultsView(resultsHandle);
    EXPECT_EQ(results.getDisplacements().size(), 3u);
    EXPECT_EQ(results.getMemberForces().size(), 2u);

    // Step 7: Export through adapter
    fs::path exportPath = tempDir / "e2e_results.json";
    auto exportResult = analysisAdapter->exportResults(resultsHandle, exportPath, truss, {});
    ASSERT_TRUE(exportResult.success);
    EXPECT_TRUE(fs::exists(exportPath));

    // Step 8: Clean up through adapters
    EXPECT_TRUE(analysisAdapter->clearResults(resultsHandle));
    EXPECT_TRUE(trussAdapter->clearTruss(trussHandle));
}

TEST_F(FacadeAdaptersIntegrationTest, MultipleSequentialWorkflows) {
    // Test multiple workflows in sequence to verify state management

    // Workflow 1
    auto result1 = trussAdapter->loadTruss(testJsonFile);
    ASSERT_TRUE(result1.success);
    auto& truss1 = trussAdapter->getTrussMutable(result1.value);
    auto analyze1 = analysisAdapter->analyze(truss1);
    ASSERT_TRUE(analyze1.success);

    // Workflow 2
    auto result2 = trussAdapter->createTruss("Workflow2");
    ASSERT_TRUE(result2.success);

    // Verify both workflows are independent
    EXPECT_TRUE(trussAdapter->isValidTrussHandle(result1.value));
    EXPECT_TRUE(trussAdapter->isValidTrussHandle(result2.value));
    EXPECT_TRUE(analysisAdapter->isValidResultsHandle(analyze1.value));

    // Clean up
    trussAdapter->clearAll();
    analysisAdapter->clearAll();

    EXPECT_FALSE(trussAdapter->isValidTrussHandle(result1.value));
    EXPECT_FALSE(trussAdapter->isValidTrussHandle(result2.value));
    EXPECT_FALSE(analysisAdapter->isValidResultsHandle(analyze1.value));
}

TEST_F(FacadeAdaptersIntegrationTest, AdaptersShareSameFacadeState) {
    // Verify that both adapters work with the same underlying facade state

    // Create truss through truss adapter
    auto createResult = trussAdapter->createTruss("SharedState");
    ASSERT_TRUE(createResult.success);
    application::TrussHandle handle = createResult.value;

    // Build minimal valid truss
    auto n1 = trussAdapter->addNode(handle, {0, 0}, core::SupportType::Pinned);
    auto n2 = trussAdapter->addNode(handle, {1, 0}, core::SupportType::RollerY);
    auto n3 = trussAdapter->addNode(handle, {0.5, 1}, core::SupportType::Free);

    application::MaterialSpec mat{200e9, "Steel"};
    application::SectionSpec sec{0.01, "Square"};
    trussAdapter->addMember(handle, n1.value, n3.value, mat, sec);
    trussAdapter->addMember(handle, n2.value, n3.value, mat, sec);
    trussAdapter->applyNodeLoad(handle, n3.value, {0, -1000});

    // Analyze through analysis adapter using same truss
    auto& truss = trussAdapter->getTrussMutable(handle);
    auto analyzeResult = analysisAdapter->analyze(truss);
    ASSERT_TRUE(analyzeResult.success);

    // Verify analysis adapter can access the results
    EXPECT_TRUE(analysisAdapter->isValidResultsHandle(analyzeResult.value));

    // Clear through one adapter should affect both
    trussAdapter->clearAll();
    EXPECT_FALSE(trussAdapter->isValidTrussHandle(handle));
    EXPECT_FALSE(analysisAdapter->isValidResultsHandle(analyzeResult.value));
}

// ============================================================
// Error Handling Integration Tests
// ============================================================

TEST_F(FacadeAdaptersIntegrationTest, InvalidHandlesAreRejected) {
    application::TrussHandle invalidTrussHandle = 99999;
    application::ResultsHandle invalidResultsHandle = 99999;

    EXPECT_FALSE(trussAdapter->isValidTrussHandle(invalidTrussHandle));
    EXPECT_FALSE(analysisAdapter->isValidResultsHandle(invalidResultsHandle));
}

TEST_F(FacadeAdaptersIntegrationTest, ErrorsPropagateThroughAdapters) {
    // Try to load non-existent file
    fs::path nonExistent = tempDir / "does_not_exist.json";
    auto loadResult = trussAdapter->loadTruss(nonExistent);

    ASSERT_FALSE(loadResult.success);
    EXPECT_FALSE(loadResult.errorMessage.empty());
}

}  // namespace
