/**
 * @file results_dock_panel.hpp
 * @brief Dockable results panel with tabbed views of analysis output.
 *
 * Phase 5: Full Q_OBJECT implementation with QTabWidget.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "gui/state/workspace_state.hpp"
#include "truss/export/export_format.hpp"

#include <QWidget>

class QTabWidget;
class QTableView;

namespace truss::core::interfaces {
class IAnalysisResultsView;
}

namespace truss::gui::model {
class MemberTableModel;
class NodeTableModel;
class ResultsTableModel;
}  // namespace truss::gui::model

namespace truss::gui {

/**
 * @brief Dockable panel (QDockWidget content widget) containing analysis results.
 *
 * Hosts a QTabWidget with four tabs:
 *  - Tab 0 (Node Results):      QTableView bound to NodeTableModel
 *  - Tab 1 (Member Results):    QTableView bound to MemberTableModel
 *  - Tab 2 (System Summary):    QTableView bound to ResultsTableModel
 *  - Tab 3 (Stiffness Matrix):  QTableWidget, lazily populated on first tab activation
 *
 * All tabs show a placeholder when WorkspaceState::hasResults() == false.
 *
 * The panel takes non-owning pointers to the Phase 3 models; it never
 * deletes them.  The export button on each tab emits exportRequested with
 * an appropriate default format for that tab.
 */
class ResultsDockPanel : public QWidget {
    Q_OBJECT

public:
    explicit ResultsDockPanel(model::NodeTableModel* nodeModel,
                              model::MemberTableModel* memberModel,
                              model::ResultsTableModel* resultsModel,
                              QWidget* parent = nullptr);

public slots:
    void onStateChanged(const truss::gui::state::WorkspaceState& state);

    /**
     * @brief Provide the results view for Stiffness Matrix population.
     *
     * Call with a valid pointer after each successful analysis, and with
     * @c nullptr when results become invalid (new model, analysis failed).
     * The pointer must remain valid until the next @c setResultsView() call.
     *
     * @param results  Non-owning pointer to the analysis results view, or nullptr.
     */
    void setResultsView(const truss::core::interfaces::IAnalysisResultsView* results);

signals:
    /// Emitted from per-tab Export button. @p suggestedFilename has no directory.
    void exportRequested(truss::ExportFormat format, const QString& suggestedFilename);

private slots:
    void onTabChanged(int index);

private:
    void buildNodeResultsTab();
    void buildMemberResultsTab();
    void buildSystemSummaryTab();
    void buildStiffnessMatrixTab();

    // Private helper: lazily build the stiffness matrix table.
    void populateStiffnessMatrix();

    QTabWidget* m_tabs{nullptr};
    QTableView* m_nodeTableView{nullptr};
    QTableView* m_memberTableView{nullptr};
    QTableView* m_resultsTableView{nullptr};
    QTableView* m_stiffnessTableView{nullptr};
    model::NodeTableModel* m_nodeModel{nullptr};
    model::MemberTableModel* m_memberModel{nullptr};
    model::ResultsTableModel* m_resultsModel{nullptr};
    /// Non-owning; valid between setResultsView() calls.
    const truss::core::interfaces::IAnalysisResultsView* m_resultsView{nullptr};
    bool m_stiffnessPopulated{false};
};

}  // namespace truss::gui
