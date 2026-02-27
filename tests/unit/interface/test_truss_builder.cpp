/**
 * @file test_truss_builder.cpp
 * @brief Unit tests for TrussBuilder interface class
 * @version 1.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 *
 * Comprehensive test suite for the TrussBuilder fluent API, covering:
 * - Node addition with automatic ID assignment
 * - Member addition with default and custom materials/sections
 * - Force application and support modification
 * - Referential integrity validation
 * - Builder lifecycle (build, reset, reuse)
 * - Error handling and validation
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "interface/truss_builder.hpp"
#include "core/model/truss.hpp"
#include "application/truss_edit_dtos.hpp"

using namespace truss;
using namespace truss::interface;
using namespace truss::core;
using namespace truss::application;

namespace {

/**
 * @brief Test fixture for TrussBuilder tests
 */
class TrussBuilderTest : public ::testing::Test {
protected:
    TrussBuilder builder;

    void SetUp() override {
        // Reset builder before each test
        builder.reset();
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// =============================================================================
// NODE ADDITION TESTS
// =============================================================================

/**
 * @test TrussBuilder can add a single node
 */
TEST_F(TrussBuilderTest, AddSingleNode) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);

    EXPECT_EQ(builder.getNodeCount(), 1);
    EXPECT_EQ(builder.getNextNodeId(), NodeId(2));
}

/**
 * @test TrussBuilder assigns sequential node IDs starting from 1
 */
TEST_F(TrussBuilderTest, NodeIdAssignmentSequential) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Free);
    builder.addNode(0.5, 1.0, SupportType::Free);

    EXPECT_EQ(builder.getNodeCount(), 3);
    EXPECT_EQ(builder.getNextNodeId(), NodeId(4));
}

/**
 * @test TrussBuilder allows adding multiple nodes with different support types
 */
TEST_F(TrussBuilderTest, AddMultipleNodesWithDifferentSupports) {
    builder
        .addNode(0.0, 0.0, SupportType::Pinned)
        .addNode(1.0, 0.0, SupportType::Pinned)
        .addNode(2.0, 0.0, SupportType::RollerY)
        .addNode(1.0, 1.0, SupportType::Free);

    EXPECT_EQ(builder.getNodeCount(), 4);
}

/**
 * @test TrussBuilder fluent API chains correctly
 */
TEST_F(TrussBuilderTest, FluentApiChaining) {
    TrussBuilder& result = builder.addNode(0.0, 0.0, SupportType::Pinned);
    EXPECT_EQ(&result, &builder);  // Should return reference to self
}

// =============================================================================
// MEMBER ADDITION TESTS
// =============================================================================

/**
 * @test TrussBuilder adds member with default material and section
 */
TEST_F(TrussBuilderTest, AddMemberWithDefaults) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    builder.addMember(NodeId(1), NodeId(2));
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getMemberCount(), 1);
}

/**
 * @test TrussBuilder adds member with custom material
 */
TEST_F(TrussBuilderTest, AddMemberWithCustomMaterial) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    MaterialSpec aluminum = MaterialSpec::Aluminum();
    SectionSpec section = SectionSpec::Circular(0.01);
    builder.addMember(NodeId(1), NodeId(2), aluminum, section);
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getMemberCount(), 1);
}

/**
 * @test TrussBuilder adds member with custom material and section
 */
TEST_F(TrussBuilderTest, AddMemberWithCustomMaterialAndSection) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    MaterialSpec concrete = MaterialSpec::Concrete();
    SectionSpec square = SectionSpec::Square(0.1);  // 10cm x 10cm
    
    builder.addMember(NodeId(1), NodeId(2), concrete, square);
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getMemberCount(), 1);
}

/**
 * @test TrussBuilder allows adding multiple members
 */
TEST_F(TrussBuilderTest, AddMultipleMembers) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    builder.addNode(0.5, 1.0, SupportType::Free);
    
    builder.addMember(NodeId(1), NodeId(2))
           .addMember(NodeId(2), NodeId(3))
           .addMember(NodeId(3), NodeId(1));
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getMemberCount(), 3);
}

