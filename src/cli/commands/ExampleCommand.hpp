/**
 * @file ExampleCommand.hpp
 * @brief Example command for running hardcoded truss analysis
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Executes a hardcoded 3-member truss analysis demonstrating
 * Application service delegation and result presentation.
 */

#pragma once

#include "ICommand.hpp"
#include "../../application/TrussApplicationService.hpp"
#include "../../application/AnalysisApplicationService.hpp"
#include "../presenters/ConsolePresenter.hpp"

namespace truss::cli::commands {

/**
 * @brief Command to run hardcoded example truss analysis
 * 
 * Demonstrates proper Application service usage:
 * - Creates truss via TrussApplicationService
 * - Performs analysis via AnalysisApplicationService
 * - Displays results via ConsolePresenter
 * - Uses Result<T> for error handling
 */
class ExampleCommand : public ICommand {
public:
    /**
     * @brief Constructor with dependency injection
     * @param trussService Injected truss application service
     * @param analysisService Injected analysis application service
     * @param presenter Injected console presenter
     * @param verbose Enable verbose output
     */
    ExampleCommand(
        truss::application::TrussApplicationService& trussService,
        truss::application::AnalysisApplicationService& analysisService,
        truss::cli::presenters::ConsolePresenter& presenter,
        bool verbose = false
    );
    
    ~ExampleCommand() override = default;
    
    // ICommand interface implementation
    int execute() override;
    std::string getName() const override { return "example"; }
    std::string getDescription() const override {
        return "Run hardcoded example truss analysis (3-member truss)";
    }

private:
    truss::application::TrussApplicationService& m_trussService;
    truss::application::AnalysisApplicationService& m_analysisService;
    truss::cli::presenters::ConsolePresenter& m_presenter;
    bool m_verbose;
};

} // namespace truss::cli::commands
