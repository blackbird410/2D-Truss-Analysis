/**
 * @file NodeInputWidget.cpp
 * @brief Implementation of the node input widget
 */

#include "MainWindow.hpp"
#include <QtWidgets/QFormLayout>
#include <QtGui/QDoubleValidator>
#include <QtCore/QRegularExpression>

namespace truss::gui {

NodeInputWidget::NodeInputWidget(QWidget *parent)
    : QWidget(parent),
      m_xCoordEdit(new QLineEdit(this)),
      m_yCoordEdit(new QLineEdit(this)),
      m_supportTypeCombo(new QComboBox(this)),
      m_addButton(new QPushButton("Add Node", this)),
      m_clearButton(new QPushButton("Clear", this)) {
    
    setupUI();
    
    connect(m_addButton, &QPushButton::clicked, this, &NodeInputWidget::addNode);
    connect(m_clearButton, &QPushButton::clicked, this, &NodeInputWidget::clearInputs);
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
    m_supportTypeCombo->addItem("Pinned X", static_cast<int>(truss::core::SupportType::PinnedX));
    m_supportTypeCombo->addItem("Pinned Y", static_cast<int>(truss::core::SupportType::PinnedY));
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
    m_supportTypeCombo->setCurrentIndex(0); // Free
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
    
    // Get the main window to access the truss object
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (mainWindow && mainWindow->getTruss()) {
        try {
            auto nodePtr = mainWindow->getTruss()->addNode(
                truss::core::Point2D{x, y}, supportType);
            size_t nodeId = nodePtr->getId();
            
            emit nodeAdded();
            
            // Update member and load input widgets with new node
            // Note: This will be handled through the main window's connectSignals() method
            
            QMessageBox::information(this, "Success", 
                QString("Node %1 added at (%2, %3)")
                .arg(nodeId).arg(x).arg(y));
                
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to add node: %1").arg(e.what()));
        }
    }
}

void NodeInputWidget::clearInputs() {
    m_xCoordEdit->setText("0.0");
    m_yCoordEdit->setText("0.0");
    m_supportTypeCombo->setCurrentIndex(0);
    m_xCoordEdit->setFocus();
}

} // namespace truss::gui
