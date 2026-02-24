/**
 * @file truss_edit_controller.hpp
 * @brief GUI controller handling interactive truss editing operations.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/interfaces/itruss_service.hpp"
#include "application/truss_edit_dtos.hpp"
#include "core/model/types.hpp"
#include "gui/presenters/truss_data_presenter.hpp"

#include <QObject>

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
     * @param trussService Pointer to ITrussService interface (enables DI and mocking)
     * @param presenter Reference to TrussDataPresenter for formatting
     * @param parent Qt parent object
     *
     * @throws std::invalid_argument if trussService is nullptr
     */
    explicit TrussEditController(truss::application::ITrussService* trussService,
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
     * @param material Material specification (Application DTO)
     * @param section Section specification (Application DTO)
     */
    void onMemberAddRequested(truss::core::NodeId startNodeId,
                              truss::core::NodeId endNodeId,
                              const truss::application::MaterialSpec& material,
                              const truss::application::SectionSpec& section);

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
     * @brief Emitted when load is applied successfully
     *
     * @param nodeId Node ID where load was applied
     * @param fx Force X component
     * @param fy Force Y component
     */
    void loadApplied(size_t nodeId, double fx, double fy);

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
    truss::application::ITrussService* m_trussService;
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
