/**
 * @file NodeInputWidget.hpp
 * @brief Widget for node input operations (Clean Architecture compliant)
 */

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include "application/TrussApplicationService.hpp"
#include <memory>

// Forward declarations
namespace truss_controllers {
    class TrussEditController;
}

namespace truss::gui {

/**
 * @brief Widget for adding nodes to the truss structure
 * 
 * This widget follows Clean Architecture principles:
 * - Uses dependency injection for services and controllers
 * - Does not own or directly access Domain entities
 * - Delegates mutations to TrussEditController
 * - Emits signals for successful operations
 * - Receives callbacks from controller for operation results
 */
class NodeInputWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct NodeInputWidget with dependency injection
     * 
     * @param trussService Application service for truss operations
     * @param editController Controller for coordinating truss edits
     * @param parent Parent widget
     */
    explicit NodeInputWidget(
        truss::application::TrussApplicationService& trussService,
        truss_controllers::TrussEditController& editController,
        QWidget *parent = nullptr);

signals:
    /**
     * @brief Emitted when node is successfully added
     */
    void nodeAdded();

public slots:
    /**
     * @brief Handle user request to add node
     * 
     * Validates inputs and delegates to controller
     */
    void addNode();
    
    /**
     * @brief Clear input fields
     */
    void clearInputs();
    
    /**
     * @brief Callback when node is added successfully
     * 
     * @param nodeId ID of newly added node
     */
    void onNodeAdded(truss::core::NodeId nodeId);
    
    /**
     * @brief Callback when operation fails
     * 
     * @param errorMessage User-friendly error message
     */
    void onOperationFailed(const QString& errorMessage);

private:
    /**
     * @brief Initialize UI components and layout
     */
    void setupUI();
    
    // Dependencies (Clean Architecture)
    truss::application::TrussApplicationService& m_trussService;
    truss_controllers::TrussEditController& m_editController;
    
    // UI Components
    QLineEdit* m_xCoordEdit;
    QLineEdit* m_yCoordEdit;
    QComboBox* m_supportTypeCombo;
    QPushButton* m_addButton;
    QPushButton* m_clearButton;
};

} // namespace truss::gui
