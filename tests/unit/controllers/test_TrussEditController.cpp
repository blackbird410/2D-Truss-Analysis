/**
 * @file test_TrussEditController.cpp
 * @brief Unit tests for TrussEditController
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * These tests verify that TrussEditController correctly orchestrates
 * Application Service calls and Presenter formatting without performing
 * business logic itself (Clean Architecture compliance).
 * 
 * Architecture: Unit Tests (GUI Controller Layer)
 * Purpose: Verify Controller orchestration logic in isolation
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include "gui/controllers/TrussEditController.hpp"
#include "gui/presenters/TrussDataPresenter.hpp"
#include "mocks/MockTrussApplicationService.hpp"

using namespace truss;
using namespace truss::test;
using namespace truss_controllers;
using namespace truss_presenters;
using namespace testing;

/**
 * @brief Test fixture for TrussEditController tests
 * 
 * Provides mock service, real presenter, and controller instance
 * for isolated Controller testing.
 */
class TrussEditControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create Qt application context for signal/slot testing
        int argc = 0;
        char* argv[] = {nullptr};
        if (!QCoreApplication::instance()) {
            app = new QCoreApplication(argc, argv);
        }
        
        // Create mock service and real presenter
        mockService = std::make_unique<MockTrussApplicationService>();
        presenter = std::make_unique<TrussDataPresenter>();
        
        // Create controller with mock service (dependency injection)
        controller = std::make_unique<TrussEditController>(
            mockService.get(), 
            *presenter
        );
        
        // Set valid truss handle for most tests
        controller->setCurrentTruss(testHandle);
    }
    
    void TearDown() override {
        controller.reset();
        presenter.reset();
        mockService.reset();
    }
    
    std::unique_ptr<MockTrussApplicationService> mockService;
    std::unique_ptr<TrussDataPresenter> presenter;
    std::unique_ptr<TrussEditController> controller;
    QCoreApplication* app = nullptr;
    
    // Test data
    const application::TrussHandle testHandle = 42;
    const core::Point2D testPosition{1.5, 2.5};
    const core::NodeId testNodeId = 123;
    const core::MemberId testMemberId = 456;
};

/**
 * @brief Test that onNodeAddRequested calls Application Service correctly
 * 
 * Acceptance Criteria:
 * - Controller calls addNode() on service with correct parameters
 * - Controller emits nodeAdded signal on success
 * - Controller emits trussModified signal on success
 * - Controller emits statusMessageChanged with Presenter-formatted message
 * - Controller does NOT perform formatting itself
 */
TEST_F(TrussEditControllerTest, NodeAddRequestDelegatesCorrectly) {
    // GIVEN: Expected service call with successful result
    core::SupportType support = core::SupportType::Free;
    EXPECT_CALL(*mockService, addNode(testHandle, _, support))
        .WillOnce(Return(application::Result<core::NodeId>::Success(testNodeId)));
    
    // Setup signal spies
    QSignalSpy nodeAddedSpy(controller.get(), &TrussEditController::nodeAdded);
    QSignalSpy trussModifiedSpy(controller.get(), &TrussEditController::trussModified);
    QSignalSpy statusMessageSpy(controller.get(), &TrussEditController::statusMessageChanged);
    
    // WHEN: User requests to add node
    controller->onNodeAddRequested(testPosition, support);
    
    // THEN: Service was called (verified by EXPECT_CALL)
    
    // AND: nodeAdded signal emitted with correct ID
    ASSERT_EQ(nodeAddedSpy.count(), 1);
    EXPECT_EQ(nodeAddedSpy.at(0).at(0).value<core::NodeId>(), testNodeId);
    
    // AND: trussModified signal emitted
    ASSERT_EQ(trussModifiedSpy.count(), 1);
    EXPECT_EQ(trussModifiedSpy.at(0).at(0).value<application::TrussHandle>(), testHandle);
    
    // AND: Status message emitted (presenter formatting verified by content)
    ASSERT_EQ(statusMessageSpy.count(), 1);
    QString statusMsg = statusMessageSpy.at(0).at(0).toString();
    EXPECT_TRUE(statusMsg.contains("Node"));
    EXPECT_TRUE(statusMsg.contains("123"));  // Node ID
    EXPECT_TRUE(statusMsg.contains("added"));
}

/**
 * @brief Test that service failure triggers error signal
 */
