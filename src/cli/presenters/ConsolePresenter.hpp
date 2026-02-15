/**
 * @file ConsolePresenter.hpp
 * @brief Console output presenter for CLI
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Formats and displays data to console via view interfaces.
 * Does not contain business logic or orchestration.
 */

#pragma once

#include "../../core/interfaces/ITrussView.hpp"
#include "../../core/interfaces/IAnalysisResultsView.hpp"
#include <string>
#include <iostream>

namespace truss::cli::presenters {

// Use proper namespace for view interfaces
using truss::core::interfaces::ITrussView;
using truss::core::interfaces::IAnalysisResultsView;

/**
 * @brief Handles all formatted console output for CLI
 * 
 * Consumes view interfaces (ITrussView, IAnalysisResultsView)
 * to format and display data without direct Domain access.
 */
class ConsolePresenter {
public:
    ConsolePresenter() = default;
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
};

} // namespace truss::cli::presenters
