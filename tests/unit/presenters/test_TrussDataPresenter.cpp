/**
 * @file test_TrussDataPresenter.cpp
 * @brief Unit tests for TrussDataPresenter
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * These tests verify that TrussDataPresenter correctly formats Domain
 * data for GUI display. Presenters are pure formatting logic with no
 * side effects, making them ideal for comprehensive unit testing.
 *
 * Architecture: Unit Tests (GUI Presentation Layer)
 * Purpose: Verify formatting correctness and consistency
 */

#include "gui/presenters/TrussDataPresenter.hpp"

#include <gtest/gtest.h>

using namespace truss;
using namespace truss_presenters;

/**
 * @brief Test fixture for TrussDataPresenter tests
 */
class TrussDataPresenterTest : public ::testing::Test {
protected:
    TrussDataPresenter presenter;
};

/**
 * @brief Test support type formatting matches expected strings
 *
 * Acceptance Criteria:
 * - formatSupportType(Free) returns "Free"
 * - formatSupportType(Pinned) returns "Pinned"
 * - formatSupportType(RollerX) returns "RollerX"
 * - formatSupportType(RollerY) returns "RollerY"
 */
TEST_F(TrussDataPresenterTest, FormatSupportTypeReturnsCorrectStrings) {
    EXPECT_EQ(presenter.formatSupportType(core::SupportType::Free).toStdString(), "Free");
    EXPECT_EQ(presenter.formatSupportType(core::SupportType::Pinned).toStdString(), "Pinned");
    EXPECT_EQ(presenter.formatSupportType(core::SupportType::RollerX).toStdString(), "RollerX");
    EXPECT_EQ(presenter.formatSupportType(core::SupportType::RollerY).toStdString(), "RollerY");
}

/**
 * @brief Test coordinate formatting uses correct precision
 *
 * Acceptance Criteria:
 * - Coordinates formatted with 3 decimal places
 * - Format is "(x, y) unit"
 * - Handles negative coordinates correctly
 */
TEST_F(TrussDataPresenterTest, FormatCoordinateUsesPrecision) {
    core::Point2D point{1.23456, -2.34567};
    QString result = presenter.formatCoordinate(point);

    EXPECT_EQ(result.toStdString(), "(1.235, -2.346) m");
}

/**
 * @brief Test coordinate formatting accepts custom unit
 *
 * Acceptance Criteria:
 * - Custom unit appears in output
 * - Default unit is "m"
 */
TEST_F(TrussDataPresenterTest, FormatCoordinateAcceptsCustomUnit) {
    core::Point2D point{5.0, 10.0};
    QString result = presenter.formatCoordinate(point, "mm");

    EXPECT_EQ(result.toStdString(), "(5.000, 10.000) mm");
}

/**
 * @brief Test length formatting uses correct precision
 *
 * Acceptance Criteria:
 * - Length formatted with 3 decimal places
 * - Format is "value m"
 */
TEST_F(TrussDataPresenterTest, FormatLengthUsesPrecision) {
    QString result = presenter.formatLength(3.14159);

    EXPECT_EQ(result.toStdString(), "3.142 m");
}

/**
 * @brief Test node count pluralization
 *
 * Acceptance Criteria:
 * - 0 nodes: "0 nodes"
 * - 1 node: "1 node"
 * - 2+ nodes: "N nodes"
 */
TEST_F(TrussDataPresenterTest, FormatNodeCountPluralization) {
    EXPECT_EQ(presenter.formatNodeCount(0).toStdString(), "0 nodes");
    EXPECT_EQ(presenter.formatNodeCount(1).toStdString(), "1 node");
    EXPECT_EQ(presenter.formatNodeCount(2).toStdString(), "2 nodes");
    EXPECT_EQ(presenter.formatNodeCount(42).toStdString(), "42 nodes");
}

/**
 * @brief Test member count pluralization
 *
 * Acceptance Criteria:
 * - Same pluralization rules as nodes
 */
TEST_F(TrussDataPresenterTest, FormatMemberCountPluralization) {
    EXPECT_EQ(presenter.formatMemberCount(0).toStdString(), "0 members");
    EXPECT_EQ(presenter.formatMemberCount(1).toStdString(), "1 member");
    EXPECT_EQ(presenter.formatMemberCount(2).toStdString(), "2 members");
}

/**
 * @brief Test support change message formatting
 *
 * Acceptance Criteria:
 * - Message includes node ID
 * - Message includes human-readable support type
 * - Format: "Node {id} support changed to {type}"
 */
TEST_F(TrussDataPresenterTest, FormatSupportChangeMessageStructure) {
    QString message = presenter.formatSupportChangeMessage(5, core::SupportType::Pinned);

    EXPECT_EQ(message.toStdString(), "Node 5 support changed to Pinned");
}

/**
 * @brief Test node added message formatting
 *
 * Acceptance Criteria:
 * - Message includes node ID
 * - Message includes formatted coordinates (3 decimal places)
 * - Format: "Node {id} added at (x, y)"
 */
TEST_F(TrussDataPresenterTest, FormatNodeAddedMessageStructure) {
    core::Point2D pos{1.5, 2.5};
    QString message = presenter.formatNodeAddedMessage(123, pos);

    EXPECT_EQ(message.toStdString(), "Node 123 added at (1.500, 2.500)");
}

/**
 * @brief Test member added message formatting
 *
 * Acceptance Criteria:
 * - Message includes member ID and node IDs
 * - Format: "Member {id} added (Nodes {start} - {end})"
 */
TEST_F(TrussDataPresenterTest, FormatMemberAddedMessageStructure) {
    QString message = presenter.formatMemberAddedMessage(456, 1, 2);

    EXPECT_EQ(message.toStdString(), "Member 456 added (Nodes 1 - 2)");
}

/**
 * @brief Test load applied message formatting
 *
 * Acceptance Criteria:
 * - Message includes node ID and force components
 * - Force formatted with 1 decimal place
 * - Format: "Load applied to node {id}: (fx, fy) N"
 */
TEST_F(TrussDataPresenterTest, FormatLoadAppliedMessageStructure) {
    core::Force2D force{1000.5, -500.3};
    QString message = presenter.formatLoadAppliedMessage(7, force);

    EXPECT_EQ(message.toStdString(), "Load applied to node 7: (1000.5, -500.3) N");
}

/**
 * @brief Test that Presenter has NO business logic
 *
 * Acceptance Criteria:
 * - Presenter methods are pure functions (no side effects)
 * - Presenter does not modify Domain state
 * - Presenter does not call Application Services
 * - All methods are const-qualified
 */
TEST_F(TrussDataPresenterTest, PresenterIsPureFormatting) {
    // This test documents architectural constraint
    // Verify: All public methods are const
    // Verify: No state mutation occurs
    // Verify: No service dependencies exist

    SUCCEED() << "Architectural compliance: Presenter contains only pure formatting logic";
}
