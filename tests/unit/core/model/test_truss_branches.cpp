/**
 * @file test_truss_branches.cpp
 * @brief Branch-coverage tests for Truss class targeting uncovered paths.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Test Coverage Targets:
 * - addNode(NodePtr) null-path branch
 * - addMember(NodePtr) null-path branch
 * - addMember(NodeId, NodeId) invalid-id throws
 * - addMember(NodePtr, NodePtr) null-node throws
 * - removeNode/removeMember null-ptr and not-found branches
 * - getFreeDofs() RollerX-specific branch
 * - getValidationErrors() all four error conditions
 * - getBoundingBoxMin/Max on empty truss → {0,0}
 * - getCentroid on empty truss → {0,0}
 * - applyForce / setSupportType on non-existent nodeId → no-op
 * - updateNode  found (true) and not-found (false) paths
 * - updateMember found (true) and not-found (false) paths
 * - isValid() with a zero-length (invalid) member
 * - getTotalWeight() with density-carrying members
 * - copy constructor and copy-assignment operator
 * - assignDofNumbers() correctness
 * - clear() resets all collections
 * - getNodeViews() / getMemberViews() population
 */

#include "../../../../src/core/model/truss.hpp"

#include <gtest/gtest.h>

using namespace truss::core;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static MaterialProperties makeMaterial(double E = 200e9, double density = 7850.0,
                                       double yield = 250e6) {
    MaterialProperties m;
    m.youngModulus  = E;
    m.density       = density;
    m.yieldStrength = yield;
    m.name          = "TestSteel";
    return m;
}

