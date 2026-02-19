/**
 * @file test_AnalysisController.cpp
 * @brief Unit tests for AnalysisController with dependency injection
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include "gui/controllers/AnalysisController.hpp"
#include "gui/presenters/AnalysisResultsPresenter.hpp"
#include "gui/presenters/ValidationPresenter.hpp"
#include "mocks/MockTrussApplicationService.hpp"
#include "mocks/MockAnalysisApplicationService.hpp"

using namespace truss_controllers;
using namespace truss::application;
using namespace truss::test;
using namespace truss_presenters;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

/**
 * @brief Test fixture for AnalysisController with mock services
 */
class AnalysisControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockTrussService = std::make_unique<MockTrussApplicationService>();
        mockAnalysisService = std::make_unique<MockAnalysisApplicationService>();
        analysisPresenter = std::make_unique<AnalysisResultsPresenter>();
        validationPresenter = std::make_unique<ValidationPresenter>();
        
        controller = std::make_unique<AnalysisController>(
            mockTrussService.get(),
            mockAnalysisService.get(),
            *analysisPresenter,
            *validationPresenter
        );
    }

    void TearDown() override {
        controller.reset();
        validationPresenter.reset();
        analysisPresenter.reset();
        mockAnalysisService.reset();
        mockTrussService.reset();
    }

    std::unique_ptr<MockTrussApplicationService> mockTrussService;
    std::unique_ptr<MockAnalysisApplicationService> mockAnalysisService;
    std::unique_ptr<AnalysisResultsPresenter> analysisPresenter;
    std::unique_ptr<ValidationPresenter> validationPresenter;
    std::unique_ptr<AnalysisController> controller;
    
    const TrussHandle testHandle = 100;
    const ResultsHandle testResultsHandle = 200;
};

/**
 * @test Constructor throws on null truss service
 */
TEST_F(AnalysisControllerTest, ConstructorThrowsOnNullTrussService) {
    EXPECT_THROW({
        AnalysisController controller(
            nullptr,
            mockAnalysisService.get(),
            *analysisPresenter,
            *validationPresenter
        );
    }, std::invalid_argument);
}

/**
 * @test Constructor throws on null analysis service
 */
TEST_F(AnalysisControllerTest, ConstructorThrowsOnNullAnalysisService) {
    EXPECT_THROW({
        AnalysisController controller(
            mockTrussService.get(),
            nullptr,
            *analysisPresenter,
            *validationPresenter
        );
    }, std::invalid_argument);
}

/**
 * @test Analysis with invalid handle emits failure
 */
TEST_F(AnalysisControllerTest, AnalysisWithInvalidHandleEmitsFailure) {
    QSignalSpy failedSpy(controller.get(), &AnalysisController::analysisFailed);
    
    controller->onAnalyzeRequested(0);
    
    EXPECT_EQ(failedSpy.count(), 1);
    EXPECT_TRUE(failedSpy.at(0).at(0).toString().contains("Invalid"));
}

/**
 * @test Successful analysis workflow
 */
TEST_F(AnalysisControllerTest, SuccessfulAnalysisWorkflow) {
    // Setup validation success
    truss::core::validation::ValidationResult validResult;
    // validResult is valid by default (no issues added)
    
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Success(validResult)));
    
    // Setup mutable truss access
    static truss::core::Truss mockTruss;
    EXPECT_CALL(*mockTrussService, getTrussMutable(testHandle))
        .WillOnce(ReturnRef(mockTruss));
    
    // Setup analysis success
    EXPECT_CALL(*mockAnalysisService, analyze(_, _))
        .WillOnce(Return(Result<ResultsHandle>::Success(testResultsHandle)));
    
    QSignalSpy startedSpy(controller.get(), &AnalysisController::analysisStarted);
    QSignalSpy completedSpy(controller.get(), &AnalysisController::analysisCompleted);
    
    controller->onAnalyzeRequested(testHandle);
    
    EXPECT_EQ(startedSpy.count(), 1);
    EXPECT_EQ(completedSpy.count(), 1);
    EXPECT_EQ(completedSpy.at(0).at(0).value<ResultsHandle>(), testResultsHandle);
}

/**
 * @test Validation failure stops analysis
 */
TEST_F(AnalysisControllerTest, ValidationFailureStopsAnalysis) {
    // Setup validation failure
    truss::core::validation::ValidationResult invalidResult;
    invalidResult.addIssue(truss::core::validation::ValidationIssue(
        truss::core::validation::ValidationSeverity::Error,
        "Structure",
        "No supports defined"
    ));
    
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Success(invalidResult)));
    
    // Analysis should NOT be called
    EXPECT_CALL(*mockAnalysisService, analyze(_, _))
        .Times(0);
    
    QSignalSpy validationFailedSpy(controller.get(), &AnalysisController::validationFailed);
    QSignalSpy completedSpy(controller.get(), &AnalysisController::analysisCompleted);
    
    controller->onAnalyzeRequested(testHandle);
    
    EXPECT_EQ(validationFailedSpy.count(), 1);
    EXPECT_EQ(completedSpy.count(), 0);
}

/**
 * @test Validation service failure emits analysis failed
 */
TEST_F(AnalysisControllerTest, ValidationServiceFailureEmitsAnalysisFailed) {
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Failure("Service error")));
    
    QSignalSpy failedSpy(controller.get(), &AnalysisController::analysisFailed);
    
    controller->onAnalyzeRequested(testHandle);
    
    EXPECT_EQ(failedSpy.count(), 1);
    EXPECT_TRUE(failedSpy.at(0).at(0).toString().contains("Service error"));
}

/**
 * @test Analysis service failure emits analysis failed
 */
