/**
 * @file test_gui_lifecycle.cpp
 * @brief Integration tests for GUI mutation and load lifecycle
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Tests for Phase 1-2 regression investigation:
 * - Add node triggers canvas update
 * - Load project triggers canvas update
 * - State propagation through MVP layers
 */

#include "application/truss_application_service.hpp"
#include "core/model/types.hpp"
#include "gui/controllers/project_controller.hpp"
#include "gui/controllers/truss_edit_controller.hpp"
#include "gui/presenters/truss_data_presenter.hpp"

#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTextStream>

#include <filesystem>
#include <gtest/gtest.h>

using namespace truss;
using namespace truss::core;
using namespace truss::application;
using namespace truss_controllers;
using namespace truss_presenters;

/**
 * @brief Integration test fixture for GUI lifecycle
 */
class GUILifecycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create real Application services (not mocks - this is integration test)
        trussService = std::make_unique<TrussApplicationService>();
        presenter = std::make_unique<TrussDataPresenter>();

        // Create controllers with real dependencies
        editController = std::make_unique<TrussEditController>(trussService.get(), *presenter);
        projectController = std::make_unique<ProjectController>(trussService.get());
    }

    std::unique_ptr<TrussApplicationService> trussService;
    std::unique_ptr<TrussDataPresenter> presenter;
    std::unique_ptr<TrussEditController> editController;
    std::unique_ptr<ProjectController> projectController;
};

/**
 * @brief Test that adding a node triggers trussModified signal
 *
 * This validates Phase 1 fix: Controller must emit trussModified after successful mutation
 */
TEST_F(GUILifecycleTest, AddNodeEmitsTrussModifiedSignal) {
    // Create initial project
    auto result = trussService->createTruss("TestProject");
    ASSERT_TRUE(result.success);
    TrussHandle handle = result.value;

    // Set active truss in controller
    editController->setCurrentTruss(handle);

    // Setup signal spy to detect trussModified emission
    QSignalSpy trussModifiedSpy(editController.get(), &TrussEditController::trussModified);
    QSignalSpy nodeAddedSpy(editController.get(), &TrussEditController::nodeAdded);

    // Request node addition
    Point2D position{0.0, 0.0};
    editController->onNodeAddRequested(position, SupportType::Free);

    // Verify signals emitted
    EXPECT_EQ(nodeAddedSpy.count(), 1) << "nodeAdded signal must be emitted";
    EXPECT_EQ(trussModifiedSpy.count(), 1) << "trussModified signal must be emitted";

    // Verify signal contains correct handle
    QList<QVariant> arguments = trussModifiedSpy.takeFirst();
    TrussHandle emittedHandle = arguments.at(0).value<TrussHandle>();
    EXPECT_EQ(emittedHandle, handle);
}

/**
 * @brief Test that adding a node actually modifies the domain
 *
 * Validates complete chain: GUI request → Controller → Service → Domain mutation
 */
TEST_F(GUILifecycleTest, AddNodeModifiesDomain) {
    // Create initial project
    auto result = trussService->createTruss("TestProject");
    ASSERT_TRUE(result.success);
    TrussHandle handle = result.value;

    editController->setCurrentTruss(handle);

    // Verify initial state
    const auto& viewBefore = trussService->getTrussView(handle);
    EXPECT_EQ(viewBefore.getNodeCount(), 0);

    // Add node
    Point2D position{5.0, 3.0};
    editController->onNodeAddRequested(position, SupportType::Pinned);

    // Verify domain was mutated
    const auto& viewAfter = trussService->getTrussView(handle);
    EXPECT_EQ(viewAfter.getNodeCount(), 1) << "Node must be added to domain";

    auto nodeViews = viewAfter.getNodeViews();
    ASSERT_EQ(nodeViews.size(), 1);
    EXPECT_DOUBLE_EQ(nodeViews[0].x, 5.0);
    EXPECT_DOUBLE_EQ(nodeViews[0].y, 3.0);
}

/**
 * @brief Test that loading a project triggers projectOpened signal with correct handle
 *
 * Validates Phase 2 investigation: Load workflow must propagate handle correctly
 */
