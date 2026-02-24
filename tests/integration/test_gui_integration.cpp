/**
 * @file test_gui_integration.cpp
 * @brief Qt Test framework integration tests for GUI components.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * @details
 * Comprehensive GUI integration tests using Qt Test framework.
 * Tests the integration between Controllers, Presenters, and Application Service.
 *
 * Architecture: Integration Tests (GUI Layer)
 * Purpose: Verify full workflow from UI → Controller → Application → Domain
 */

#include "application/truss_application_service.hpp"
#include "application/truss_edit_dtos.hpp"
#include "core/model/types.hpp"
#include "gui/controllers/analysis_controller.hpp"
#include "gui/controllers/project_controller.hpp"
#include "gui/controllers/truss_edit_controller.hpp"
#include "gui/presenters/analysis_results_presenter.hpp"
#include "gui/presenters/truss_data_presenter.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

using namespace truss;
using namespace truss::application;
using namespace truss_controllers;
using namespace truss_presenters;

/**
 * @brief Integration tests for TrussEditController with real Application Service
 */
class TrussEditControllerIntegrationTest : public QObject {
    Q_OBJECT

private:
    TrussApplicationService* service;
    TrussDataPresenter* presenter;
    TrussEditController* controller;
    TrussHandle testHandle;

private slots:
    void initTestCase() {
        // Initialize service and presenter once for all tests
        service = new TrussApplicationService();
        presenter = new TrussDataPresenter();
    }

    void init() {
        // Create fresh controller and truss for each test
        controller = new TrussEditController(service, *presenter);

        auto result = service->createTruss("Integration Test Truss");
        QVERIFY(result.success);
        testHandle = result.value;
        controller->setCurrentTruss(testHandle);
    }

    void cleanup() {
        // Clean up after each test
        service->clearTruss(testHandle);
        delete controller;
    }

    void cleanupTestCase() {
        // Clean up global resources
        service->clearAll();
        delete presenter;
        delete service;
    }

    /**
     * @brief Test adding a node emits correct signals and updates service
     */
    void testNodeAddition() {
        // GIVEN: Signal spy on nodeAdded and trussModified
        QSignalSpy nodeAddedSpy(controller, &TrussEditController::nodeAdded);
        QSignalSpy trussModifiedSpy(controller, &TrussEditController::trussModified);
        QSignalSpy statusSpy(controller, &TrussEditController::statusMessageChanged);

        // WHEN: Request to add node
        core::Point2D position{1.5, 2.5};
        controller->onNodeAddRequested(position, core::SupportType::Free);

        // THEN: Signals emitted
        QCOMPARE(nodeAddedSpy.count(), 1);
        QCOMPARE(trussModifiedSpy.count(), 1);
        QCOMPARE(statusSpy.count(), 1);

        // AND: Node was actually added to service
        const auto& trussView = service->getTrussView(testHandle);
        QCOMPARE(trussView.getNodeCount(), static_cast<size_t>(1));

        // AND: Status message contains node ID and position
        QString statusMsg = statusSpy.at(0).at(0).toString();
        QVERIFY(statusMsg.contains("Node"));
        QVERIFY(statusMsg.contains("1.500"));
        QVERIFY(statusMsg.contains("2.500"));
    }

    /**
     * @brief Test adding multiple nodes
     */
    void testMultipleNodeAddition() {
        QSignalSpy nodeAddedSpy(controller, &TrussEditController::nodeAdded);

        // Add 3 nodes
        controller->onNodeAddRequested(core::Point2D{0, 0}, core::SupportType::Pinned);
        controller->onNodeAddRequested(core::Point2D{3, 0}, core::SupportType::RollerY);
        controller->onNodeAddRequested(core::Point2D{1.5, 2}, core::SupportType::Free);

        QCOMPARE(nodeAddedSpy.count(), 3);

        const auto& trussView = service->getTrussView(testHandle);
        QCOMPARE(trussView.getNodeCount(), static_cast<size_t>(3));
    }

