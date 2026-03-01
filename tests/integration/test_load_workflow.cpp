/**
 * @file test_load_workflow.cpp
 * @brief Integration test for complete load workflow.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "application/truss_application_service.hpp"
#include "infrastructure/io/fileio_factory.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace truss::application;
using namespace truss::infrastructure::io;

class LoadWorkflowTest : public ::testing::Test {
protected:
    void SetUp() override {
        service = std::make_unique<TrussApplicationService>();
        testFile = std::filesystem::temp_directory_path() / "test_load.json";
        createTestFile();
    }

    void TearDown() override {
        if (std::filesystem::exists(testFile)) {
            std::filesystem::remove(testFile);
        }
    }

    void createTestFile() {
        std::ofstream file(testFile);
        file << R"({
  "metadata": {
    "name": "Test Load Project"
  },
  "nodes": [
    {
      "id": 1,
      "x": 0.0,
      "y": 0.0,
      "support": "pinned"
    },
    {
      "id": 2,
      "x": 5.0,
      "y": 3.0,
      "support": "free",
      "fx": 1000.0,
      "fy": -1000.0
    },
    {
      "id": 3,
      "x": 10.0,
      "y": 0.0,
      "support": "roller_y"
    }
  ],
  "members": [
    {
      "id": 1,
      "startNode": 1,
      "endNode": 2,
      "area": 0.002,
      "youngsModulus": 200e9
    },
    {
      "id": 2,
      "startNode": 2,
      "endNode": 3,
      "area": 0.002,
      "youngsModulus": 200e9
    },
    {
      "id": 3,
      "startNode": 1,
      "endNode": 3,
      "area": 0.002,
      "youngsModulus": 200e9
    }
  ]
})";
        file.close();
    }

    std::unique_ptr<TrussApplicationService> service;
    std::filesystem::path testFile;
};

TEST_F(LoadWorkflowTest, LoadProjectReconstructsDomain) {
    // Load the project
    auto result = service->loadTruss(testFile);

    ASSERT_TRUE(result.success) << "Load failed: " << result.errorMessage;
    EXPECT_NE(result.value, 0) << "Invalid handle returned";

    TrussHandle handle = result.value;

    // Verify truss exists and has correct structure
    EXPECT_TRUE(service->isValidTrussHandle(handle));

    // Get view to verify nodes and members
    const auto& view = service->getTrussView(handle);

    EXPECT_EQ(view.getNodeCount(), 3) << "Expected 3 nodes";
    EXPECT_EQ(view.getMemberCount(), 3) << "Expected 3 members";

    // Verify node data
    auto nodeViews = view.getNodeViews();
    ASSERT_EQ(nodeViews.size(), 3);

    // Node 1: (0, 0) PINNED
    EXPECT_EQ(nodeViews[0].id, 1);
    EXPECT_DOUBLE_EQ(nodeViews[0].x, 0.0);
    EXPECT_DOUBLE_EQ(nodeViews[0].y, 0.0);

    // Node 2: (5, 3) FREE with load
    EXPECT_EQ(nodeViews[1].id, 2);
    EXPECT_DOUBLE_EQ(nodeViews[1].x, 5.0);
    EXPECT_DOUBLE_EQ(nodeViews[1].y, 3.0);

    // Node 3: (10, 0) ROLLER
    EXPECT_EQ(nodeViews[2].id, 3);
    EXPECT_DOUBLE_EQ(nodeViews[2].x, 10.0);
    EXPECT_DOUBLE_EQ(nodeViews[2].y, 0.0);

    // Verify member data
    auto memberViews = view.getMemberViews();
    ASSERT_EQ(memberViews.size(), 3);

    EXPECT_EQ(memberViews[0].startNodeId, 1);
    EXPECT_EQ(memberViews[0].endNodeId, 2);

    EXPECT_EQ(memberViews[1].startNodeId, 2);
    EXPECT_EQ(memberViews[1].endNodeId, 3);

    EXPECT_EQ(memberViews[2].startNodeId, 1);
    EXPECT_EQ(memberViews[2].endNodeId, 3);
}

TEST_F(LoadWorkflowTest, MultipleLoadsCreateUniqueHandles) {
    auto result1 = service->loadTruss(testFile);
    ASSERT_TRUE(result1.success);

    auto result2 = service->loadTruss(testFile);
    ASSERT_TRUE(result2.success);

    EXPECT_NE(result1.value, result2.value) << "Handles should be unique";

    // Both should be valid
    EXPECT_TRUE(service->isValidTrussHandle(result1.value));
    EXPECT_TRUE(service->isValidTrussHandle(result2.value));

    // Both should have correct data
    const auto& view1 = service->getTrussView(result1.value);
    const auto& view2 = service->getTrussView(result2.value);

    EXPECT_EQ(view1.getNodeCount(), 3);
    EXPECT_EQ(view2.getNodeCount(), 3);
}

TEST_F(LoadWorkflowTest, LoadNonexistentFileFails) {
    auto badPath = std::filesystem::temp_directory_path() / "nonexistent.json";
    auto result = service->loadTruss(badPath);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(LoadWorkflowTest, LoadInvalidJsonFails) {
    auto badFile = std::filesystem::temp_directory_path() / "invalid.json";
    std::ofstream file(badFile);
    file << "{ invalid json syntax }";
    file.close();

    auto result = service->loadTruss(badFile);

    EXPECT_FALSE(result.success);

    std::filesystem::remove(badFile);
}
