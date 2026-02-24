/**
 * @file test_ui_load_diagnosis.cpp
 * @brief Diagnostic test for UI-triggered project load workflow.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This test simulates the exact UI load path to diagnose why
 * geometry doesn't appear on canvas after UI-triggered load.
 */

#include "application/truss_application_service.hpp"
#include "core/model/types.hpp"
#include "gui/controllers/project_controller.hpp"
#include "gui/controllers/truss_edit_controller.hpp"
#include "gui/presenters/truss_data_presenter.hpp"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTextStream>

#include <gtest/gtest.h>

using namespace truss;
using namespace truss::core;
using namespace truss::application;
using namespace truss_controllers;
using namespace truss_presenters;

/**
 * @brief Diagnostic test for UI load flow
 */
class UILoadDiagnosisTest : public ::testing::Test {
protected:
    void SetUp() override {
        trussService = std::make_unique<TrussApplicationService>();
        presenter = std::make_unique<TrussDataPresenter>();

        editController = std::make_unique<TrussEditController>(trussService.get(), *presenter);
        projectController = std::make_unique<ProjectController>(trussService.get());

        // Simulate MainWindow signal connections
        QObject::connect(projectController.get(),
                         &ProjectController::projectOpened,
                         [this](TrussHandle handle, const QString&) {
                             // Simulate MainWindow behavior
                             editController->setCurrentTruss(handle);
                             simulatedCanvasHandle = handle;
                         });
    }

    std::unique_ptr<TrussApplicationService> trussService;
    std::unique_ptr<TrussDataPresenter> presenter;
    std::unique_ptr<TrussEditController> editController;
    std::unique_ptr<ProjectController> projectController;

    TrussHandle simulatedCanvasHandle = 0;
};

/**
 * @brief Test exact UI load path with diagnostic logging
 */
TEST_F(UILoadDiagnosisTest, UI_LoadPath_HandlePropagation) {
    // Create test file (simulates user selecting file)
    QString filepath = QDir::temp().filePath("ui_load_diagnosis.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "UILoadTest"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 5.0, "y": 0.0, "support": "roller_y"},
            {"id": 3, "x": 2.5, "y": 3.0, "support": "free"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 3, "area": 0.002, "youngsModulus": 200e9},
            {"id": 2, "startNode": 2, "endNode": 3, "area": 0.002, "youngsModulus": 200e9},
            {"id": 3, "startNode": 1, "endNode": 2, "area": 0.002, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Setup signal spy
    QSignalSpy projectOpenedSpy(projectController.get(), &ProjectController::projectOpened);
    QSignalSpy operationFailedSpy(projectController.get(), &ProjectController::operationFailed);

    // *** SIMULATE UI ACTION: User clicks File → Open → Selects file ***
    projectController->onOpenProject(filepath);

    // Check for errors
    if (operationFailedSpy.count() > 0) {
        QString error = operationFailedSpy.takeFirst().at(0).toString();
        FAIL() << "Load operation failed: " << error.toStdString();
    }

    // Verify signal emitted
    ASSERT_EQ(projectOpenedSpy.count(), 1) << "projectOpened signal must be emitted";

    // Verify handle propagation
    TrussHandle projectHandle = projectController->getCurrentTruss();
    TrussHandle editHandle = editController->getCurrentTruss();

    EXPECT_NE(projectHandle, 0) << "ProjectController must have valid handle";
    EXPECT_NE(editHandle, 0) << "EditController must have valid handle";
    EXPECT_NE(simulatedCanvasHandle, 0) << "Canvas must have valid handle";

    EXPECT_EQ(projectHandle, editHandle) << "Handles must match";
    EXPECT_EQ(projectHandle, simulatedCanvasHandle) << "Canvas handle must match";

    // Verify geometry available
    const auto& view = trussService->getTrussView(projectHandle);
    EXPECT_EQ(view.getNodeCount(), 3);
    EXPECT_EQ(view.getMemberCount(), 3);

    // Simulate canvas paint
    if (simulatedCanvasHandle != 0) {
        const auto& canvasView = trussService->getTrussView(simulatedCanvasHandle);
        auto nodeViews = canvasView.getNodeViews();
    } else {
        FAIL() << "Canvas handle is invalid after load";
    }
    // Cleanup
    QFile::remove(filepath);
}

/**
 * @brief Test that verifies DrawingCanvas receives handle update
 */
TEST_F(UILoadDiagnosisTest, DrawingCanvas_ReceivesHandleUpdate) {
    // Create minimal valid truss file
    QString filepath = QDir::temp().filePath("canvas_handle_test.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "CanvasTest"},
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

    // Track whether canvas slot was called
    bool canvasSlotCalled = false;
    TrussHandle receivedHandle = 0;

    // Connect a lambda that simulates DrawingCanvas::setTrussHandle()
    QObject::connect(projectController.get(),
                     &ProjectController::projectOpened,
                     [&canvasSlotCalled, &receivedHandle](TrussHandle handle, const QString&) {
                         canvasSlotCalled = true;
                         receivedHandle = handle;
                     });

    // Trigger load
    projectController->onOpenProject(filepath);

    // Verify canvas received update
    EXPECT_TRUE(canvasSlotCalled) << "Canvas setTrussHandle() must be called";
    EXPECT_NE(receivedHandle, 0) << "Canvas must receive valid handle";

    // Cleanup
    QFile::remove(filepath);
}
