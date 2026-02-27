/**
 * @file member_input_widget.hpp
 * @brief Widget for creating and configuring truss members.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/truss_application_service.hpp"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <memory>

// Forward declarations
namespace truss_controllers {
class TrussEditController;
}

namespace truss::gui {

/**
 * @brief Widget for adding members to the truss structure
 *
 * This widget follows Clean Architecture principles:
 * - Uses dependency injection for services and controllers
 * - Does not own or directly access Domain entities
 * - Delegates mutations to TrussEditController
 * - Emits signals for successful operations
 * - Receives callbacks from controller for operation results
 */
class MemberInputWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct MemberInputWidget with dependency injection
     *
     * @param trussService Application service for truss operations
     * @param editController Controller for coordinating truss edits
     * @param parent Parent widget
     */
    explicit MemberInputWidget(truss::application::TrussApplicationService& trussService,
                               truss_controllers::TrussEditController& editController,
                               QWidget* parent = nullptr);

signals:
    /**
     * @brief Emitted when member is successfully added
     */
    void memberAdded();

public slots:
    /**
     * @brief Handle user request to add member
     *
     * Validates inputs and delegates to controller
     */
    void addMember();

    /**
     * @brief Clear input fields
     */
    void clearInputs();

    /**
     * @brief Update node lists from current truss
     *
     * @param trussHandle Handle to truss for querying nodes
     */
    void updateNodeList(truss::application::TrussHandle trussHandle);

    /**
     * @brief Callback when member is added successfully
     *
     * @param memberId ID of newly added member
     */
    void onMemberAdded(truss::core::MemberId memberId);

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
    truss::application::TrussHandle m_currentTrussHandle;

    // UI Components
    QComboBox* m_startNodeCombo;
    QComboBox* m_endNodeCombo;
    QLineEdit* m_areaEdit;
    QLineEdit* m_youngModulusEdit;
    QLineEdit* m_densityEdit;
    QLineEdit* m_yieldStrengthEdit;
    QPushButton* m_addButton;
    QPushButton* m_clearButton;
};

}  // namespace truss::gui
