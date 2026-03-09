/**
 * @file test_truss_validator_branches.cpp
 * @brief Branch-coverage tests for TrussValidator — exercises specific paths
 *        not covered by test_truss_validator.cpp.
 * @version 3.0.0
 * @date 2026-03-08
 */

#include "../../src/core/model/member.hpp"
#include "../../src/core/model/node.hpp"
#include "../../src/core/model/truss.hpp"
#include "../../src/core/validation/truss_validator.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <limits>

using namespace truss::core;
using namespace truss::core::validation;

// ============================================================
// Fixture: provides a validator and small helper trusses
// ============================================================
class ValidatorBranchTest : public ::testing::Test {
protected:
    TrussValidator validator;

    // Minimal 2-node, 1-member truss with proper supports.
    // Allows adding extra conditions for specific branch testing.
    Truss makeTwoNodeTruss(SupportType s1 = SupportType::Pinned,
                           SupportType s2 = SupportType::RollerX) {
        Truss t("Two-Node");
        auto n1 = t.addNode(0.0, 0.0, s1);
        auto n2 = t.addNode(4.0, 0.0, s2);
        t.addMember(n1, n2);
        return t;
    }

    // Deterministic valid triangular truss with an applied force.
    Truss makeValidTriangle() {
        Truss t("Triangle");
        auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
        auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
        auto n3 = t.addNode(2.0, 3.0, SupportType::Free);
        t.addMember(n1, n2);
        t.addMember(n1, n3);
        t.addMember(n2, n3);
        t.applyForce(3, 0.0, -1000.0);
        return t;
    }
};

// ============================================================
// Geometry: NaN / Infinity coordinate
// ============================================================

TEST_F(ValidatorBranchTest, NanCoordinateInNodeDetected) {
    Truss t("NaN Coords");
    // Add nodes — node 3 has a NaN x coordinate directly at creation
    const double nan = std::numeric_limits<double>::quiet_NaN();
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
    auto n3 = t.addNode(nan, 3.0, SupportType::Free);  // NaN x

    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n3);
    t.applyForce(3, 0.0, -1000.0);

    auto result = validator.validate(t);

    auto geomIssues = result.getIssuesByCategory("Geometry");
    bool foundNaN = false;
    for (const auto& issue : geomIssues) {
        if (issue.message.find("NaN") != std::string::npos ||
            issue.message.find("infinity") != std::string::npos ||
            issue.message.find("invalid") != std::string::npos) {
            foundNaN = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Fatal);
            break;
        }
    }
    EXPECT_TRUE(foundNaN) << "Expected a fatal geometry issue for NaN coordinate";
}

TEST_F(ValidatorBranchTest, InfinityCoordinateInNodeDetected) {
    Truss t("Inf Coords");
    const double inf = std::numeric_limits<double>::infinity();
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
    auto n3 = t.addNode(2.0, inf, SupportType::Free);  // Infinity y

    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n3);
    t.applyForce(3, 0.0, -1000.0);

    auto result = validator.validate(t);

    auto geomIssues = result.getIssuesByCategory("Geometry");
    bool found = false;
    for (const auto& issue : geomIssues) {
        if (issue.severity == ValidationSeverity::Fatal) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected fatal geometry issue for infinity coordinate";
}

// ============================================================
// Materials: density ≤ 0 generates Warning
// ============================================================

TEST_F(ValidatorBranchTest, NonPositiveDensityGeneratesWarning) {
    Truss t("Low Density");
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);

    MaterialProperties mat;
    mat.youngModulus = 200e9;
    mat.density = 0.0;  // ← triggers density warning
    mat.yieldStrength = 250e6;

    SectionProperties sec;
    sec.area = 0.001;

    t.addMember(n1, n2, mat, sec);
    t.applyForce(2, 0.0, -5000.0);

    auto result = validator.validate(t);

    auto matIssues = result.getIssuesByCategory("Material");
    bool foundDensity = false;
    for (const auto& issue : matIssues) {
        if (issue.message.find("density") != std::string::npos &&
            issue.severity == ValidationSeverity::Warning) {
            foundDensity = true;
            break;
        }
    }
    EXPECT_TRUE(foundDensity) << "Expected a Warning for non-positive density";
}

