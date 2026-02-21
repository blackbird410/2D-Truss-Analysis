/**
 * @file ExampleCommand.cpp
 * @brief Implementation of example command
 * 
 * TECHNICAL DEBT:
 * - Includes core/model/Truss.hpp for direct geometry construction
 * - Required by getTrussMutable() which returns Truss& reference
 * - Necessitates access to Domain types: SupportType, MaterialProperties, etc.
 * - FUTURE: Replace with Application-layer Builder API to eliminate this dependency
 */

#include "ExampleCommand.hpp"
#include "../../core/model/Truss.hpp"

namespace truss::cli::commands {

ExampleCommand::ExampleCommand(
    truss::application::TrussApplicationService& trussService,
    truss::application::AnalysisApplicationService& analysisService,
    truss::cli::presenters::ConsolePresenter& presenter,
    bool verbose
) : m_trussService(trussService),
    m_analysisService(analysisService),
    m_presenter(presenter),
    m_verbose(verbose)
{}

int ExampleCommand::execute() {
    using namespace truss::application;
    using namespace truss::core;
    
    m_presenter.displayInfo("Running example truss analysis...\n");
    
    // Create truss via Application service
    auto createResult = m_trussService.createTruss("Example Truss");
    if (!createResult) {
        m_presenter.displayError(createResult.errorMessage);
        return 1;
    }
    TrussHandle handle = createResult.value;
    
    // Build geometry
    auto& truss = m_trussService.getTrussMutable(handle);
    
    // Add nodes
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);   // Fixed support
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerX);  // Roller support (Y-constrained)
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);     // Free node
    
    // Define material properties (steel)
    MaterialProperties steel;
    steel.youngModulus = 200e9; // Pa
    steel.density = 7850;       // kg/m³
    steel.yieldStrength = 250e6; // Pa
    steel.name = "Steel S355";
    
    // Define section properties
    SectionProperties section;
    section.area = 2e-3;        // m² (20 cm²)
    section.designation = "L50x50x5";
    
    // Add members
    truss.addMember(node1, node2, steel, section);
    truss.addMember(node1, node3, steel, section);
    truss.addMember(node2, node3, steel, section);
    
    // Apply loads
    truss.applyForce(node3->getId(), Force2D(0.0, -50000.0));   // 50 kN downward
    
    // Display statistics via view interface
    const truss::core::interfaces::ITrussView& trussView = m_trussService.getTrussView(handle);
    m_presenter.displayTrussStatistics(trussView);
    
    // Validate via Application service
    auto validateResult = m_trussService.validateTruss(handle);
    if (!validateResult) {
        m_presenter.displayError(validateResult.errorMessage);
        m_trussService.clearTruss(handle);
        return 1;
    }
    
    if (!validateResult.value.isValid()) {
        m_presenter.displayError("Truss validation failed");
        m_trussService.clearTruss(handle);
        return 1;
    }
    
    // Analyze via Application service
    m_presenter.displayInfo("Performing structural analysis...");
    
    // Configure analysis options
    truss::core::analysis::AnalysisOptions options;
    options.verbose = m_verbose;
    options.useDirectSolver = true;
    options.computeReactions = true;
    
    auto analysisResult = m_analysisService.analyze(truss, options);
    
    if (!analysisResult) {
        m_presenter.displayError(analysisResult.errorMessage);
        m_trussService.clearTruss(handle);
        return 1;
    }
    
    // Display results via view interface
    const truss::core::interfaces::IAnalysisResultsView& resultsView = 
        m_analysisService.getResultsView(analysisResult.value);
    m_presenter.displayAnalysisResults(resultsView);
    
    m_presenter.displaySuccess("Analysis completed successfully!");
    
    // Cleanup via Application service 
    m_trussService.clearTruss(handle);
    m_analysisService.clearResults(analysisResult.value);
    
    return 0;
}

} // namespace truss::cli::commands
