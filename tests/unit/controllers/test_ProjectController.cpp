/**
 * @file test_ProjectController.cpp
 * @brief Unit tests for ProjectController with dependency injection
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include "gui/controllers/ProjectController.hpp"
#include "mocks/MockTrussApplicationService.hpp"

using namespace truss_controllers;
using namespace truss::application;
using namespace truss::test;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

/**
 * @brief Test fixture for ProjectController with mock service
 */
class ProjectControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockService = std::make_unique<NiceMock<MockTrussApplicationService>>();
        controller = std::make_unique<ProjectController>(mockService.get());
    }

    void TearDown() override {
        controller.reset();
        mockService.reset();
    }

    std::unique_ptr<NiceMock<MockTrussApplicationService>> mockService;
    std::unique_ptr<ProjectController> controller;
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
 * @test New project creates truss and emits signal
 */
TEST_F(ProjectControllerTest, NewProjectDelegatesCorrectly) {
    // Arrange
    const TrussHandle expectedHandle = 42;
    Result<TrussHandle> successResult(expectedHandle);
    
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(successResult));
    
    QSignalSpy projectCreatedSpy(controller.get(), &ProjectController::projectCreated);
    QSignalSpy statusSpy(controller.get(), &ProjectController::statusMessageChanged);
    
    // Act
    controller->onNewProject();
    
    // Assert
    EXPECT_EQ(projectCreatedSpy.count(), 1);
    EXPECT_EQ(projectCreatedSpy.takeFirst().at(0).value<TrussHandle>(), expectedHandle);
    
    EXPECT_EQ(controller->getCurrentTruss(), expectedHandle);
    EXPECT_TRUE(controller->getCurrentFilepath().isEmpty());
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test New project failure triggers error signal
 */
TEST_F(ProjectControllerTest, NewProjectFailureTriggersError) {
    // Arrange
    Result<TrussHandle> failureResult("Failed to create truss");
    
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(failureResult));
    
    QSignalSpy errorSpy(controller.get(), &ProjectController::operationFailed);
    QSignalSpy createdSpy(controller.get(), &ProjectController::projectCreated);
    
    // Act
    controller->onNewProject();
    
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.takeFirst().at(0).toString().contains("create"));
    
    EXPECT_EQ(createdSpy.count(), 0);
}

/**
 * @test Open project loads file and emits signal
 */
TEST_F(ProjectControllerTest, OpenProjectDelegatesCorrectly) {
    // Arrange
    const QString filepath = "/path/to/project.json";
    const TrussHandle expectedHandle = 123;
    Result<TrussHandle> successResult(expectedHandle);
    
    EXPECT_CALL(*mockService, loadTruss(std::filesystem::path(filepath.toStdString())))
        .WillOnce(Return(successResult));
    
    QSignalSpy openedSpy(controller.get(), &ProjectController::projectOpened);
    
    // Act
    controller->onOpenProject(filepath);
    
    // Assert
    EXPECT_EQ(openedSpy.count(), 1);
    QList<QVariant> arguments = openedSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<TrussHandle>(), expectedHandle);
    EXPECT_EQ(arguments.at(1).toString(), filepath);
    
    EXPECT_EQ(controller->getCurrentTruss(), expectedHandle);
    EXPECT_EQ(controller->getCurrentFilepath(), filepath);
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test Open project failure triggers error signal
 */
TEST_F(ProjectControllerTest, OpenProjectFailureTriggersError) {
    // Arrange
    const QString filepath = "/invalid/path.json";
    Result<TrussHandle> failureResult("File not found");
    
    EXPECT_CALL(*mockService, loadTruss(std::filesystem::path(filepath.toStdString())))
        .WillOnce(Return(failureResult));
    
    QSignalSpy errorSpy(controller.get(), &ProjectController::operationFailed);
    QSignalSpy openedSpy(controller.get(), &ProjectController::projectOpened);
    
    // Act
    controller->onOpenProject(filepath);
    
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.takeFirst().at(0).toString().contains("File not found"));
    
    EXPECT_EQ(openedSpy.count(), 0);
}

/**
 * @test Save project with filepath delegates correctly
 */
