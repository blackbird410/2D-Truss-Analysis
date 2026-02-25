/**
 * @file analysis_facade.hpp
 * @brief High-level API for performing structural analysis
 */

#pragma once

#include "analysis_options.hpp"

#include <memory>

namespace truss {

// Forward declarations
class Truss;
struct AnalysisResults;

/**
 * @class AnalysisFacade
 * @brief Simplified interface for performing 2D truss structural analysis
 *
 * This is the primary entry point for analysis operations:
 * 1. Load truss structure
 * 2. Configure analysis options
 * 3. Execute analysis
 * 4. Retrieve results
 *
 * Usage example:
 * @code
 * auto facade = std::make_unique<AnalysisFacade>();
 * auto results = facade->analyze(trussStructure, options);
 * @endcode
 */
class AnalysisFacade {
public:
    virtual ~AnalysisFacade() = default;

    /**
     * Perform structural analysis on a truss
     * @param truss The truss structure to analyze
     * @param options Analysis configuration parameters
     * @return Results containing displacements, forces, and stresses
     * @throws std::exception if analysis fails
     */
    virtual AnalysisResults analyze(const Truss& truss, const AnalysisOptions& options) = 0;
};

}  // namespace truss
