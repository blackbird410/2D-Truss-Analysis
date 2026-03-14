/**
 * @file example_command.hpp
 * @brief Example command for running hardcoded truss analysis.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Executes a hardcoded 3-member truss analysis demonstrating
 * Interface layer facade and fluent builder pattern usage.
 */

#pragma once

#include "../../interface/itruss_analysis_facade.hpp"
#include "../../interface/truss_builder.hpp"
#include "../presenters/console_presenter.hpp"
#include "icommand.hpp"

namespace truss::cli::commands {

/**
 * @brief Command to run hardcoded example truss analysis
 *
 * Demonstrates proper Interface layer usage:
 * - Creates truss via TrussBuilder fluent API
 * - Performs analysis via ITrussAnalysisFacade
 * - Displays results via ConsolePresenter
 * - Uses Result<T> for error handling
 * - NO direct core API usage
 */
class ExampleCommand : public ICommand {
public:
    /**
     * @brief Constructor with dependency injection
     * @param facade Injected truss analysis facade
     * @param presenter Injected console presenter
     * @param verbose Enable verbose output
     */
    ExampleCommand(truss::interface::ITrussAnalysisFacade& facade,
                   truss::cli::presenters::ConsolePresenter& presenter,
                   bool verbose = false);

    ~ExampleCommand() override = default;

    // ICommand interface implementation
    int execute() override;
    std::string getName() const override { return "example"; }
    std::string getDescription() const override {
        return "Run hardcoded example truss analysis (3-member truss)";
    }

private:
    truss::interface::ITrussAnalysisFacade& m_facade;
    truss::cli::presenters::ConsolePresenter& m_presenter;
    bool m_verbose;
};

}  // namespace truss::cli::commands