TEST_F(ProjectControllerTest, SaveProjectDelegatesCorrectly) {
    // Arrange - First open a project
    const QString filepath = "/path/to/project.json";
    const TrussHandle handle = 100;
    
    EXPECT_CALL(*mockService, loadTruss(std::filesystem::path(filepath.toStdString())))
        .WillOnce(Return(Result<TrussHandle>(handle)));
    controller->onOpenProject(filepath);
    
    // Mark as modified
    controller->markAsModified();
    EXPECT_TRUE(controller->hasUnsavedChanges());
    
    // Setup save expectation
    EXPECT_CALL(*mockService, saveTruss(handle, std::filesystem::path(filepath.toStdString()), _))
        .WillOnce(Return(Result<bool>(true)));
    
    QSignalSpy savedSpy(controller.get(), &ProjectController::projectSaved);
    
    // Act
    controller->onSaveProject();
    
    // Assert
    EXPECT_EQ(savedSpy.count(), 1);
    EXPECT_EQ(savedSpy.takeFirst().at(0).toString(), filepath);
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test Save without filepath requests Save As
 */
TEST_F(ProjectControllerTest, SaveWithoutFilepathRequestsSaveAs) {
    // Arrange - Create new project (no filepath)
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(Result<TrussHandle>(1)));
    controller->onNewProject();
    
    QSignalSpy saveAsSpy(controller.get(), &ProjectController::saveAsRequested);
    
    // Act
    controller->onSaveProject();
    
    // Assert
    EXPECT_EQ(saveAsSpy.count(), 1);
}

/**
 * @test Save As updates filepath
 */
TEST_F(ProjectControllerTest, SaveAsUpdatesFilepath) {
    // Arrange - Create new project
    const TrussHandle handle = 200;
    EXPECT_CALL(*mockService, createTruss(_))
        .WillOnce(Return(Result<TrussHandle>(handle)));
    controller->onNewProject();
    
    controller->markAsModified();
    
    const QString newFilepath = "/new/path/project.json";
    EXPECT_CALL(*mockService, saveTruss(handle, std::filesystem::path(newFilepath.toStdString()), _))
        .WillOnce(Return(Result<bool>(true)));
    
    QSignalSpy savedSpy(controller.get(), &ProjectController::projectSaved);
    
    // Act
    controller->onSaveProjectAs(newFilepath);
    
    // Assert
    EXPECT_EQ(savedSpy.count(), 1);
    EXPECT_EQ(controller->getCurrentFilepath(), newFilepath);
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test Unsaved changes tracking
 */
TEST_F(ProjectControllerTest, UnsavedChangesTracking) {
    // Initial state
    EXPECT_FALSE(controller->hasUnsavedChanges());
    
    // Mark as modified
    controller->markAsModified();
    EXPECT_TRUE(controller->hasUnsavedChanges());
    
    // Mark as saved
    controller->markAsSaved();
    EXPECT_FALSE(controller->hasUnsavedChanges());
}

/**
 * @test Close project emits signal
 */
TEST_F(ProjectControllerTest, CloseProjectEmitsSignal) {
    // Arrange
    QSignalSpy closedSpy(controller.get(), &ProjectController::projectClosed);
    
    // Act
    controller->onCloseProject();
    
    // Assert
    EXPECT_EQ(closedSpy.count(), 1);
}

/**
 * @test Save failure triggers error signal
 */
TEST_F(ProjectControllerTest, SaveFailureTriggersError) {
    // Arrange - Open project
    const QString filepath = "/path/to/project.json";
    const TrussHandle handle = 300;
    
    EXPECT_CALL(*mockService, loadTruss(std::filesystem::path(filepath.toStdString())))
        .WillOnce(Return(Result<TrussHandle>(handle)));
    controller->onOpenProject(filepath);
    
    // Setup save failure
    EXPECT_CALL(*mockService, saveTruss(handle, std::filesystem::path(filepath.toStdString()), _))
        .WillOnce(Return(Result<bool>("Disk write error")));
    
    QSignalSpy errorSpy(controller.get(), &ProjectController::operationFailed);
    QSignalSpy savedSpy(controller.get(), &ProjectController::projectSaved);
    
    // Act
    controller->onSaveProject();
    
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.takeFirst().at(0).toString().contains("write error"));
    EXPECT_EQ(savedSpy.count(), 0);
}