static SectionProperties makeSection(double area = 1e-3) {
    SectionProperties s;
    s.area        = area;
    s.designation = "TestSection";
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// addNode(NodePtr) null branch
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, AddNullNodeIsNoOp) {
    Truss truss;
    NodePtr result = truss.addNode(nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(truss.getNodeCount(), 0);
}

TEST(TrussBranchesTest, AddValidNodePtrInsertsNode) {
    Truss truss;
    auto existing = std::make_shared<Node>(99, Point2D{1.0, 2.0}, SupportType::Free);
    NodePtr result = truss.addNode(existing);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(truss.getNodeCount(), 1);
    // ID should be reassigned to 1 by the aggregate
    EXPECT_EQ(result->getId(), 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// addMember(MemberPtr) null branch
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, AddNullMemberPtrIsNoOp) {
    Truss truss;
    MemberPtr result = truss.addMember(nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(truss.getMemberCount(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// addMember throwing branches
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, AddMemberByNodeIdThrowsOnInvalidStartId) {
    Truss truss;
    truss.addNode(0.0, 0.0);  // id=1
    truss.addNode(1.0, 0.0);  // id=2

    EXPECT_THROW(truss.addMember(999, 2, makeMaterial(), makeSection()),
                 std::invalid_argument);
}

TEST(TrussBranchesTest, AddMemberByNodeIdThrowsOnInvalidEndId) {
    Truss truss;
    truss.addNode(0.0, 0.0);  // id=1
    truss.addNode(1.0, 0.0);  // id=2

    EXPECT_THROW(truss.addMember(1, 999, makeMaterial(), makeSection()),
                 std::invalid_argument);
}

TEST(TrussBranchesTest, AddMemberByNodePtrThrowsOnNullStart) {
    Truss truss;
    auto node2 = truss.addNode(1.0, 0.0);

    EXPECT_THROW(truss.addMember(nullptr, node2, makeMaterial(), makeSection()),
                 std::invalid_argument);
}

TEST(TrussBranchesTest, AddMemberByNodePtrThrowsOnNullEnd) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0);

    EXPECT_THROW(truss.addMember(node1, nullptr, makeMaterial(), makeSection()),
                 std::invalid_argument);
}

// ─────────────────────────────────────────────────────────────────────────────
// removeNode / removeMember null and not-found branches
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, RemoveNullNodeReturnsFalse) {
    Truss truss;
    EXPECT_FALSE(truss.removeNode(nullptr));
}

TEST(TrussBranchesTest, RemoveNodeByIdNotFoundReturnsFalse) {
    Truss truss;
    truss.addNode(0.0, 0.0);
    EXPECT_FALSE(truss.removeNode(999));
}

TEST(TrussBranchesTest, RemoveNullMemberReturnsFalse) {
    Truss truss;
    EXPECT_FALSE(truss.removeMember(nullptr));
}

TEST(TrussBranchesTest, RemoveMemberByIdNotFoundReturnsFalse) {
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(1.0, 0.0);
    truss.addMember(n1, n2, makeMaterial(), makeSection());
    EXPECT_FALSE(truss.removeMember(999));
}

TEST(TrussBranchesTest, RemoveNodeByPtrSucceeds) {
    Truss truss;
    auto node = truss.addNode(0.0, 0.0);
    EXPECT_EQ(truss.getNodeCount(), 1);
    EXPECT_TRUE(truss.removeNode(node));
    EXPECT_EQ(truss.getNodeCount(), 0);
}

TEST(TrussBranchesTest, RemoveMemberByPtrSucceeds) {
    Truss truss;
    auto n1     = truss.addNode(0.0, 0.0);
    auto n2     = truss.addNode(1.0, 0.0);
    auto member = truss.addMember(n1, n2, makeMaterial(), makeSection());
    EXPECT_EQ(truss.getMemberCount(), 1);
    EXPECT_TRUE(truss.removeMember(member));
    EXPECT_EQ(truss.getMemberCount(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// getFreeDofs() – RollerX branch (free X, constrained Y)
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, FreeDofsRollerXContributesOneFreeDof) {
    Truss truss;
    truss.addNode(0.0, 0.0, SupportType::RollerX);  // X free, Y pinned → +1
    truss.addNode(1.0, 0.0, SupportType::Pinned);   // both pinned        → +0

    EXPECT_EQ(truss.getFreeDofs(), 1);
    EXPECT_EQ(truss.getConstrainedDofs(), 3);  // 4 total - 1 free = 3
}

TEST(TrussBranchesTest, FreeDofsAllSupportTypesCombined) {
    Truss truss;
    truss.addNode(0.0, 0.0, SupportType::Free);    // +2
    truss.addNode(1.0, 0.0, SupportType::Pinned);  // +0
    truss.addNode(2.0, 0.0, SupportType::RollerX); // +1 (X free)
    truss.addNode(3.0, 0.0, SupportType::RollerY); // +1 (Y free)

    // total DOFs = 8, free = 4
    EXPECT_EQ(truss.getTotalDofs(), 8);
    EXPECT_EQ(truss.getFreeDofs(), 4);
    EXPECT_EQ(truss.getConstrainedDofs(), 4);
}

// ─────────────────────────────────────────────────────────────────────────────
// getValidationErrors() – each error condition
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, ValidationErrorsOnlyOneNode) {
    Truss truss;
    truss.addNode(0.0, 0.0, SupportType::Pinned);

    auto errors = truss.getValidationErrors();
    // Expect: "at least 2 nodes" and "at least 1 member" and others
    EXPECT_GE(errors.size(), 1u);
    bool hasNodeError = false;
    for (const auto& e : errors) {
        if (e.find("2 nodes") != std::string::npos || e.find("node") != std::string::npos) {
            hasNodeError = true;
        }
    }
    EXPECT_TRUE(hasNodeError);
}

TEST(TrussBranchesTest, ValidationErrorsTwoNodesNoMembers) {
    Truss truss;
    truss.addNode(0.0, 0.0, SupportType::Pinned);
    truss.addNode(1.0, 0.0, SupportType::RollerY);

    auto errors = truss.getValidationErrors();
    EXPECT_GE(errors.size(), 1u);
    bool hasMemberError = false;
    for (const auto& e : errors) {
        if (e.find("member") != std::string::npos) {
            hasMemberError = true;
        }
    }
    EXPECT_TRUE(hasMemberError);
}

TEST(TrussBranchesTest, ValidationErrorsKinematicallyUnstable) {
    // 2 Free nodes connected by 1 member: 0 constrained DOFs → kinematically unstable
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Free);
    auto n2 = truss.addNode(1.0, 0.0, SupportType::Free);
    truss.addMember(n1, n2, makeMaterial(), makeSection());

    auto errors = truss.getValidationErrors();
    bool hasStabilityError = false;
    for (const auto& e : errors) {
        if (e.find("stable") != std::string::npos || e.find("kinematic") != std::string::npos ||
            e.find("determinate") != std::string::npos) {
            hasStabilityError = true;
        }
    }
    EXPECT_TRUE(hasStabilityError);
}

TEST(TrussBranchesTest, ValidationErrorsStaticallyIndeterminate) {
    // Statically indeterminate: 2*n != m + r
    // 3 nodes, 4 members, 3 reactions → 2*3=6 != 4+3=7 → indeterminate
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);   // 2 reactions
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerY);  // 1 reaction
    auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
    truss.addMember(n1, n2, makeMaterial(), makeSection());
    truss.addMember(n1, n3, makeMaterial(), makeSection());
    truss.addMember(n2, n3, makeMaterial(), makeSection());
    truss.addMember(n1, n2, makeMaterial(), makeSection()); // duplicate → 4 members

    auto errors = truss.getValidationErrors();
    bool hasIndeterminateError = false;
    for (const auto& e : errors) {
        if (e.find("determinate") != std::string::npos) {
            hasIndeterminateError = true;
        }
    }
    EXPECT_TRUE(hasIndeterminateError);
}

