/**
 * @file ConsolePresenter.hpp
 * @brief Console output presenter for CLI
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Formats and displays data to console via view interfaces.
 * Does not contain business logic or orchestration.
 *
 * Phase 5A.2: Refactored to use IApplicationOutput abstraction.
 * Eliminates direct console I/O coupling.
 */

#pragma once

#include "../../application/interfaces/iapplication_output.hpp"
#include "../../core/interfaces/ianalysis_results_view.hpp"
#include "../../core/interfaces/itruss_view.hpp"

#include <string>

namespace truss::cli::presenters {

// Use proper namespace for view interfaces
using truss::core::interfaces::IAnalysisResultsView;
using truss::core::interfaces::ITrussView;

/**
 * @brief Handles all formatted console output for CLI
 *
 * Consumes view interfaces (ITrussView, IAnalysisResultsView)
 * to format and display data without direct Domain access.
 *
 * Architecture:
 * - Depends on IApplicationOutput (Application layer abstraction)
 * - Formats data into strings (presentation concern)
 * - Delegates output routing to abstraction (no direct I/O)
 */
class ConsolePresenter {
public:
    /**
     * @brief Construct presenter with output dependency
     * @param output Application output abstraction for message routing
     *
     * The output reference must outlive this presenter instance.
     * Typically injected from composition root (main_app.cpp).
     */
    explicit ConsolePresenter(application::interfaces::IApplicationOutput& output);

    ~ConsolePresenter() = default;

    /**
     * @brief Display application header
     */
    void displayHeader() const;

    /**
     * @brief Display truss statistics via view interface
     * @param trussView Read-only view of truss data
     */
    void displayTrussStatistics(const ITrussView& trussView) const;

    /**
     * @brief Display analysis results via view interface
     * @param resultsView Read-only view of analysis results
     */
    void displayAnalysisResults(const IAnalysisResultsView& resultsView) const;

    /**
     * @brief Display error message to stderr
     * @param message Error message
     */
    void displayError(const std::string& message) const;

    /**
     * @brief Display success message to stdout
     * @param message Success message
     */
    void displaySuccess(const std::string& message) const;

    /**
     * @brief Display info message to stdout
     * @param message Info message
     */
    void displayInfo(const std::string& message) const;

private:
    application::interfaces::IApplicationOutput& m_output;
};

}  // namespace truss::cli::presenters
