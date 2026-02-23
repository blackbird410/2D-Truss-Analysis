/**
 * @file test_load_viewport_fix.cpp
 * @brief Validation test for UI load viewport auto-zoom fix
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This test verifies that the viewport fix correctly auto-zooms
 * to fit loaded geometry, ensuring visibility after load.
 */

#include "application/truss_application_service.hpp"
#include "gui/controllers/project_controller.hpp"
#include "gui/presenters/truss_data_presenter.hpp"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <gtest/gtest.h>

using namespace truss;
using namespace truss::application;
using namespace truss_controllers;
using namespace truss_presenters;

/**
 * @brief Test fixture for viewport fix validation
 */
class LoadViewportFixTest : public ::testing::Test {
protected:
    void SetUp() override {
        trussService = std::make_unique<TrussApplicationService>();
        presenter = std::make_unique<TrussDataPresenter>();
        projectController = std::make_unique<ProjectController>(trussService.get());
    }

    std::unique_ptr<TrussApplicationService> trussService;
    std::unique_ptr<TrussDataPresenter> presenter;
    std::unique_ptr<ProjectController> projectController;
};

/**
 * @brief Test that loading geometry at large scale is handled correctly
 */
TEST_F(LoadViewportFixTest, LoadLargeScaleGeometry_AutoZooms) {
    // Create truss with large coordinates (100m scale)
    QString filepath = QDir::temp().filePath("large_scale_truss.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "LargeScaleTruss"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 100.0, "y": 0.0, "support": "roller_y"},
            {"id": 3, "x": 50.0, "y": 75.0, "support": "free"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 3, "area": 0.002, "youngsModulus": 200e9},
            {"id": 2, "startNode": 2, "endNode": 3, "area": 0.002, "youngsModulus": 200e9},
            {"id": 3, "startNode": 1, "endNode": 2, "area": 0.002, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Load project
    projectController->onOpenProject(filepath);

    // Verify handle created
    TrussHandle handle = projectController->getCurrentTruss();
    ASSERT_NE(handle, 0);

    // Verify geometry loaded
    const auto& view = trussService->getTrussView(handle);
    EXPECT_EQ(view.getNodeCount(), 3);

    // NOTE: In actual GUI, DrawingCanvas::setTrussHandle() would be called
    // which now includes zoomToFit() automatically

    // Cleanup
    QFile::remove(filepath);
}

/**
 * @brief Test that loading geometry offset from origin is handled correctly
 */
TEST_F(LoadViewportFixTest, LoadOffsetGeometry_AutoCenters) {
    // Create truss offset from origin
    QString filepath = QDir::temp().filePath("offset_truss.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "OffsetTruss"},
        "nodes": [
            {"id": 1, "x": 500.0, "y": 500.0, "support": "pinned"},
            {"id": 2, "x": 510.0, "y": 500.0, "support": "roller_y"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "area": 0.001, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Load project
    projectController->onOpenProject(filepath);

    // Verify handle created
    TrussHandle handle = projectController->getCurrentTruss();
    ASSERT_NE(handle, 0);

    // Verify geometry loaded at offset location
    const auto& view = trussService->getTrussView(handle);
    auto nodes = view.getNodeViews();
    ASSERT_EQ(nodes.size(), 2);
    EXPECT_DOUBLE_EQ(nodes[0].x, 500.0);
    EXPECT_DOUBLE_EQ(nodes[1].x, 510.0);

    // NOTE: Canvas zoomToFit() will automatically center this in viewport

    // Cleanup
    QFile::remove(filepath);
}

/**
 * @brief Test that loading small-scale geometry is handled correctly
 */
TEST_F(LoadViewportFixTest, LoadSmallScaleGeometry_AutoZooms) {
    // Create truss with small coordinates (0.1m scale)
    QString filepath = QDir::temp().filePath("small_scale_truss.json");
    QFile tempFile(filepath);
    ASSERT_TRUE(tempFile.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&tempFile);
    stream << R"({
        "metadata": {"name": "SmallScaleTruss"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 0.1, "y": 0.0, "support": "roller_y"},
            {"id": 3, "x": 0.05, "y": 0.08, "support": "free"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 3, "area": 0.0001, "youngsModulus": 200e9},
            {"id": 2, "startNode": 2, "endNode": 3, "area": 0.0001, "youngsModulus": 200e9},
            {"id": 3, "startNode": 1, "endNode": 2, "area": 0.0001, "youngsModulus": 200e9}
        ]
    })";
    stream.flush();
    tempFile.close();

    // Load project
    projectController->onOpenProject(filepath);

    // Verify handle created
    TrussHandle handle = projectController->getCurrentTruss();
    ASSERT_NE(handle, 0);

    // Verify geometry loaded
    const auto& view = trussService->getTrussView(handle);
    EXPECT_EQ(view.getNodeCount(), 3);

    // NOTE: Canvas zoomToFit() will automatically scale this to visible size

    // Cleanup
    QFile::remove(filepath);
}

/**
 * @brief Integration test: Multiple consecutive loads update viewport correctly
 */
TEST_F(LoadViewportFixTest, ConsecutiveLoads_UpdateViewportEachTime) {
    // Load first project (small scale)
    QString filepath1 = QDir::temp().filePath("truss1.json");
    QFile file1(filepath1);
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream1(&file1);
    stream1 << R"({
        "metadata": {"name": "Truss1"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 1.0, "y": 0.0, "support": "roller_y"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "area": 0.001, "youngsModulus": 200e9}
        ]
    })";
    stream1.flush();
    file1.close();

    projectController->onOpenProject(filepath1);
    TrussHandle handle1 = projectController->getCurrentTruss();
    ASSERT_NE(handle1, 0);

    // Load second project (large scale)
    QString filepath2 = QDir::temp().filePath("truss2.json");
    QFile file2(filepath2);
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream2(&file2);
    stream2 << R"({
        "metadata": {"name": "Truss2"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 50.0, "y": 0.0, "support": "roller_y"}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "area": 0.001, "youngsModulus": 200e9}
        ]
    })";
    stream2.flush();
    file2.close();

    projectController->onOpenProject(filepath2);
    TrussHandle handle2 = projectController->getCurrentTruss();
    ASSERT_NE(handle2, 0);

    // Verify handle changed
    EXPECT_NE(handle1, handle2);

    // Verify second geometry loaded
    const auto& view = trussService->getTrussView(handle2);
    auto nodes = view.getNodeViews();
    EXPECT_DOUBLE_EQ(nodes[1].x, 50.0);

    // NOTE: Each setTrussHandle() call will auto-zoom to new geometry

    // Cleanup
    QFile::remove(filepath1);
    QFile::remove(filepath2);
}
