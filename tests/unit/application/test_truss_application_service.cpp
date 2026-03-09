/**
 * @file test_truss_application_service.cpp
 * @brief Unit tests for TrussApplicationService facade.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Test Coverage:
 * - Lifecycle management (create, clear)
 * - File I/O (load, save with various formats)
 * - Validation integration
 * - Handle management (multiple handles, invalidation)
 * - Error handling (Result<T> pattern, exception safety)
 * - Interface abstraction (ITrussView access)
 */

#include "../../../src/application/truss_application_service.hpp"
#include "../../../src/application/truss_edit_dtos.hpp"
#include "../../../src/core/model/truss.hpp"
#include "../../../src/infrastructure/io/fileio_factory.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

using namespace truss::application;
using namespace truss::core;
using namespace truss::infrastructure::io;

/**
 * @brief Test fixture for TrussApplicationService
 */
class TrussApplicationServiceTest : public ::testing::Test {
protected:
    TrussApplicationService service;
    std::filesystem::path tempDir;

    void SetUp() override {
        // Create temporary directory for test files (unique per fixture instance)
        // Using both timestamp and thread ID to avoid collisions in parallel execution
        std::ostringstream oss;
        oss << "truss_app_service_test-"
            << std::chrono::system_clock::now().time_since_epoch().count() << "-"
            << std::this_thread::get_id();
        tempDir = std::filesystem::temp_directory_path() / oss.str();
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        // Clean up temporary files
        if (std::filesystem::exists(tempDir)) {
            std::filesystem::remove_all(tempDir);
        }
    }

    /**
     * @brief Create a minimal valid JSON file for testing
     */
    std::filesystem::path createTestJsonFile(const std::string& filename) {
        auto filepath = tempDir / filename;
        std::ofstream file(filepath);
        file << R"({
            "metadata": {
                "name": "Test Truss"
            },
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
                {"id": 2, "x": 4.0, "y": 0.0, "support": "roller_y"},
                {"id": 3, "x": 2.0, "y": 3.0, "support": "free", "fx": 0.0, "fy": -1000.0}
            ],
            "members": [
                {"id": 1, "startNode": 1, "endNode": 2, "area": 0.01, "youngsModulus": 200e9},
                {"id": 2, "startNode": 1, "endNode": 3, "area": 0.01, "youngsModulus": 200e9},
                {"id": 3, "startNode": 2, "endNode": 3, "area": 0.01, "youngsModulus": 200e9}
            ]
        })";
        file.close();
        return filepath;
    }

    /**
     * @brief Create a minimal valid XML file for testing
     */
    std::filesystem::path createTestXmlFile(const std::string& filename) {
        auto filepath = tempDir / filename;
        std::ofstream file(filepath);
        file << R"(<?xml version="1.0"?>
<truss>
    <metadata name="Test Truss"/>
    <nodes>
        <node id="1" x="0.0" y="0.0" support="pinned"/>
        <node id="2" x="4.0" y="0.0" support="roller_y"/>
        <node id="3" x="2.0" y="3.0" support="free" fx="0.0" fy="-1000.0"/>
    </nodes>
    <members>
        <member id="1" startNode="1" endNode="2" area="0.01" youngsModulus="200e9"/>
        <member id="2" startNode="1" endNode="3" area="0.01" youngsModulus="200e9"/>
        <member id="3" startNode="2" endNode="3" area="0.01" youngsModulus="200e9"/>
    </members>
</truss>)";
        file.close();
        return filepath;
    }

    /**
     * @brief Create an invalid JSON file (malformed syntax)
     */
    std::filesystem::path createInvalidJsonFile(const std::string& filename) {
        auto filepath = tempDir / filename;
        std::ofstream file(filepath);
        file << R"({
            "truss": {
                "name": "Invalid",
                "nodes": [
                    {"id": 1, "x": 0.0  // Missing closing brace
                ]
            }
        })";
        file.close();
        return filepath;
    }

    /**
     * @brief Create a structurally invalid truss file (valid syntax, invalid structure)
     */
    std::filesystem::path createStructurallyInvalidFile(const std::string& filename) {
        auto filepath = tempDir / filename;
        std::ofstream file(filepath);
        file << R"({
            "truss": {
                "name": "Structurally Invalid",
                "nodes": [
                    {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"}
                ],
                "members": []
            }
        })";
        file.close();
        return filepath;
    }
};

