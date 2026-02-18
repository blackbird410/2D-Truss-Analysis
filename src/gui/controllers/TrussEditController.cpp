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
    // TODO: Implement in Phase 3B - Add addNode() to TrussApplicationService
    emit operationFailed("Node addition not yet implemented");
    
    /* Phase 3B implementation:
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.addNode(m_currentHandle, position, supportType);
    
    if (result.success) {
        emit nodeAdded(result.value);
        emit trussModified(m_currentHandle);
        
        QString statusMsg = QString("Node %1 added at %2")
            .arg(result.value)
            .arg(m_presenter.formatCoordinate(position));
        emit statusMessageChanged(statusMsg);
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
    */
}

void TrussEditController::onMemberAddRequested(
    truss::core::NodeId startNodeId,
    truss::core::NodeId endNodeId,
    const truss::core::MaterialProperties& material,
    const truss::core::SectionProperties& section)
{
    // TODO: Implement in Phase 3B - Add addMember() to TrussApplicationService
    emit operationFailed("Member addition not yet implemented");
    
    /* Phase 3B implementation:
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
        
        QString statusMsg = QString("Member %1 added (Nodes %2 - %3)")
            .arg(result.value)
            .arg(startNodeId)
            .arg(endNodeId);
        emit statusMessageChanged(statusMsg);
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
    */
}

void TrussEditController::onNodeRemoveRequested(truss::core::NodeId nodeId) {
    // TODO: Implement in Phase 3B - Add removeNode() to TrussApplicationService
    emit operationFailed("Node removal not yet implemented");
    
    /* Phase 3B implementation:
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
    */
}

void TrussEditController::onMemberRemoveRequested(truss::core::MemberId memberId) {
    // TODO: Implement in Phase 3B - Add removeMember() to TrussApplicationService
    emit operationFailed("Member removal not yet implemented");
    
    /* Phase 3B implementation:
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
    */
}

void TrussEditController::onSupportTypeChanged(
    truss::core::NodeId nodeId,
    truss::core::SupportType supportType)
{
    // TODO: Implement in Phase 3B - Add setNodeSupport() to TrussApplicationService
    emit operationFailed("Support type change not yet implemented");
    
    /* Phase 3B implementation:
    if (!validateCurrentHandle()) {
        return;
    }
    
    auto result = m_trussService.setNodeSupport(m_currentHandle, nodeId, supportType);
    
    if (result.success) {
        emit trussModified(m_currentHandle);
        
        QString statusMsg = QString("Node %1 support changed to %2")
            .arg(nodeId)
            .arg(m_presenter.formatSupportType(supportType));
        emit statusMessageChanged(statusMsg);
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
    */
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
        
        QString statusMsg = QString("Load applied to Node %1: (%2, %3) N")
            .arg(nodeId)
            .arg(force.fx, 0, 'f', 1)
            .arg(force.fy, 0, 'f', 1);
        emit statusMessageChanged(statusMsg);
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onLoadCleared(truss::core::NodeId nodeId) {
    // TODO: Implement in Phase 3B - Add clearNodeLoad() to TrussApplicationService
    emit operationFailed("Load clearing not yet implemented");
    
    /* Phase 3B implementation:
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
    */
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
