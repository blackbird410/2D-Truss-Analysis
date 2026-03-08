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
    std::vector<NodeId> affectedNodes;       ///< Nodes involved in the issue
    std::vector<MemberId> affectedMembers;   ///< Members involved in the issue

    ValidationIssue(ValidationSeverity sev,
                    std::string cat,
                    std::string msg,
                    std::string detail = "")
        : severity(sev), category(std::move(cat)), message(std::move(msg)),
          technicalDetail(std::move(detail)) {}
};

/**
 * @brief Complete validation result for a truss system
 */
class ValidationResult {
public:
    ValidationResult() = default;

    /**
     * @brief Add a validation issue to the result
     * @param issue The issue to add
     */
    void addIssue(const ValidationIssue& issue) { m_issues.push_back(issue); }

    /**
     * @brief Check if truss passes all critical validations
     * @return true if no errors or fatal issues exist
     */
    bool isValid() const { return !hasErrors() && !hasFatal(); }

    /**
     * @brief Check if any error-level issues exist
     * @return true if errors are present
     */
    bool hasErrors() const { return countBySeverity(ValidationSeverity::Error) > 0; }

    /**
     * @brief Check if any fatal-level issues exist
     * @return true if fatal issues are present
     */
    bool hasFatal() const { return countBySeverity(ValidationSeverity::Fatal) > 0; }

    /**
     * @brief Check if any warning-level issues exist
     * @return true if warnings are present
     */
    bool hasWarnings() const { return countBySeverity(ValidationSeverity::Warning) > 0; }

    /**
     * @brief Get total count of all issues
     * @return Number of issues
     */
    [[maybe_unused]] size_t getIssueCount() const { return m_issues.size(); }

    /**
     * @brief Get all validation issues
     * @return Vector of all issues
     */
    const std::vector<ValidationIssue>& getIssues() const { return m_issues; }

    /**
     * @brief Get issues filtered by severity level
     * @param severity Severity level to filter by
     * @return Vector of matching issues
     */
    [[maybe_unused]] std::vector<ValidationIssue>
    getIssuesBySeverity(ValidationSeverity severity) const;

    /**
     * @brief Get issues filtered by category
     * @param category Category name to filter by
     * @return Vector of matching issues
     */
    [[maybe_unused]] std::vector<ValidationIssue>
    getIssuesByCategory(const std::string& category) const;

    /**
     * @brief Count issues by severity level
     * @param severity Severity level to count
     * @return Number of issues with specified severity
     */
    size_t countBySeverity(ValidationSeverity severity) const;

    /**
     * @brief Get human-readable summary of validation results
     * @return Summary string with issue counts
     */
    std::string getSummary() const;

    /**
     * @brief Get all error-level messages
     * @return Vector of error message strings
     */
    std::vector<std::string> getErrorMessages() const;

    /**
     * @brief Get all warning-level messages
     * @return Vector of warning message strings
     */
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
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateStructuralCompleteness(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate geometric properties
     * Checks: non-zero member lengths, no duplicate nodes, valid coordinates
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateGeometry(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate material properties
     * Checks: positive Young's modulus, positive area, positive density
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateMaterials(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate boundary conditions
     * Checks: adequate constraints, no over-constraints, valid support types
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateBoundaryConditions(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate static determinacy
     * Checks: 2n = m + r equation, proper constraint distribution
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateStaticDeterminacy(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate kinematic stability
     * Checks: no mechanisms, adequate constraints to prevent rigid body motion
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateKinematicStability(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate load definitions
     * Checks: loads applied to existing nodes, non-zero loads, no conflicts
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateLoads(const Truss& truss, ValidationResult& result);

    /**
     * @brief Validate member connectivity
     * Checks: all members connect valid nodes, no self-loops, no duplicate members
     * @param truss The truss to validate
     * @param result Validation result to populate with issues
     */
    static void validateConnectivity(const Truss& truss, ValidationResult& result);

private:
    /**
     * @brief Check if structure has minimum required constraints
     * @param truss The truss to check
     * @return true if minimum constraints are satisfied
     */
    static bool checkMinimumConstraints(const Truss& truss);

    /**
     * @brief Check if structure is stable against rigid body motion
     * @param truss The truss to check
     * @return true if structure is kinematically stable
     */
    static bool checkRigidBodyStability(const Truss& truss);

    /**
     * @brief Check for coincident nodes (nodes at same location)
     * @param truss The truss to check
     * @param tolerance Distance tolerance for coincidence detection
     * @return true if coincident nodes exist
     */
    [[maybe_unused]] static bool hasCoincidentNodes(const Truss& truss,
                                                    Real tolerance = Constants::GEOMETRY_TOLERANCE);

    /**
     * @brief Check for duplicate members (same start/end nodes)
     * @param truss The truss to check
     * @return true if duplicate members exist
     */
    [[maybe_unused]] static bool hasDuplicateMembers(const Truss& truss);

    /**
     * @brief Check for zero-length members
     * @param truss The truss to check
     * @param tolerance Length tolerance for zero detection
     * @return true if zero-length members exist
     */
    [[maybe_unused]] static bool
    hasZeroLengthMembers(const Truss& truss, Real tolerance = Constants::GEOMETRY_TOLERANCE);

    /**
     * @brief Find all pairs of coincident nodes
     * @param truss The truss to analyze
     * @param tolerance Distance tolerance for coincidence detection
     * @return Vector of node ID pairs that are coincident
     */
    static std::vector<std::pair<NodeId, NodeId>> findCoincidentNodePairs(const Truss& truss,
                                                                          Real tolerance);

    /**
     * @brief Find all zero-length members
     * @param truss The truss to analyze
     * @param tolerance Length tolerance for zero detection
     * @return Vector of member IDs with zero or near-zero length
     */
    static std::vector<MemberId> findZeroLengthMembers(const Truss& truss, Real tolerance);

    /**
     * @brief Find all pairs of duplicate members
     * @param truss The truss to analyze
     * @return Vector of member ID pairs that are duplicates
     */
    static std::vector<std::pair<MemberId, MemberId>> findDuplicateMembers(const Truss& truss);
};

}  // namespace truss::core::validation