// ============================================================================
// LIFECYCLE TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, CreateTruss_ReturnsValidHandle) {
    auto result = service.createTruss("NewTruss");

    ASSERT_TRUE(result.success);
    EXPECT_GT(result.value, 0);  // Handle should be non-zero
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, CreateTruss_WithName_StoresName) {
    auto result = service.createTruss("MyBridge");
    ASSERT_TRUE(result.success);

    const auto& view = service.getTrussView(result.value);
    EXPECT_EQ(view.getName(), "MyBridge");
}

TEST_F(TrussApplicationServiceTest, CreateTruss_EmptyName_Succeeds) {
    auto result = service.createTruss("");

    EXPECT_TRUE(result.success);
}

TEST_F(TrussApplicationServiceTest, CreateMultipleTrusses_ReturnsUniqueHandles) {
    auto result1 = service.createTruss("Truss1");
    auto result2 = service.createTruss("Truss2");
    auto result3 = service.createTruss("Truss3");

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
    ASSERT_TRUE(result3.success);

    // Handles should be unique
    EXPECT_NE(result1.value, result2.value);
    EXPECT_NE(result1.value, result3.value);
    EXPECT_NE(result2.value, result3.value);
}

TEST_F(TrussApplicationServiceTest, ClearTruss_ValidHandle_ReturnsTrue) {
    auto result = service.createTruss("Truss");
    ASSERT_TRUE(result.success);

    bool cleared = service.clearTruss(result.value);
    EXPECT_TRUE(cleared);
}

TEST_F(TrussApplicationServiceTest, ClearTruss_InvalidHandle_ReturnsFalse) {
    bool cleared = service.clearTruss(99999);
    EXPECT_FALSE(cleared);
}

TEST_F(TrussApplicationServiceTest, GetTrussView_AfterClear_ThrowsException) {
    auto result = service.createTruss("Truss");
    ASSERT_TRUE(result.success);

    service.clearTruss(result.value);

    EXPECT_THROW(service.getTrussView(result.value), std::invalid_argument);
}

TEST_F(TrussApplicationServiceTest, ClearAll_RemovesAllTrusses) {
    auto result1 = service.createTruss("Truss1");
    auto result2 = service.createTruss("Truss2");

    service.clearAll();

    EXPECT_THROW(service.getTrussView(result1.value), std::invalid_argument);
    EXPECT_THROW(service.getTrussView(result2.value), std::invalid_argument);
}

// ============================================================================
// LOAD TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, LoadTruss_ValidJsonFile_Succeeds) {
    auto filepath = createTestJsonFile("valid.json");

    auto result = service.loadTruss(filepath);

    ASSERT_TRUE(result.success) << "Error: " << result.errorMessage;
    EXPECT_GT(result.value, 0);
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, LoadTruss_ValidJsonFile_LoadsCorrectData) {
    auto filepath = createTestJsonFile("valid.json");

    auto result = service.loadTruss(filepath);
    ASSERT_TRUE(result.success);

    const auto& view = service.getTrussView(result.value);
    EXPECT_EQ(view.getName(), "Test Truss");

    auto nodeViews = view.getNodeViews();
    EXPECT_EQ(nodeViews.size(), 3);

    auto memberViews = view.getMemberViews();
    EXPECT_EQ(memberViews.size(), 3);
}

TEST_F(TrussApplicationServiceTest, LoadTruss_ValidXmlFile_Succeeds) {
    auto filepath = createTestXmlFile("valid.xml");

    auto result = service.loadTruss(filepath);

    ASSERT_TRUE(result.success);
    EXPECT_GT(result.value, 0);
}

TEST_F(TrussApplicationServiceTest, LoadTruss_ExplicitFormat_Succeeds) {
    auto filepath = createTestJsonFile("valid.json");

    auto result = service.loadTruss(filepath, FileFormat::JSON);

    ASSERT_TRUE(result.success);
}

TEST_F(TrussApplicationServiceTest, LoadTruss_NonExistentFile_ReturnsFailure) {
    auto filepath = tempDir / "nonexistent.json";

    auto result = service.loadTruss(filepath);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    EXPECT_EQ(result.value, 0);  // Invalid handle
}

