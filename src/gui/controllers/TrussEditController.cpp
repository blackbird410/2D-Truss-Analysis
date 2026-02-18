#include "TrussEditController.hpp"

namespace truss_controllers {

TrussEditController::TrussEditController(
    truss::application::TrussApplicationService& trussService,
    truss_presenters::TrussDataPresenter& presenter,
    QObject* parent)
    : QObject(parent)
    , m_trussService(trussService)
    , m_presenter(presenter)
    , m_currentHandle()
{
}

void TrussEditController::setCurrentTruss(truss::application::TrussHandle handle) {
    m_currentHandle = handle;
}

bool TrussEditController::validateCurrentHandle() {
    if (m_currentHandle == 0) {
        emit operationFailed("No truss is currently loaded");
        return false;
    }
    return true;
}

void TrussEditController::onNodeAddRequested(
    const truss::core::Point2D& position,
    truss::core::SupportType supportType)
{
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.addNode(m_currentHandle, position, supportType);
    
    if (result.success) {
        emit nodeAdded(result.value);
        emit trussModified(m_currentHandle);
        emit statusMessageChanged(m_presenter.formatNodeAddedMessage(result.value, position));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onMemberAddRequested(
    truss::core::NodeId startNodeId,
    truss::core::NodeId endNodeId,
    const truss::application::MaterialSpec& material,
    const truss::application::SectionSpec& section)
{
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.addMember(
        m_currentHandle,
        startNodeId,
        endNodeId,
        material,
        section
    );
    
    if (result.success) {
        emit memberAdded(result.value);
        emit trussModified(m_currentHandle);
        emit statusMessageChanged(m_presenter.formatMemberAddedMessage(result.value, startNodeId, endNodeId));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onNodeRemoveRequested(truss::core::NodeId nodeId) {
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.removeNode(m_currentHandle, nodeId);
    
    if (result.success) {
        emit trussModified(m_currentHandle);
        emit statusMessageChanged(QString("Node %1 removed").arg(nodeId));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onMemberRemoveRequested(truss::core::MemberId memberId) {
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.removeMember(m_currentHandle, memberId);
    
    if (result.success) {
        emit trussModified(m_currentHandle);
        emit statusMessageChanged(QString("Member %1 removed").arg(memberId));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onSupportTypeChanged(
    truss::core::NodeId nodeId,
    truss::core::SupportType supportType)
{
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.setNodeSupport(m_currentHandle, nodeId, supportType);
    
    if (result.success) {
        emit trussModified(m_currentHandle);
        emit statusMessageChanged(m_presenter.formatSupportChangeMessage(nodeId, supportType));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onLoadApplied(
    truss::core::NodeId nodeId,
    const truss::core::Force2D& force)
{
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.applyNodeLoad(m_currentHandle, nodeId, force);
    
    if (result.success) {
        emit trussModified(m_currentHandle);
        emit loadApplied(nodeId, force.fx, force.fy);
        emit statusMessageChanged(m_presenter.formatLoadAppliedMessage(nodeId, force));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onLoadCleared(truss::core::NodeId nodeId) {
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.clearNodeLoad(m_currentHandle, nodeId);
    
    if (result.success) {
        emit trussModified(m_currentHandle);
        emit statusMessageChanged(QString("Load cleared from Node %1").arg(nodeId));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onClearTrussRequested() {
    if (!validateCurrentHandle()) {
        return;
    }
    
    bool result = m_trussService.clearTruss(m_currentHandle);
    
    if (result) {
        emit trussModified(m_currentHandle);
        emit statusMessageChanged("Truss cleared");
    } else {
        emit operationFailed("Failed to clear truss");
    }
}

}  // namespace truss_controllers
