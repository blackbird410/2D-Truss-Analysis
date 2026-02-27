/**
 * @file test_lifecycle_integration.cpp
 * @brief Integration tests for application lifecycle (Application layer only).
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * These tests verify the Application service layer handles the expected
 * lifecycle flows correctly, independent of GUI controllers.
 */

#include "application/analysis_application_service.hpp"
#include "application/truss_application_service.hpp"

#include <gtest/gtest.h>

using namespace truss::application;
using namespace truss::core;

/**
 * @brief Test fixture for lifecycle integration tests
 */
class LifecycleIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        trussService = std::make_unique<TrussApplicationService>();
        analysisService = std::make_unique<AnalysisApplicationService>();
    }

    std::unique_ptr<TrussApplicationService> trussService;
    std::unique_ptr<AnalysisApplicationService> analysisService;
};

/**
 * @brief Test that creating a truss produces valid handle
 */
TEST_F(LifecycleIntegrationTest, CreateTrussProducesValidHandle) {
    auto result = trussService->createTruss("Test");

    EXPECT_TRUE(result.success);
    EXPECT_NE(result.value, 0) << "Truss handle should be non-zero";
}

/**
 * @brief Test that adding a node succeeds with valid handle
 */
TEST_F(LifecycleIntegrationTest, AddNodeWithValidHandleSucceeds) {
    auto trussResult = trussService->createTruss("Test");
    ASSERT_TRUE(trussResult.success);

    TrussHandle handle = trussResult.value;
    Point2D position{0.0, 0.0};
    SupportType support = SupportType::Free;

    auto nodeResult = trussService->addNode(handle, position, support);

    EXPECT_TRUE(nodeResult.success) << nodeResult.errorMessage;
    EXPECT_EQ(nodeResult.value, 1) << "First node should have ID 1";
}

/**
 * @brief Test that adding a member succeeds after nodes exist
 *
 * This tests the fix for the "Invalid node IDs for member" error.
 * Node IDs should be used as-is (1, 2, 3...), not decremented.
 */
