/**
 * @file itruss_analysis_facade.hpp
 * @brief Abstract interface for TrussAnalysisFacade (for adapters and mocks)
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/interfaces/ianalysis_service.hpp"
#include "application/interfaces/itruss_service.hpp"
#include "application/result.hpp"
#include "core/interfaces/ianalysis_results_view.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "core/model/truss.hpp"
#include "truss/analysis/analysis_options.hpp"
#include "truss/export/export_format.hpp"

#include <filesystem>
#include <string>

namespace truss::interface {

/**
 * @brief Result of a complete analysis workflow
 *
 * Encapsulates all handles needed to access analysis results,
 * providing a unified return type for facade workflow methods.
 * Defined here (on the interface) so that all consumers of
 * ITrussAnalysisFacade can use the type without depending on the
 * concrete TrussAnalysisFacade header.
 */
struct AnalysisWorkflowResult {
    bool success{false};
    std::string errorMessage;

    // Resource handles (valid only if success == true)
    application::TrussHandle trussHandle{0};
    application::ResultsHandle resultsHandle{0};

    // Convenience methods
    explicit operator bool() const noexcept { return success; }
    bool isValid() const noexcept { return success; }

    static AnalysisWorkflowResult Success(application::TrussHandle th,
                                          application::ResultsHandle rh) {
        return {true, "", th, rh};
    }

    static AnalysisWorkflowResult Failure(const std::string& error) {
        return {false, error, 0, 0};
    }
};

/**
 * @brief Combined interface for truss and analysis services
 *
 * This interface combines ITrussService and IAnalysisService into a single
 * unified interface. It resolves the diamond inheritance problem by
 * explicitly declaring the clearAll() method once.
 *
 * Note: isValidTrussHandle and isValidResultsHandle are already inherited
 * from the respective base interfaces and do not need redeclaration.
 */
class ITrussAnalysisFacade : public truss::application::ITrussService,
                             public truss::application::IAnalysisService {
public:
    virtual ~ITrussAnalysisFacade() = default;

    // Resolve clearAll() ambiguity from multiple inheritance
    // Both ITrussService and IAnalysisService declare clearAll()
    // Implementations should clear both truss and analysis resources
    virtual void clearAll() = 0;

    // All other methods are inherited from base interfaces:
    // - ITrussService: createTruss, loadTruss, saveTruss, clearTruss,
    //                  isValidTrussHandle, getTrussView, getTrussMutable,
    //                  validateTruss, addNode, addMember, removeNode,
    //                  removeMember, setNodeSupport, applyNodeLoad, clearNodeLoad
    // - IAnalysisService: analyze, getResultsView, exportResults,
    //                     clearResults, isValidResultsHandle
};

}  // namespace truss::interface