TEST_F(ValidatorBranchTest, NegativeDensityGeneratesWarning) {
    Truss t("Neg Density");
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);

    MaterialProperties mat;
    mat.youngModulus = 200e9;
    mat.density = -7800.0;  // Negative
    mat.yieldStrength = 250e6;

    SectionProperties sec;
    sec.area = 0.001;

    t.addMember(n1, n2, mat, sec);
    t.applyForce(2, 0.0, -5000.0);

    auto result = validator.validate(t);

    auto matIssues = result.getIssuesByCategory("Material");
    bool found = false;
    for (const auto& issue : matIssues) {
        if (issue.message.find("density") != std::string::npos &&
            issue.severity == ValidationSeverity::Warning) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// ============================================================
// Materials: yield strength ≤ 0 generates Warning
// ============================================================

TEST_F(ValidatorBranchTest, NonPositiveYieldStrengthGeneratesWarning) {
    Truss t("Zero Yield");
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);

    MaterialProperties mat;
    mat.youngModulus = 200e9;
    mat.density = 7800.0;
    mat.yieldStrength = 0.0;  // ← triggers yield strength warning

    SectionProperties sec;
    sec.area = 0.001;

    t.addMember(n1, n2, mat, sec);
    t.applyForce(2, 0.0, -5000.0);

    auto result = validator.validate(t);

    auto matIssues = result.getIssuesByCategory("Material");
    bool found = false;
    for (const auto& issue : matIssues) {
        if (issue.message.find("yield") != std::string::npos &&
            issue.severity == ValidationSeverity::Warning) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected a Warning for zero yield strength";
}

// ============================================================
// Loads: NaN / Infinity force values
// ============================================================

TEST_F(ValidatorBranchTest, NanForceValueDetected) {
    Truss t = makeValidTriangle();
    // Override force on node 3 with NaN
    t.applyForce(3, std::numeric_limits<double>::quiet_NaN(), 0.0);

    auto result = validator.validate(t);

    auto loadIssues = result.getIssuesByCategory("Loads");
    bool found = false;
    for (const auto& issue : loadIssues) {
        if (issue.severity == ValidationSeverity::Error &&
            (issue.message.find("NaN") != std::string::npos ||
             issue.message.find("invalid") != std::string::npos)) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected an Error for NaN force value";
}

TEST_F(ValidatorBranchTest, InfinityForceValueDetected) {
    Truss t = makeValidTriangle();
    t.applyForce(3, 0.0, std::numeric_limits<double>::infinity());

    auto result = validator.validate(t);

    auto loadIssues = result.getIssuesByCategory("Loads");
    bool found = false;
    for (const auto& issue : loadIssues) {
        if (issue.severity == ValidationSeverity::Error) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected an Error for infinity force value";
}

// ============================================================
// Kinematic Stability: only RollerX supports — no X constraint
// ============================================================

// With 3 RollerX nodes: constrainedDofs = 3, hasYConstraint = true,
// but hasXConstraint = false → "No horizontal constraints" Fatal error.
TEST_F(ValidatorBranchTest, OnlyRollerXSupports_NoXConstraint_KinematicFatal) {
    Truss t("RollerX Only");
    auto n1 = t.addNode(0.0, 0.0, SupportType::RollerX);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
    auto n3 = t.addNode(2.0, 3.0, SupportType::RollerX);
    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n3);
    t.applyForce(3, 0.0, -1000.0);

    auto result = validator.validate(t);

    EXPECT_FALSE(result.isValid());

    auto kinIssues = result.getIssuesByCategory("Kinematic Stability");
    bool foundNoX = false;
    for (const auto& issue : kinIssues) {
        if (issue.message.find("horizontal") != std::string::npos ||
            issue.message.find("X direction") != std::string::npos) {
            foundNoX = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Fatal);
            break;
        }
    }
    EXPECT_TRUE(foundNoX) << "Expected Fatal kinematic error about missing X constraint";
}

// ============================================================
// Kinematic Stability: only RollerY supports — no Y constraint
// ============================================================

// With 3 RollerY nodes: constrainedDofs = 3, hasXConstraint = true,
// but hasYConstraint = false → "No vertical constraints" Fatal error.
TEST_F(ValidatorBranchTest, OnlyRollerYSupports_NoYConstraint_KinematicFatal) {
    Truss t("RollerY Only");
    auto n1 = t.addNode(0.0, 0.0, SupportType::RollerY);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerY);
    auto n3 = t.addNode(2.0, 3.0, SupportType::RollerY);
    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n3);
    t.applyForce(3, 0.0, -1000.0);

    auto result = validator.validate(t);

    EXPECT_FALSE(result.isValid());

    auto kinIssues = result.getIssuesByCategory("Kinematic Stability");
    bool foundNoY = false;
    for (const auto& issue : kinIssues) {
        if (issue.message.find("vertical") != std::string::npos ||
            issue.message.find("Y direction") != std::string::npos) {
            foundNoY = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Fatal);
            break;
        }
    }
    EXPECT_TRUE(foundNoY) << "Expected Fatal kinematic error about missing Y constraint";
}

