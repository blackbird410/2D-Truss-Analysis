/**
 * @file inspector_controller.cpp
 * @brief InspectorController implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/inspector_controller.hpp"

#include "application/truss_edit_dtos.hpp"
#include "core/model/types.hpp"
#include "interface/itruss_analysis_facade.hpp"

#include <algorithm>

namespace truss::gui::ctrl {

InspectorController::InspectorController(truss::interface::ITrussAnalysisFacade& facade,
                                         QObject* parent)
    : QObject{parent}, m_facade{facade} {}

void InspectorController::onTrussHandleUpdated(std::size_t trussHandle) {
    m_trussHandle = trussHandle;
}

void InspectorController::onNodeSelectionChanged(NodeId nodeId) {
    if (m_trussHandle == 0)
        return;

    const auto& view = m_facade.getTrussView(m_trussHandle);
    const auto nodes = view.getNodeViews();

    auto it = std::find_if(
        nodes.begin(), nodes.end(), [nodeId](const NodeView& nv) { return nv.id == nodeId; });
    if (it != nodes.end()) {
        emit nodeViewReady(*it);
    }
}

void InspectorController::onMemberSelectionChanged(MemberId memberId) {
    if (m_trussHandle == 0)
        return;

    const auto& view = m_facade.getTrussView(m_trussHandle);
    const auto members = view.getMemberViews();

    auto it = std::find_if(members.begin(), members.end(), [memberId](const MemberView& mv) {
        return mv.id == memberId;
    });
    if (it != members.end()) {
        emit memberViewReady(*it);
    }
}

void InspectorController::onSelectionCleared() {
    emit selectionCleared();
}

void InspectorController::onSupportChangeRequested(NodeId nodeId, SupportType type) {
    if (m_trussHandle == 0)
        return;

    auto result = m_facade.setNodeSupport(m_trussHandle, nodeId, type);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to set support: " + result.errorMessage));
    }
}

void InspectorController::onLoadChangeRequested(NodeId nodeId, Force2D load) {
    if (m_trussHandle == 0)
        return;

    application::Result<bool> result{false, false, "No truss handle"};
    if (load.fx == 0.0 && load.fy == 0.0) {
        result = m_facade.clearNodeLoad(m_trussHandle, nodeId);
    } else {
        result = m_facade.applyNodeLoad(m_trussHandle, nodeId, load);
    }

    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to apply load: " + result.errorMessage));
    }
}

void InspectorController::onNodePositionChangeRequested(NodeId nodeId, truss::core::Point2D pos) {
    if (m_trussHandle == 0) {
        emit operationFailed(QStringLiteral("No active truss."));
        return;
    }

    const application::NodeUpdateSpec update{pos.x, pos.y};
    auto result = m_facade.updateNode(m_trussHandle, nodeId, update);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to update node position: " + result.errorMessage));
    }
}

void InspectorController::onMemberPropertiesChangeRequested(MemberId memberId,
                                                            truss::application::MaterialSpec mat,
                                                            truss::application::SectionSpec sec) {
    if (m_trussHandle == 0) {
        emit operationFailed(QStringLiteral("No active truss."));
        return;
    }

    // Update in-place: member ID and connectivity are preserved.
    const application::MemberUpdateSpec update{mat, sec};
    auto result = m_facade.updateMember(m_trussHandle, memberId, update);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to update member: " + result.errorMessage));
    }
}

}  // namespace truss::gui::ctrl