    /**
     * @brief Test adding member with Application DTOs (not Domain types)
     */
    void testMemberAdditionWithDTOs() {
        // GIVEN: Two nodes exist
        auto nodeResult1 = service->addNode(
            testHandle, core::Point2D{0, 0}, core::SupportType::Pinned);
        auto nodeResult2 = service->addNode(
            testHandle, core::Point2D{3, 0}, core::SupportType::Free);
        QVERIFY(nodeResult1.success);
        QVERIFY(nodeResult2.success);

        QSignalSpy memberAddedSpy(controller, &TrussEditController::memberAdded);
        QSignalSpy errorSpy(controller, &TrussEditController::operationFailed);
        QSignalSpy statusSpy(controller, &TrussEditController::statusMessageChanged);

        // WHEN: Add member using Application DTOs (NOT Domain types)
        MaterialSpec material{200e9, "Steel"};
        SectionSpec section{0.01, "Circular"};
        controller->onMemberAddRequested(nodeResult1.value, nodeResult2.value, material, section);

        // THEN: No errors
        QCOMPARE(errorSpy.count(), 0);

        // AND: Member added successfully
        QCOMPARE(memberAddedSpy.count(), 1);

        const auto& trussView = service->getTrussView(testHandle);
        QCOMPARE(trussView.getMemberCount(), static_cast<size_t>(1));

        // AND: Status message formatted by Presenter
        QVERIFY(statusSpy.count() > 0);
        QString statusMsg = statusSpy.last().at(0).toString();
        QVERIFY(statusMsg.contains("Member"));
    }

    /**
     * @brief Test support type change uses Presenter for formatting
     */
    void testSupportTypeChange() {
        // GIVEN: Node exists
        controller->onNodeAddRequested(core::Point2D{0, 0}, core::SupportType::Free);

        QSignalSpy statusSpy(controller, &TrussEditController::statusMessageChanged);

        // WHEN: Change support type
        controller->onSupportTypeChanged(0, core::SupportType::Pinned);

        // THEN: Status message formatted by Presenter (not Controller)
        QCOMPARE(statusSpy.count(), 1);
        QString statusMsg = statusSpy.at(0).at(0).toString();
        QVERIFY(statusMsg.contains("Node 0"));
        QVERIFY(statusMsg.contains("Pinned"));

        // Verify NO switch statement in Controller (formatting delegated)
        // This is architectural compliance - message comes from
        // Presenter.formatSupportChangeMessage()
    }

    /**
     * @brief Test load application with force vector
     */
    void testLoadApplication() {
        // GIVEN: Node exists
        controller->onNodeAddRequested(core::Point2D{0, 0}, core::SupportType::Free);

        QSignalSpy loadAppliedSpy(controller, &TrussEditController::loadApplied);
        QSignalSpy statusSpy(controller, &TrussEditController::statusMessageChanged);

        // WHEN: Apply load
        core::Force2D force{1000.0, -500.0};
        controller->onLoadApplied(0, force);

        // THEN: Signals emitted
        QCOMPARE(loadAppliedSpy.count(), 1);
        QCOMPARE(statusSpy.count(), 1);

        // AND: Status message shows force values
        QString statusMsg = statusSpy.at(0).at(0).toString();
        QVERIFY(statusMsg.contains("1000"));
        QVERIFY(statusMsg.contains("500"));
    }

    /**
     * @brief Test node removal workflow
     */
    void testNodeRemoval() {
        // GIVEN: Node exists
        auto nodeResult = service->addNode(
            testHandle, core::Point2D{0, 0}, core::SupportType::Free);
        QVERIFY(nodeResult.success);

        QSignalSpy trussModifiedSpy(controller, &TrussEditController::trussModified);
        QSignalSpy errorSpy(controller, &TrussEditController::operationFailed);

        // WHEN: Remove node
        controller->onNodeRemoveRequested(nodeResult.value);

        // THEN: No errors
        QCOMPARE(errorSpy.count(), 0);

        // AND: Node removed
        QCOMPARE(trussModifiedSpy.count(), 1);

        const auto& trussView = service->getTrussView(testHandle);
        QCOMPARE(trussView.getNodeCount(), static_cast<size_t>(0));
    }