// ============================================================
// Boundary Conditions / checkRigidBodyStability:
// 1 RollerX + 1 RollerY = 2 total constraints → < 3 → false
// ============================================================

TEST_F(ValidatorBranchTest, OneRollerXOneRollerY_InsufficientForRigidBodyStability) {
    // 1 RollerX (fixes Y) + 1 RollerY (fixes X) = 2 total constrained DOFs
    // checkRigidBodyStability: xCount=1, yCount=1, total=2 < 3 → returns false
    Truss t("RollerX+RollerY");
    auto n1 = t.addNode(0.0, 0.0, SupportType::RollerX);  // fixes Y
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerY);  // fixes X
    t.addMember(n1, n2);
    t.applyForce(2, 0.0, -5000.0);

    auto result = validator.validate(t);

    EXPECT_FALSE(result.isValid());

    // Should have BC error about rigid body motion
    auto bcIssues = result.getIssuesByCategory("Boundary Conditions");
    EXPECT_FALSE(bcIssues.empty()) << "Expected boundary condition issues";
}

// ============================================================
// ValidationResult: getWarningMessages() exercised
// ============================================================

TEST_F(ValidatorBranchTest, GetWarningMessages_ReturnsWarningsOnly) {
    // A valid truss with no loads will produce a "No external forces" Warning
    Truss t;
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
    auto n3 = t.addNode(2.0, 3.0, SupportType::Free);
    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n3);
    // No forces → triggers "No external forces" warning

    auto result = validator.validate(t);

    auto warnings = result.getWarningMessages();
    // Must have at least the "No external forces" warning message
    EXPECT_FALSE(warnings.empty()) << "Expected at least one warning message";

    // All returned strings should correspond to Warning-level issues
    for (const auto& msg : warnings) {
        EXPECT_FALSE(msg.empty());
        // Messages are formatted as "[Category] message"
        EXPECT_EQ(msg[0], '[');
    }
}

// ============================================================
// ValidationResult: countBySeverity with multiple severities
// ============================================================

TEST_F(ValidatorBranchTest, CountBySeverity_AccuratelyCountsEachLevel) {
    // An empty truss generates Fatals but no Infos (or very few)
    Truss empty;
    auto result = validator.validate(empty);

    size_t fatals = result.countBySeverity(ValidationSeverity::Fatal);
    size_t errors = result.countBySeverity(ValidationSeverity::Error);
    size_t warnings = result.countBySeverity(ValidationSeverity::Warning);
    size_t infos = result.countBySeverity(ValidationSeverity::Info);

    EXPECT_GT(fatals, 0u) << "Empty truss should produce Fatal issues";
    // Total must equal total issue count
    EXPECT_EQ(fatals + errors + warnings + infos, result.getIssueCount());
}