TEST_F(TrussApplicationServiceTest, LoadTruss_InvalidJsonSyntax_ReturnsFailure) {
    auto filepath = createInvalidJsonFile("invalid.json");

    auto result = service.loadTruss(filepath);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, LoadTruss_StructurallyInvalid_ReturnsFailure) {
    auto filepath = createStructurallyInvalidFile("invalid_structure.json");

    auto result = service.loadTruss(filepath);

    // File loads successfully but validation should fail
    // (Depending on implementation, this might succeed at load and fail at validation)
    // If validation is done during load:
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, LoadTruss_EmptyPath_ReturnsFailure) {
    auto result = service.loadTruss("");

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

// ============================================================================
// SAVE TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, SaveTruss_ValidHandle_Succeeds) {
    // Load a valid truss first
    auto loadPath = createTestJsonFile("valid.json");
    auto loadResult = service.loadTruss(loadPath);
    ASSERT_TRUE(loadResult.success);

    auto filepath = tempDir / "output.json";
    auto saveResult = service.saveTruss(loadResult.value, filepath, true);

    EXPECT_TRUE(saveResult.success) << "Error: " << saveResult.errorMessage;
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(TrussApplicationServiceTest, SaveTruss_JsonFormat_CreatesValidFile) {
    // Load a valid truss first
    auto loadPath = createTestJsonFile("valid.json");
    auto loadResult = service.loadTruss(loadPath);
    ASSERT_TRUE(loadResult.success);

    auto filepath = tempDir / "output.json";
    auto saveResult = service.saveTruss(loadResult.value, filepath, FileFormat::JSON, true);
    ASSERT_TRUE(saveResult.success) << "Error: " << saveResult.errorMessage;

    // Verify file can be loaded back
    auto reloadResult = service.loadTruss(filepath);
    EXPECT_TRUE(reloadResult.success) << "Error: " << reloadResult.errorMessage;
}

TEST_F(TrussApplicationServiceTest, SaveTruss_XmlFormat_CreatesValidFile) {
    // Load a valid truss first
    auto loadPath = createTestJsonFile("valid.json");
    auto loadResult = service.loadTruss(loadPath);
    ASSERT_TRUE(loadResult.success);

    auto filepath = tempDir / "output.xml";
    auto saveResult = service.saveTruss(loadResult.value, filepath, FileFormat::XML, true);
    ASSERT_TRUE(saveResult.success) << "Error: " << saveResult.errorMessage;

    // Verify file can be loaded back
    auto reloadResult = service.loadTruss(filepath);
    EXPECT_TRUE(reloadResult.success) << "Error: " << reloadResult.errorMessage;
}

TEST_F(TrussApplicationServiceTest, SaveTruss_WithoutOverwrite_ExistingFile_ReturnsFailure) {
    // Load a valid truss first
    auto loadPath = createTestJsonFile("valid.json");
    auto loadResult = service.loadTruss(loadPath);
    ASSERT_TRUE(loadResult.success);

    auto filepath = tempDir / "output.json";

    // First save
    auto saveResult1 = service.saveTruss(loadResult.value, filepath, true);
    ASSERT_TRUE(saveResult1.success) << "Error: " << saveResult1.errorMessage;

    // Second save without overwrite flag
    auto saveResult2 = service.saveTruss(loadResult.value, filepath, false);
    EXPECT_FALSE(saveResult2.success);
    EXPECT_FALSE(saveResult2.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, SaveTruss_WithOverwrite_ExistingFile_Succeeds) {
    // Load a valid truss first
    auto loadPath = createTestJsonFile("valid.json");
    auto loadResult = service.loadTruss(loadPath);
    ASSERT_TRUE(loadResult.success);

    auto filepath = tempDir / "output.json";

    // First save
    auto saveResult1 = service.saveTruss(loadResult.value, filepath, true);
    ASSERT_TRUE(saveResult1.success) << "Error: " << saveResult1.errorMessage;

    // Second save with overwrite flag
    auto saveResult2 = service.saveTruss(loadResult.value, filepath, true);
    EXPECT_TRUE(saveResult2.success) << "Error: " << saveResult2.errorMessage;
}

TEST_F(TrussApplicationServiceTest, SaveTruss_InvalidHandle_ReturnsFailure) {
    auto filepath = tempDir / "output.json";

    auto result = service.saveTruss(99999, filepath, true);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, SaveTruss_InvalidPath_ReturnsFailure) {
    auto createResult = service.createTruss("SaveTest");
    ASSERT_TRUE(createResult.success);

    // Try to save to a directory that doesn't exist and can't be created
    auto filepath = std::filesystem::path("/invalid/nonexistent/path/output.json");

    auto result = service.saveTruss(createResult.value, filepath, true);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, SaveAndLoad_RoundTrip_PreservesData) {
    // Load original
    auto originalPath = createTestJsonFile("original.json");
    auto loadResult = service.loadTruss(originalPath);
    ASSERT_TRUE(loadResult.success);

    const auto& originalView = service.getTrussView(loadResult.value);
    auto originalNodes = originalView.getNodeViews();
    auto originalMembers = originalView.getMemberViews();

    // Save to new location
    auto savePath = tempDir / "roundtrip.json";
    service.saveTruss(loadResult.value, savePath, true);

    // Load again
    auto reloadResult = service.loadTruss(savePath);
    ASSERT_TRUE(reloadResult.success);

    const auto& reloadedView = service.getTrussView(reloadResult.value);
    auto reloadedNodes = reloadedView.getNodeViews();
    auto reloadedMembers = reloadedView.getMemberViews();

    // Verify data preserved
    EXPECT_EQ(originalView.getName(), reloadedView.getName());
    EXPECT_EQ(originalNodes.size(), reloadedNodes.size());
    EXPECT_EQ(originalMembers.size(), reloadedMembers.size());
}

// ============================================================================
// VALIDATION TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, ValidateTruss_ValidStructure_ReturnsValid) {
    auto filepath = createTestJsonFile("valid.json");
    auto loadResult = service.loadTruss(filepath);
    ASSERT_TRUE(loadResult.success);

    auto validationResult = service.validateTruss(loadResult.value);

    ASSERT_TRUE(validationResult.success);
    EXPECT_TRUE(validationResult.value.isValid());
}

TEST_F(TrussApplicationServiceTest, ValidateTruss_EmptyTruss_ReturnsInvalid) {
    auto createResult = service.createTruss("Empty");
    ASSERT_TRUE(createResult.success);

    auto validationResult = service.validateTruss(createResult.value);

    ASSERT_TRUE(validationResult.success);           // Operation succeeded
    EXPECT_FALSE(validationResult.value.isValid());  // But truss is invalid
    EXPECT_TRUE(validationResult.value.hasFatal());
}

TEST_F(TrussApplicationServiceTest, ValidateTruss_InvalidHandle_ReturnsFailure) {
    auto result = service.validateTruss(99999);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

// ============================================================================
// HANDLE MANAGEMENT TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, GetTrussView_ValidHandle_ReturnsView) {
    auto result = service.createTruss("Test");
    ASSERT_TRUE(result.success);

    const auto& view = service.getTrussView(result.value);
    EXPECT_EQ(view.getName(), "Test");
}

TEST_F(TrussApplicationServiceTest, GetTrussView_InvalidHandle_ThrowsException) {
    EXPECT_THROW(service.getTrussView(99999), std::invalid_argument);
}

TEST_F(TrussApplicationServiceTest, GetTrussMutable_ValidHandle_ReturnsReference) {
    auto result = service.createTruss("Test");
    ASSERT_TRUE(result.success);

    auto& truss = service.getTrussMutable(result.value);

    // Modify truss
    auto node = truss.addNode(0.0, 0.0);

    // Verify modification persisted
    const auto& view = service.getTrussView(result.value);
    EXPECT_EQ(view.getNodeViews().size(), 1);
}

TEST_F(TrussApplicationServiceTest, GetTrussMutable_InvalidHandle_ThrowsException) {
    EXPECT_THROW(service.getTrussMutable(99999), std::invalid_argument);
}

TEST_F(TrussApplicationServiceTest, MultipleHandles_IndependentLifecycles) {
    auto handle1 = service.createTruss("Truss1").value;
    auto handle2 = service.createTruss("Truss2").value;
    auto handle3 = service.createTruss("Truss3").value;

    // Clear handle2
    service.clearTruss(handle2);

    // handle1 and handle3 should still be valid
    EXPECT_NO_THROW(service.getTrussView(handle1));
    EXPECT_NO_THROW(service.getTrussView(handle3));

    // handle2 should be invalid
    EXPECT_THROW(service.getTrussView(handle2), std::invalid_argument);
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, ResultType_SuccessCase_HasCorrectFlags) {
    auto result = service.createTruss("Test");

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result);  // operator bool()
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, ResultType_FailureCase_HasCorrectFlags) {
    auto result = service.loadTruss("nonexistent.json");

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result);  // operator bool()
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, ErrorMessages_AreDescriptive) {
    auto result = service.loadTruss("nonexistent.json");

    EXPECT_FALSE(result.success);
    // Error message should contain useful information
    EXPECT_GT(result.errorMessage.length(), 10);
}