// ─────────────────────────────────────────────────────────────────────────────
// getBoundingBoxMin/Max and getCentroid on empty truss
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, BoundingBoxMinEmptyTrussReturnsOrigin) {
    Truss truss;
    auto pt = truss.getBoundingBoxMin();
    EXPECT_DOUBLE_EQ(pt.x, 0.0);
    EXPECT_DOUBLE_EQ(pt.y, 0.0);
}

TEST(TrussBranchesTest, BoundingBoxMaxEmptyTrussReturnsOrigin) {
    Truss truss;
    auto pt = truss.getBoundingBoxMax();
    EXPECT_DOUBLE_EQ(pt.x, 0.0);
    EXPECT_DOUBLE_EQ(pt.y, 0.0);
}

TEST(TrussBranchesTest, CentroidEmptyTrussReturnsOrigin) {
    Truss truss;
    auto pt = truss.getCentroid();
    EXPECT_DOUBLE_EQ(pt.x, 0.0);
    EXPECT_DOUBLE_EQ(pt.y, 0.0);
}

TEST(TrussBranchesTest, BoundingBoxWithNodesCorrect) {
    Truss truss;
    truss.addNode(-2.0, 1.0);
    truss.addNode(3.0, -4.0);
    truss.addNode(0.0, 5.0);

    auto minPt = truss.getBoundingBoxMin();
    auto maxPt = truss.getBoundingBoxMax();
    EXPECT_DOUBLE_EQ(minPt.x, -2.0);
    EXPECT_DOUBLE_EQ(minPt.y, -4.0);
    EXPECT_DOUBLE_EQ(maxPt.x, 3.0);
    EXPECT_DOUBLE_EQ(maxPt.y, 5.0);
}

TEST(TrussBranchesTest, CentroidWithNodes) {
    Truss truss;
    truss.addNode(0.0, 0.0);
    truss.addNode(2.0, 0.0);
    truss.addNode(1.0, 2.0);

    auto c = truss.getCentroid();
    EXPECT_NEAR(c.x, 1.0, 1e-10);
    EXPECT_NEAR(c.y, 2.0 / 3.0, 1e-10);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyForce and setSupportType on non-existent nodeId → no-op
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, ApplyForceToNonexistentNodeIsNoOp) {
    Truss truss;
    auto node = truss.addNode(0.0, 0.0);
    // Force on existing node first
    truss.applyForce(1, Force2D(100.0, 0.0));
    EXPECT_NEAR(node->getAppliedForce().fx, 100.0, 1e-10);

    // Force on non-existent node → no-op (no crash, existing unchanged)
    EXPECT_NO_THROW(truss.applyForce(999, Force2D(50.0, 0.0)));
    EXPECT_NEAR(node->getAppliedForce().fx, 100.0, 1e-10);  // unchanged
}

TEST(TrussBranchesTest, ApplyForceXYOverloadOnNonexistentNodeIsNoOp) {
    Truss truss;
    EXPECT_NO_THROW(truss.applyForce(999, 50.0, -100.0));
    EXPECT_EQ(truss.getNodeCount(), 0);
}

TEST(TrussBranchesTest, SetSupportTypeOnNonexistentNodeIsNoOp) {
    Truss truss;
    auto node = truss.addNode(0.0, 0.0, SupportType::Free);
    EXPECT_NO_THROW(truss.setSupportType(999, SupportType::Pinned));
    // Existing node support type unchanged
    EXPECT_EQ(node->getSupportType(), SupportType::Free);
}

TEST(TrussBranchesTest, SetSupportTypeOnExistingNodeUpdates) {
    Truss truss;
    auto node = truss.addNode(0.0, 0.0, SupportType::Free);
    truss.setSupportType(1, SupportType::Pinned);
    EXPECT_EQ(node->getSupportType(), SupportType::Pinned);
}

