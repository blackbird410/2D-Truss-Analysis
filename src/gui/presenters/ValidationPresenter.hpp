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

#include <QString>
#include <vector>
#include "core/validation/TrussValidator.hpp"

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
        QString summaryMessage;           // Overall validation summary
        std::vector<QString> fatalErrors; // Fatal-level issues
        std::vector<QString> errors;      // Error-level issues
        std::vector<QString> warnings;    // Warning-level issues
        std::vector<QString> infos;       // Info-level issues
        bool isValid;                     // Overall validation status
    };
    
    /**
     * @brief Format validation result for display
     * 
     * @param result Validation result from Domain layer
     * @return ValidationDisplay Formatted validation information
     */
    ValidationDisplay formatValidation(const truss::core::validation::ValidationResult& result) const;
    
    /**
     * @brief Generate summary message
     * 
     * @param result Validation result
     * @return QString Summary message
     */
    QString generateSummary(const truss::core::validation::ValidationResult& result) const;
    
    /**
     * @brief Format single validation issue
     * 
     * @param issue Validation issue
     * @return QString Formatted issue string
     */
    QString formatIssue(const truss::core::validation::ValidationIssue& issue) const;
    
    /**
     * @brief Get severity icon/prefix
     * 
     * @param severity Issue severity
     * @return QString Icon or prefix string
     */
    QString getSeverityIcon(truss::core::validation::ValidationSeverity severity) const;

private:
    /**
     * @brief Group issues by severity
     */
    void groupIssuesBySeverity(
        const std::vector<truss::core::validation::ValidationIssue>& issues,
        std::vector<QString>& fatalErrors,
        std::vector<QString>& errors,
        std::vector<QString>& warnings,
        std::vector<QString>& infos) const;
};

}  // namespace truss_presenters