TEST_F(AnalysisControllerTest, AnalysisServiceFailureEmitsAnalysisFailed) {
    // Setup validation success
    truss::core::validation::ValidationResult validResult;
    // validResult is valid by default (no issues added)
    
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Success(validResult)));
    
    static truss::core::Truss mockTruss;
    EXPECT_CALL(*mockTrussService, getTrussMutable(testHandle))
        .WillOnce(ReturnRef(mockTruss));
    
    // Setup analysis failure
    EXPECT_CALL(*mockAnalysisService, analyze(_, _))
        .WillOnce(Return(Result<ResultsHandle>::Failure("Matrix singular")));
    
    QSignalSpy failedSpy(controller.get(), &AnalysisController::analysisFailed);
    
    controller->onAnalyzeRequested(testHandle);
    
    EXPECT_EQ(failedSpy.count(), 1);
    EXPECT_TRUE(failedSpy.at(0).at(0).toString().contains("Matrix singular"));
}

/**
 * @test Export without results handle fails
 */
TEST_F(AnalysisControllerTest, ExportWithoutResultsHandleFails) {
    QSignalSpy exportFailedSpy(controller.get(), &AnalysisController::exportFailed);
    
    controller->onExportRequested(0, "/path/to/output.json");
    
    EXPECT_EQ(exportFailedSpy.count(), 1);
    EXPECT_TRUE(exportFailedSpy.at(0).at(0).toString().contains("No analysis results"));
}

/**
 * @test Export requires truss handle
 */
TEST_F(AnalysisControllerTest, ExportRequiresTrussHandle) {
    QSignalSpy exportFailedSpy(controller.get(), &AnalysisController::exportFailed);
    
    // Try export without doing analysis first (no truss handle set)
    controller->onExportRequested(testResultsHandle, "/path/to/output.json");
    
    EXPECT_EQ(exportFailedSpy.count(), 1);
    EXPECT_TRUE(exportFailedSpy.at(0).at(0).toString().contains("No truss model"));
}

/**
 * @test Successful export workflow
 */
TEST_F(AnalysisControllerTest, SuccessfulExportWorkflow) {
    // First do successful analysis to set up truss handle
    truss::core::validation::ValidationResult validResult;
    // validResult is valid by default (no issues added)
    
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Success(validResult)));
    
    static truss::core::Truss mockTruss;
    EXPECT_CALL(*mockTrussService, getTrussMutable(testHandle))
        .Times(2)  // Once for analysis, once for export
        .WillRepeatedly(ReturnRef(mockTruss));
    
    EXPECT_CALL(*mockAnalysisService, analyze(_, _))
        .WillOnce(Return(Result<ResultsHandle>::Success(testResultsHandle)));
    
    controller->onAnalyzeRequested(testHandle);
    
    // Now export
    QString filepath = "/path/to/output.json";
    EXPECT_CALL(*mockAnalysisService, exportResults(testResultsHandle, _, _, _))
        .WillOnce(Return(Result<bool>::Success(true)));
    
    QSignalSpy exportCompletedSpy(controller.get(), &AnalysisController::exportCompleted);
    
    controller->onExportRequested(testResultsHandle, filepath);
    
    EXPECT_EQ(exportCompletedSpy.count(), 1);
    EXPECT_EQ(exportCompletedSpy.at(0).at(0).toString(), filepath);
}

/**
 * @test Export failure emits export failed
 */
TEST_F(AnalysisControllerTest, ExportFailureEmitsExportFailed) {
    // Setup analysis first
    truss::core::validation::ValidationResult validResult;
    // validResult is valid by default (no issues added)
    
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Success(validResult)));
    
    static truss::core::Truss mockTruss;
    EXPECT_CALL(*mockTrussService, getTrussMutable(testHandle))
        .Times(2)
        .WillRepeatedly(ReturnRef(mockTruss));
    
    EXPECT_CALL(*mockAnalysisService, analyze(_, _))
        .WillOnce(Return(Result<ResultsHandle>::Success(testResultsHandle)));
    
    controller->onAnalyzeRequested(testHandle);
    
    // Export failure
    EXPECT_CALL(*mockAnalysisService, exportResults(testResultsHandle, _, _, _))
        .WillOnce(Return(Result<bool>::Failure("Write permission denied")));
    
    QSignalSpy exportFailedSpy(controller.get(), &AnalysisController::exportFailed);
    
    controller->onExportRequested(testResultsHandle, "/path/to/output.json");
    
    EXPECT_EQ(exportFailedSpy.count(), 1);
    EXPECT_TRUE(exportFailedSpy.at(0).at(0).toString().contains("Write permission denied"));
}

/**
 * @test Signal emissions for status updates
 */
TEST_F(AnalysisControllerTest, SignalEmissionsForStatusUpdates) {
    truss::core::validation::ValidationResult validResult;
    // validResult is valid by default (no issues added)
    
    EXPECT_CALL(*mockTrussService, validateTruss(testHandle))
        .WillOnce(Return(Result<truss::core::validation::ValidationResult>::Success(validResult)));
    
    static truss::core::Truss mockTruss;
    EXPECT_CALL(*mockTrussService, getTrussMutable(testHandle))
        .WillOnce(ReturnRef(mockTruss));
    
    EXPECT_CALL(*mockAnalysisService, analyze(_, _))
        .WillOnce(Return(Result<ResultsHandle>::Success(testResultsHandle)));
    
    QSignalSpy statusSpy(controller.get(), &AnalysisController::statusMessageChanged);
    
    controller->onAnalyzeRequested(testHandle);
    
    // Should have multiple status updates
    EXPECT_GE(statusSpy.count(), 2);  // At least "Validating..." and "Running analysis..."
}

