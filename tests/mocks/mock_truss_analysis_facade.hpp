/**
 * @file mock_truss_analysis_facade.hpp
 * @brief GoogleMock for TrussAnalysisFacade
 * @version 3.0.0
 * @date 2026-02-28
 * @author Neil Taison Rigaud
 *
 * Mock facade for testing adapters in isolation.
 * This allows verifying that adapters correctly delegate to the facade
 * without needing the full facade implementation.
 */

#pragma once

#include "application/result.hpp"
#include "application/interfaces/ianalysis_service.hpp"
#include "application/interfaces/itruss_service.hpp"

#include <gmock/gmock.h>
#include <filesystem>
#include <string>

namespace truss::test {

/**
 * @brief Mock TrussAnalysisFacade for adapter testing
 *
 * Provides mocked versions of all public Facade methods to enable
 * isolated testing of FacadeTrussServiceAdapter and FacadeAnalysisServiceAdapter.
 */
class MockTrussAnalysisFacade {
public:
    virtual ~MockTrussAnalysisFacade() = default;

    // ITrussService operations
    MOCK_METHOD(application::Result<application::TrussHandle>,
                createTruss,
                (const std::string& name),
                ());

    MOCK_METHOD(application::Result<application::TrussHandle>,
                loadTruss,
                (const std::filesystem::path& filepath),
                ());

    MOCK_METHOD(application::Result<bool>,
                saveTruss,
                (application::TrussHandle handle,
                 const std::filesystem::path& filepath,
                 bool overwrite),
                ());

    MOCK_METHOD(bool, clearTruss, (application::TrussHandle handle), ());

    MOCK_METHOD(bool, isValidTrussHandle, (application::TrussHandle handle), (const));

    MOCK_METHOD(const core::interfaces::ITrussView&,
                getTrussView,
                (application::TrussHandle handle),
                (const));

    MOCK_METHOD(core::Truss&, getTrussMutable, (application::TrussHandle handle), ());

    MOCK_METHOD(application::Result<core::validation::ValidationResult>,
                validateTruss,
                (application::TrussHandle handle),
                ());

    MOCK_METHOD(application::Result<core::NodeId>,
                addNode,
                (application::TrussHandle handle,
                 const core::Point2D& position,
                 core::SupportType supportType),
                ());

    MOCK_METHOD(application::Result<core::MemberId>,
                addMember,
                (application::TrussHandle handle,
                 core::NodeId startNodeId,
                 core::NodeId endNodeId,
                 const application::MaterialSpec& material,
                 const application::SectionSpec& section),
                ());

    MOCK_METHOD(application::Result<bool>,
                removeNode,
                (application::TrussHandle handle, core::NodeId nodeId),
                ());

    MOCK_METHOD(application::Result<bool>,
                removeMember,
                (application::TrussHandle handle, core::MemberId memberId),
                ());

    MOCK_METHOD(application::Result<bool>,
                setNodeSupport,
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 core::SupportType supportType),
                ());

    MOCK_METHOD(application::Result<bool>,
                applyNodeLoad,
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 const core::Force2D& force),
                ());

    MOCK_METHOD(application::Result<bool>,
                clearNodeLoad,
                (application::TrussHandle handle, core::NodeId nodeId),
                ());

    // IAnalysisService operations
    MOCK_METHOD(application::Result<application::ResultsHandle>,
                analyze,
                (const core::Truss& truss, const core::analysis::AnalysisOptions& options),
                ());

    MOCK_METHOD(const core::interfaces::IAnalysisResultsView&,
                getResultsView,
                (application::ResultsHandle handle),
                (const));

    MOCK_METHOD(application::Result<bool>,
                exportResults,
                (application::ResultsHandle handle,
                 truss::ExportFormat format,
                 const std::filesystem::path& filepath,
                 const core::Truss& truss,
                 const infrastructure::export_::ExportOptions& options),
                ());

    MOCK_METHOD(application::Result<bool>,
                exportResults,
                (application::ResultsHandle handle,
                 const std::filesystem::path& filepath,
                 const core::Truss& truss,
                 const infrastructure::export_::ExportOptions& options),
                ());

    MOCK_METHOD(bool, clearResults, (application::ResultsHandle handle), ());

    MOCK_METHOD(bool, isValidResultsHandle, (application::ResultsHandle handle), (const));

    // Resource management
    MOCK_METHOD(void, clearAll, (), ());

    MOCK_METHOD(void,
                clearWorkflow,
                (application::TrussHandle trussHandle, application::ResultsHandle resultsHandle),
                ());
};

}  // namespace truss::test
