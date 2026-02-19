/**
 * @file test_ProjectController.cpp
 * @brief Unit tests for ProjectController with dependency injection
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <filesystem>
#include <fstream>
#include "gui/controllers/ProjectController.hpp"
#include "mocks/MockTrussApplicationService.hpp"

using namespace truss_controllers;
using namespace truss::application;
using namespace truss::test;
using ::testing::_;
using ::testing::Return;

/**
 * @brief Test fixture for ProjectController with mock service
 */
class ProjectControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockService = std::make_unique<MockTrussApplicationService>();
        controller = std::make_unique<ProjectController>(mockService.get());
    }

    void TearDown() override {
        controller.reset();
        mockService.reset();
    }

    std::unique_ptr<MockTrussApplicationService> mockService;
    std::unique_ptr<ProjectController> controller;
    
    // Helper: Create a temporary test file
    std::filesystem::path createTempFile() {
        auto path = std::filesystem::temp_directory_path() / "test_project.json";
        std::ofstream file(path);
        file << R"({"nodes":[],"members":[]})";
        file.close();
        return path;
    }
};

/**
 * @test Constructor validation
 */
TEST_F(ProjectControllerTest, ConstructorThrowsOnNullService) {
    EXPECT_THROW({
        ProjectController controller(nullptr);
    }, std::invalid_argument);
}

/**
 * @test New project creates truss and emits signal (no unsaved changes)
 */
TEST_F(ProjectControllerTest, NewProjectCreatesEmptyTruss) {
    const TrussHandle expectedHandle = 42;
    
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(Result<TrussHandle>::Success(expectedHandle)));
    
    QSignalSpy createdSpy(controller.get(), &ProjectController::projectCreated);
    QSignalSpy statusSpy(controller.get(), &ProjectController::statusMessageChanged);
    
    controller->onNewProject();
    
    EXPECT_EQ(createdSpy.count(), 1);
    EXPECT_EQ(createdSpy.at(0).at(0).value<TrussHandle>(), expectedHandle);
    EXPECT_EQ(controller->getCurrentTruss(), expectedHandle);
    EXPECT_TRUE(controller->getCurrentFilepath().isEmpty());
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test New project with unsaved changes triggers confirmation request
 */
TEST_F(ProjectControllerTest, NewProjectWithUnsavedChangesRequestsConfirmation) {
    controller->markAsModified();
    
    QSignalSpy confirmSpy(controller.get(), &ProjectController::unsavedChangesConfirmationRequested);
    QSignalSpy createdSpy(controller.get(), &ProjectController::projectCreated);
    
    controller->onNewProject();
    
    EXPECT_EQ(confirmSpy.count(), 1);
    EXPECT_EQ(createdSpy.count(), 0); // Should not create new project
}

/**
 * @test New project failure emits error signal
 */
TEST_F(ProjectControllerTest, NewProjectFailureEmitsError) {
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(Result<TrussHandle>::Failure("Failed to create truss")));
    
    QSignalSpy errorSpy(controller.get(), &ProjectController::operationFailed);
    
    controller->onNewProject();
    
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.at(0).at(0).toString().contains("Failed"));
}

/**
 * @test Open project loads file successfully
 */
TEST_F(ProjectControllerTest, OpenProjectLoadsFile) {
    auto tempFile = createTempFile();
    QString filepath = QString::fromStdString(tempFile.string());
    const TrussHandle expectedHandle = 100;
    
    EXPECT_CALL(*mockService, loadTruss(std::filesystem::path(tempFile)))
        .WillOnce(Return(Result<TrussHandle>::Success(expectedHandle)));
    
    QSignalSpy openedSpy(controller.get(), &ProjectController::projectOpened);
    
    controller->onOpenProject(filepath);
    
    EXPECT_EQ(openedSpy.count(), 1);
    EXPECT_EQ(openedSpy.at(0).at(0).value<TrussHandle>(), expectedHandle);
    EXPECT_EQ(controller->getCurrentTruss(), expectedHandle);
    EXPECT_EQ(controller->getCurrentFilepath(), filepath);
    
    std::filesystem::remove(tempFile);
}

/**
 * @test Open project with unsaved changes requests confirmation
 */
TEST_F(ProjectControllerTest, OpenProjectWithUnsavedChangesRequestsConfirmation) {
    controller->markAsModified();
    
    QSignalSpy confirmSpy(controller.get(), &ProjectController::unsavedChangesConfirmationRequested);
    
    controller->onOpenProject("/some/path.json");
    
    EXPECT_EQ(confirmSpy.count(), 1);
}

/**
 * @test Open non-existent file emits error
 */
