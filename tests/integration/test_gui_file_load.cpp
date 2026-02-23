/**
 * @file test_gui_file_load.cpp
 * @brief Diagnostic test for GUI file loading regression
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This test diagnoses why files selected via QFileDialog
 * are not being loaded into the application.
 */

#include "application/truss_application_service.hpp"
#include "gui/controllers/project_controller.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <filesystem>
#include <gtest/gtest.h>

using namespace truss;
using namespace truss::application;
using namespace truss_controllers;

class GUIFileLoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        trussService = std::make_unique<TrussApplicationService>();
        projectController = std::make_unique<ProjectController>(trussService.get());

        // Create test file
        testFilePath = QDir::temp().filePath("gui_load_test.json");
        QFile file(testFilePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream stream(&file);
        stream << R"({
            "metadata": {"name": "GUI Load Test"},
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
                {"id": 2, "x": 5.0, "y": 0.0, "support": "roller_y"}
            ],
            "members": [
                {"id": 1, "startNode": 1, "endNode": 2, "area": 0.001, "youngsModulus": 200e9}
            ]
        })";
        stream.flush();
        file.close();
    }

    void TearDown() override { QFile::remove(testFilePath); }

    std::unique_ptr<TrussApplicationService> trussService;
    std::unique_ptr<ProjectController> projectController;
    QString testFilePath;
};

/**
 * @brief Test that QString filepath is correctly converted to std::filesystem::path
 */
TEST_F(GUIFileLoadTest, PathConversion_QStringToFilesystem) {
    qDebug() << "[TEST] Original QString path:" << testFilePath;

    // Simulate ProjectController's path conversion
    QByteArray utf8Data = testFilePath.toUtf8();
    std::u8string u8str(reinterpret_cast<const char8_t*>(utf8Data.constData()), utf8Data.size());
    std::filesystem::path fsPath(u8str);

    qDebug() << "[TEST] Converted std::filesystem::path:"
             << QString::fromStdString(fsPath.string());
    qDebug() << "[TEST] File exists (filesystem::exists):" << std::filesystem::exists(fsPath);
    qDebug() << "[TEST] File exists (QFile):" << QFile::exists(testFilePath);

    EXPECT_TRUE(std::filesystem::exists(fsPath))
        << "Path conversion failed - file not found via std::filesystem";
}

/**
 * @brief Test direct TrussApplicationService::loadTruss with filesystem::path
 */
TEST_F(GUIFileLoadTest, DirectLoad_ViaApplicationService) {
    QByteArray utf8Data = testFilePath.toUtf8();
    std::u8string u8str(reinterpret_cast<const char8_t*>(utf8Data.constData()), utf8Data.size());
    std::filesystem::path fsPath(u8str);

    qDebug() << "[TEST] Loading via TrussApplicationService::loadTruss()";
    auto result = trussService->loadTruss(fsPath);

    qDebug() << "[TEST] Load result - success:" << result.success;
    if (!result.success) {
        qDebug() << "[TEST] Error message:" << QString::fromStdString(result.errorMessage);
    } else {
        qDebug() << "[TEST] Handle:" << result.value;

        // Verify loaded data
        const auto& view = trussService->getTrussView(result.value);
        qDebug() << "[TEST] Node count:" << view.getNodeCount();
        qDebug() << "[TEST] Member count:" << view.getMemberCount();

        EXPECT_EQ(view.getNodeCount(), 2);
        EXPECT_EQ(view.getMemberCount(), 1);
    }

    ASSERT_TRUE(result.success) << "Direct load via ApplicationService failed";
}

/**
 * @brief Test ProjectController::onOpenProject (full UI path simulation)
 */
TEST_F(GUIFileLoadTest, UIPath_ViaProjectController) {
    // Connect signal to capture result
    bool signalReceived = false;
    TrussHandle receivedHandle = 0;
    QString receivedPath;

    QObject::connect(projectController.get(),
                     &ProjectController::projectOpened,
                     [&](TrussHandle handle, const QString& filepath) {
                         signalReceived = true;
                         receivedHandle = handle;
                         receivedPath = filepath;
                         qDebug() << "[SIGNAL] projectOpened - handle:" << handle
                                  << "path:" << filepath;
                     });

    QString errorMsg;
    QObject::connect(
        projectController.get(), &ProjectController::operationFailed, [&](const QString& error) {
            errorMsg = error;
            qDebug() << "[SIGNAL] operationFailed:" << error;
        });

    // Simulate UI call
    qDebug() << "[TEST] Calling ProjectController::onOpenProject()";
    projectController->onOpenProject(testFilePath);

    // Check signals
    if (!signalReceived) {
        FAIL() << "projectOpened signal not emitted. Error: " << errorMsg.toStdString();
    }

    EXPECT_NE(receivedHandle, 0) << "Invalid handle received";
    EXPECT_EQ(receivedPath, testFilePath) << "Path mismatch";

    // Verify loaded data
    const auto& view = trussService->getTrussView(receivedHandle);
    qDebug() << "[TEST] Loaded truss - nodes:" << view.getNodeCount()
             << "members:" << view.getMemberCount();

    EXPECT_EQ(view.getNodeCount(), 2);
    EXPECT_EQ(view.getMemberCount(), 1);
}

/**
 * @brief Test with absolute path (what QFileDialog returns)
 */
TEST_F(GUIFileLoadTest, AbsolutePath_FromFileDialog) {
    // QFileDialog returns absolute paths
    QString absolutePath = QFileInfo(testFilePath).absoluteFilePath();
    qDebug() << "[TEST] Absolute path:" << absolutePath;

    bool signalReceived = false;
    QObject::connect(projectController.get(),
                     &ProjectController::projectOpened,
                     [&](TrussHandle, const QString&) { signalReceived = true; });

    projectController->onOpenProject(absolutePath);

    EXPECT_TRUE(signalReceived) << "Failed to load file via absolute path";
}
