/**
 * @file ValidationPresenter.hpp
 * @brief Presenter for formatting validation results for GUI display
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This presenter formats validation results into user-friendly display messages:
 * - Groups issues by severity (Fatal, Error, Warning, Info)
 * - Generates validation summary messages
 * - Formats individual validation issues with icons
 * - Provides severity icons for visual feedback
 *
 * Architecture: GUI Presentation Layer (MVP Pattern)
 * Dependencies: Domain validation types (ValidationResult, ValidationIssue)
 */

#pragma once

#include "core/validation/truss_validator.hpp"

#include <QString>

#include <vector>

namespace truss_presenters {

/**
 * @brief Formats validation results for display in GUI
 *
 * This presenter handles formatting of validation issues,
 * grouping by severity, and generating user-friendly error messages.
 */
class ValidationPresenter {
public:
    /**
     * @brief Formatted validation display data
     */
    struct ValidationDisplay {
        QString summaryMessage;            // Overall validation summary
        std::vector<QString> fatalErrors;  // Fatal-level issues
        std::vector<QString> errors;       // Error-level issues
        std::vector<QString> warnings;     // Warning-level issues
        std::vector<QString> infos;        // Info-level issues
        bool isValid;                      // Overall validation status
    };

    /**
     * @brief Format validation result for display
     *
     * @param result Validation result from Domain layer
     * @return ValidationDisplay Formatted validation information
     */
    static ValidationDisplay
    formatValidation(const truss::core::validation::ValidationResult& result);

    /**
     * @brief Generate summary message
     *
     * @param result Validation result
     * @return QString Summary message
     */
    static QString generateSummary(const truss::core::validation::ValidationResult& result);

    /**
     * @brief Format single validation issue
     *
     * @param issue Validation issue
     * @return QString Formatted issue string
     */
    static QString formatIssue(const truss::core::validation::ValidationIssue& issue);

    /**
     * @brief Get severity icon/prefix
     *
     * @param severity Issue severity
     * @return QString Icon or prefix string
     */
    static QString getSeverityIcon(truss::core::validation::ValidationSeverity severity);

private:
    /**
     * @brief Group issues by severity
     */
    static void
    groupIssuesBySeverity(const std::vector<truss::core::validation::ValidationIssue>& issues,
                          std::vector<QString>& fatalErrors,
                          std::vector<QString>& errors,
                          std::vector<QString>& warnings,
                          std::vector<QString>& infos);
};

}  // namespace truss_presenters