// ============================================================
// ValidationResult: getSummary — FAILED branch
// ============================================================

TEST_F(ValidatorBranchTest, GetSummary_FailedBranchContainsFAILED) {
    Truss empty;
    auto result = validator.validate(empty);

    EXPECT_FALSE(result.isValid());
    std::string summary = result.getSummary();
    EXPECT_NE(summary.find("FAILED"), std::string::npos)
        << "getSummary() should contain 'FAILED' for invalid truss. Got: " << summary;
}

// ============================================================
// ValidationResult: getSummary — PASSED branch
// ============================================================

TEST_F(ValidatorBranchTest, GetSummary_PassedBranchContainsPASSED) {
    Truss t = makeValidTriangle();
    auto result = validator.validate(t);

    EXPECT_TRUE(result.isValid()) << "Triangle truss should be valid";
    std::string summary = result.getSummary();
    EXPECT_NE(summary.find("PASSED"), std::string::npos)
        << "getSummary() should contain 'PASSED' for valid truss. Got: " << summary;
}

// ============================================================
// Connectivity: member with null start/end node pointer
// (achieved by creating a dangling member-like object)
// We test indirectly by verifying self-loop detection works
// for nodes added repeatedly — this exercises the connectivity loop.
// ============================================================

TEST_F(ValidatorBranchTest, AddMemberSelfLoopConnectivityError) {
    Truss t;
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);

    // Self-loop: n1 → n1
    t.addMember(n1, n1);
    t.addMember(n1, n2);  // Provide a real member too

    auto result = validator.validate(t);
    EXPECT_FALSE(result.isValid());

    auto connIssues = result.getIssuesByCategory("Connectivity");
    bool found = false;
    for (const auto& issue : connIssues) {
        if (issue.message.find("itself") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// ============================================================
// Loads: totalLoads counter — multiple loaded nodes
// ============================================================

TEST_F(ValidatorBranchTest, MultipleLoadedNodes_InfoMessageContainsCount) {
    Truss t;
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
    auto n3 = t.addNode(2.0, 3.0, SupportType::Free);
    auto n4 = t.addNode(6.0, 3.0, SupportType::Free);
    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n4);
    t.addMember(n3, n4);
    t.addMember(n2, n3);

    // Apply force to two nodes
    t.applyForce(3, 0.0, -1000.0);
    t.applyForce(4, -500.0, 0.0);

    auto result = validator.validate(t);

    // Find the "N node(s) have applied forces" info message
    auto loadIssues = result.getIssuesByCategory("Loads");
    bool found = false;
    for (const auto& issue : loadIssues) {
        if (issue.message.find("node(s) have applied forces") != std::string::npos) {
            found = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Info);
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected a Loads/Info message with loaded node count";
}

// ============================================================
// Connectivity: member references non-existent node
// Achieved by removing a node while a member still references it.
// Covers the `if (!startExists || !endExists)` Fatal branch.
// ============================================================

TEST_F(ValidatorBranchTest, RemoveNodeLeavingDanglingMember_ConnectivityFatal) {
    Truss t("Dangling");
    auto n1 = t.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = t.addNode(4.0, 0.0, SupportType::RollerX);
    auto n3 = t.addNode(2.0, 3.0, SupportType::Free);
    t.addMember(n1, n2);
    t.addMember(n1, n3);
    t.addMember(n2, n3);

    // Remove n2 WITHOUT removing its connected members → dangling reference
    t.removeNode(n2->getId());

    auto result = validator.validate(t);

    // Must have at least one Fatal "Connectivity" issue about non-existent node
    auto connectIssues = result.getIssuesByCategory("Connectivity");
    bool foundNonExistent = false;
    for (const auto& issue : connectIssues) {
        if (issue.severity == ValidationSeverity::Fatal &&
            issue.message.find("non-existent node") != std::string::npos) {
            foundNonExistent = true;
            break;
        }
    }
    EXPECT_TRUE(foundNonExistent)
        << "Expected a Fatal Connectivity issue for member referencing removed node";
}