// ============================================================================
// INTERFACE ABSTRACTION TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, ITrussView_ProvidesReadOnlyAccess) {
    auto filepath = createTestJsonFile("test.json");
    auto loadResult = service.loadTruss(filepath);
    ASSERT_TRUE(loadResult.success);

    const auto& view = service.getTrussView(loadResult.value);

    // Should be able to read data
    EXPECT_EQ(view.getName(), "Test Truss");
    EXPECT_EQ(view.getNodeViews().size(), 3);
    EXPECT_EQ(view.getMemberViews().size(), 3);

    // Type check: should return ITrussView interface
    // (compile-time check - if this compiles, interface is correctly exposed)
    const truss::core::interfaces::ITrussView& interface = view;
    (void)interface;  // Suppress unused warning
}

TEST_F(TrussApplicationServiceTest, NodeViews_ContainCorrectData) {
    auto filepath = createTestJsonFile("test.json");
    auto loadResult = service.loadTruss(filepath);
    ASSERT_TRUE(loadResult.success);

    const auto& view = service.getTrussView(loadResult.value);
    auto nodes = view.getNodeViews();

    ASSERT_EQ(nodes.size(), 3);

    // Check first node (0, 0, pinned)
    const auto& node1 = nodes[0];
    EXPECT_EQ(node1.id, 1);
    EXPECT_DOUBLE_EQ(node1.x, 0.0);
    EXPECT_DOUBLE_EQ(node1.y, 0.0);
}