    /**
     * @brief Test member removal workflow
     */
    void testMemberRemoval() {
        // GIVEN: Member exists
        auto nodeResult1 = service->addNode(
            testHandle, core::Point2D{0, 0}, core::SupportType::Pinned);
        auto nodeResult2 = service->addNode(
            testHandle, core::Point2D{3, 0}, core::SupportType::Free);
        QVERIFY(nodeResult1.success);
        QVERIFY(nodeResult2.success);

        MaterialSpec material{200e9, "Steel"};
        SectionSpec section{0.01, "Square"};
        auto memberResult = service->addMember(
            testHandle, nodeResult1.value, nodeResult2.value, material, section);
        QVERIFY(memberResult.success);

        // WHEN: Remove member
        controller->onMemberRemoveRequested(memberResult.value);

        // THEN: Member removed
        const auto& trussView = service->getTrussView(testHandle);
        QCOMPARE(trussView.getMemberCount(), static_cast<size_t>(0));
        QCOMPARE(trussView.getNodeCount(), static_cast<size_t>(2));  // Nodes remain
    }

    /**
     * @brief Test invalid handle produces error signal
     */
    void testInvalidHandleError() {
        // GIVEN: Controller with no truss set
        TrussEditController invalidController(service, *presenter);

        QSignalSpy errorSpy(&invalidController, &TrussEditController::operationFailed);

        // WHEN: Try operation without setting truss
        invalidController.onNodeAddRequested(core::Point2D{0, 0}, core::SupportType::Free);

        // THEN: Error signal emitted
        QCOMPARE(errorSpy.count(), 1);
        QString errorMsg = errorSpy.at(0).at(0).toString();
        QVERIFY(errorMsg.contains("No active project"));
    }

    /**
     * @brief Test clear truss workflow
     */
    void testClearTruss() {
        // GIVEN: Truss with nodes and members
        controller->onNodeAddRequested(core::Point2D{0, 0}, core::SupportType::Pinned);
        controller->onNodeAddRequested(core::Point2D{3, 0}, core::SupportType::Free);
        MaterialSpec material{200e9, "Steel"};
        SectionSpec section{0.01, "Circular"};
        controller->onMemberAddRequested(0, 1, material, section);

        // WHEN: Clear truss
        controller->onClearTrussRequested();

        // THEN: Handle invalidated (service removes truss)
        QVERIFY(!service->isValidHandle(testHandle));
    }
};

/**
 * @brief Integration tests for TrussDataPresenter with real Domain data
 */
class TrussDataPresenterIntegrationTest : public QObject {
    Q_OBJECT

private:
    TrussApplicationService* service;
    TrussDataPresenter* presenter;
    TrussHandle testHandle;

private slots:
    void initTestCase() {
        service = new TrussApplicationService();
        presenter = new TrussDataPresenter();
    }

    void init() {
        auto result = service->createTruss("Presenter Test Truss");
        QVERIFY(result.success);
        testHandle = result.value;
    }

    void cleanup() { service->clearTruss(testHandle); }

    void cleanupTestCase() {
        service->clearAll();
        delete presenter;
        delete service;
    }

    /**
     * @brief Test formatting real truss status
     */
    void testFormatRealTrussStatus() {
        // GIVEN: Truss with nodes and members
        service->addNode(testHandle, core::Point2D{0, 0}, core::SupportType::Pinned);
        service->addNode(testHandle, core::Point2D{3, 0}, core::SupportType::Free);
        service->addNode(testHandle, core::Point2D{1.5, 2}, core::SupportType::Free);

        MaterialSpec material{200e9, "Steel"};
        SectionSpec section{0.01, "Circular"};
        service->addMember(testHandle, 0, 1, material, section);
        service->addMember(testHandle, 1, 2, material, section);
        service->addMember(testHandle, 2, 0, material, section);

        // WHEN: Format status
        const auto& trussView = service->getTrussView(testHandle);
        auto status = presenter->formatStatus(trussView);

        // THEN: Status message contains valid formatted output
        QVERIFY(!status.statusMessage.isEmpty());
    }

