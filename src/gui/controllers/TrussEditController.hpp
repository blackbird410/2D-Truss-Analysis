/**
 * @file TrussEditController.hpp
 * @brief Controller for coordinating truss editing operations
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This controller mediates between GUI views and the Application layer for truss editing:
 * - Node add/remove operations (Phase 3B)
 * - Member add/remove operations (Phase 3B)
 * - Support type modifications (Phase 3B)
 * - Load application/removal (Phase 3B)
 * - Truss clearing operations
 * 
 * Uses Qt signal/slot mechanism for loose coupling between UI and business logic.
 * 
 * Architecture: GUI Controller Layer (MVP Pattern)
 * Dependencies: TrussApplicationService, TrussDataPresenter
 */

#pragma once

#include <QObject>
#include "application/TrussApplicationService.hpp"
#include "gui/presenters/TrussDataPresenter.hpp"
#include "core/model/Types.hpp"

namespace truss_controllers {

/**
 * @brief Mediates truss editing operations between View and Application layer
 * 
 * This controller handles all truss editing operations (add/remove nodes/members,
 * modify supports and loads) by coordinating with TrussApplicationService.
 * Follows Clean Architecture: Controller orchestrates, does not perform business logic.
 */
class TrussEditController : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Construct TrussEditController
     * 
     * @param trussService Reference to TrussApplicationService
     * @param presenter Reference to TrussDataPresenter for formatting
     * @param parent Qt parent object
     */
    explicit TrussEditController(
        truss::application::TrussApplicationService& trussService,
        truss_presenters::TrussDataPresenter& presenter,
        QObject* parent = nullptr);
    
    /**
     * @brief Set current truss handle
     * 
     * @param handle Handle to current truss being edited
     */
    void setCurrentTruss(truss::application::TrussHandle handle);
    
    /**
     * @brief Get current truss handle
     * 
     * @return truss::application::TrussHandle Current handle (may be invalid)
     */
    truss::application::TrussHandle getCurrentTruss() const { return m_currentHandle; }

public slots:
    /**
     * @brief Handle request to add node
     * 
     * @param position Node position in world coordinates
     * @param supportType Support condition (default: Free)
     */
    void onNodeAddRequested(const truss::core::Point2D& position,
                             truss::core::SupportType supportType = truss::core::SupportType::Free);
    
    /**
     * @brief Handle request to add member
     * 
     * @param startNodeId Start node identifier
     * @param endNodeId End node identifier
     * @param material Material properties
     * @param section Section properties
     */
    void onMemberAddRequested(truss::core::NodeId startNodeId,
                               truss::core::NodeId endNodeId,
                               const truss::core::MaterialProperties& material,
                               const truss::core::SectionProperties& section);
    
    /**
     * @brief Handle request to remove node
     * 
     * @param nodeId Node to remove
     */
    void onNodeRemoveRequested(truss::core::NodeId nodeId);
    
    /**
     * @brief Handle request to remove member
     * 
     * @param memberId Member to remove
     */
    void onMemberRemoveRequested(truss::core::MemberId memberId);
    
    /**
     * @brief Handle request to change node support type
     * 
     * @param nodeId Node to modify
     * @param supportType New support type
     */
    void onSupportTypeChanged(truss::core::NodeId nodeId, truss::core::SupportType supportType);
    
    /**
     * @brief Handle request to apply load to node
     * 
     * @param nodeId Node to apply load
     * @param force Force vector
     */
    void onLoadApplied(truss::core::NodeId nodeId, const truss::core::Force2D& force);
    
    /**
     * @brief Handle request to clear load from node
     * 
     * @param nodeId Node to clear load
     */
    void onLoadCleared(truss::core::NodeId nodeId);
    
    /**
     * @brief Handle request to clear entire truss
     */
    void onClearTrussRequested();

signals:
    /**
     * @brief Emitted when truss is modified
     * 
     * @param handle Handle to modified truss
     */
    void trussModified(truss::application::TrussHandle handle);
    
    /**
     * @brief Emitted when node is added successfully
     * 
     * @param nodeId ID of newly added node
     */
    void nodeAdded(truss::core::NodeId nodeId);
    
    /**
     * @brief Emitted when member is added successfully
     * 
     * @param memberId ID of newly added member
     */
    void memberAdded(truss::core::MemberId memberId);
    
    /**
     * @brief Emitted when operation fails
     * 
     * @param errorMessage User-friendly error message
     */
    void operationFailed(const QString& errorMessage);
    
    /**
     * @brief Emitted to update status bar message
     * 
     * @param message Status message
     */
    void statusMessageChanged(const QString& message);

private:
    truss::application::TrussApplicationService& m_trussService;
    truss_presenters::TrussDataPresenter& m_presenter;
    truss::application::TrussHandle m_currentHandle;
    
    /**
     * @brief Check if current handle is valid
     * 
     * @return true if valid, false otherwise (emits operationFailed)
     */
    bool validateCurrentHandle();
};

}  // namespace truss_controllers
