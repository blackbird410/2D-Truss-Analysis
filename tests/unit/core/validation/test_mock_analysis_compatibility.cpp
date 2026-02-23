/**
 * @file test_mock_analysis_compatibility.cpp
 * @brief Validates MockAnalysisApplicationService interface compliance
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "gui/controllers/analysis_controller.hpp"
#include "gui/presenters/analysis_results_presenter.hpp"
#include "gui/presenters/validation_presenter.hpp"
#include "mocks/MockAnalysisApplicationService.hpp"
#include "mocks/MockTrussApplicationService.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <type_traits>

/**
 * @brief Test suite for MockAnalysisApplicationService compatibility
 */
class MockAnalysisCompatibilityTest : public ::testing::Test {};

/**
 * @test Verify MockAnalysisApplicationService inherits from IAnalysisService
 */
TEST_F(MockAnalysisCompatibilityTest, MockInheritsFromIAnalysisService) {
    // Compile-time check
    static_assert(
        std::is_base_of_v<truss::application::IAnalysisService, MockAnalysisApplicationService>,
        "MockAnalysisApplicationService must inherit from IAnalysisService");

    // Runtime verification
    MockAnalysisApplicationService mock;
    truss::application::IAnalysisService* service = &mock;

    EXPECT_NE(service, nullptr);
}

/**
 * @test Verify AnalysisController accepts mock pointer
 */
TEST_F(MockAnalysisCompatibilityTest, ControllerAcceptsMockPointer) {
    truss::test::MockTrussApplicationService mockTrussService;
    MockAnalysisApplicationService mockAnalysisService;
    truss_presenters::AnalysisResultsPresenter analysisPresenter;
    truss_presenters::ValidationPresenter validationPresenter;

    // Should compile and construct without errors
    EXPECT_NO_THROW({
        truss_controllers::AnalysisController controller(
            &mockTrussService, &mockAnalysisService, analysisPresenter, validationPresenter);
    });
}

/**
 * @test Verify all interface methods are overridden
 *
 * This test doesn't need to execute - if it compiles, all pure virtual
 * methods from IAnalysisService have been implemented.
 */
TEST_F(MockAnalysisCompatibilityTest, AllInterfaceMethodsAreOverridden) {
    MockAnalysisApplicationService mock;

    // If this compiles, all pure virtuals are implemented
    [[maybe_unused]] truss::application::IAnalysisService* service = &mock;

    SUCCEED();
}
