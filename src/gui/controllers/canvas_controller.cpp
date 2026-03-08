/**
 * @file canvas_controller.cpp
 * @brief CanvasController implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/canvas_controller.hpp"

#include "application/truss_edit_dtos.hpp"
#include "interface/itruss_analysis_facade.hpp"

namespace truss::gui::ctrl {

CanvasController::CanvasController(truss::interface::ITrussAnalysisFacade& facade, QObject* parent)
    : QObject{parent}, m_facade{facade} {}

void CanvasController::onTrussHandleUpdated(std::size_t trussHandle) {
    m_trussHandle = trussHandle;
}

void CanvasController::onNodeDropRequested(truss::core::Point2D pos,
                                           truss::core::SupportType support) {
    if (m_trussHandle == 0) {
        emit operationFailed(QStringLiteral("No active truss: create or open a project first."));
        return;
    }

    auto result = m_facade.addNode(m_trussHandle, pos, support);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(QString::fromStdString("Failed to add node: " + result.errorMessage));
    }
}

void CanvasController::onMemberDrawRequested(truss::core::NodeId startId,
                                             truss::core::NodeId endId) {
    if (m_trussHandle == 0) {
        emit operationFailed(QStringLiteral("No active truss: create or open a project first."));
        return;
    }

    // Use the current default material / section (updated via onDefaultMaterialChanged).
    auto result = m_facade.addMember(m_trussHandle, startId, endId, m_currentMat, m_currentSec);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to add member: " + result.errorMessage));
    }
}

void CanvasController::onNodeDeleteRequested(truss::core::NodeId id) {
    if (m_trussHandle == 0)
        return;

    auto result = m_facade.removeNode(m_trussHandle, id);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to remove node: " + result.errorMessage));
    }
}

void CanvasController::onMemberDeleteRequested(truss::core::MemberId id) {
    if (m_trussHandle == 0)
        return;

    auto result = m_facade.removeMember(m_trussHandle, id);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to remove member: " + result.errorMessage));
    }
}

void CanvasController::onDefaultMaterialChanged(truss::application::MaterialSpec mat,
                                                truss::application::SectionSpec sec) {
    m_currentMat = mat;
    m_currentSec = sec;
}

}  // namespace truss::gui::ctrl
