/**
 * @file main_window_controller.cpp
 * @brief MainWindowController full implementation (Phase 6).
 *
 * Phase 4: Constructor, WorkspaceState accessor / mutator, stateChanged signal.
 * Phase 6: Creates and owns all sub-controllers and Qt Item Models.
 *          Wires inter-controller signals.  Implements full cascade refresh
 *          in onTrussModified and onAnalysisCompleted.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/controllers/main_window_controller.hpp"

#include "gui/controllers/analysis_controller_v2.hpp"
#include "gui/controllers/canvas_controller.hpp"
#include "gui/controllers/export_controller.hpp"
#include "gui/controllers/inspector_controller.hpp"
#include "gui/controllers/project_controller_v2.hpp"
#include "gui/models/member_table_model.hpp"
#include "gui/models/node_table_model.hpp"
#include "gui/models/results_table_model.hpp"
#include "gui/models/validation_list_model.hpp"
#include "interface/itruss_analysis_facade.hpp"

namespace truss::gui::ctrl {

// ============================================================
// Construction
// ============================================================

MainWindowController::MainWindowController(truss::interface::ITrussAnalysisFacade& facade,
                                           QObject* parent)
    : QObject(parent)
    , m_facade(&facade)
    // Sub-controllers
    , m_canvasController   (std::make_unique<CanvasController>   (facade, this))
    , m_inspectorController(std::make_unique<InspectorController>(facade, this))
    , m_analysisController (std::make_unique<AnalysisController> (facade, this))
    , m_projectController  (std::make_unique<ProjectController>  (facade, m_confirmProvider, this))
    , m_exportController   (std::make_unique<ExportController>   (facade, this))
    // Qt Item Models
    , m_nodeModel      (std::make_unique<model::NodeTableModel>      (this))
    , m_memberModel    (std::make_unique<model::MemberTableModel>    (this))
    , m_validationModel(std::make_unique<model::ValidationListModel> (this))
    , m_resultsModel   (std::make_unique<model::ResultsTableModel>   (this))
{
    // ----------------------------------------------------------------
    // Inter-controller signal wiring
    // (Panel ↔ controller wiring is done in MainWindowV2::connectSignals)
    // ----------------------------------------------------------------

    // CanvasController → MainWindowController
    connect(m_canvasController.get(), &CanvasController::trussModified,
            this, &MainWindowController::onTrussModified);

    // InspectorController mutations → MainWindowController
    connect(m_inspectorController.get(), &InspectorController::trussModified,
            this, &MainWindowController::onTrussModified);

    // AnalysisController lifecycle → MainWindowController
    connect(m_analysisController.get(), &AnalysisController::analysisStarted,
            this, &MainWindowController::onAnalysisStarted);
    connect(m_analysisController.get(), &AnalysisController::analysisCompleted,
            this, &MainWindowController::onAnalysisCompleted);
    connect(m_analysisController.get(), &AnalysisController::analysisFailed,
            this, &MainWindowController::onAnalysisFailed);

    // ProjectController → MainWindowController
    connect(m_projectController.get(), &ProjectController::trussCreated,
            this, &MainWindowController::onTrussModified);
    connect(m_projectController.get(), &ProjectController::trussLoaded,
            this, [this](std::size_t handle, const QString& /*path*/) {
                onTrussModified(handle);
            });
    connect(m_projectController.get(), &ProjectController::projectSaved,
            this, &MainWindowController::onProjectSaved);

    // MainWindowController → Sub-controller handle cascades
    // When truss handle changes, all controllers need to know
    connect(this, &MainWindowController::stateChanged,
            this, [this](const state::WorkspaceState& s) {
                const std::size_t h = s.trussHandle;
                m_canvasController->onTrussHandleUpdated(h);
                m_inspectorController->onTrussHandleUpdated(h);
                m_analysisController->onTrussHandleUpdated(h);
                m_projectController->onTrussHandleUpdated(h);
                // Dirty propagation to ProjectController
                m_projectController->setDirty(s.isDirty);
                // Results handle to ExportController
                m_exportController->onResultsHandleUpdated(s.resultsHandle);
            });
}

MainWindowController::~MainWindowController() = default;

// ============================================================
// Accessors — state
// ============================================================

const state::WorkspaceState& MainWindowController::state() const noexcept
{
    return m_state;
}

void MainWindowController::setState(state::WorkspaceState newState)
{
    if (m_state == newState) return;
    m_state = std::move(newState);
    emit stateChanged(m_state);
}

// ============================================================
// Accessors — sub-controllers
// ============================================================

CanvasController*    MainWindowController::canvasController()    const noexcept { return m_canvasController.get(); }
InspectorController* MainWindowController::inspectorController() const noexcept { return m_inspectorController.get(); }
AnalysisController*  MainWindowController::analysisController()  const noexcept { return m_analysisController.get(); }
ProjectController*   MainWindowController::projectController()   const noexcept { return m_projectController.get(); }
ExportController*    MainWindowController::exportController()    const noexcept { return m_exportController.get(); }

// ============================================================
// Accessors — models
// ============================================================

model::NodeTableModel*      MainWindowController::nodeModel()       const noexcept { return m_nodeModel.get(); }
model::MemberTableModel*    MainWindowController::memberModel()     const noexcept { return m_memberModel.get(); }
model::ValidationListModel* MainWindowController::validationModel() const noexcept { return m_validationModel.get(); }
model::ResultsTableModel*   MainWindowController::resultsModel()    const noexcept { return m_resultsModel.get(); }

// ============================================================
// Slots
// ============================================================

void MainWindowController::onTrussModified(std::size_t trussHandle)
{
    state::WorkspaceState next = m_state;
    next.trussHandle   = trussHandle;
    next.phase         = state::WorkspacePhase::ModelBuilding;
    next.resultsHandle = 0;
    next.isDirty       = true;
    next.lastError.clear();

    // Refresh node and member models from the new truss view
    const core::interfaces::ITrussView* viewPtr = nullptr;
    try {
        if (trussHandle != 0) {
            const auto& view = m_facade->getTrussView(trussHandle);
            m_nodeModel->refresh(view);
            m_nodeModel->setHasResults(false);
            m_memberModel->refresh(view);
            m_memberModel->setHasResults(false);
            viewPtr = &view;

            // Run validation and update validation model
            auto valResult = m_facade->validateTruss(trussHandle);
            if (valResult.success) {
                m_validationModel->refresh(valResult.value);
            }
        }
    } catch (...) {
        // getTrussView may throw for invalid handles — silently ignore
    }

    emit trussViewChanged(viewPtr);
    setState(std::move(next));
}

void MainWindowController::onAnalysisStarted()
{
    state::WorkspaceState next = m_state;
    next.phase = state::WorkspacePhase::Analysing;
    next.lastError.clear();
    setState(std::move(next));
}

void MainWindowController::onAnalysisCompleted(std::size_t resultsHandle)
{
    state::WorkspaceState next = m_state;
    next.resultsHandle = resultsHandle;
    next.phase         = state::WorkspacePhase::ResultsReady;
    next.lastError.clear();
    // TODO Phase 6: trigger ResultsTableModel::refresh() and MemberTableModel results refresh
    setState(std::move(next));
}

void MainWindowController::onAnalysisFailed(const QString& errorMessage)
{
    state::WorkspaceState next = m_state;
    next.phase        = state::WorkspacePhase::ModelBuilding;
    next.resultsHandle = 0;
    next.lastError    = errorMessage.toStdString();
    setState(std::move(next));
}

}  // namespace truss::gui::ctrl
