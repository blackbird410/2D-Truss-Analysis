/**
 * @file inspector_controller.hpp
 * @brief Controller mediating between InspectorPanel user actions and
 *        ITrussService facade calls for property changes.
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

#include <QObject>

namespace truss::application { class ITrussService; }

namespace truss::gui::ctrl {

/**
 * @brief Drives InspectorPanel content in response to canvas selection changes
 *        and applies property edits via ITrussService.
 *
 * Workflow:
 *  1. Canvas emits selectionChanged(NodeId/MemberId)
 *  2. InspectorController receives onSelectionChanged, queries getTrussView,
 *     and calls panel->showNodeEditor(nodeView) or showMemberEditor(memberView)
 *  3. Inspector emits loadChangeRequested / memberPropertyChanged
 *  4. InspectorController calls applyNodeLoad / setMemberProperties on service
 *  5. On success emits trussModified; on failure emits operationFailed
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement full selection/edit workflow,
 *       hold current trussHandle as member state.
 */
class InspectorController : public QObject {
public:
    explicit InspectorController(QObject* parent = nullptr) : QObject(parent) {}

    // TODO Phase 5: explicit InspectorController(application::ITrussService* service,
    //                                              QObject* parent = nullptr)
    // TODO Phase 5: public slots:
    //   void onNodeSelectionChanged(std::uint32_t nodeId)
    //   void onMemberSelectionChanged(std::uint32_t memberId)
    //   void onSupportChangeRequested(std::uint32_t nodeId, core::SupportType type)
    //   void onLoadChangeRequested(std::uint32_t nodeId, core::Force2D load)
    //   void onTrussHandleUpdated(std::size_t trussHandle)
    // TODO Phase 5: signals:
    //   void trussModified(std::size_t trussHandle)
    //   void operationFailed(const QString& message)
};

}  // namespace truss::gui::ctrl
