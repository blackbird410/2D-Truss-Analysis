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

// Must be included before mock methods so MockTrussAnalysisFacade can inherit.
#include "application/interfaces/ianalysis_service.hpp"
#include "application/interfaces/itruss_service.hpp"
#include "application/result.hpp"
#include "interface/itruss_analysis_facade.hpp"
#include "interface/truss_builder.hpp"

#include <filesystem>
#include <gmock/gmock.h>
#include <string>

namespace truss::test {

/**
 * @brief Mock TrussAnalysisFacade for adapter testing
 *
 * Provides mocked versions of all public Facade methods to enable
 * isolated testing of FacadeTrussServiceAdapter and FacadeAnalysisServiceAdapter.
 */
class MockTrussAnalysisFacade : public truss::interface::ITrussAnalysisFacade {
public:
    ~MockTrussAnalysisFacade() override = default;

    // ---- ITrussService operations ----
    MOCK_METHOD(application::Result<application::TrussHandle>,
                createTruss,
                (const std::string& name),
                (override));

    MOCK_METHOD(application::Result<application::TrussHandle>,
                loadTruss,
                (const std::filesystem::path& filepath),
                (override));

    MOCK_METHOD(application::Result<bool>,
                saveTruss,
                (application::TrussHandle handle,
                 const std::filesystem::path& filepath,
                 bool overwrite),
                (override));

    MOCK_METHOD(bool, clearTruss, (application::TrussHandle handle), (override));

    MOCK_METHOD(bool, isValidTrussHandle, (application::TrussHandle handle), (const, override));

    MOCK_METHOD(const core::interfaces::ITrussView&,
                getTrussView,
                (application::TrussHandle handle),
                (const, override));

    MOCK_METHOD(core::Truss&, getTrussMutable, (application::TrussHandle handle), (override));

    MOCK_METHOD(application::Result<core::validation::ValidationResult>,
                validateTruss,
                (application::TrussHandle handle),
                (override));

    MOCK_METHOD(application::Result<core::NodeId>,
                addNode,
                (application::TrussHandle handle,
                 const core::Point2D& position,
                 core::SupportType supportType),
                (override));

    MOCK_METHOD(application::Result<core::MemberId>,
                addMember,
                (application::TrussHandle handle,
                 core::NodeId startNodeId,
                 core::NodeId endNodeId,
                 const application::MaterialSpec& material,
                 const application::SectionSpec& section),
                (override));

    MOCK_METHOD(application::Result<bool>,
                removeNode,
                (application::TrussHandle handle, core::NodeId nodeId),
                (override));

    MOCK_METHOD(application::Result<bool>,
                removeMember,
                (application::TrussHandle handle, core::MemberId memberId),
                (override));

    MOCK_METHOD(application::Result<bool>,
                setNodeSupport,
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 core::SupportType supportType),
                (override));

    MOCK_METHOD(application::Result<bool>,
                applyNodeLoad,
                (application::TrussHandle handle, core::NodeId nodeId, const core::Force2D& force),
                (override));

    MOCK_METHOD(application::Result<bool>,
                clearNodeLoad,
                (application::TrussHandle handle, core::NodeId nodeId),
                (override));

    MOCK_METHOD(application::Result<bool>,
                updateNode,
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 const application::NodeUpdateSpec& update),
                (override));

    MOCK_METHOD(application::Result<bool>,
                updateMember,
                (application::TrussHandle handle,
                 core::MemberId memberId,
                 const application::MemberUpdateSpec& update),
                (override));

    // ---- IAnalysisService operations ----
    MOCK_METHOD(application::Result<application::ResultsHandle>,
                analyze,
                (const core::Truss& truss, const core::analysis::AnalysisOptions& options),
                (override));

    MOCK_METHOD(const core::interfaces::IAnalysisResultsView&,
                getResultsView,
                (application::ResultsHandle handle),
                (const, override));

    MOCK_METHOD(application::Result<bool>,
                exportResults,
                (application::ResultsHandle handle,
                 truss::ExportFormat format,
                 const std::filesystem::path& filepath,
                 const core::Truss& truss,
                 const infrastructure::export_::ExportOptions& options),
                (override));

    MOCK_METHOD(application::Result<bool>,
                exportResults,
                (application::ResultsHandle handle,
                 const std::filesystem::path& filepath,
                 const core::Truss& truss,
                 const infrastructure::export_::ExportOptions& options),
                (override));

    MOCK_METHOD(bool, clearResults, (application::ResultsHandle handle), (override));

    MOCK_METHOD(bool, isValidResultsHandle, (application::ResultsHandle handle), (const, override));

    // ---- ITrussAnalysisFacade-only operation ----
    // clearAll() resolves the diamond-inheritance ambiguity from ITrussService
    // and IAnalysisService; it is the only additional pure virtual declared by
    // ITrussAnalysisFacade itself.
    MOCK_METHOD(void, clearAll, (), (override));

    // ---- Workflow methods declared on ITrussAnalysisFacade ----
    // These are the high-level orchestration methods consumed by CLI commands.
    // Mocking them here enables full isolation of CLI command tests.
    MOCK_METHOD(truss::interface::AnalysisWorkflowResult,
                analyzeFromFile,
                (const std::filesystem::path& filepath,
                 const core::analysis::AnalysisOptions& options),
                (override));

    MOCK_METHOD(truss::interface::AnalysisWorkflowResult,
                analyzeInteractive,
                (truss::interface::TrussBuilder & builder,
                 const core::analysis::AnalysisOptions& options),
                (override));

    MOCK_METHOD(core::validation::ValidationResult,
                validateFromFile,
                (const std::filesystem::path& filepath),
                (override));

    MOCK_METHOD(bool,
                exportResults,
                (application::ResultsHandle resultsHandle,
                 truss::ExportFormat format,
                 const std::filesystem::path& filepath,
                 const infrastructure::export_::ExportOptions& options),
                (override));

    MOCK_METHOD(bool,
                exportResults,
                (application::ResultsHandle resultsHandle,
                 const std::filesystem::path& filepath,
                 const infrastructure::export_::ExportOptions& options),
                (override));

    // NOTE: clearWorkflow is a concrete helper on TrussAnalysisFacade only —
    // it is NOT part of ITrussAnalysisFacade and must NOT be mocked here.
};

}  // namespace truss::test
