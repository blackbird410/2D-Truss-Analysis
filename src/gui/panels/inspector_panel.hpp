/**
 * @file inspector_panel.hpp
 * @brief Context-sensitive inspector panel for node and member property editing.
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

namespace truss::gui {

/**
 * @brief Context-sensitive property editor panel (right side of main window).
 *
 * Uses a QStackedWidget internally with four pages:
 *  - Page 0 (Project Summary): project name, node/member/DOF counts
 *  - Page 1 (Node Editor):     position, support type, applied loads
 *  - Page 2 (Member Editor):   material, section area, read-only analysis output
 *  - Page 3 (Validation):      QListView bound to ValidationListModel
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement all four pages with QStackedWidget,
 *       add showProjectSummary(), showNodeEditor(NodeView), showMemberEditor(MemberView),
 *       showValidationSummary() public slots, emit loadChangeRequested and
 *       memberPropertyChanged signals.
 */
class InspectorPanel : public QWidget {
public:
    explicit InspectorPanel(QWidget* parent = nullptr) : QWidget(parent) {}

    // TODO Phase 5: void showProjectSummary()
    // TODO Phase 5: void showNodeEditor(const core::interfaces::NodeView& node)
    // TODO Phase 5: void showMemberEditor(const core::interfaces::MemberView& member)
    // TODO Phase 5: void showValidationSummary()
    // TODO Phase 5: signals: loadChangeRequested, memberPropertyChanged, supportChangeRequested
};

}  // namespace truss::gui
