/**
 * @file inspector_controller.hpp
 * @brief Controller mediating InspectorPanel user actions and
 *        ITrussAnalysisFacade calls for property changes.
 *
 * Phase 5: Full Q_OBJECT implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"

#include <QObject>
#include <QString>

#include <cstddef>

namespace truss::interface {
class ITrussAnalysisFacade;
}

namespace truss::gui::ctrl {

using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;
using truss::core::Force2D;
using truss::core::MemberId;
using truss::core::NodeId;
using truss::core::SupportType;

/**
 * @brief Drives InspectorPanel content in response to canvas selection changes
 *        and applies property edits via ITrussAnalysisFacade.
 *
 * Workflow:
 *  1. Canvas emits selectionChanged(NodeId / MemberId).
 *  2. InspectorController queries getTrussView and emits nodeViewReady / memberViewReady.
 *  3. InspectorPanel emits supportChangeRequested / loadChangeRequested.
 *  4. InspectorController calls setNodeSupport / applyNodeLoad on the facade.
 *  5. On success emits trussModified; on failure emits operationFailed.
 */
class InspectorController : public QObject {
    Q_OBJECT

public:
    explicit InspectorController(truss::interface::ITrussAnalysisFacade& facade,
                                  QObject*                                 parent = nullptr);

public slots:
    void onNodeSelectionChanged(NodeId nodeId);
    void onMemberSelectionChanged(MemberId memberId);
    void onSelectionCleared();
    void onSupportChangeRequested(NodeId nodeId, SupportType type);
    void onLoadChangeRequested(NodeId nodeId, Force2D load);
    /// Update the active truss handle (called by MainWindowController).
    void onTrussHandleUpdated(std::size_t trussHandle);

signals:
    void nodeViewReady(NodeView node);
    void memberViewReady(MemberView member);
    void selectionCleared();
    void trussModified(std::size_t trussHandle);
    void operationFailed(const QString& message);

private:
    truss::interface::ITrussAnalysisFacade& m_facade;
    std::size_t                              m_trussHandle{0};
};

}  // namespace truss::gui::ctrl