TEST_F(TrussApplicationServiceTest, MemberViews_ContainCorrectData) {
    auto filepath = createTestJsonFile("test.json");
    auto loadResult = service.loadTruss(filepath);
    ASSERT_TRUE(loadResult.success);

    const auto& view = service.getTrussView(loadResult.value);
    auto members = view.getMemberViews();

    ASSERT_EQ(members.size(), 3);

    // Check first member
    const auto& member1 = members[0];
    EXPECT_EQ(member1.id, 1);
    EXPECT_EQ(member1.startNodeId, 1);
    EXPECT_EQ(member1.endNodeId, 2);
    EXPECT_DOUBLE_EQ(member1.area, 0.01);
}

// ============================================================================
// MOVE SEMANTICS TESTS
// ============================================================================

TEST_F(TrussApplicationServiceTest, MoveConstructor_TransfersOwnership) {
    auto result = service.createTruss("Test");
    ASSERT_TRUE(result.success);
    auto handle = result.value;

    // Move service
    TrussApplicationService movedService = std::move(service);

    // Moved-to service should have the truss
    EXPECT_NO_THROW(movedService.getTrussView(handle));
}

TEST_F(TrussApplicationServiceTest, MoveAssignment_TransfersOwnership) {
    auto result = service.createTruss("Test");
    ASSERT_TRUE(result.success);
    auto handle = result.value;

    // Create another service and move-assign
    TrussApplicationService otherService;
    otherService = std::move(service);

    // Moved-to service should have the truss
    EXPECT_NO_THROW(otherService.getTrussView(handle));
}

// ============================================================================
// GUI-FACING METHODS — addNode
// ============================================================================

TEST_F(TrussApplicationServiceTest, AddNode_InvalidHandle_ReturnsFailure) {
    auto result = service.addNode(9999, Point2D{0.0, 0.0}, SupportType::Free);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, AddNode_ValidHandle_ReturnsNodeId) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto result = service.addNode(cr.value, Point2D{1.0, 0.0}, SupportType::Pinned);
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.value, 0u);
}

TEST_F(TrussApplicationServiceTest, AddNode_MarksModified) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    EXPECT_FALSE(service.hasUnsavedChanges(cr.value));
    auto result = service.addNode(cr.value, Point2D{2.0, 3.0}, SupportType::Free);
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(service.hasUnsavedChanges(cr.value));
}

// ============================================================================
// GUI-FACING METHODS — addMember
// ============================================================================

