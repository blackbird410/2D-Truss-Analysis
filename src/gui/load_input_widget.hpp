/**
 * @file LoadInputWidget.hpp
 * @brief Widget for applying loads to nodes
 * @version 3.0.0
 */

#pragma once

#include "application/truss_application_service.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

namespace truss_controllers {
class TrussEditController;
}

namespace truss::gui {
/**
 * @brief Widget for load input
 *
 * Provides UI for applying forces to nodes.
 * Uses TrussEditController for load application (Clean Architecture).
 */
class LoadInputWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct LoadInputWidget with dependency injection
     *
     * @param trussService Service for truss data access
     * @param editController Controller for load operations
     * @param parent Qt parent widget
     */
    explicit LoadInputWidget(application::TrussApplicationService& trussService,
                             truss_controllers::TrussEditController& editController,
                             QWidget* parent = nullptr);

signals:
    /**
     * @brief Emitted when load is successfully added
     */
    void loadAdded();

public slots:
    /**
     * @brief Add load to selected node
     */
    void addLoad();

    /**
     * @brief Clear input fields
     */
    void clearInputs();

    /**
     * @brief Update node list from current truss
     * @param trussHandle Handle to current truss
     */
    void updateNodeList(application::TrussHandle trussHandle);

private slots:
    /**
     * @brief Handle successful load application
     * @param nodeId Node ID where load was applied
     * @param fx Force X component
     * @param fy Force Y component
     */
    void onLoadApplied(size_t nodeId, double fx, double fy);

    /**
     * @brief Handle failed load application
     * @param errorMessage Error description
     */
    void onOperationFailed(const QString& errorMessage);

private:
    void setupUI();

    // Dependencies
    application::TrussApplicationService& m_trussService;
    truss_controllers::TrussEditController& m_editController;

    // Current state
    application::TrussHandle m_currentTrussHandle;

    // UI components
    QComboBox* m_nodeCombo;
    QLineEdit* m_fxEdit;
    QLineEdit* m_fyEdit;
    QPushButton* m_addButton;
    QPushButton* m_clearButton;
};

}  // namespace truss::gui
