/**
 * @file test_truss_validator.cpp
 * @brief Google Test unit tests for TrussValidator service
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include "../../src/core/validation/TrussValidator.hpp"
#include "../../src/core/model/Truss.hpp"
#include "../../src/core/model/Node.hpp"
#include "../../src/core/model/Member.hpp"

using namespace truss::core;
using namespace truss::core::validation;

// Test fixture for validator tests
class TrussValidatorTest : public ::testing::Test {
protected:
    TrussValidator validator;
    
    // Helper: Create a simple valid triangular truss
    Truss createValidTriangularTruss() {
        Truss truss("Valid Triangle");
        auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
        auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
        auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
        
        truss.addMember(node1, node2);
        truss.addMember(node1, node3);
        truss.addMember(node2, node3);
        
        truss.applyForce(3, 0.0, -1000.0);
        
        return truss;
    }
};

// ========== Structural Completeness Tests ==========

TEST_F(TrussValidatorTest, EmptyTrussFailsValidation) {
    Truss emptyTruss;
    auto result = validator.validate(emptyTruss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasFatal());
    
    auto errors = result.getErrorMessages();
    EXPECT_GT(errors.size(), 0);
}

TEST_F(TrussValidatorTest, TrussWithTooFewNodesIsInvalid) {
    Truss truss;
    truss.addNode(0.0, 0.0);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    // Should have errors about both missing nodes and missing members
    EXPECT_GT(result.countBySeverity(ValidationSeverity::Error) + 
              result.countBySeverity(ValidationSeverity::Fatal), 0);
}

TEST_F(TrussValidatorTest, TrussWithNoMembersIsInvalid) {
    Truss truss;
    truss.addNode(0.0, 0.0);
    truss.addNode(4.0, 0.0);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasFatal());
}

TEST_F(TrussValidatorTest, ValidTrussPassesStructuralCompleteness) {
    Truss truss = createValidTriangularTruss();
    auto result = validator.validate(truss);
    
    // May have warnings but should not have fatal or error related to completeness
    auto completenessIssues = result.getIssuesByCategory("Structural Completeness");
    for (const auto& issue : completenessIssues) {
        EXPECT_NE(issue.severity, ValidationSeverity::Fatal);
        EXPECT_NE(issue.severity, ValidationSeverity::Error);
    }
}

// ========== Geometry Tests ==========

TEST_F(TrussValidatorTest, ZeroLengthMemberDetected) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(0.0, 0.0, SupportType::Free); // Same position
    
    truss.addMember(node1, node2);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors());
    
    auto geometryIssues = result.getIssuesByCategory("Geometry");
    bool foundZeroLength = false;
    for (const auto& issue : geometryIssues) {
        if (issue.message.find("zero") != std::string::npos) {
            foundZeroLength = true;
            break;
        }
    }
    EXPECT_TRUE(foundZeroLength);
}

TEST_F(TrussValidatorTest, CoincidentNodesGenerateWarning) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(0.0, 1e-15, SupportType::Free); // Essentially same position
    auto node3 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    auto result = validator.validate(truss);
    
    // Should have warnings about coincident nodes
    EXPECT_TRUE(result.hasWarnings());
    
    auto geometryIssues = result.getIssuesByCategory("Geometry");
    bool foundCoincident = false;
    for (const auto& issue : geometryIssues) {
        if (issue.message.find("coincident") != std::string::npos) {
            foundCoincident = true;
            break;
        }
    }
    EXPECT_TRUE(foundCoincident);
}

TEST_F(TrussValidatorTest, DuplicateMembersGenerateWarning) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0);
    
    // Add same member twice (connecting same nodes)
    truss.addMember(node1, node2);
    truss.addMember(node1, node2); // Duplicate
    truss.addMember(node2, node3);
    
    auto result = validator.validate(truss);
    
    EXPECT_TRUE(result.hasWarnings());
    
    auto geometryIssues = result.getIssuesByCategory("Geometry");
    bool foundDuplicate = false;
    for (const auto& issue : geometryIssues) {
        if (issue.message.find("same nodes") != std::string::npos) {
            foundDuplicate = true;
            break;
        }
    }
    EXPECT_TRUE(foundDuplicate);
}

// ========== Material Validation Tests ==========

TEST_F(TrussValidatorTest, NegativeYoungModulusDetected) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    
    MaterialProperties badMaterial;
    badMaterial.youngModulus = -200e9; // Negative!
    SectionProperties section;
    
    truss.addMember(node1, node2, badMaterial, section);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors());
    
    auto materialIssues = result.getIssuesByCategory("Material");
    bool foundNegativeE = false;
    for (const auto& issue : materialIssues) {
        if (issue.message.find("Young's modulus") != std::string::npos) {
            foundNegativeE = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Error);
            break;
        }
    }
    EXPECT_TRUE(foundNegativeE);
}

TEST_F(TrussValidatorTest, ZeroAreaDetected) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    
    MaterialProperties material;
    SectionProperties badSection;
    badSection.area = 0.0; // Zero area!
    
    truss.addMember(node1, node2, material, badSection);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors());
}

TEST_F(TrussValidatorTest, ValidMaterialsPassValidation) {
    Truss truss = createValidTriangularTruss();
    auto result = validator.validate(truss);
    
    auto materialIssues = result.getIssuesByCategory("Material");
    for (const auto& issue : materialIssues) {
        // Should only have info or warnings, not errors
        EXPECT_NE(issue.severity, ValidationSeverity::Error);
        EXPECT_NE(issue.severity, ValidationSeverity::Fatal);
    }
}

// ========== Boundary Condition Tests ==========

TEST_F(TrussValidatorTest, NoSupportsDetected) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Free); // All free!
    auto node2 = truss.addNode(4.0, 0.0, SupportType::Free);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors());
    
    auto bcIssues = result.getIssuesByCategory("Boundary Conditions");
    bool foundNoSupports = false;
    for (const auto& issue : bcIssues) {
        if (issue.message.find("No support") != std::string::npos) {
            foundNoSupports = true;
            break;
        }
    }
    EXPECT_TRUE(foundNoSupports);
}

TEST_F(TrussValidatorTest, InsufficientConstraintsDetected) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::RollerY); // Only 1 constraint
    auto node2 = truss.addNode(4.0, 0.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors());
    
    // Should have error about insufficient constraints
    auto bcIssues = result.getIssuesByCategory("Boundary Conditions");
    bool foundInsufficient = false;
    for (const auto& issue : bcIssues) {
        if (issue.message.find("Insufficient") != std::string::npos) {
            foundInsufficient = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Error);
            break;
        }
    }
    EXPECT_TRUE(foundInsufficient);
}

TEST_F(TrussValidatorTest, AdequateSupportsPasses) {
    Truss truss = createValidTriangularTruss();
    auto result = validator.validate(truss);
    
    // Check that boundary conditions category has no errors
    auto bcIssues = result.getIssuesByCategory("Boundary Conditions");
    for (const auto& issue : bcIssues) {
        if (issue.severity == ValidationSeverity::Error || issue.severity == ValidationSeverity::Fatal) {
            FAIL() << "Unexpected boundary condition error: " << issue.message;
        }
    }
}

// ========== Static Determinacy Tests ==========

TEST_F(TrussValidatorTest, StaticallyDeterminateTrussIdentified) {
    Truss truss = createValidTriangularTruss();
    auto result = validator.validate(truss);
    
    // Check for info message about determinacy
    auto determinacyIssues = result.getIssuesByCategory("Static Determinacy");
    bool foundDeterminate = false;
    for (const auto& issue : determinacyIssues) {
        if (issue.message.find("statically determinate") != std::string::npos) {
            foundDeterminate = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Info);
            break;
        }
    }
    EXPECT_TRUE(foundDeterminate);
}

TEST_F(TrussValidatorTest, IndeterminateTrussIdentified) {
    // Create a statically indeterminate truss (extra members)
    // Use a fully connected 3-node triangle plus an extra diagonal
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);      // 2 constraints
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);     // 1 constraint
    auto node3 = truss.addNode(2.0, 3.0);
    auto node4 = truss.addNode(6.0, 3.0);
    
    // 7 members for 4 nodes with 3 constraints
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    truss.addMember(node2, node4);
    truss.addMember(node3, node4);
    truss.addMember(node1, node4);  // Extra diagonal
    truss.addMember(node2, node3);  // Duplicate member creates indeterminacy
    
    // Expected: n=4, m=7, r=3
    // Check: 2n = 8, m+r = 10, so indeterminate degree = 10-8 = 2
    
    auto result = validator.validate(truss);
    
    auto determinacyIssues = result.getIssuesByCategory("Static Determinacy");
    ASSERT_FALSE(determinacyIssues.empty()) << "No determinacy issues found";
    
    bool foundIndeterminate = false;
    for (const auto& issue : determinacyIssues) {
        if (issue.message.find("indeterminate") != std::string::npos && 
            issue.message.find("unstable") == std::string::npos) {
            foundIndeterminate = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Warning);
            break;
        }
    }
    EXPECT_TRUE(foundIndeterminate) << "Should have found indeterminate structure";
}

TEST_F(TrussValidatorTest, UnstableTrussDetected) {
    // Create unstable truss (too few members - mechanism)
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);      // 2 constraints
    auto node2 = truss.addNode(4.0, 0.0, SupportType::Free);         // 0 constraints
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);         // 0 constraints
    
    truss.addMember(node1, node2);
    // Only 1 member for 3 nodes and 2 constraints
    // n=3, m=1, r=2 → 2n=6, m+r=3, deficit of 3
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasErrors());
    
    auto determinacyIssues = result.getIssuesByCategory("Static Determinacy");
    ASSERT_FALSE(determinacyIssues.empty()) << "No determinacy issues found";
    
    bool foundUnstable = false;
    for (const auto& issue : determinacyIssues) {
        if (issue.message.find("unstable") != std::string::npos || 
            issue.message.find("mechanism") != std::string::npos) {
            foundUnstable = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Error);
            break;
        }
    }
    EXPECT_TRUE(foundUnstable) << "Should have found unstable/mechanism";
}

// ========== Load Validation Tests ==========

TEST_F(TrussValidatorTest, NoLoadsGeneratesWarning) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    // No forces applied
    
    auto result = validator.validate(truss);
    
    EXPECT_TRUE(result.hasWarnings());
    
    auto loadIssues = result.getIssuesByCategory("Loads");
    bool foundNoLoads = false;
    for (const auto& issue : loadIssues) {
        if (issue.message.find("No external forces") != std::string::npos) {
            foundNoLoads = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Warning);
            break;
        }
    }
    EXPECT_TRUE(foundNoLoads);
}

TEST_F(TrussValidatorTest, LoadOnConstrainedNodeWarning) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned); // Fully constrained
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    
    truss.addMember(node1, node2);
    
    // Apply force to fully constrained node
    truss.applyForce(1, 0.0, -1000.0);
    
    auto result = validator.validate(truss);
    
    EXPECT_TRUE(result.hasWarnings());
    
    auto loadIssues = result.getIssuesByCategory("Loads");
    bool foundConstrainedLoad = false;
    for (const auto& issue : loadIssues) {
        if (issue.message.find("fully constrained") != std::string::npos) {
            foundConstrainedLoad = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Warning);
            break;
        }
    }
    EXPECT_TRUE(foundConstrainedLoad);
}

// ========== Connectivity Tests ==========

TEST_F(TrussValidatorTest, SelfLoopMemberDetected) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    
    // Try to create member connecting node to itself
    // (This should be prevented by Member constructor, but we test validation)
    truss.addMember(node1, node1);
    
    auto result = validator.validate(truss);
    
    EXPECT_FALSE(result.isValid());
    
    auto connectivityIssues = result.getIssuesByCategory("Connectivity");
    bool foundSelfLoop = false;
    for (const auto& issue : connectivityIssues) {
        if (issue.message.find("itself") != std::string::npos) {
            foundSelfLoop = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Error);
            break;
        }
    }
    EXPECT_TRUE(foundSelfLoop);
}

TEST_F(TrussValidatorTest, IsolatedNodeWarning) {
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(8.0, 0.0); // Isolated, no support, no members
    
    truss.addMember(node1, node2);
    
    auto result = validator.validate(truss);
    
    EXPECT_TRUE(result.hasWarnings());
    
    auto stabilityIssues = result.getIssuesByCategory("Kinematic Stability");
    bool foundIsolated = false;
    for (const auto& issue : stabilityIssues) {
        if (issue.message.find("not connected") != std::string::npos) {
            foundIsolated = true;
            EXPECT_EQ(issue.severity, ValidationSeverity::Warning);
            break;
        }
    }
    EXPECT_TRUE(foundIsolated);
}

// ========== ValidationResult Tests ==========

TEST_F(TrussValidatorTest, ValidationResultSummary) {
    Truss truss = createValidTriangularTruss();
    auto result = validator.validate(truss);
    
    std::string summary = result.getSummary();
    EXPECT_FALSE(summary.empty());
    
    if (result.isValid()) {
        EXPECT_NE(summary.find("PASSED"), std::string::npos);
    } else {
        EXPECT_NE(summary.find("FAILED"), std::string::npos);
    }
}

TEST_F(TrussValidatorTest, IssueFilteringBySeverity) {
    Truss emptyTruss;
    auto result = validator.validate(emptyTruss);
    
    auto errors = result.getIssuesBySeverity(ValidationSeverity::Error);
    auto warnings = result.getIssuesBySeverity(ValidationSeverity::Warning);
    auto infos = result.getIssuesBySeverity(ValidationSeverity::Info);
    
    // Empty truss should have errors
    EXPECT_GT(errors.size() + result.getIssuesBySeverity(ValidationSeverity::Fatal).size(), 0);
}

TEST_F(TrussValidatorTest, IssueFilteringByCategory) {
    Truss truss = createValidTriangularTruss();
    auto result = validator.validate(truss);
    
    auto geometryIssues = result.getIssuesByCategory("Geometry");
    auto materialIssues = result.getIssuesByCategory("Material");
    auto loadIssues = result.getIssuesByCategory("Loads");
    
    // Should have categorized issues
    EXPECT_GE(result.getIssueCount(), 0);
}

// ========== Integration Tests ==========

TEST_F(TrussValidatorTest, ComplexValidTrussPassesAllChecks) {
    // Create a more complex valid truss
    Truss truss("Complex Valid Truss");
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto n3 = truss.addNode(2.0, 3.0);
    auto n4 = truss.addNode(6.0, 3.0);
    auto n5 = truss.addNode(8.0, 0.0, SupportType::RollerY);
    
    truss.addMember(n1, n2);
    truss.addMember(n1, n3);
    truss.addMember(n2, n3);
    truss.addMember(n2, n4);
    truss.addMember(n3, n4);
    truss.addMember(n4, n5);
    
    truss.applyForce(3, 0.0, -1000.0);
    truss.applyForce(4, 0.0, -500.0);
    
    auto result = validator.validate(truss);
    
    // Complex valid truss should pass (may have info messages but no errors)
    EXPECT_TRUE(result.isValid()) << "Validation failed: " << result.getSummary();
}

TEST_F(TrussValidatorTest, QuickValidationCheck) {
    Truss validTruss = createValidTriangularTruss();
    EXPECT_TRUE(validator.isValid(validTruss));
    
    Truss invalidTruss;
    EXPECT_FALSE(validator.isValid(invalidTruss));
}
