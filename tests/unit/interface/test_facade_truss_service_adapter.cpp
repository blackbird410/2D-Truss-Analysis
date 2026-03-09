/**
 * @file test_facade_truss_service_adapter.cpp
 * @brief Unit tests for FacadeTrussServiceAdapter
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 *
 * Comprehensive test suite verifying that FacadeTrussServiceAdapter:
 * - Correctly implements ITrussService interface
 * - Delegates all calls to TrussAnalysisFacade public methods
 * - Maintains proper encapsulation (no friend access)
 * - Forwards return values correctly
 * - Forwards all parameters correctly
 */

#include "interface/facade_truss_service_adapter.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss;
using namespace truss::interface;
using namespace truss::test;
using namespace testing;

namespace {

/**
 * @brief Test fixture for FacadeTrussServiceAdapter unit tests
 *
 * Uses MockTrussAnalysisFacade to verify delegation without
 * depending on actual Facade implementation.
 */
class FacadeTrussServiceAdapterTest : public ::testing::Test {
protected:
    truss::test::MockTrussAnalysisFacade mockFacade;
    std::unique_ptr<truss::interface::FacadeTrussServiceAdapter> adapter;

    void SetUp() override {
        // Safe injection of mock via interface
        adapter = std::make_unique<truss::interface::FacadeTrussServiceAdapter>(mockFacade);
    }
};

// ============================================================
// Lifecycle Operations Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, CreateTrussDelegatesToFacade) {
    // Arrange
    const std::string testName = "TestTruss";
    const application::TrussHandle expectedHandle = 42;

    EXPECT_CALL(mockFacade, createTruss(testName))
        .WillOnce(Return(application::Result<application::TrussHandle>::Success(expectedHandle)));

    // Act
    auto result = adapter->createTruss(testName);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.value, expectedHandle);
}

TEST_F(FacadeTrussServiceAdapterTest, CreateTrussForwardsError) {
    // Arrange
    const std::string testName = "ErrorTruss";
    const std::string errorMsg = "Creation failed";

    EXPECT_CALL(mockFacade, createTruss(testName))
        .WillOnce(Return(application::Result<application::TrussHandle>::Failure(errorMsg)));

    // Act
    auto result = adapter->createTruss(testName);

    // Assert
    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

TEST_F(FacadeTrussServiceAdapterTest, LoadTrussDelegatesToFacade) {
    // Arrange
    const std::filesystem::path testPath = "/tmp/test.json";
    const application::TrussHandle expectedHandle = 123;

    EXPECT_CALL(mockFacade, loadTruss(testPath))
        .WillOnce(Return(application::Result<application::TrussHandle>::Success(expectedHandle)));

    // Act
    auto result = adapter->loadTruss(testPath);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.value, expectedHandle);
}

TEST_F(FacadeTrussServiceAdapterTest, SaveTrussDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const std::filesystem::path testPath = "/tmp/output.json";
    const bool overwrite = true;

    EXPECT_CALL(mockFacade, saveTruss(testHandle, testPath, overwrite))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->saveTruss(testHandle, testPath, overwrite);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeTrussServiceAdapterTest, ClearTrussDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;

    EXPECT_CALL(mockFacade, clearTruss(testHandle)).WillOnce(Return(true));

    // Act
    bool result = adapter->clearTruss(testHandle);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(FacadeTrussServiceAdapterTest, ClearAllDelegatesToFacade) {
    // Arrange
    EXPECT_CALL(mockFacade, clearAll()).Times(1);

    // Act
    adapter->clearAll();

    // Assert - verification via EXPECT_CALL
}

TEST_F(FacadeTrussServiceAdapterTest, IsValidHandleDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;

    EXPECT_CALL(mockFacade, isValidTrussHandle(testHandle)).WillOnce(Return(true));

    // Act
    bool result = adapter->isValidTrussHandle(testHandle);

    // Assert
    EXPECT_TRUE(result);
}

// ============================================================
// Data Access Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, GetTrussViewDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;

    // Create a mock truss and mock view (we'll use NiceMock to avoid warnings)
    core::Truss mockTruss("TestTruss");

    EXPECT_CALL(mockFacade, getTrussView(testHandle))
        .WillOnce(ReturnRef(static_cast<const core::interfaces::ITrussView&>(mockTruss)));

    // Act
    const auto& view = adapter->getTrussView(testHandle);

    // Assert
    EXPECT_EQ(view.getName(), "TestTruss");
}

