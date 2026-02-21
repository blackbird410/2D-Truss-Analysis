/**
 * @file test_analysis.cpp
 * @brief Basic test for the AnalysisEngine
 */

#include "AnalysisEngine.hpp"
#include "Truss.hpp"
#include "src/infrastructure/logging/logger_factory.hpp"
#include <sstream>

using namespace truss::core;

int main() {
    auto logger = truss::infrastructure::logging::LoggerFactory::createConsoleLogger(
        truss::infrastructure::logging::LogLevel::Info,
        true
    );

    try {
        // Create a simple statically determinate truss system
        Truss truss("Test Truss");
        auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);    // Left support (2 constraints)
        auto node2 = truss.addNode(2.0, 0.0, SupportType::RollerY);   // Right support (1 constraint)
        auto node3 = truss.addNode(1.0, 1.0, SupportType::Free);      // Top node (0 constraints)

        // Add members to form a triangle
        truss.addMember(node1, node2);  // Bottom member
        truss.addMember(node1, node3);  // Left member  
        truss.addMember(node2, node3);  // Right member

        truss.applyForce(node3->getId(), Force2D(0.0, -10000.0)); // Apply downward load on the top node

        // Perform analysis
        AnalysisEngine engine;
        auto results = engine.analyze(truss);

        // Output results
        logger->info("Analysis successful!");
        {
            std::ostringstream oss;
            oss << "Maximum displacement: " << results.maxDisplacement;
            logger->info(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "Maximum stress: " << results.maxStress;
            logger->info(oss.str());
        }

    } catch (const std::exception& e) {
        logger->error(std::string("Analysis failed: ") + e.what());
        return 1;
    }
    return 0;
}