// ─────────────────────────────────────────────────────────────────────────────
// updateNode – found (true) and not-found (false)
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, UpdateNodeFoundReturnsTrue) {
    Truss truss;
    auto node = truss.addNode(0.0, 0.0);
    bool ok = truss.updateNode(1, Point2D{5.0, 6.0});
    EXPECT_TRUE(ok);
    EXPECT_NEAR(node->getX(), 5.0, 1e-10);
    EXPECT_NEAR(node->getY(), 6.0, 1e-10);
}

TEST(TrussBranchesTest, UpdateNodeNotFoundReturnsFalse) {
    Truss truss;
    truss.addNode(0.0, 0.0);
    bool ok = truss.updateNode(999, Point2D{5.0, 6.0});
    EXPECT_FALSE(ok);
}

// ─────────────────────────────────────────────────────────────────────────────
// updateMember – found (true) and not-found (false)
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, UpdateMemberFoundReturnsTrue) {
    Truss truss;
    auto n1     = truss.addNode(0.0, 0.0);
    auto n2     = truss.addNode(3.0, 4.0);
    auto member = truss.addMember(n1, n2, makeMaterial(200e9), makeSection(1e-3));

    MaterialProperties newMat = makeMaterial(100e9);
    SectionProperties  newSec = makeSection(2e-3);
    bool ok = truss.updateMember(member->getId(), newMat, newSec);
    EXPECT_TRUE(ok);
    EXPECT_NEAR(member->getMaterial().youngModulus, 100e9, 1.0);
    EXPECT_NEAR(member->getSection().area, 2e-3, 1e-10);
}

TEST(TrussBranchesTest, UpdateMemberNotFoundReturnsFalse) {
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(1.0, 0.0);
    truss.addMember(n1, n2, makeMaterial(), makeSection());

    bool ok = truss.updateMember(999, makeMaterial(), makeSection());
    EXPECT_FALSE(ok);
}

// ─────────────────────────────────────────────────────────────────────────────
// isValid() with an invalid member (zero-length → start == end position)
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, IsValidWithZeroLengthMemberReturnsFalse) {
    Truss truss;
    // Two nodes at the exact same position → zero-length member
    auto n1 = truss.addNode(2.0, 3.0);
    auto n2 = truss.addNode(2.0, 3.0);  // same coords
    truss.addMember(n1, n2, makeMaterial(), makeSection());

    EXPECT_FALSE(truss.isValid());
}

TEST(TrussBranchesTest, IsValidWithValidMembersReturnsTrue) {
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(1.0, 0.0);
    truss.addMember(n1, n2, makeMaterial(), makeSection());
    EXPECT_TRUE(truss.isValid());
}

TEST(TrussBranchesTest, IsValidEmptyTrussReturnsFalse) {
    Truss truss;
    EXPECT_FALSE(truss.isValid());
}

TEST(TrussBranchesTest, IsValidOneMemberNoMembersReturnsFalse) {
    Truss truss;
    truss.addNode(0.0, 0.0);
    EXPECT_FALSE(truss.isValid());
}

// ─────────────────────────────────────────────────────────────────────────────
// getTotalWeight()
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, TotalWeightEmptyTrussIsZero) {
    Truss truss;
    EXPECT_DOUBLE_EQ(truss.getTotalWeight(), 0.0);
}

