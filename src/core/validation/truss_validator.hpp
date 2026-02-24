/**
 * @file truss_validator.hpp
 * @brief Validation service for truss structural systems.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This service provides comprehensive validation of truss models,
 * separating validation logic from domain entities following
 * Single Responsibility Principle.
 */

#pragma once

#include "../model/load.hpp"
#include "../model/member.hpp"
#include "../model/node.hpp"
#include "../model/truss.hpp"

#include <memory>
#include <string>
#include <vector>

namespace truss::core::validation {

/**
 * @brief Severity level of validation issues
 */
enum class ValidationSeverity {
    Info,     ///< Informational message
    Warning,  ///< Potential issue but analysis may proceed
    Error,    ///< Critical issue preventing valid analysis
    Fatal     ///< Structural impossibility or data corruption
};

/**
 * @brief Validation result for a single check
 */
struct ValidationIssue {
    ValidationSeverity severity;
    std::string category;                   ///< e.g., "Geometry", "Boundary", "Material"
    std::string message;                    ///< Human-readable description
    std::string technicalDetail;            ///< Technical explanation for engineers
    std::vector<NodeId> affectedNodes;      ///< Nodes involved in the issue
    std::vector<MemberId> affectedMembers;  ///< Members involved in the issue

    ValidationIssue(ValidationSeverity sev,
                    const std::string& cat,
                    const std::string& msg,
                    const std::string& detail = "")
        : severity(sev), category(cat), message(msg), technicalDetail(detail) {}
};

/**
 * @brief Complete validation result for a truss system
 */
class ValidationResult {
public:
    ValidationResult() = default;

    void addIssue(const ValidationIssue& issue) { m_issues.push_back(issue); }

    bool isValid() const { return !hasErrors() && !hasFatal(); }
    bool hasErrors() const { return countBySeverity(ValidationSeverity::Error) > 0; }
    bool hasFatal() const { return countBySeverity(ValidationSeverity::Fatal) > 0; }
    bool hasWarnings() const { return countBySeverity(ValidationSeverity::Warning) > 0; }

    [[maybe_unused]] size_t getIssueCount() const { return m_issues.size(); }
    const std::vector<ValidationIssue>& getIssues() const { return m_issues; }

    [[maybe_unused]] std::vector<ValidationIssue>
    getIssuesBySeverity(ValidationSeverity severity) const;
    [[maybe_unused]] std::vector<ValidationIssue>
    getIssuesByCategory(const std::string& category) const;

    size_t countBySeverity(ValidationSeverity severity) const;

    std::string getSummary() const;
    std::vector<std::string> getErrorMessages() const;
    [[maybe_unused]] std::vector<std::string> getWarningMessages() const;

private:
    std::vector<ValidationIssue> m_issues;
};

/**
 * @brief Comprehensive validation service for truss structural systems
 *
 * Validates all aspects of a truss model before analysis:
 * - Structural completeness (nodes, members exist)
 * - Geometric validity (non-zero member lengths, no coincident nodes)
 * - Material validity (positive properties)
 * - Boundary conditions (adequate constraints)
 * - Static determinacy and stability
 * - Load definitions (valid application points)
 */
class TrussValidator {
public:
    TrussValidator() = default;
    ~TrussValidator() = default;

    /**
     * @brief Perform comprehensive validation of a truss system
     * @param truss The truss to validate
     * @return Complete validation result with all issues found
     */
    static ValidationResult validate(const Truss& truss);

    /**
     * @brief Quick validation check (returns only pass/fail)
     * @param truss The truss to validate
     * @return true if truss passes all critical checks
     */
    static bool isValid(const Truss& truss);

    // Individual validation categories (can be called independently)

    /**
     * @brief Validate structural completeness
     * Checks: minimum node count, minimum member count, no null pointers
     */
    static void validateStructuralCompleteness(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate geometric properties
     * Checks: non-zero member lengths, no duplicate nodes, valid coordinates
     */
    static void validateGeometry(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate material properties
     * Checks: positive Young's modulus, positive area, positive density
     */
    static void validateMaterials(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate boundary conditions
     * Checks: adequate constraints, no over-constraints, valid support types
     */
    static void validateBoundaryConditions(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate static determinacy
     * Checks: 2n = m + r equation, proper constraint distribution
     */
    static void validateStaticDeterminacy(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate kinematic stability
     * Checks: no mechanisms, adequate constraints to prevent rigid body motion
     */
    static void validateKinematicStability(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate load definitions
     * Checks: loads applied to existing nodes, non-zero loads, no conflicts
     */
    static void validateLoads(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate member connectivity
     * Checks: all members connect valid nodes, no self-loops, no duplicate members
     */
    static void validateConnectivity(const Truss& truss, ValidationResult& result);

private:
    // Helper methods for specific checks
    static bool checkMinimumConstraints(const Truss& truss);
    static bool checkRigidBodyStability(const Truss& truss);
    [[maybe_unused]] static bool hasCoincidentNodes(const Truss& truss,
                                                    Real tolerance = Constants::GEOMETRY_TOLERANCE);
    [[maybe_unused]] static bool hasDuplicateMembers(const Truss& truss);
    [[maybe_unused]] static bool
    hasZeroLengthMembers(const Truss& truss, Real tolerance = Constants::GEOMETRY_TOLERANCE);

    static std::vector<std::pair<NodeId, NodeId>> findCoincidentNodePairs(const Truss& truss,
                                                                          Real tolerance);
    static std::vector<MemberId> findZeroLengthMembers(const Truss& truss, Real tolerance);
    static std::vector<std::pair<MemberId, MemberId>> findDuplicateMembers(const Truss& truss);
};

}  // namespace truss::core::validation