TEST_F(ProjectControllerTest, OpenNonExistentFileEmitsError) {
    QSignalSpy errorSpy(controller.get(), &ProjectController::operationFailed);
    
    controller->onOpenProject("/nonexistent/file.json");
    
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.at(0).at(0).toString().contains("not found"));
}

/**
 * @test Save project without filepath requests Save As
 */
TEST_F(ProjectControllerTest, SaveWithoutFilepathRequestsSaveAs) {
    // Create a project first
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(Result<TrussHandle>::Success(1)));
    controller->onNewProject();
    
    QSignalSpy saveAsSpy(controller.get(), &ProjectController::saveAsRequested);
    
    controller->onSaveProject();
    
    EXPECT_EQ(saveAsSpy.count(), 1);
}

/**
 * @test Save project with filepath succeeds
 */
TEST_F(ProjectControllerTest, SaveProjectWithFilepath) {
    auto tempFile = createTempFile();
    QString filepath = QString::fromStdString(tempFile.string());
    const TrussHandle handle = 50;
    
    // First open a project
    EXPECT_CALL(*mockService, loadTruss(_))
        .WillOnce(Return(Result<TrussHandle>::Success(handle)));
    controller->onOpenProject(filepath);
    
    // Mark as modified and save
    controller->markAsModified();
    
    EXPECT_CALL(*mockService, saveTruss(handle, std::filesystem::path(tempFile), _))
        .WillOnce(Return(Result<bool>::Success(true)));
    
    QSignalSpy savedSpy(controller.get(), &ProjectController::projectSaved);
    
    controller->onSaveProject();
    
    EXPECT_EQ(savedSpy.count(), 1);
    EXPECT_FALSE(controller->hasUnsavedChanges());
    
    std::filesystem::remove(tempFile);
}

/**
 * @test Save As updates filepath
 */
TEST_F(ProjectControllerTest, SaveAsUpdatesFilepath) {
    const TrussHandle handle = 200;
    auto tempFile = createTempFile();
    QString newFilepath = QString::fromStdString(tempFile.string());
    
    // Create new project
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(Result<TrussHandle>::Success(handle)));
    controller->onNewProject();
    
    controller->markAsModified();
    
    EXPECT_CALL(*mockService, saveTruss(handle, std::filesystem::path(tempFile), _))
        .WillOnce(Return(Result<bool>::Success(true)));
    
    QSignalSpy savedSpy(controller.get(), &ProjectController::projectSaved);
    
    controller->onSaveProjectAs(newFilepath);
    
    EXPECT_EQ(savedSpy.count(), 1);
    EXPECT_EQ(controller->getCurrentFilepath(), newFilepath);
    EXPECT_FALSE(controller->hasUnsavedChanges());
    
    std::filesystem::remove(tempFile);
}

/**
 * @test Unsaved changes tracking
 */
TEST_F(ProjectControllerTest, UnsavedChangesTracking) {
    EXPECT_FALSE(controller->hasUnsavedChanges());
    
    controller->markAsModified();
    EXPECT_TRUE(controller->hasUnsavedChanges());
    
    controller->markAsSaved();
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test Close project without unsaved changes
 */
TEST_F(ProjectControllerTest, CloseProjectWithoutUnsavedChanges) {
    QSignalSpy closedSpy(controller.get(), &ProjectController::projectClosed);
    
    controller->onCloseProject();
    
    EXPECT_EQ(closedSpy.count(), 1);
}

/**
 * @test Close project with unsaved changes requests confirmation
 */
TEST_F(ProjectControllerTest, CloseProjectWithUnsavedChangesRequestsConfirmation) {
    controller->markAsModified();
    
    QSignalSpy confirmSpy(controller.get(), &ProjectController::unsavedChangesConfirmationRequested);
    QSignalSpy closedSpy(controller.get(), &ProjectController::projectClosed);
    
    controller->onCloseProject();
    
    EXPECT_EQ(confirmSpy.count(), 1);
    EXPECT_EQ(closedSpy.count(), 0);
}

/**
 * @test Save failure emits error
 */
TEST_F(ProjectControllerTest, SaveFailureEmitsError) {
    auto tempFile = createTempFile();
    QString filepath = QString::fromStdString(tempFile.string());
    const TrussHandle handle = 300;
    
    EXPECT_CALL(*mockService, loadTruss(_))
        .WillOnce(Return(Result<TrussHandle>::Success(handle)));
    controller->onOpenProject(filepath);
    
    EXPECT_CALL(*mockService, saveTruss(handle, _, _))
        .WillOnce(Return(Result<bool>::Failure("Write error")));
    
    QSignalSpy errorSpy(controller.get(), &ProjectController::operationFailed);
    
    controller->onSaveProject();
    
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.at(0).at(0).toString().contains("Write error"));
    
    std::filesystem::remove(tempFile);
}