TEST_F(TrussApplicationServiceTest, AddMember_InvalidHandle_ReturnsFailure) {
    MaterialSpec mat{200e9, "Steel"};
    SectionSpec sec{0.01, "Square"};
    auto result = service.addMember(9999, 1, 2, mat, sec);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, AddMember_ValidNodes_ReturnsMemberId) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto n1 = service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto n2 = service.addNode(cr.value, Point2D{1.0, 0.0}, SupportType::RollerY);
    ASSERT_TRUE(n1.success);
    ASSERT_TRUE(n2.success);

    MaterialSpec mat{200e9, "Steel"};
    SectionSpec sec{0.01, "Circular"};
    auto result = service.addMember(cr.value, n1.value, n2.value, mat, sec);
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.value, 0u);
}

/**
 * @test AddMember with non-existent node IDs causes Truss::addMember to throw
 *       std::invalid_argument, which is caught and returned as a Failure.
 *
 * Covers the catch block (line 250) in TrussApplicationService::addMember.
 * Truss::addMember throws when either nodeId is not found in the truss.
 */
TEST_F(TrussApplicationServiceTest, AddMember_InvalidNodeIds_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    // Add one node but use non-existent nodeIds (9998, 9999) for the member —
    // Truss::addMember will throw std::invalid_argument, caught by the service.
    MaterialSpec mat{200e9, "Steel"};
    SectionSpec sec{0.01, "Square"};
    auto result = service.addMember(cr.value, NodeId{9998}, NodeId{9999}, mat, sec);
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.errorMessage.find("Failed to add member"), std::string::npos);
}

// ============================================================================
// GUI-FACING METHODS — removeNode
// ============================================================================

TEST_F(TrussApplicationServiceTest, RemoveNode_InvalidHandle_ReturnsFailure) {
    auto result = service.removeNode(9999, 1);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, RemoveNode_NotFound_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto result = service.removeNode(cr.value, 9999);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, RemoveNode_ValidNode_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    auto result = service.removeNode(cr.value, nr.value);
    EXPECT_TRUE(result.success);
}

// ============================================================================
// GUI-FACING METHODS — removeMember
// ============================================================================

TEST_F(TrussApplicationServiceTest, RemoveMember_InvalidHandle_ReturnsFailure) {
    auto result = service.removeMember(9999, 1);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, RemoveMember_NotFound_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto result = service.removeMember(cr.value, 9999);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, RemoveMember_ValidMember_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto n1 = service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto n2 = service.addNode(cr.value, Point2D{1.0, 0.0}, SupportType::RollerY);
    ASSERT_TRUE(n1.success);
    ASSERT_TRUE(n2.success);
    MaterialSpec mat{200e9, "Steel"};
    SectionSpec sec{0.01, "Square"};
    auto mr = service.addMember(cr.value, n1.value, n2.value, mat, sec);
    ASSERT_TRUE(mr.success);
    auto result = service.removeMember(cr.value, mr.value);
    EXPECT_TRUE(result.success);
}

// ============================================================================
// GUI-FACING METHODS — setNodeSupport
// ============================================================================

TEST_F(TrussApplicationServiceTest, SetNodeSupport_InvalidHandle_ReturnsFailure) {
    auto result = service.setNodeSupport(9999, 1, SupportType::Pinned);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, SetNodeSupport_ValidNode_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    auto result = service.setNodeSupport(cr.value, nr.value, SupportType::Pinned);
    EXPECT_TRUE(result.success);
}

// ============================================================================
// GUI-FACING METHODS — applyNodeLoad
// ============================================================================