TEST_F(FacadeTrussServiceAdapterTest, GetTrussMutableDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    core::Truss mockTruss("MutableTruss");

    EXPECT_CALL(mockFacade, getTrussMutable(testHandle)).WillOnce(ReturnRef(mockTruss));

    // Act
    auto& truss = adapter->getTrussMutable(testHandle);

    // Assert
    EXPECT_EQ(truss.getName(), "MutableTruss");
}

// ============================================================
// Validation Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, ValidateTrussDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    core::validation::ValidationResult
        validResult;  // Default constructor creates valid result (no issues)

    EXPECT_CALL(mockFacade, validateTruss(testHandle))
        .WillOnce(
            Return(application::Result<core::validation::ValidationResult>::Success(validResult)));

    // Act
    auto result = adapter->validateTruss(testHandle);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value.isValid());
}

// ============================================================
// Node Operations Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, AddNodeDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::Point2D position{1.0, 2.0};
    const core::SupportType support = core::SupportType::Pinned;
    const core::NodeId expectedId = 1;

    EXPECT_CALL(mockFacade, addNode(testHandle, _, support))
        .WillOnce(Return(application::Result<core::NodeId>::Success(expectedId)));

    // Act
    auto result = adapter->addNode(testHandle, position, support);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.value, expectedId);
}

TEST_F(FacadeTrussServiceAdapterTest, AddMemberDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::NodeId startNode = 1;
    const core::NodeId endNode = 2;
    const application::MaterialSpec material{200e9, "Steel"};
    const application::SectionSpec section{0.01, "Square"};
    const core::MemberId expectedId = 10;

    EXPECT_CALL(mockFacade, addMember(testHandle, startNode, endNode, _, _))
        .WillOnce(Return(application::Result<core::MemberId>::Success(expectedId)));

    // Act
    auto result = adapter->addMember(testHandle, startNode, endNode, material, section);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.value, expectedId);
}

