/**
 * @file example_command.cpp
 * @brief CLI command for generating example truss models.
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 *
 * REFACTORING NOTE:
 * - Now uses TrussBuilder fluent API for truss construction
 * - Uses TrussAnalysisFacade for analysis orchestration
 * - Eliminates direct core/model/truss.hpp dependency
 * - Demonstrates best practices for Interface layer usage
 */

#include "example_command.hpp"

namespace truss::cli::commands {

ExampleCommand::ExampleCommand(truss::interface::TrussAnalysisFacade& facade,
                               truss::cli::presenters::ConsolePresenter& presenter,
                               bool verbose)
    : m_facade(facade), m_presenter(presenter), m_verbose(verbose) {}

int ExampleCommand::execute() {
    using namespace truss::interface;
    using namespace truss::core;

    m_presenter.displayHeader();
    m_presenter.displayInfo("Running example 3-member truss analysis...\n");

    // Build truss using fluent interface (no direct core API usage)
    TrussBuilder builder("Example 3-Member Truss");
    builder.setName("Example 3-Member Truss")
        .addNode(0.0, 0.0, SupportType::Pinned)   // Node 1: Fixed support
        .addNode(4.0, 0.0, SupportType::RollerX)  // Node 2: Roller support
        .addNode(2.0, 3.0, SupportType::Free)     // Node 3: Free node
        .addMember(NodeId(1), NodeId(2))          // Member 1-2
        .addMember(NodeId(1), NodeId(3))          // Member 1-3
        .addMember(NodeId(2), NodeId(3))          // Member 2-3
        .applyForce(NodeId(3), 0.0, -50000.0);    // 50 kN downward load

    if (m_verbose) {
        m_presenter.displayInfo("Truss structure created using fluent builder API.\n");
    }

    // Analyze using facade
    m_presenter.displayInfo("Performing structural analysis...\n");
    auto analysisResult = m_facade.analyzeInteractive(builder);

    if (!analysisResult) {
        m_presenter.displayError("Analysis failed: " + analysisResult.errorMessage);
        return 1;
    }

    if (m_verbose) {
        m_presenter.displaySuccess("Analysis completed successfully.");
    }

    // Display results
    const auto& resultsView = m_facade.getResultsView(analysisResult.resultsHandle);
    m_presenter.displayAnalysisResults(resultsView);

    m_presenter.displaySuccess("\nExample analysis complete!");
    return 0;
}

}  // namespace truss::cli::commands
