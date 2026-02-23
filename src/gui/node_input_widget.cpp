/**
 * @file NodeInputWidget.cpp
 * @brief Implementation of the node input widget
 */

#include "node_input_widget.hpp"

#include "controllers/truss_edit_controller.hpp"

#include <QtCore/QRegularExpression>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>

namespace truss::gui {

NodeInputWidget::NodeInputWidget(truss::application::TrussApplicationService& trussService,
                                 truss_controllers::TrussEditController& editController,
                                 QWidget* parent)
    : QWidget(parent), m_trussService(trussService), m_editController(editController),
      m_xCoordEdit(new QLineEdit(this)), m_yCoordEdit(new QLineEdit(this)),
      m_supportTypeCombo(new QComboBox(this)), m_addButton(new QPushButton("Add Node", this)),
      m_clearButton(new QPushButton("Clear", this)) {
    setupUI();

    // Connect UI signals
    connect(m_addButton, &QPushButton::clicked, this, &NodeInputWidget::addNode);
    connect(m_clearButton, &QPushButton::clicked, this, &NodeInputWidget::clearInputs);

    // Connect controller signals
    connect(&m_editController,
            &truss_controllers::TrussEditController::nodeAdded,
            this,
            &NodeInputWidget::onNodeAdded);
    connect(&m_editController,
            &truss_controllers::TrussEditController::operationFailed,
            this,
            &NodeInputWidget::onOperationFailed);
}

void NodeInputWidget::setupUI() {
    auto* layout = new QFormLayout(this);

    // Set up validators for coordinate inputs
    auto* doubleValidator = new QDoubleValidator(this);
    doubleValidator->setDecimals(6);
    m_xCoordEdit->setValidator(doubleValidator);
    m_yCoordEdit->setValidator(doubleValidator);

    // Set up support type combo box
    m_supportTypeCombo->addItem("Free", static_cast<int>(truss::core::SupportType::Free));
    m_supportTypeCombo->addItem("Pinned", static_cast<int>(truss::core::SupportType::Pinned));
    m_supportTypeCombo->addItem("Roller X", static_cast<int>(truss::core::SupportType::RollerX));
    m_supportTypeCombo->addItem("Roller Y", static_cast<int>(truss::core::SupportType::RollerY));

    // Add to layout
    layout->addRow("X Coordinate:", m_xCoordEdit);
    layout->addRow("Y Coordinate:", m_yCoordEdit);
    layout->addRow("Support Type:", m_supportTypeCombo);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();

    layout->addRow(buttonLayout);

    // Set default values
    m_xCoordEdit->setText("0.0");
    m_yCoordEdit->setText("0.0");
    m_supportTypeCombo->setCurrentIndex(0);  // Free
}

void NodeInputWidget::addNode() {
    bool xOk, yOk;
    double x = m_xCoordEdit->text().toDouble(&xOk);
    double y = m_yCoordEdit->text().toDouble(&yOk);

    if (!xOk || !yOk) {
        QMessageBox::warning(this, "Invalid Input", "Please enter valid coordinates.");
        return;
    }

    auto supportType = static_cast<truss::core::SupportType>(
        m_supportTypeCombo->currentData().toInt());

    // Delegate to controller (Clean Architecture)
    truss::core::Point2D position{x, y};
    m_editController.onNodeAddRequested(position, supportType);
}

void NodeInputWidget::clearInputs() {
    m_xCoordEdit->setText("0.0");
    m_yCoordEdit->setText("0.0");
    m_supportTypeCombo->setCurrentIndex(0);
    m_xCoordEdit->setFocus();
}

void NodeInputWidget::onNodeAdded(truss::core::NodeId nodeId) {
    emit nodeAdded();

    double x = m_xCoordEdit->text().toDouble();
    double y = m_yCoordEdit->text().toDouble();

    QMessageBox::information(
        this,
        "Success",
        QString("Node %1 added at (%2, %3)").arg(nodeId).arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));
}

void NodeInputWidget::onOperationFailed(const QString& errorMessage) {
    QMessageBox::critical(this, "Error", QString("Failed to add node: %1").arg(errorMessage));
}

}  // namespace truss::gui