TEST_F(FacadeTrussServiceAdapterTest, RemoveNodeDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::NodeId nodeId = 5;

    EXPECT_CALL(mockFacade, removeNode(testHandle, nodeId))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->removeNode(testHandle, nodeId);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeTrussServiceAdapterTest, RemoveMemberDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::MemberId memberId = 10;

    EXPECT_CALL(mockFacade, removeMember(testHandle, memberId))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->removeMember(testHandle, memberId);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeTrussServiceAdapterTest, SetNodeSupportDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::NodeId nodeId = 3;
    const core::SupportType support = core::SupportType::Pinned;

    EXPECT_CALL(mockFacade, setNodeSupport(testHandle, nodeId, support))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->setNodeSupport(testHandle, nodeId, support);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

// ============================================================
// Load Operations Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, ApplyNodeLoadDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::NodeId nodeId = 2;
    const core::Force2D force{1000.0, -500.0};

    EXPECT_CALL(mockFacade, applyNodeLoad(testHandle, nodeId, _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->applyNodeLoad(testHandle, nodeId, force);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeTrussServiceAdapterTest, ClearNodeLoadDelegatesToFacade) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const core::NodeId nodeId = 2;

    EXPECT_CALL(mockFacade, clearNodeLoad(testHandle, nodeId))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->clearNodeLoad(testHandle, nodeId);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

// ============================================================
// Error Propagation Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, ErrorsAreCorrectlyPropagated) {
    // Test that various error conditions are properly forwarded
    const application::TrussHandle testHandle = 999;  // Invalid handle
    const std::string errorMsg = "Invalid handle";

    EXPECT_CALL(mockFacade, removeNode(testHandle, _))
        .WillOnce(Return(application::Result<bool>::Failure(errorMsg)));

    auto result = adapter->removeNode(testHandle, 1);

    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

TEST_F(FacadeTrussServiceAdapterTest, MultipleCallsWork) {
    // Verify adapter can handle multiple sequential calls
    const application::TrussHandle handle1 = 10;
    const application::TrussHandle handle2 = 20;

    EXPECT_CALL(mockFacade, clearTruss(handle1)).WillOnce(Return(true));
    EXPECT_CALL(mockFacade, clearTruss(handle2)).WillOnce(Return(true));

    EXPECT_TRUE(adapter->clearTruss(handle1));
    EXPECT_TRUE(adapter->clearTruss(handle2));
}

// ============================================================
// Additional Error-Forwarding Coverage
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, LoadTrussForwardsError) {
    // Arrange
    const std::filesystem::path missingFile = "/does/not/exist.json";
    const std::string errorMsg = "File not found";

    EXPECT_CALL(mockFacade, loadTruss(missingFile))
        .WillOnce(Return(application::Result<application::TrussHandle>::Failure(errorMsg)));

    // Act
    auto result = adapter->loadTruss(missingFile);

    // Assert
    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

TEST_F(FacadeTrussServiceAdapterTest, SaveTrussForwardsError) {
    // Arrange
    const application::TrussHandle testHandle = 42;
    const std::filesystem::path readOnlyPath = "/read/only/output.json";
    const std::string errorMsg = "Cannot write to path";

    EXPECT_CALL(mockFacade, saveTruss(testHandle, readOnlyPath, false))
        .WillOnce(Return(application::Result<bool>::Failure(errorMsg)));

    // Act
    auto result = adapter->saveTruss(testHandle, readOnlyPath, false);

    // Assert
    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

TEST_F(FacadeTrussServiceAdapterTest, IsValidHandleReturnsFalseForInvalidHandle) {
    // Arrange — zero and large out-of-range values are both invalid
    const application::TrussHandle zeroHandle = 0;
    const application::TrussHandle bigHandle = 99999;

    EXPECT_CALL(mockFacade, isValidTrussHandle(zeroHandle)).WillOnce(Return(false));
    EXPECT_CALL(mockFacade, isValidTrussHandle(bigHandle)).WillOnce(Return(false));

    // Act & Assert
    EXPECT_FALSE(adapter->isValidTrussHandle(zeroHandle));
    EXPECT_FALSE(adapter->isValidTrussHandle(bigHandle));
}

TEST_F(FacadeTrussServiceAdapterTest, ValidateTrussForwardsError) {
    // Arrange
    const application::TrussHandle badHandle = 999;
    const std::string errorMsg = "Invalid truss handle";

    EXPECT_CALL(mockFacade, validateTruss(badHandle))
        .WillOnce(
            Return(application::Result<core::validation::ValidationResult>::Failure(errorMsg)));

    // Act
    auto result = adapter->validateTruss(badHandle);

    // Assert
    ASSERT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, errorMsg);
}

TEST_F(FacadeTrussServiceAdapterTest, ClearTrussReturnsFalseForUnknownHandle) {
    // Arrange — clearing an unknown handle should return false
    const application::TrussHandle unknownHandle = 555;

    EXPECT_CALL(mockFacade, clearTruss(unknownHandle)).WillOnce(Return(false));

    // Act & Assert
    EXPECT_FALSE(adapter->clearTruss(unknownHandle));
}

// ============================================================
// Node / Member Update Delegation Tests
// ============================================================

TEST_F(FacadeTrussServiceAdapterTest, UpdateNodeDelegatesToFacade) {
    // Arrange
    const application::TrussHandle handle = 1;
    const core::NodeId nodeId = 10;
    const application::NodeUpdateSpec spec{3.0, 4.0};

    EXPECT_CALL(mockFacade, updateNode(handle, nodeId, _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->updateNode(handle, nodeId, spec);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(FacadeTrussServiceAdapterTest, UpdateMemberDelegatesToFacade) {
    // Arrange
    const application::TrussHandle handle = 1;
    const core::MemberId memberId = 5;
    const application::MemberUpdateSpec spec{
        application::MaterialSpec{200e9, "Steel"},
        application::SectionSpec{0.01, "Square"}};

    EXPECT_CALL(mockFacade, updateMember(handle, memberId, _))
        .WillOnce(Return(application::Result<bool>::Success(true)));

    // Act
    auto result = adapter->updateMember(handle, memberId, spec);

    // Assert
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

}  // namespace