TEST(TrussBranchesTest, TotalWeightWithMemberIsPositive) {
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(3.0, 4.0);  // length = 5.0 m
    // density=7850 kg/m³, area=1e-3 m² → weight = 7850 * 1e-3 * 5.0 * g
    // (exact formula depends on implementation; just check > 0)
    truss.addMember(n1, n2, makeMaterial(200e9, 7850.0), makeSection(1e-3));
    EXPECT_GT(truss.getTotalWeight(), 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Copy constructor and copy-assignment operator
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, CopyConstructorCreatesIndependentCopy) {
    Truss original("Original");
    auto n1 = original.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = original.addNode(1.0, 0.0, SupportType::RollerY);
    original.addMember(n1, n2, makeMaterial(), makeSection());
    original.applyForce(1, Force2D(100.0, 0.0));

    Truss copy(original);  // copy constructor

    EXPECT_EQ(copy.getName(), "Original");
    EXPECT_EQ(copy.getNodeCount(), 2);
    EXPECT_EQ(copy.getMemberCount(), 1);
    EXPECT_NEAR(copy.getNode(1)->getAppliedForce().fx, 100.0, 1e-10);
}

TEST(TrussBranchesTest, CopyAssignmentReplacesContent) {
    Truss trussA("A");
    trussA.addNode(0.0, 0.0);
    trussA.addNode(1.0, 0.0);

    Truss trussB("B");
    trussB.addNode(5.0, 5.0);

    trussB = trussA;  // assignment operator

    EXPECT_EQ(trussB.getName(), "A");
    EXPECT_EQ(trussB.getNodeCount(), 2);
}

TEST(TrussBranchesTest, SelfAssignmentIsNoOp) {
    Truss truss("Self");
    truss.addNode(0.0, 0.0);
    truss = truss;  // self-assignment
    EXPECT_EQ(truss.getNodeCount(), 1);
    EXPECT_EQ(truss.getName(), "Self");
}

// ─────────────────────────────────────────────────────────────────────────────
// assignDofNumbers()
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, AssignDofNumbersPopulatesSequentially) {
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(1.0, 0.0);
    auto n3 = truss.addNode(2.0, 0.0);

    truss.assignDofNumbers();

    // Node 1: dofX=0, dofY=1; Node 2: dofX=2, dofY=3; Node 3: dofX=4, dofY=5
    EXPECT_EQ(n1->getDofX(), 0u);
    EXPECT_EQ(n1->getDofY(), 1u);
    EXPECT_EQ(n2->getDofX(), 2u);
    EXPECT_EQ(n2->getDofY(), 3u);
    EXPECT_EQ(n3->getDofX(), 4u);
    EXPECT_EQ(n3->getDofY(), 5u);
}

// ─────────────────────────────────────────────────────────────────────────────
// clear()
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, ClearResetsAllCollections) {
    Truss truss("ClearTest");
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(1.0, 0.0);
    truss.addMember(n1, n2, makeMaterial(), makeSection());
    truss.applyForce(1, Force2D(100.0, 0.0));

    EXPECT_EQ(truss.getNodeCount(), 2);
    EXPECT_EQ(truss.getMemberCount(), 1);
    EXPECT_TRUE(truss.hasAppliedForces());

    truss.clear();

    EXPECT_EQ(truss.getNodeCount(), 0);
    EXPECT_EQ(truss.getMemberCount(), 0);
    EXPECT_FALSE(truss.hasAppliedForces());

    // IDs should reset — new node should get ID 1 again
    auto newNode = truss.addNode(5.0, 5.0);
    EXPECT_EQ(newNode->getId(), 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// getNodeViews() and getMemberViews()
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussBranchesTest, GetNodeViewsPopulatesAllFields) {
    Truss truss;
    auto node = truss.addNode(3.0, 4.0, SupportType::Pinned);
    truss.applyForce(1, Force2D(10.0, -20.0));

    auto views = truss.getNodeViews();
    ASSERT_EQ(views.size(), 1u);
    EXPECT_EQ(views[0].id, node->getId());
    EXPECT_NEAR(views[0].x, 3.0, 1e-10);
    EXPECT_NEAR(views[0].y, 4.0, 1e-10);
    EXPECT_EQ(views[0].support, SupportType::Pinned);
    EXPECT_NEAR(views[0].fx, 10.0, 1e-10);
    EXPECT_NEAR(views[0].fy, -20.0, 1e-10);
}

TEST(TrussBranchesTest, GetMemberViewsPopulatesAllFields) {
    Truss truss;
    auto n1  = truss.addNode(0.0, 0.0);
    auto n2  = truss.addNode(3.0, 4.0);  // length=5.0
    auto mat = makeMaterial(200e9, 7850.0, 250e6);
    auto sec = makeSection(1e-3);
    truss.addMember(n1, n2, mat, sec);

    auto views = truss.getMemberViews();
    ASSERT_EQ(views.size(), 1u);
    EXPECT_EQ(views[0].startNodeId, n1->getId());
    EXPECT_EQ(views[0].endNodeId, n2->getId());
    EXPECT_NEAR(views[0].length, 5.0, 1e-10);
    EXPECT_NEAR(views[0].youngModulus, 200e9, 1.0);
    EXPECT_NEAR(views[0].area, 1e-3, 1e-10);
}

TEST(TrussBranchesTest, GetNodeViewsEmptyTrussReturnsEmpty) {
    Truss truss;
    EXPECT_TRUE(truss.getNodeViews().empty());
}

TEST(TrussBranchesTest, GetMemberViewsEmptyTrussReturnsEmpty) {
    Truss truss;
    EXPECT_TRUE(truss.getMemberViews().empty());
}