TEST_F(TrussEditControllerTest, NodeAddFailureTriggersErrorSignal) {
    // GIVEN: Service returns failure
    core::SupportType support = core::SupportType::Pinned;
    EXPECT_CALL(*mockService, addNode(testHandle, _, support))
        .WillOnce(Return(application::Result<core::NodeId>::Failure("Duplicate node position")));
    
    QSignalSpy failureSpy(controller.get(), &TrussEditController::operationFailed);
    QSignalSpy nodeAddedSpy(controller.get(), &TrussEditController::nodeAdded);
    
    // WHEN: User requests to add node
    controller->onNodeAddRequested(testPosition, support);
    
    // THEN: operationFailed signal emitted with error message
    ASSERT_EQ(failureSpy.count(), 1);
    QString errorMsg = failureSpy.at(0).at(0).toString();
    EXPECT_TRUE(errorMsg.contains("Duplicate node position"));
    
    // AND: nodeAdded signal NOT emitted
    EXPECT_EQ(nodeAddedSpy.count(), 0);
}

/**
 * @brief Test that onMemberAddRequested uses Application DTOs (not Domain types)
 * 
 * Acceptance Criteria:
 * - Controller accepts MaterialSpec and SectionSpec (Application DTOs)
 * - Controller does NOT accept MaterialProperties or SectionProperties (Domain types)
 * - This ensures Domain decoupling per Phase 2 stabilization
 */
TEST_F(TrussEditControllerTest, MemberAddUsesApplicationDTOs) {
    // GIVEN: Application-layer DTOs
    application::MaterialSpec material{200e9, "Steel"};
    application::SectionSpec section{0.01, "Circular"};
    core::NodeId startNode = 1;
    core::NodeId endNode = 2;
    
    // EXPECT: Service receives DTOs (not Domain types)
    EXPECT_CALL(*mockService, addMember(testHandle, startNode, endNode, _, _))
        .WillOnce(Return(application::Result<core::MemberId>::Success(testMemberId)));
    
    QSignalSpy memberAddedSpy(controller.get(), &TrussEditController::memberAdded);
    
    // WHEN: Request member addition with Application DTOs
    controller->onMemberAddRequested(startNode, endNode, material, section);
    
    // THEN: Service called correctly (verified by EXPECT_CALL)
    
    // AND: memberAdded signal emitted
    ASSERT_EQ(memberAddedSpy.count(), 1);
    EXPECT_EQ(memberAddedSpy.at(0).at(0).value<core::MemberId>(), testMemberId);
}

/**
 * @brief Test that support type change delegates to service
 */
TEST_F(TrussEditControllerTest, SupportTypeChangeDelegatesToService) {
    // GIVEN: Expected service call
    core::SupportType newSupport = core::SupportType::RollerX;
    EXPECT_CALL(*mockService, setNodeSupport(testHandle, testNodeId, newSupport))
        .WillOnce(Return(application::Result<bool>::Success(true)));
    
    QSignalSpy statusMessageSpy(controller.get(), &TrussEditController::statusMessageChanged);
    
    // WHEN: User changes support type
    controller->onSupportTypeChanged(testNodeId, newSupport);
    
    // THEN: Service called correctly
    
    // AND: Status message emitted (presenter formatting)
    ASSERT_EQ(statusMessageSpy.count(), 1);
    QString statusMsg = statusMessageSpy.at(0).at(0).toString();
    EXPECT_TRUE(statusMsg.contains("support"));
    EXPECT_TRUE(statusMsg.contains("changed"));
}

/**
 * @brief Test that load application delegates correctly
 */
TEST_F(TrussEditControllerTest, LoadApplicationDelegatesToService) {
    // GIVEN: Expected service call
    core::Force2D force{1000.0, -500.0};
    EXPECT_CALL(*mockService, applyNodeLoad(testHandle, testNodeId, _))
        .WillOnce(Return(application::Result<bool>::Success(true)));
    
    QSignalSpy loadAppliedSpy(controller.get(), &TrussEditController::loadApplied);
    
    // WHEN: User applies load
    controller->onLoadApplied(testNodeId, force);
    
    // THEN: Service called correctly
    
    // AND: loadApplied signal emitted
    ASSERT_EQ(loadAppliedSpy.count(), 1);
    EXPECT_EQ(loadAppliedSpy.at(0).at(0).value<size_t>(), testNodeId);
    EXPECT_DOUBLE_EQ(loadAppliedSpy.at(0).at(1).toDouble(), 1000.0);
    EXPECT_DOUBLE_EQ(loadAppliedSpy.at(0).at(2).toDouble(), -500.0);
}

/**
 * @brief Test that Controller delegates ALL formatting to Presenter
 * 
 * Acceptance Criteria:
 * - Controller contains NO switch statements for enum-to-string
 * - Controller contains NO QString formatting logic
 * - All status messages come from Presenter methods
 * - This ensures MVP pattern compliance per Phase 3 stabilization
 */