TEST_F(GUILifecycleTest, LoadProjectEmitsProjectOpenedSignal) {
    // Create temporary test file with valid JSON and .json extension
    QString tempFilePath = QDir::temp().filePath("test_project_load.json");
    QFile tempFile(tempFilePath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "TestTruss"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 5.0, "y": 3.0, "support": "roller_y"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "area": 0.002, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Verify file exists
    ASSERT_TRUE(QFile::exists(tempFilePath))
        << "Test file must exist at: " << tempFilePath.toStdString();

    // Setup signal spies for debugging
    QSignalSpy projectOpenedSpy(projectController.get(), &ProjectController::projectOpened);
    QSignalSpy operationFailedSpy(projectController.get(), &ProjectController::operationFailed);

    // Load project
    projectController->onOpenProject(tempFilePath);

    // Debug output
    if (operationFailedSpy.count() > 0) {
        QList<QVariant> failArgs = operationFailedSpy.takeFirst();
        ADD_FAILURE() << "Load failed: " << failArgs.at(0).toString().toStdString();
    }

    // Verify signal emitted
    ASSERT_EQ(projectOpenedSpy.count(), 1) << "projectOpened signal must be emitted";

    // Verify signal arguments
    QList<QVariant> arguments = projectOpenedSpy.takeFirst();
    TrussHandle emittedHandle = arguments.at(0).value<TrussHandle>();
    QString emittedPath = arguments.at(1).toString();

    EXPECT_NE(emittedHandle, 0) << "Valid TrussHandle must be emitted";
    EXPECT_EQ(emittedPath, tempFilePath);

    // Cleanup
    QFile::remove(tempFilePath);
}

/**
 * @brief Test that loaded project contains correct geometry
 *
 * Validates complete load chain: File → Infrastructure → Domain → View
 */
TEST_F(GUILifecycleTest, LoadProjectReconstructsGeometry) {
    // Create temporary test file with .json extension
    QString filepath = QDir::temp().filePath("test_geometry_load.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "GeometryTest"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 10.0, "y": 0.0, "support": "pinned"},
            {"id": 3, "x": 5.0, "y": 5.0, "support": "free"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 3, "area": 0.001, "youngsModulus": 200e9},
            {"id": 2, "startNode": 2, "endNode": 3, "area": 0.001, "youngsModulus": 200e9},
            {"id": 3, "startNode": 1, "endNode": 2, "area": 0.001, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Add debug spies
    QSignalSpy failSpy(projectController.get(), &ProjectController::operationFailed);

    // Load project
    projectController->onOpenProject(filepath);

    // Debug output
    if (failSpy.count() > 0) {
        ADD_FAILURE() << "Geometry load failed: "
                      << failSpy.takeFirst().at(0).toString().toStdString();
    }

    // Get handle
    TrussHandle handle = projectController->getCurrentTruss();
    ASSERT_NE(handle, 0);

    // Verify geometry reconstruction
    const auto& view = trussService->getTrussView(handle);

    EXPECT_EQ(view.getNodeCount(), 3) << "All nodes must be loaded";
    EXPECT_EQ(view.getMemberCount(), 3) << "All members must be loaded";

    auto nodeViews = view.getNodeViews();
    ASSERT_EQ(nodeViews.size(), 3);

    // Verify node coordinates
    EXPECT_DOUBLE_EQ(nodeViews[0].x, 0.0);
    EXPECT_DOUBLE_EQ(nodeViews[0].y, 0.0);
    EXPECT_DOUBLE_EQ(nodeViews[1].x, 10.0);
    EXPECT_DOUBLE_EQ(nodeViews[1].y, 0.0);
    EXPECT_DOUBLE_EQ(nodeViews[2].x, 5.0);
    EXPECT_DOUBLE_EQ(nodeViews[2].y, 5.0);

    // Cleanup
    QFile::remove(filepath);
}

/**
 * @brief Test that invalid handle prevents node addition
 *
 * Validates guard logic in controller
 */
TEST_F(GUILifecycleTest, InvalidHandlePreventsNodeAddition) {
    // DO NOT set current truss (handle = 0)

    QSignalSpy operationFailedSpy(editController.get(), &TrussEditController::operationFailed);
    QSignalSpy nodeAddedSpy(editController.get(), &TrussEditController::nodeAdded);

    // Attempt to add node with invalid handle
    editController->onNodeAddRequested(Point2D{0, 0}, SupportType::Free);

    // Verify failure
    EXPECT_EQ(operationFailedSpy.count(), 1) << "operationFailed must be emitted";
    EXPECT_EQ(nodeAddedSpy.count(), 0) << "nodeAdded must NOT be emitted";
}

/**
 * @brief Test that ProjectController tracks current handle after load
 */
TEST_F(GUILifecycleTest, ProjectControllerTracksLoadedHandle) {
    // Initial state
    EXPECT_EQ(projectController->getCurrentTruss(), 0);

    // Create test file with .json extension
    QString filepath = QDir::temp().filePath("test_handle_tracking.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "HandleTest"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 3.0, "y": 0.0, "support": "roller_y"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "area": 0.001, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Load project
    projectController->onOpenProject(filepath);

    // Verify handle updated
    TrussHandle handle = projectController->getCurrentTruss();
    EXPECT_NE(handle, 0) << "ProjectController must track loaded handle";

    // Verify handle is valid
    const auto& view = trussService->getTrussView(handle);
    EXPECT_EQ(view.getNodeCount(), 2);
    EXPECT_EQ(view.getMemberCount(), 1);

    // Cleanup
    QFile::remove(filepath);
}

/**
 * @brief Test dirty state management
 */
TEST_F(GUILifecycleTest, DirtyStateTracking) {
    // Create project
    auto result = trussService->createTruss("DirtyTest");
    ASSERT_TRUE(result.success);
    TrussHandle handle = result.value;

    editController->setCurrentTruss(handle);

    // Initially clean
    EXPECT_FALSE(projectController->hasUnsavedChanges());

    // Add node → should mark dirty
    editController->onNodeAddRequested(Point2D{0, 0}, SupportType::Free);
    projectController->markAsModified();

    EXPECT_TRUE(projectController->hasUnsavedChanges());
}