// =============================================================================
// REFERENTIAL INTEGRITY VALIDATION TESTS
// =============================================================================

/**
 * @test TrussBuilder throws when adding member with invalid start node
 */
TEST_F(TrussBuilderTest, AddMemberInvalidStartNode) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(
        builder.addMember(NodeId(999), NodeId(1)),
        std::invalid_argument
    );
}

/**
 * @test TrussBuilder throws when adding member with invalid end node
 */
TEST_F(TrussBuilderTest, AddMemberInvalidEndNode) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(
        builder.addMember(NodeId(1), NodeId(999)),
        std::invalid_argument
    );
}

/**
 * @test TrussBuilder throws when adding member before any nodes
 */
TEST_F(TrussBuilderTest, AddMemberBeforeNodes) {
    EXPECT_THROW(
        builder.addMember(NodeId(1), NodeId(2)),
        std::invalid_argument
    );
}

/**
 * @test TrussBuilder throws when creating self-loop member
 */
TEST_F(TrussBuilderTest, AddMemberSelfLoop) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(
        builder.addMember(NodeId(1), NodeId(1)),
        std::invalid_argument
    );
}

// =============================================================================
// FORCE APPLICATION TESTS
// =============================================================================

/**
 * @test TrussBuilder applies force to node
 */
TEST_F(TrussBuilderTest, ApplyForceToNode) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    builder.addMember(NodeId(1), NodeId(2))
           .applyForce(NodeId(2), 1000.0, -500.0);
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
}

/**
 * @test TrussBuilder applies multiple forces to different nodes
 */
TEST_F(TrussBuilderTest, ApplyMultipleForces) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Free);
    builder.addNode(0.5, 1.0, SupportType::Free);
    
    builder.addMember(NodeId(1), NodeId(2))
           .addMember(NodeId(2), NodeId(3))
           .applyForce(NodeId(2), 1000.0, 0.0)
           .applyForce(NodeId(3), 0.0, -1500.0);
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
}

/**
 * @test TrussBuilder throws when applying force to invalid node
 */
TEST_F(TrussBuilderTest, ApplyForceInvalidNode) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(
        builder.applyForce(NodeId(999), 100.0, 0.0),
        std::invalid_argument
    );
}

// =============================================================================
// SUPPORT MODIFICATION TESTS
// =============================================================================

/**
 * @test TrussBuilder modifies support type of existing node
 */
TEST_F(TrussBuilderTest, SetSupportType) {
    builder.addNode(0.0, 0.0, SupportType::Free);
    builder.addNode(1.0, 0.0, SupportType::Free);
    
    builder.setSupport(NodeId(1), SupportType::Pinned)
           .setSupport(NodeId(2), SupportType::RollerY)
           .addMember(NodeId(1), NodeId(2));
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
}

/**
 * @test TrussBuilder throws when setting support for invalid node
 */
TEST_F(TrussBuilderTest, SetSupportInvalidNode) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(
        builder.setSupport(NodeId(999), SupportType::Pinned),
        std::invalid_argument
    );
}

// =============================================================================
// BUILD VALIDATION TESTS
// =============================================================================

/**
 * @test TrussBuilder throws when building with insufficient nodes
 */
TEST_F(TrussBuilderTest, BuildInsufficientNodes) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(builder.build(), std::runtime_error);
}

/**
 * @test TrussBuilder throws when building with no members
 */
TEST_F(TrussBuilderTest, BuildNoMembers) {
    builder.addNode(0.0, 0.0, SupportType::Pinned)
           .addNode(1.0, 0.0, SupportType::Pinned);
    
    EXPECT_THROW(builder.build(), std::runtime_error);
}

/**
 * @test TrussBuilder successfully builds valid truss
 */
TEST_F(TrussBuilderTest, BuildValid) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    builder.addMember(NodeId(1), NodeId(2));
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getNodeCount(), 2);
    EXPECT_EQ(truss->getMemberCount(), 1);
}

// =============================================================================
// RESET AND REUSE TESTS
// =============================================================================

/**
 * @test TrussBuilder resets to initial state
 */