TEST_F(TrussEditControllerTest, AllFormattingDelegatedToPresenter) {
    // This test ensures architectural correctness by verifying
    // that status messages contain presenter-formatted content
    
    // Test 1: Node addition message
    EXPECT_CALL(*mockService, addNode(testHandle, _, _))
        .WillOnce(Return(application::Result<core::NodeId>::Success(testNodeId)));
    
    QSignalSpy statusSpy(controller.get(), &TrussEditController::statusMessageChanged);
    controller->onNodeAddRequested(testPosition);
    
    // Verify message contains presenter formatting (coordinates with precision)
    QString msg = statusSpy.at(0).at(0).toString();
    EXPECT_TRUE(msg.contains("1.500") || msg.contains("1.50"));  // Presenter formats coordinates
    EXPECT_TRUE(msg.contains("2.500") || msg.contains("2.50"));
    
    // Test 2: Support change message
    statusSpy.clear();
    EXPECT_CALL(*mockService, setNodeSupport(testHandle, testNodeId, core::SupportType::Pinned))
        .WillOnce(Return(application::Result<bool>::Success(true)));
    
    controller->onSupportTypeChanged(testNodeId, core::SupportType::Pinned);
    
    // Verify presenter formatted support type name
    msg = statusSpy.at(0).at(0).toString();
    EXPECT_TRUE(msg.contains("Pinned") || msg.contains("pinned"));  // Presenter formats enum
}

/**
 * @brief Test that invalid handle triggers operationFailed signal
 * 
 * Acceptance Criteria:
 * - Controller validates handle before service calls
 * - Controller emits operationFailed("No truss is currently loaded") on invalid handle
 * - Controller does NOT call service with invalid handle
 */
TEST_F(TrussEditControllerTest, InvalidHandleTriggersFailureSignal) {
    // GIVEN: Controller with no truss set (handle = 0)
    controller->setCurrentTruss(0);
    
    // EXPECT: Service is NOT called
    EXPECT_CALL(*mockService, addNode(_, _, _)).Times(0);
    
    QSignalSpy failureSpy(controller.get(), &TrussEditController::operationFailed);
    
    // WHEN: User requests operation
    controller->onNodeAddRequested(testPosition);
    
    // THEN: operationFailed signal emitted
    ASSERT_EQ(failureSpy.count(), 1);
    QString errorMsg = failureSpy.at(0).at(0).toString();
    EXPECT_TRUE(errorMsg.contains("truss") || errorMsg.contains("loaded"));
}

/**
 * @brief Test node removal delegates correctly
 */
TEST_F(TrussEditControllerTest, NodeRemovalDelegatesToService) {
    // GIVEN: Expected service call
    EXPECT_CALL(*mockService, removeNode(testHandle, testNodeId))
        .WillOnce(Return(application::Result<bool>::Success(true)));
    
    QSignalSpy statusSpy(controller.get(), &TrussEditController::statusMessageChanged);
    
    // WHEN: User removes node
    controller->onNodeRemoveRequested(testNodeId);
    
    // THEN: Service called correctly
    
    // AND: Status message emitted
    ASSERT_EQ(statusSpy.count(), 1);
}

/**
 * @brief Test member removal delegates correctly
 */
TEST_F(TrussEditControllerTest, MemberRemovalDelegatesToService) {
    // GIVEN: Expected service call
    EXPECT_CALL(*mockService, removeMember(testHandle, testMemberId))
        .WillOnce(Return(application::Result<bool>::Success(true)));
    
    QSignalSpy statusSpy(controller.get(), &TrussEditController::statusMessageChanged);
    
    // WHEN: User removes member
    controller->onMemberRemoveRequested(testMemberId);
    
    // THEN: Service called correctly
    
    // AND: Status message emitted
    ASSERT_EQ(statusSpy.count(), 1);
}

/**
 * @brief Test load clearing delegates correctly
 */
TEST_F(TrussEditControllerTest, LoadClearingDelegatesToService) {
    // GIVEN: Expected service call
    EXPECT_CALL(*mockService, clearNodeLoad(testHandle, testNodeId))
        .WillOnce(Return(application::Result<bool>::Success(true)));
    
    QSignalSpy statusSpy(controller.get(), &TrussEditController::statusMessageChanged);
    
    // WHEN: User clears load
    controller->onLoadCleared(testNodeId);
    
    // THEN: Service called correctly
    
    // AND: Status message emitted
    ASSERT_EQ(statusSpy.count(), 1);
}

/**
 * @brief Test truss clearing delegates correctly
 */
TEST_F(TrussEditControllerTest, TrussClearingDelegatesToService) {
    // GIVEN: Expected service call
    EXPECT_CALL(*mockService, clearTruss(testHandle))
        .WillOnce(Return(true));
    
    QSignalSpy statusSpy(controller.get(), &TrussEditController::statusMessageChanged);
    
    // WHEN: User clears truss
    controller->onClearTrussRequested();
    
    // THEN: Service called correctly
    
    // AND: Status message emitted
    ASSERT_EQ(statusSpy.count(), 1);}