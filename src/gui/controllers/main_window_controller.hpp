/**
 * @file main_window_controller.hpp
 * @brief Top-level orchestrator controller: owns WorkspaceState, all sub-controllers,
 *        and all Qt Item Models.  Receives domain events and cascades model refreshes.
 *
 * Phase 4: Q_OBJECT, WorkspaceState ownership, stateChanged signal.
 * Phase 6: Sub-controller creation + ownership, model refresh cascade,
 *          full onTrussModified / onAnalysisCompleted / onAnalysisFailed
 *          implementations with model refresh.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "gui/interfaces/iconfirmation_provider.hpp"
#include "gui/state/workspace_state.hpp"

#include <QObject>
#include <QString>

#include <memory>

namespace truss::interface { class ITrussAnalysisFacade; }

namespace truss::core::interfaces {
class ITrussView;
class IAnalysisResultsView;
}

namespace truss::core::validation { class ValidationResult; }

namespace truss::gui::ctrl {
class CanvasController;
class InspectorController;
class AnalysisController;
class ProjectController;
class ExportController;
}

namespace truss::gui::model {
class NodeTableModel;
class MemberTableModel;
class ValidationListModel;
class ResultsTableModel;
}

namespace truss::gui::ctrl {

/**
 * @brief Owns WorkspaceState, all sub-controllers, and all Qt Item Models.
 *
 * MainWindowController is the composition root for the entire new GUI
 * architecture.  On construction it:
 * - Creates all five sub-controllers (CanvasController, InspectorController,
 *   AnalysisController, ProjectController, ExportController)
 * - Creates all four Qt Item Models (NodeTableModel, MemberTableModel,
 *   ValidationListModel, ResultsTableModel)
 * - Wires all inter-controller signal/slot connections
 *
 * Panels (InspectorPanel, AnalysisControlBar, ResultsDockPanel) receive model
 * pointers and controller pointers via accessors exposed from this class.
 * MainWindowV2 is responsible for wiring panel ↔ controller signals.
 *
 * Design constraints:
 * - setState() deep-compares old and new state; only emits stateChanged when
 *   the value actually changes.
 * - The confirmation provider is owned by this controller (ModalConfirmProvider).
 */
class MainWindowController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Construct with a mandatory facade reference.
     *
     * Creates all sub-controllers and models; wires inter-controller signals.
     *
     * @param facade  Application facade; must outlive this controller.
     * @param parent  Qt parent for memory management.
     */
    explicit MainWindowController(truss::interface::ITrussAnalysisFacade& facade,
                                  QObject* parent = nullptr);

    ~MainWindowController() override;

    // Disable copy and move — controller has identity semantics
    MainWindowController(const MainWindowController&)            = delete;
    MainWindowController& operator=(const MainWindowController&) = delete;
    MainWindowController(MainWindowController&&)                  = delete;
    MainWindowController& operator=(MainWindowController&&)       = delete;

    // -----------------------------------------------------------------------
    // State accessors
    // -----------------------------------------------------------------------

    /// @brief Read-only access to the current workspace state.
    [[nodiscard]] const state::WorkspaceState& state() const noexcept;

    /**
     * @brief Replace the workspace state and emit @c stateChanged if it changed.
     *
     * @param newState  Desired new state.
     */
    void setState(state::WorkspaceState newState);

    // -----------------------------------------------------------------------
    // Sub-controller accessors
    // -----------------------------------------------------------------------

    [[nodiscard]] CanvasController*    canvasController()    const noexcept;
    [[nodiscard]] InspectorController* inspectorController() const noexcept;
    [[nodiscard]] AnalysisController*  analysisController()  const noexcept;
    [[nodiscard]] ProjectController*   projectController()   const noexcept;
    [[nodiscard]] ExportController*    exportController()    const noexcept;

    // -----------------------------------------------------------------------
    // Qt Item Model accessors (non-owning pointers — lifetime is this object)
    // -----------------------------------------------------------------------

    [[nodiscard]] model::NodeTableModel*       nodeModel()        const noexcept;
    [[nodiscard]] model::MemberTableModel*     memberModel()      const noexcept;
    [[nodiscard]] model::ValidationListModel*  validationModel()  const noexcept;
    [[nodiscard]] model::ResultsTableModel*    resultsModel()     const noexcept;

signals:
    /**
     * @brief Emitted whenever the workspace state transitions to a new value.
     */
    void stateChanged(const truss::gui::state::WorkspaceState& newState);

    /**
     * @brief Emitted after model refresh; carries a pointer to the new truss view.
     *
     * Connected to TrussCanvasWidget::refresh().  The pointer is valid until the
     * next call to onTrussModified or until the truss handle is cleared.
     */
    void trussViewChanged(const truss::core::interfaces::ITrussView* view);

public slots:
    /**
     * @brief Called when the structural model has been modified.
     *
     * Transitions to ModelBuilding, refreshes NodeTableModel and
     * MemberTableModel, runs validation, and emits trussViewChanged.
     *
     * @param trussHandle  Handle to the modified truss.
     */
    void onTrussModified(std::size_t trussHandle);

    /**
     * @brief Called when background analysis starts (from AnalysisController).
     *
     * Transitions the phase to @c Analysing.
     */
    void onAnalysisStarted();

    /**
     * @brief Called when a background analysis has completed successfully.
     *
     * Transitions to ResultsReady, refreshes ResultsTableModel and
     * MemberTableModel results columns.
     *
     * @param resultsHandle  Handle to the completed analysis results.
     */
    void onAnalysisCompleted(std::size_t resultsHandle);

    /**
     * @brief Called when a background analysis has failed.
     *
     * Transitions back to ModelBuilding and stores the error message.
     *
     * @param errorMessage  Human-readable error description.
     */
    void onAnalysisFailed(const QString& errorMessage);

private:
    truss::interface::ITrussAnalysisFacade* m_facade{nullptr};
    state::WorkspaceState                   m_state;

    // Confirmation provider (owned; used by ProjectController)
    truss::gui::interfaces::ModalConfirmProvider m_confirmProvider;

    // Sub-controllers (owned)
    std::unique_ptr<CanvasController>    m_canvasController;
    std::unique_ptr<InspectorController> m_inspectorController;
    std::unique_ptr<AnalysisController>  m_analysisController;
    std::unique_ptr<ProjectController>   m_projectController;
    std::unique_ptr<ExportController>    m_exportController;

    // Qt Item Models (owned)
    std::unique_ptr<model::NodeTableModel>       m_nodeModel;
    std::unique_ptr<model::MemberTableModel>     m_memberModel;
    std::unique_ptr<model::ValidationListModel>  m_validationModel;
    std::unique_ptr<model::ResultsTableModel>    m_resultsModel;
};

}  // namespace truss::gui::ctrl
