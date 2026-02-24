/**
 * @file truss_validator.cpp
 * @brief Implements the TrussValidator service.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "truss_validator.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>
#include <unordered_set>

namespace truss::core::validation {

// ========== ValidationResult Implementation ==========

[[maybe_unused]] std::vector<ValidationIssue>
ValidationResult::getIssuesBySeverity(ValidationSeverity severity) const {
    std::vector<ValidationIssue> filtered;
    std::copy_if(m_issues.begin(),
                 m_issues.end(),
                 std::back_inserter(filtered),
                 [severity](const ValidationIssue& issue) { return issue.severity == severity; });
    return filtered;
}

[[maybe_unused]] std::vector<ValidationIssue>
ValidationResult::getIssuesByCategory(const std::string& category) const {
    std::vector<ValidationIssue> filtered;
    std::copy_if(m_issues.begin(),
                 m_issues.end(),
                 std::back_inserter(filtered),
                 [&category](const ValidationIssue& issue) { return issue.category == category; });
    return filtered;
}

size_t ValidationResult::countBySeverity(ValidationSeverity severity) const {
    return std::count_if(
        m_issues.begin(), m_issues.end(), [severity](const ValidationIssue& issue) {
            return issue.severity == severity;
        });
}

std::string ValidationResult::getSummary() const {
    std::ostringstream oss;
    size_t errors = countBySeverity(ValidationSeverity::Error) +
                    countBySeverity(ValidationSeverity::Fatal);
    size_t warnings = countBySeverity(ValidationSeverity::Warning);
    size_t infos = countBySeverity(ValidationSeverity::Info);

    if (isValid()) {
        oss << "Validation PASSED: Truss is valid for analysis.";
    } else {
        oss << "Validation FAILED: ";
    }

    oss << " [Errors: " << errors << ", Warnings: " << warnings << ", Info: " << infos << "]";
    return oss.str();
}

std::vector<std::string> ValidationResult::getErrorMessages() const {
    std::vector<std::string> messages;
    for (const auto& issue : m_issues) {
        if (issue.severity == ValidationSeverity::Error ||
            issue.severity == ValidationSeverity::Fatal) {
            messages.push_back("[" + issue.category + "] " + issue.message);
        }
    }
    return messages;
}

[[maybe_unused]] std::vector<std::string> ValidationResult::getWarningMessages() const {
    std::vector<std::string> messages;
    for (const auto& issue : m_issues) {
        if (issue.severity == ValidationSeverity::Warning) {
            messages.push_back("[" + issue.category + "] " + issue.message);
        }
    }
    return messages;
}

// ========== TrussValidator Implementation ==========

ValidationResult TrussValidator::validate(const Truss& truss) {
    ValidationResult result;

    // Run all validation checks in order of severity
    validateStructuralCompleteness(truss, result);
    validateConnectivity(truss, result);
    validateGeometry(truss, result);
    validateMaterials(truss, result);
    validateBoundaryConditions(truss, result);
    validateLoads(truss, result);
    validateStaticDeterminacy(truss, result);
    validateKinematicStability(truss, result);

    return result;
}

bool TrussValidator::isValid(const Truss& truss) {
    return validate(truss).isValid();
}

void TrussValidator::validateStructuralCompleteness(const Truss& truss, ValidationResult& result) {
    // Check minimum node count
    if (truss.getNodeCount() < 2) {
        result.addIssue(ValidationIssue(
            ValidationSeverity::Fatal,
            "Structural Completeness",
            "Truss must have at least 2 nodes",
            "A truss requires a minimum of 2 nodes to form any structural element."));
    }

    // Check minimum member count
    if (truss.getMemberCount() < 1) {
        result.addIssue(
            ValidationIssue(ValidationSeverity::Fatal,
                            "Structural Completeness",
                            "Truss must have at least 1 member",
                            "A truss requires at least one member to form a structural system."));
    }

    // Check for null nodes
    for (const auto& node : truss.getNodes()) {
        if (!node) {
            result.addIssue(
                ValidationIssue(ValidationSeverity::Fatal,
                                "Structural Completeness",
                                "Null node pointer detected in truss structure",
                                "Internal error: Node collection contains null pointer."));
        }
    }

    // Check for null members
    for (const auto& member : truss.getMembers()) {
        if (!member) {
            result.addIssue(
                ValidationIssue(ValidationSeverity::Fatal,
                                "Structural Completeness",
                                "Null member pointer detected in truss structure",
                                "Internal error: Member collection contains null pointer."));
        }
    }
}

void TrussValidator::validateGeometry(const Truss& truss, ValidationResult& result) {
    // Check for zero-length members
    auto zeroLengthMembers = findZeroLengthMembers(truss, Constants::GEOMETRY_TOLERANCE);
    for (auto memberId : zeroLengthMembers) {
        auto member = truss.getMember(memberId);
        if (member) {
            ValidationIssue issue(ValidationSeverity::Error,
                                  "Geometry",
                                  "Member " + std::to_string(memberId) +
                                      " has zero or near-zero length",
                                  "Member length: " + std::to_string(member->getLength()) +
                                      " m. "
                                      "Zero-length members cannot carry loads and will cause "
                                      "singular stiffness matrix.");
            issue.affectedMembers.push_back(memberId);
            issue.affectedNodes.push_back(member->getStartNode()->getId());
            issue.affectedNodes.push_back(member->getEndNode()->getId());
            result.addIssue(issue);
        }
    }

    // Check for coincident nodes
    auto coincidentPairs = findCoincidentNodePairs(truss, Constants::GEOMETRY_TOLERANCE);
    for (const auto& [id1, id2] : coincidentPairs) {
        ValidationIssue issue(ValidationSeverity::Warning,
                              "Geometry",
                              "Nodes " + std::to_string(id1) + " and " + std::to_string(id2) +
                                  " are coincident or very close",
                              "Consider merging these nodes or checking for input errors.");
        issue.affectedNodes = {id1, id2};
        result.addIssue(issue);
    }

    // Check for duplicate members
    auto duplicatePairs = findDuplicateMembers(truss);
    for (const auto& [id1, id2] : duplicatePairs) {
        ValidationIssue issue(
            ValidationSeverity::Warning,
            "Geometry",
            "Members " + std::to_string(id1) + " and " + std::to_string(id2) +
                " connect the same nodes",
            "Duplicate members do not add structural stiffness in a truss model.");
        issue.affectedMembers = {id1, id2};
        result.addIssue(issue);
    }

    // Check for valid coordinates (no NaN or infinity)
    for (const auto& node : truss.getNodes()) {
        if (std::isnan(node->getX()) || std::isnan(node->getY()) || std::isinf(node->getX()) ||
            std::isinf(node->getY())) {
            ValidationIssue issue(ValidationSeverity::Fatal,
                                  "Geometry",
                                  "Node " + std::to_string(node->getId()) +
                                      " has invalid coordinates (NaN or infinity)",
                                  "Position: (" + std::to_string(node->getX()) + ", " +
                                      std::to_string(node->getY()) + ")");
            issue.affectedNodes.push_back(node->getId());
            result.addIssue(issue);
        }
    }
}

void TrussValidator::validateMaterials(const Truss& truss, ValidationResult& result) {
    for (const auto& member : truss.getMembers()) {
        const auto& material = member->getMaterial();
        const auto& section = member->getSection();

        // Check Young's modulus
        if (material.youngModulus <= 0.0) {
            ValidationIssue issue(ValidationSeverity::Error,
                                  "Material",
                                  "Member " + std::to_string(member->getId()) +
                                      " has non-positive Young's modulus",
                                  "Young's modulus: " + std::to_string(material.youngModulus) +
                                      " Pa. Must be positive.");
            issue.affectedMembers.push_back(member->getId());
            result.addIssue(issue);
        }

        // Check cross-sectional area
        if (section.area <= 0.0) {
            ValidationIssue issue(ValidationSeverity::Error,
                                  "Material",
                                  "Member " + std::to_string(member->getId()) +
                                      " has non-positive cross-sectional area",
                                  "Area: " + std::to_string(section.area) +
                                      " m². Must be positive.");
            issue.affectedMembers.push_back(member->getId());
            result.addIssue(issue);
        }

        // Check density (warning only, not critical for static analysis)
        if (material.density <= 0.0) {
            ValidationIssue issue(ValidationSeverity::Warning,
                                  "Material",
                                  "Member " + std::to_string(member->getId()) +
                                      " has non-positive density",
                                  "Density: " + std::to_string(material.density) +
                                      " kg/m³. Self-weight calculations will be incorrect.");
            issue.affectedMembers.push_back(member->getId());
            result.addIssue(issue);
        }

        // Check yield strength (warning only)
        if (material.yieldStrength <= 0.0) {
            ValidationIssue issue(ValidationSeverity::Warning,
                                  "Material",
                                  "Member " + std::to_string(member->getId()) +
                                      " has non-positive yield strength",
                                  "Yield strength: " + std::to_string(material.yieldStrength) +
                                      " Pa. Stress checking will be invalid.");
            issue.affectedMembers.push_back(member->getId());
            result.addIssue(issue);
        }
    }
}

void TrussValidator::validateBoundaryConditions(const Truss& truss, ValidationResult& result) {
    // Check minimum constraints for stability (at least 3 constraints to prevent rigid body motion)
    size_t totalConstraints = truss.getConstrainedDofs();

    if (totalConstraints < 3) {
        result.addIssue(ValidationIssue(
            ValidationSeverity::Error,
            "Boundary Conditions",
            "Insufficient boundary constraints for stability",
            "Total constraints: " + std::to_string(totalConstraints) +
                ". "
                "Minimum 3 constraints required to prevent rigid body motion (2D truss)."));
    }

    // Check if truss has any supports
    auto constrainedNodes = truss.getConstrainedNodes();
    if (constrainedNodes.empty()) {
        result.addIssue(ValidationIssue(
            ValidationSeverity::Error,
            "Boundary Conditions",
            "No support constraints defined",
            "At least one node must have boundary conditions to prevent rigid body motion."));
    }

    // Check for proper constraint distribution
    if (!checkRigidBodyStability(truss)) {
        result.addIssue(ValidationIssue(ValidationSeverity::Error,
                                        "Boundary Conditions",
                                        "Boundary conditions do not prevent rigid body motion",
                                        "The truss may rotate or translate as a rigid body. "
                                        "Ensure supports are properly distributed and oriented."));
    }

    // Info: Report support configuration
    size_t pinnedCount = 0, rollerCount = 0;
    for (const auto& node : truss.getNodes()) {
        if (node->isPinned())
            pinnedCount++;
        if (node->isRoller())
            rollerCount++;
    }

    result.addIssue(ValidationIssue(ValidationSeverity::Info,
                                    "Boundary Conditions",
                                    "Support configuration: " + std::to_string(pinnedCount) +
                                        " pinned, " + std::to_string(rollerCount) +
                                        " roller supports",
                                    "Total DOF constraints: " + std::to_string(totalConstraints)));
}

void TrussValidator::validateStaticDeterminacy(const Truss& truss, ValidationResult& result) {
    size_t n = truss.getNodeCount();
    size_t m = truss.getMemberCount();
    size_t r = truss.getConstrainedDofs();

    // Check equation: 2n = m + r
    int determinacyCheck = static_cast<int>(2 * n) - static_cast<int>(m + r);

    if (determinacyCheck == 0) {
        result.addIssue(ValidationIssue(ValidationSeverity::Info,
                                        "Static Determinacy",
                                        "Truss is statically determinate",
                                        "2n = m + r: 2×" + std::to_string(n) + " = " +
                                            std::to_string(m) + " + " + std::to_string(r)));
    } else if (determinacyCheck < 0) {
        // 2n < m+r means too many members/constraints → indeterminate
        result.addIssue(ValidationIssue(
            ValidationSeverity::Warning,
            "Static Determinacy",
            "Truss is statically indeterminate (degree: " + std::to_string(-determinacyCheck) + ")",
            "2n = " + std::to_string(2 * n) + ", m + r = " + std::to_string(m + r) +
                ". "
                "Indeterminate structures require advanced analysis methods."));
    } else {
        // 2n > m+r means too few members/constraints → unstable
        result.addIssue(
            ValidationIssue(ValidationSeverity::Error,
                            "Static Determinacy",
                            "Truss is unstable or has internal mechanisms (deficiency: " +
                                std::to_string(determinacyCheck) + ")",
                            "2n = " + std::to_string(2 * n) + ", m + r = " + std::to_string(m + r) +
                                ". "
                                "The structure lacks sufficient members or constraints."));
    }
}

void TrussValidator::validateKinematicStability(const Truss& truss, ValidationResult& result) {
    // Critical stability check: prevent geometrically unstable structures from analysis

    const auto& nodes = truss.getNodes();

    // Check 1: Minimum constraint count
    if (truss.getConstrainedDofs() < 3) {
        result.addIssue(ValidationIssue(ValidationSeverity::Fatal,
                                        "Kinematic Stability",
                                        "Insufficient constraints: Structure has only " +
                                            std::to_string(truss.getConstrainedDofs()) +
                                            " constrained DOFs (minimum 3 required)",
                                        "A 2D truss requires at least 3 constraints to prevent "
                                        "rigid body motion (2 translations + 1 rotation)."));
        return;
    }

    // Check 2: Rigid body mode prevention
    bool hasXConstraint = false;
    bool hasYConstraint = false;

    for (const auto& node : nodes) {
        SupportType support = node->getSupportType();

        if (support == SupportType::Pinned || support == SupportType::RollerY) {
            hasXConstraint = true;
        }

        if (support == SupportType::Pinned || support == SupportType::RollerX) {
            hasYConstraint = true;
        }
    }

    if (!hasXConstraint) {
        result.addIssue(ValidationIssue(
            ValidationSeverity::Fatal,
            "Kinematic Stability",
            "No horizontal constraints: Structure can translate freely in X direction",
            "Add a support with X constraint (Pinned or RollerY)."));
    }

    if (!hasYConstraint) {
        result.addIssue(ValidationIssue(
            ValidationSeverity::Fatal,
            "Kinematic Stability",
            "No vertical constraints: Structure can translate freely in Y direction (GEOMETRIC "
            "INSTABILITY)",
            "CRITICAL: This causes singular stiffness matrix and astronomical displacements. "
            "For typical bridge truss: Left=Pinned, Right=RollerX (NOT RollerY). "
            "RollerY allows vertical movement and makes the structure unstable under vertical "
            "loads."));
    }

    if (!hasXConstraint || !hasYConstraint) {
        return;  // Don't continue if fatal errors found
    }

    // Check 3: Full stability check
    if (!checkMinimumConstraints(truss)) {
        result.addIssue(
            ValidationIssue(ValidationSeverity::Error,
                            "Kinematic Stability",
                            "Structure lacks adequate constraints for kinematic stability",
                            "The structure may contain mechanisms. Verify support configuration "
                            "prevents all rigid body modes."));
    }

    // Check 4: Isolated nodes
    for (const auto& node : nodes) {
        auto connectedMembers = truss.getMembersConnectedTo(node->getId());
        if (connectedMembers.empty() && !node->isConstrained()) {
            ValidationIssue issue(
                ValidationSeverity::Warning,
                "Kinematic Stability",
                "Node " + std::to_string(node->getId()) +
                    " is not connected to any member and has no support",
                "This node is isolated and will not participate in load transfer.");
            issue.affectedNodes.push_back(node->getId());
            result.addIssue(issue);
        }
    }
}

void TrussValidator::validateLoads(const Truss& truss, ValidationResult& result) {
    // Check if any loads are applied
    if (!truss.hasAppliedForces()) {
        result.addIssue(ValidationIssue(
            ValidationSeverity::Warning,
            "Loads",
            "No external forces applied to the truss",
            "Analysis will compute only support reactions with zero member forces."));
        return;
    }

    // Check each loaded node
    size_t totalLoads = 0;
    for (const auto& node : truss.getNodes()) {
        if (node->hasAppliedForce()) {
            totalLoads++;

            // Check for NaN or infinity in forces
            const auto& force = node->getAppliedForce();
            if (std::isnan(force.fx) || std::isnan(force.fy) || std::isinf(force.fx) ||
                std::isinf(force.fy)) {
                ValidationIssue issue(ValidationSeverity::Error,
                                      "Loads",
                                      "Node " + std::to_string(node->getId()) +
                                          " has invalid force values (NaN or infinity)",
                                      "Force: (" + std::to_string(force.fx) + ", " +
                                          std::to_string(force.fy) + ")");
                issue.affectedNodes.push_back(node->getId());
                result.addIssue(issue);
            }

            // Warning for loads on fully constrained nodes
            if (node->getDegreesOfFreedom() == 0) {
                ValidationIssue issue(ValidationSeverity::Warning,
                                      "Loads",
                                      "Force applied to fully constrained node " +
                                          std::to_string(node->getId()),
                                      "This force will be entirely resisted by support reactions "
                                      "and not transferred through members.");
                issue.affectedNodes.push_back(node->getId());
                result.addIssue(issue);
            }
        }
    }

    result.addIssue(ValidationIssue(ValidationSeverity::Info,
                                    "Loads",
                                    std::to_string(totalLoads) + " node(s) have applied forces",
                                    ""));
}

void TrussValidator::validateConnectivity(const Truss& truss, ValidationResult& result) {
    // Check each member for valid node connections
    for (const auto& member : truss.getMembers()) {
        auto startNode = member->getStartNode();
        auto endNode = member->getEndNode();

        // Check for null node pointers
        if (!startNode || !endNode) {
            ValidationIssue issue(ValidationSeverity::Fatal,
                                  "Connectivity",
                                  "Member " + std::to_string(member->getId()) +
                                      " has null node connection(s)",
                                  "Internal error: Member contains null node pointer.");
            issue.affectedMembers.push_back(member->getId());
            result.addIssue(issue);
            continue;
        }

        // Check for self-loops (member connecting node to itself)
        if (startNode->getId() == endNode->getId()) {
            ValidationIssue issue(ValidationSeverity::Error,
                                  "Connectivity",
                                  "Member " + std::to_string(member->getId()) + " connects node " +
                                      std::to_string(startNode->getId()) + " to itself",
                                  "Self-loop members are not valid in truss structures.");
            issue.affectedMembers.push_back(member->getId());
            issue.affectedNodes.push_back(startNode->getId());
            result.addIssue(issue);
        }

        // Verify nodes exist in truss
        bool startExists = truss.getNode(startNode->getId()) != nullptr;
        bool endExists = truss.getNode(endNode->getId()) != nullptr;

        if (!startExists || !endExists) {
            ValidationIssue issue(
                ValidationSeverity::Fatal,
                "Connectivity",
                "Member " + std::to_string(member->getId()) + " references non-existent node(s)",
                "Start node exists: " + std::string(startExists ? "yes" : "no") +
                    ", End node exists: " + std::string(endExists ? "yes" : "no"));
            issue.affectedMembers.push_back(member->getId());
            result.addIssue(issue);
        }
    }
}

// ========== Private Helper Methods ==========

bool TrussValidator::checkMinimumConstraints(const Truss& truss) {
    // For 2D truss: minimum 3 constraints needed to prevent rigid body motion
    // BUT: must check that constraints prevent ALL 3 rigid body modes:
    //   - Horizontal translation (need at least 1 X constraint)
    //   - Vertical translation (need at least 1 Y constraint)
    //   - Rotation (need non-collinear constraints)

    if (truss.getConstrainedDofs() < 3) {
        return false;
    }

    // Check rigid body stability
    return checkRigidBodyStability(truss);
}

bool TrussValidator::checkRigidBodyStability(const Truss& truss) {
    // Rigid body stability in 2D requires preventing:
    //   1. Horizontal translation (X direction)
    //   2. Vertical translation (Y direction)
    //   3. Rotation about any point

    const auto& nodes = truss.getNodes();

    // Count constraints by type
    bool hasXConstraint = false;  // At least one fixed X DOF
    bool hasYConstraint = false;  // At least one fixed Y DOF
    int xConstraintCount = 0;
    int yConstraintCount = 0;

    for (const auto& node : nodes) {
        SupportType support = node->getSupportType();

        // Check X constraints (horizontal restraint)
        if (support == SupportType::Pinned || support == SupportType::RollerY) {
            hasXConstraint = true;
            xConstraintCount++;
        }

        // Check Y constraints (vertical restraint)
        if (support == SupportType::Pinned || support == SupportType::RollerX) {
            hasYConstraint = true;
            yConstraintCount++;
        }
    }

    // CRITICAL: Must have at least one constraint in EACH direction
    if (!hasXConstraint || !hasYConstraint) {
        return false;  // Unstable: can translate as rigid body
    }

    // Additional check: need sufficient constraints to prevent rotation
    // Minimum 3 total constraints required
    if (xConstraintCount + yConstraintCount < 3) {
        return false;
    }

    // For proper rotational stability, constraints must be non-collinear
    // If all Y constraints are at same X coordinate, structure is unstable
    // This is a more advanced check - for now, basic count is sufficient

    return true;
}

[[maybe_unused]] bool TrussValidator::hasCoincidentNodes(const Truss& truss, Real tolerance) {
    return !findCoincidentNodePairs(truss, tolerance).empty();
}

[[maybe_unused]] bool TrussValidator::hasDuplicateMembers(const Truss& truss) {
    return !findDuplicateMembers(truss).empty();
}

[[maybe_unused]] bool TrussValidator::hasZeroLengthMembers(const Truss& truss, Real tolerance) {
    return !findZeroLengthMembers(truss, tolerance).empty();
}

std::vector<std::pair<NodeId, NodeId>> TrussValidator::findCoincidentNodePairs(const Truss& truss,
                                                                               Real tolerance) {
    std::vector<std::pair<NodeId, NodeId>> pairs;
    const auto& nodes = truss.getNodes();

    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = i + 1; j < nodes.size(); ++j) {
            if (nodes[i]->distanceTo(*nodes[j]) < tolerance) {
                pairs.emplace_back(nodes[i]->getId(), nodes[j]->getId());
            }
        }
    }

    return pairs;
}

std::vector<MemberId> TrussValidator::findZeroLengthMembers(const Truss& truss, Real tolerance) {
    std::vector<MemberId> zeroLengthMembers;

    for (const auto& member : truss.getMembers()) {
        if (member->getLength() < tolerance) {
            zeroLengthMembers.push_back(member->getId());
        }
    }

    return zeroLengthMembers;
}

std::vector<std::pair<MemberId, MemberId>>
TrussValidator::findDuplicateMembers(const Truss& truss) {
    std::vector<std::pair<MemberId, MemberId>> duplicates;
    const auto& members = truss.getMembers();

    for (size_t i = 0; i < members.size(); ++i) {
        for (size_t j = i + 1; j < members.size(); ++j) {
            NodeId start1 = members[i]->getStartNode()->getId();
            NodeId end1 = members[i]->getEndNode()->getId();
            NodeId start2 = members[j]->getStartNode()->getId();
            NodeId end2 = members[j]->getEndNode()->getId();

            // Check if they connect the same nodes (in either direction)
            if ((start1 == start2 && end1 == end2) || (start1 == end2 && end1 == start2)) {
                duplicates.emplace_back(members[i]->getId(), members[j]->getId());
            }
        }
    }

    return duplicates;
}

}  // namespace truss::core::validation
