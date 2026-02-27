/**
 * @file validation_presenter.cpp
 * @brief Formats validation results for GUI display.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "validation_presenter.hpp"

#include <QList>

#include <sstream>

namespace truss_presenters {

ValidationPresenter::ValidationDisplay
ValidationPresenter::formatValidation(const truss::core::validation::ValidationResult& result) {
    ValidationDisplay display;

    display.isValid = result.isValid();
    display.summaryMessage = generateSummary(result);

    // Group issues by severity
    groupIssuesBySeverity(
        result.getIssues(), display.fatalErrors, display.errors, display.warnings, display.infos);

    return display;
}

QString
ValidationPresenter::generateSummary(const truss::core::validation::ValidationResult& result) {
    if (result.isValid()) {
        return "Validation passed: Structure is ready for analysis.";
    }

    size_t fatalCount = 0;
    size_t errorCount = 0;
    size_t warningCount = 0;
    size_t infoCount = 0;

    for (const auto& issue : result.getIssues()) {
        switch (issue.severity) {
            case truss::core::validation::ValidationSeverity::Fatal:
                ++fatalCount;
                break;
            case truss::core::validation::ValidationSeverity::Error:
                ++errorCount;
                break;
            case truss::core::validation::ValidationSeverity::Warning:
                ++warningCount;
                break;
            case truss::core::validation::ValidationSeverity::Info:
                ++infoCount;
                break;
        }
    }

    QString summary = "Validation failed: ";

    QStringList parts;
    if (fatalCount > 0) {
        parts << QString("%1 fatal error%2").arg(fatalCount).arg(fatalCount == 1 ? "" : "s");
    }
    if (errorCount > 0) {
        parts << QString("%1 error%2").arg(errorCount).arg(errorCount == 1 ? "" : "s");
    }
    if (warningCount > 0) {
        parts << QString("%1 warning%2").arg(warningCount).arg(warningCount == 1 ? "" : "s");
    }
    if (infoCount > 0) {
        parts << QString("%1 info").arg(infoCount);
    }

    summary += parts.join(", ");
    return summary;
}

QString ValidationPresenter::formatIssue(const truss::core::validation::ValidationIssue& issue) {
    return QString("%1 [%2]: %3")
        .arg(getSeverityIcon(issue.severity))
        .arg(QString::fromStdString(issue.category))
        .arg(QString::fromStdString(issue.message));
}

QString ValidationPresenter::getSeverityIcon(truss::core::validation::ValidationSeverity severity) {
    switch (severity) {
        case truss::core::validation::ValidationSeverity::Fatal:
            return "❌";
        case truss::core::validation::ValidationSeverity::Error:
            return "⚠️";
        case truss::core::validation::ValidationSeverity::Warning:
            return "⚡";
        case truss::core::validation::ValidationSeverity::Info:
            return "ℹ️";
        default:
            return "";
    }
}

void ValidationPresenter::groupIssuesBySeverity(
    const std::vector<truss::core::validation::ValidationIssue>& issues,
    std::vector<QString>& fatalErrors,
    std::vector<QString>& errors,
    std::vector<QString>& warnings,
    std::vector<QString>& infos) {
    for (const auto& issue : issues) {
        QString formatted = formatIssue(issue);

        switch (issue.severity) {
            case truss::core::validation::ValidationSeverity::Fatal:
                fatalErrors.push_back(formatted);
                break;
            case truss::core::validation::ValidationSeverity::Error:
                errors.push_back(formatted);
                break;
            case truss::core::validation::ValidationSeverity::Warning:
                warnings.push_back(formatted);
                break;
            case truss::core::validation::ValidationSeverity::Info:
                infos.push_back(formatted);
                break;
        }
    }
}

}  // namespace truss_presenters
