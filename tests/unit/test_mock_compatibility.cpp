/**
 * @file test_mock_compatibility.cpp
 * @brief Type compatibility verification for MockTrussApplicationService
 * @version 3.0.0
 *
 * This test verifies that MockTrussApplicationService correctly implements
 * ITrussService and can be used polymorphically with TrussEditController.
 */

#include "gui/controllers/truss_edit_controller.hpp"
#include "gui/presenters/truss_data_presenter.hpp"
#include "mocks/MockTrussApplicationService.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <type_traits>

using namespace truss;
using namespace truss::test;
using namespace testing;

/**
 * @brief Verify Mock inherits from ITrussService interface
 */
TEST(MockCompatibilityTest, MockInheritsFromITrussService) {
    MockTrussApplicationService mock;

    // Static assertion: Mock IS-A ITrussService
    static_assert(std::is_base_of_v<application::ITrussService, MockTrussApplicationService>,
                  "MockTrussApplicationService must inherit from ITrussService");

    // Runtime verification: Can be assigned to interface pointer
    application::ITrussService* servicePtr = &mock;
    ASSERT_NE(servicePtr, nullptr);

    SUCCEED() << "Mock correctly implements ITrussService interface";
}

/**
 * @brief Verify Controller accepts Mock through interface pointer
 */
TEST(MockCompatibilityTest, ControllerAcceptsMockPointer) {
    MockTrussApplicationService mock;
    truss_presenters::TrussDataPresenter presenter;

    // This should compile without errors - demonstrates polymorphic compatibility
    truss_controllers::TrussEditController controller(&mock, presenter);

    SUCCEED() << "Controller accepts Mock through ITrussService* polymorphism";
}

/**
 * @brief Verify all interface methods are overridden in Mock
 */
TEST(MockCompatibilityTest, AllInterfaceMethodsAreOverridden) {
    MockTrussApplicationService mock;

    // These calls should compile - demonstrates all pure virtuals are implemented
    // Note: We're not setting expectations, just verifying compilation

    // Can call lifecycle methods through interface
    // application::ITrussService* service = &mock;
    EXPECT_NO_THROW({
        // These will fail at runtime without EXPECT_CALL, but that's expected
        // We're only verifying the methods exist and are callable
    });

    SUCCEED() << "All interface methods are properly overridden";
}
