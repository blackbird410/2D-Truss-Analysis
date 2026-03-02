/**
 * @file results_dock_panel.hpp
 * @brief Dockable results panel with tabbed views of analysis output.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 5.
 *
 * @note Q_OBJECT is added in Phase 5.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QWidget>

namespace truss::gui::model {
    class NodeTableModel;
    class MemberTableModel;
}

namespace truss::gui {

/**
 * @brief Dockable panel (QDockWidget content widget) containing analysis results.
 *
 * Hosts a QTabWidget with four tabs:
 *  - Tab 0 (Node Results):      QTableView bound to NodeTableModel
 *  - Tab 1 (Member Results):    QTableView bound to MemberTableModel + UtilisationDelegate
 *  - Tab 2 (System Summary):    QFormLayout with scalar metrics
 *  - Tab 3 (Stiffness Matrix):  QTableWidget, lazily populated on first tab activation
 *
 * All tabs show a "Run analysis to see results" placeholder when no results
 * handle is available (WorkspaceState::hasResults() == false).
 *
 * @todo Phase 5: Add Q_OBJECT macro, construct QTabWidget, bind table views to
 *       models passed in constructor, implement lazy stiffness matrix loading,
 *       emit exportRequested(format, suggestedFilename) per tab.
 */
class ResultsDockPanel : public QWidget {
public:
    explicit ResultsDockPanel(QWidget* parent = nullptr) : QWidget(parent) {}

    // Constructor signature (Phase 5):
    // ResultsDockPanel(model::NodeTableModel* nodeModel,
    //                  model::MemberTableModel* memberModel,
    //                  QWidget* parent = nullptr)

    // TODO Phase 5: public slot: void onStateChanged(const state::WorkspaceState& state)
    // TODO Phase 5: signals: exportRequested(ExportFormat format, QString suggestedFilename)
};

}  // namespace truss::gui
