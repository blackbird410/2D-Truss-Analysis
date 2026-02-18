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
        
        QString statusMsg = QString("Node %1 added at (%2, %3)")
            .arg(result.value)
            .arg(position.x, 0, 'f', 3)
            .arg(position.y, 0, 'f', 3);
        emit statusMessageChanged(statusMsg);
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void TrussEditController::onMemberAddRequested(
    truss::core::NodeId startNodeId,
    truss::core::NodeId endNodeId,
    const truss::core::MaterialProperties& material,
    const truss::core::SectionProperties& section)
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
        
        QString statusMsg = QString("Member %1 added (Nodes %2 - %3)")
            .arg(result.value)
            .arg(startNodeId)
            .arg(endNodeId);
        emit statusMessageChanged(statusMsg);
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
        
        QString supportTypeStr;
        switch (supportType) {
            case truss::core::SupportType::Free: supportTypeStr = "Free"; break;
            case truss::core::SupportType::Pinned: supportTypeStr = "Pinned"; break;
            case truss::core::SupportType::PinnedX: supportTypeStr = "Pinned X"; break;
            case truss::core::SupportType::PinnedY: supportTypeStr = "Pinned Y"; break;
            case truss::core::SupportType::RollerX: supportTypeStr = "Roller X"; break;
            case truss::core::SupportType::RollerY: supportTypeStr = "Roller Y"; break;
        }
        
        QString statusMsg = QString("Node %1 support changed to %2")
            .arg(nodeId)
            .arg(supportTypeStr);
        emit statusMessageChanged(statusMsg);
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