    /**
     * @brief Test formatting node information
     */
    void testFormatNodeInfo() {
        // GIVEN: Node with load
        service->addNode(testHandle, core::Point2D{1.5, 2.5}, core::SupportType::Pinned);
        service->applyNodeLoad(testHandle, 0, core::Force2D{1000, -500});

        const auto& trussView = service->getTrussView(testHandle);
        auto nodeViews = trussView.getNodeViews();

        // WHEN: Format node info
        QString info = presenter->formatNodeInfo(nodeViews[0]);

        // THEN: Contains meaningful node information
        QVERIFY(!info.isEmpty());
        QVERIFY(info.contains("Pinned"));
    }

    /**
     * @brief Test coordinate formatting precision
     */
    void testCoordinatePrecisionWithRealData() {
        core::Point2D point{1.23456789, -2.98765432};
        QString formatted = presenter->formatCoordinate(point);

        // Should be formatted to 3 decimal places
        QVERIFY(formatted.contains("1.235"));
        QVERIFY(formatted.contains("-2.988"));
    }

    /**
     * @brief Test statistics generation from real truss
     */
    void testGenerateStatisticsFromRealTruss() {
        // GIVEN: Truss with mixed supports and loads
        service->addNode(testHandle, core::Point2D{0, 0}, core::SupportType::Pinned);
        service->addNode(testHandle, core::Point2D{3, 0}, core::SupportType::RollerY);
        service->addNode(testHandle, core::Point2D{1.5, 2}, core::SupportType::Free);
        service->applyNodeLoad(testHandle, 2, core::Force2D{0, -1000});

        // WHEN: Generate statistics
        const auto& trussView = service->getTrussView(testHandle);
        QString stats = presenter->generateStatistics(trussView);

        // THEN: Contains all counts
        QVERIFY(stats.contains("Nodes: 3"));
        QVERIFY(stats.contains("Supports: 2"));
        QVERIFY(stats.contains("Loads: 1"));
    }
};

/**
 * @brief Integration tests for ProjectController
 */
class ProjectControllerIntegrationTest : public QObject {
    Q_OBJECT

private:
    TrussApplicationService* service;
    ProjectController* controller;

private slots:
    void initTestCase() {
        service = new TrussApplicationService();
        controller = new ProjectController(service);
    }

    void cleanupTestCase() {
        delete controller;
        delete service;
    }

    /**
     * @brief Test creating new project
     */
    void testNewProjectCreation() {
        QSignalSpy projectCreatedSpy(controller, &ProjectController::projectCreated);

        controller->onNewProject();

        QCOMPARE(projectCreatedSpy.count(), 1);
        TrussHandle handle = projectCreatedSpy.at(0).at(0).value<TrussHandle>();
        QVERIFY(service->isValidHandle(handle));
    }

    /**
     * @brief Test unsaved changes tracking
     */
    void testUnsavedChangesTracking() {
        // Create new project
        controller->onNewProject();
        TrussHandle handle = controller->getCurrentTruss();

        // Initially no unsaved changes (for new empty project)
        // Note: This depends on implementation - may be false or true

        // Modify truss
        service->addNode(handle, core::Point2D{0, 0}, core::SupportType::Free);
        controller->markAsModified();

        // Now has unsaved changes
        QVERIFY(controller->hasUnsavedChanges());

        // Mark as saved
        controller->markAsSaved();
        QVERIFY(!controller->hasUnsavedChanges());
    }
};

// Custom test runner
int main(int argc, char** argv) {
    int failures = 0;

    TrussEditControllerIntegrationTest editTests;
    failures |= QTest::qExec(&editTests, argc, argv);

    TrussDataPresenterIntegrationTest presenterTests;
    failures |= QTest::qExec(&presenterTests, argc, argv);

    ProjectControllerIntegrationTest projectTests;
    failures |= QTest::qExec(&projectTests, argc, argv);

    return failures;
}

#include "test_gui_integration.moc"