TEST_F(LifecycleIntegrationTest, AddMemberAfterNodesSucceeds) {
    // Create truss and add two nodes
    auto trussResult = trussService->createTruss("Test");
    ASSERT_TRUE(trussResult.success);
    TrussHandle handle = trussResult.value;

    auto node1 = trussService->addNode(handle, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto node2 = trussService->addNode(handle, Point2D{5.0, 0.0}, SupportType::Free);

    ASSERT_TRUE(node1.success);
    ASSERT_TRUE(node2.success);
    ASSERT_EQ(node1.value, 1);
    ASSERT_EQ(node2.value, 2);

    // Create member between nodes using actual IDs (not decremented)
    MaterialSpec material{200e9, "Steel"};  // 200 GPa
    SectionSpec section{0.001, "100x100"};  // 1000 mm²

    auto memberResult = trussService->addMember(handle,
                                                node1.value,  // ID 1 (not 0)
                                                node2.value,  // ID 2 (not 1)
                                                material,
                                                section);

    EXPECT_TRUE(memberResult.success)
        << "Adding member between valid nodes should succeed: " << memberResult.errorMessage;
    EXPECT_EQ(memberResult.value, 1) << "First member should have ID 1";
}

/**
 * @brief Test that analysis without nodes fails gracefully
 */
TEST_F(LifecycleIntegrationTest, AnalyzeWithoutNodesFailsGracefully) {
    auto trussResult = trussService->createTruss("Test");
    ASSERT_TRUE(trussResult.success);

    TrussHandle handle = trussResult.value;
    auto validationResult = trussService->validateTruss(handle);

    ASSERT_TRUE(validationResult.success) << "Validation should not fail";
    EXPECT_FALSE(validationResult.value.isValid()) << "Empty truss should fail validation";
    EXPECT_GT(validationResult.value.getIssueCount(), 0) << "Validation should report issues";
}

/**
 * @brief Test creating multiple trusses produces unique handles
 */
TEST_F(LifecycleIntegrationTest, MultipleTrussesHaveUniqueHandles) {
    auto truss1 = trussService->createTruss("Truss1");
    auto truss2 = trussService->createTruss("Truss2");
    auto truss3 = trussService->createTruss("Truss3");

    ASSERT_TRUE(truss1.success);
    ASSERT_TRUE(truss2.success);
    ASSERT_TRUE(truss3.success);

    EXPECT_NE(truss1.value, truss2.value);
    EXPECT_NE(truss2.value, truss3.value);
    EXPECT_NE(truss1.value, truss3.value);
}

/**
 * @brief Test member addition with correct node IDs (regression test)
 *
 * Verifies: Member addition uses actual node IDs (1, 2, 3...)
 * not zero-based indices (0, 1, 2...).
 */
TEST_F(LifecycleIntegrationTest, MemberAdditionUsesCorrectNodeIDs) {
    auto trussResult = trussService->createTruss("Test");
    ASSERT_TRUE(trussResult.success);
    TrussHandle handle = trussResult.value;

    // Add nodes with IDs 1, 2, 3
    auto n1 = trussService->addNode(handle, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto n2 = trussService->addNode(handle, Point2D{5.0, 0.0}, SupportType::Free);
    auto n3 = trussService->addNode(handle, Point2D{2.5, 3.0}, SupportType::Free);

    ASSERT_TRUE(n1.success && n2.success && n3.success);
    EXPECT_EQ(n1.value, 1);
    EXPECT_EQ(n2.value, 2);
    EXPECT_EQ(n3.value, 3);

    MaterialSpec material{200e9, "Steel"};
    SectionSpec section{0.001, "100x100"};

    // Add member between node 1 and 2 (not 0 and 1)
    auto m1 = trussService->addMember(handle, 1, 2, material, section);
    EXPECT_TRUE(m1.success) << "Member 1-2: " << m1.errorMessage;

    // Add member between node 2 and 3 (not 1 and 2)
    auto m2 = trussService->addMember(handle, 2, 3, material, section);
    EXPECT_TRUE(m2.success) << "Member 2-3: " << m2.errorMessage;

    // Add member between node 3 and 1 (not 2 and 0)
    auto m3 = trussService->addMember(handle, 3, 1, material, section);
    EXPECT_TRUE(m3.success) << "Member 3-1: " << m3.errorMessage;

    // Verify all members added
    const auto& trussView = trussService->getTrussView(handle);
    EXPECT_EQ(trussView.getMemberCount(), 3);
}

/**
 * @brief Test that member addition with invalid node ID fails properly
 */
TEST_F(LifecycleIntegrationTest, MemberAdditionWithInvalidNodeIDFails) {
    auto trussResult = trussService->createTruss("Test");
    ASSERT_TRUE(trussResult.success);
    TrussHandle handle = trussResult.value;

    auto n1 = trussService->addNode(handle, Point2D{0.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(n1.success);

    MaterialSpec material{200e9, "Steel"};
    SectionSpec section{0.001, "100x100"};

    // Try to add member with non-existent node ID
    auto memberResult = trussService->addMember(handle, 1, 999, material, section);

    EXPECT_FALSE(memberResult.success) << "Adding member with invalid node ID should fail";
    EXPECT_FALSE(memberResult.errorMessage.empty()) << "Should provide error message";
}

/**
 * @brief Test that member addition with node ID 0 fails
 *
 * This is the regression test for the bug where DrawingCanvas was
 * passing nodeId - 1, resulting in ID 0 being passed to addMember.
 */
TEST_F(LifecycleIntegrationTest, MemberAdditionWithNodeIDZeroFails) {
    auto trussResult = trussService->createTruss("Test");
    ASSERT_TRUE(trussResult.success);
    TrussHandle handle = trussResult.value;

    auto n1 = trussService->addNode(handle, Point2D{0.0, 0.0}, SupportType::Free);
    auto n2 = trussService->addNode(handle, Point2D{5.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(n1.success && n2.success);

    MaterialSpec material{200e9, "Steel"};
    SectionSpec section{0.001, "100x100"};

    // Try to add member with node ID 0 (invalid)
    auto memberResult = trussService->addMember(handle, 0, 1, material, section);

    EXPECT_FALSE(memberResult.success) << "Node ID 0 is invalid and should be rejected";
}