TEST_F(TrussApplicationServiceTest, ApplyNodeLoad_InvalidHandle_ReturnsFailure) {
    auto result = service.applyNodeLoad(9999, 1, Force2D{0.0, -1000.0});
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, ApplyNodeLoad_ValidNode_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = service.addNode(cr.value, Point2D{1.0, 1.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    auto result = service.applyNodeLoad(cr.value, nr.value, Force2D{0.0, -5000.0});
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(service.hasUnsavedChanges(cr.value));
}

// ============================================================================
// GUI-FACING METHODS — clearNodeLoad
// ============================================================================

TEST_F(TrussApplicationServiceTest, ClearNodeLoad_InvalidHandle_ReturnsFailure) {
    auto result = service.clearNodeLoad(9999, 1);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, ClearNodeLoad_NodeNotFound_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto result = service.clearNodeLoad(cr.value, 9999);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, ClearNodeLoad_ValidNode_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = service.addNode(cr.value, Point2D{1.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    service.applyNodeLoad(cr.value, nr.value, Force2D{100.0, -200.0});
    auto result = service.clearNodeLoad(cr.value, nr.value);
    EXPECT_TRUE(result.success);
}

// ============================================================================
// GUI-FACING METHODS — updateNode
// ============================================================================

TEST_F(TrussApplicationServiceTest, UpdateNode_InvalidHandle_ReturnsFailure) {
    NodeUpdateSpec spec{1.5, 2.5};
    auto result = service.updateNode(9999, 1, spec);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, UpdateNode_NodeNotFound_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    NodeUpdateSpec spec{1.5, 2.5};
    auto result = service.updateNode(cr.value, 9999, spec);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, UpdateNode_ValidNode_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto nr = service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Free);
    ASSERT_TRUE(nr.success);
    NodeUpdateSpec spec{3.0, 4.0};
    auto result = service.updateNode(cr.value, nr.value, spec);
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(service.hasUnsavedChanges(cr.value));
}

// ============================================================================
// GUI-FACING METHODS — updateMember
// ============================================================================

TEST_F(TrussApplicationServiceTest, UpdateMember_InvalidHandle_ReturnsFailure) {
    MemberUpdateSpec spec{MaterialSpec{200e9, "Steel"}, SectionSpec{0.01, "Square"}};
    auto result = service.updateMember(9999, 1, spec);
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceTest, UpdateMember_MemberNotFound_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    MemberUpdateSpec spec{MaterialSpec{200e9, "Steel"}, SectionSpec{0.01, "Square"}};
    auto result = service.updateMember(cr.value, 9999, spec);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, UpdateMember_ValidMember_Succeeds) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto n1 = service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto n2 = service.addNode(cr.value, Point2D{1.0, 0.0}, SupportType::RollerY);
    ASSERT_TRUE(n1.success);
    ASSERT_TRUE(n2.success);
    auto mr = service.addMember(
        cr.value, n1.value, n2.value, MaterialSpec{200e9, "Steel"}, SectionSpec{0.01, "Square"});
    ASSERT_TRUE(mr.success);
    MemberUpdateSpec spec{MaterialSpec{70e9, "Aluminum"}, SectionSpec{0.005, "Circular"}};
    auto result = service.updateMember(cr.value, mr.value, spec);
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(service.hasUnsavedChanges(cr.value));
}

// ============================================================================
// GUI-FACING METHODS — hasUnsavedChanges
// ============================================================================

TEST_F(TrussApplicationServiceTest, HasUnsavedChanges_InvalidHandle_ReturnsFalse) {
    EXPECT_FALSE(service.hasUnsavedChanges(9999));
}

TEST_F(TrussApplicationServiceTest, HasUnsavedChanges_NewTruss_ReturnsFalse) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    EXPECT_FALSE(service.hasUnsavedChanges(cr.value));
}

TEST_F(TrussApplicationServiceTest, HasUnsavedChanges_AfterModification_ReturnsTrue) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    service.addNode(cr.value, Point2D{0.0, 0.0}, SupportType::Free);
    EXPECT_TRUE(service.hasUnsavedChanges(cr.value));
}

// ============================================================================
// FILE I/O — unknown extension branches
// ============================================================================

TEST_F(TrussApplicationServiceTest, LoadTruss_UnknownExtension_ReturnsFailure) {
    auto path = tempDir / "data.csv";
    {
        std::ofstream f(path);
        f << "dummy";
    }
    auto result = service.loadTruss(path);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(TrussApplicationServiceTest, SaveTruss_UnknownExtension_ReturnsFailure) {
    auto cr = service.createTruss("T");
    ASSERT_TRUE(cr.success);
    auto path = tempDir / "output.csv";
    auto result = service.saveTruss(cr.value, path, true);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

// ============================================================================
// saveTruss — invalid truss (empty) branch
// Exercises if (!validationResult.isValid()) inside saveTruss
// ============================================================================

TEST_F(TrussApplicationServiceTest, SaveTruss_EmptyTruss_ReturnsFailure) {
    // Create an empty (invalid) truss — no nodes, no members, no supports
    auto cr = service.createTruss("Empty");
    ASSERT_TRUE(cr.success);

    auto path = tempDir / "empty_bad.json";
    auto result = service.saveTruss(cr.value, path, /*overwrite=*/true);

    // Empty truss fails validation → saveTruss returns failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}