TEST_F(TrussBuilderTest, ResetClearsState) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    builder.addMember(NodeId(1), NodeId(2));
    
    builder.reset();
    
    // After reset, should throw when building (no nodes/members)
    EXPECT_THROW(builder.build(), std::runtime_error);
}

/**
 * @test TrussBuilder can be reused after reset
 */
TEST_F(TrussBuilderTest, ReuseAfterReset) {
    // First truss
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    builder.addMember(NodeId(1), NodeId(2));
    auto truss1 = builder.build();
    
    // Reset and build second truss
    builder.reset();
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(2.0, 0.0, SupportType::Pinned);
    builder.addMember(NodeId(1), NodeId(2));
    auto truss2 = builder.build();
    
    ASSERT_NE(truss1, nullptr);
    ASSERT_NE(truss2, nullptr);
    EXPECT_NE(truss1, truss2);  // Different truss instances
}

/**
 * @test TrussBuilder node IDs restart from 1 after reset
 */
TEST_F(TrussBuilderTest, NodeIdRestartAfterReset) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    builder.reset();
    
    builder.addNode(0.0, 0.0, SupportType::Pinned);

    EXPECT_EQ(builder.getNodeCount(), 1);
    EXPECT_EQ(builder.getNextNodeId(), NodeId(2));
}

// =============================================================================
// DEFAULT MATERIAL/SECTION TESTS
// =============================================================================

/**
 * @test TrussBuilder uses steel as default material
 */
TEST_F(TrussBuilderTest, DefaultMaterialIsSteel) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    builder.addMember(NodeId(1), NodeId(2));  // No material specified
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    
    // Verify default material properties (Steel: E = 200 GPa)
    auto members = truss->getMembers();
    ASSERT_EQ(members.size(), 1);
    EXPECT_DOUBLE_EQ(members[0]->getMaterial().youngModulus, 200e9);
}

/**
 * @test TrussBuilder uses default circular section
 */
TEST_F(TrussBuilderTest, DefaultSectionIsCircular) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Pinned);
    
    builder.addMember(NodeId(1), NodeId(2));  // No section specified
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    
    // Verify default section has reasonable area
    auto members = truss->getMembers();
    ASSERT_EQ(members.size(), 1);
    EXPECT_GT(members[0]->getSection().area, 0.0);
}

// =============================================================================
// COMPLEX TRUSS TESTS
// =============================================================================

/**
 * @test TrussBuilder constructs simple triangular truss
 */
TEST_F(TrussBuilderTest, BuildSimpleTriangularTruss) {
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(2.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 1.5, SupportType::Free);
    
    builder.addMember(NodeId(1), NodeId(2))
           .addMember(NodeId(2), NodeId(3))
           .addMember(NodeId(3), NodeId(1))
           .applyForce(NodeId(3), 0.0, -10000.0);
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getNodeCount(), 3);
    EXPECT_EQ(truss->getMemberCount(), 3);
}

/**
 * @test TrussBuilder constructs Warren truss
 */
TEST_F(TrussBuilderTest, BuildWarrenTruss) {
    // Bottom nodes
    builder.addNode(0.0, 0.0, SupportType::Pinned);
    builder.addNode(1.0, 0.0, SupportType::Free);
    builder.addNode(2.0, 0.0, SupportType::Pinned);
    
    // Top nodes
    builder.addNode(0.5, 1.0, SupportType::Free);
    builder.addNode(1.5, 1.0, SupportType::Free);
    
    // Bottom chord
    builder.addMember(NodeId(1), NodeId(2))
           .addMember(NodeId(2), NodeId(3));
    
    // Top chord
    builder.addMember(NodeId(4), NodeId(5));
    
    // Diagonals
    builder.addMember(NodeId(1), NodeId(4))
           .addMember(NodeId(4), NodeId(2))
           .addMember(NodeId(2), NodeId(5))
           .addMember(NodeId(5), NodeId(3));
    
    // Apply loads
    builder.applyForce(NodeId(4), 0.0, -5000.0)
           .applyForce(NodeId(5), 0.0, -5000.0);
    
    auto truss = builder.build();
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getNodeCount(), 5);
    EXPECT_EQ(truss->getMemberCount(), 7);
}

}  // namespace
