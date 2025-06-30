/**
 * @file LoadInputWidget.cpp
 * @brief Implementation of the load input widget
 */

#include "MainWindow.hpp"
#include <QtWidgets/QFormLayout>
#include <QtGui/QDoubleValidator>

namespace truss::gui {

LoadInputWidget::LoadInputWidget(QWidget *parent)
    : QWidget(parent),
      m_nodeCombo(new QComboBox(this)),
      m_fxEdit(new QLineEdit(this)),
      m_fyEdit(new QLineEdit(this)),
      m_addButton(new QPushButton("Add Load", this)),
      m_clearButton(new QPushButton("Clear", this)) {
    
    setupUI();
    
    connect(m_addButton, &QPushButton::clicked, this, &LoadInputWidget::addLoad);
    connect(m_clearButton, &QPushButton::clicked, this, &LoadInputWidget::clearInputs);
}

void LoadInputWidget::setupUI() {
    auto* layout = new QFormLayout(this);
    
    // Set up validators for force inputs (can be negative)
    auto* doubleValidator = new QDoubleValidator(this);
    doubleValidator->setDecimals(6);
    m_fxEdit->setValidator(doubleValidator);
    m_fyEdit->setValidator(doubleValidator);
    
    // Add to layout
    layout->addRow("Node:", m_nodeCombo);
    layout->addRow("Force X (N):", m_fxEdit);
    layout->addRow("Force Y (N):", m_fyEdit);
    
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    
    layout->addRow(buttonLayout);
    
    // Set default values
    m_fxEdit->setText("0.0");
    m_fyEdit->setText("0.0");
    
    // Add tooltip information
    m_fxEdit->setToolTip("Horizontal force component (positive = right)");
    m_fyEdit->setToolTip("Vertical force component (positive = up)");
}

void LoadInputWidget::addLoad() {
    int nodeIndex = m_nodeCombo->currentIndex();
    
    if (nodeIndex < 0) {
        QMessageBox::warning(this, "Invalid Input", "Please select a node.");
        return;
    }
    
    bool fxOk, fyOk;
    double fx = m_fxEdit->text().toDouble(&fxOk);
    double fy = m_fyEdit->text().toDouble(&fyOk);
    
    if (!fxOk || !fyOk) {
        QMessageBox::warning(this, "Invalid Input", "Please enter valid force values.");
        return;
    }
    
    if (fx == 0.0 && fy == 0.0) {
        QMessageBox::warning(this, "Invalid Input", "Load magnitude cannot be zero.");
        return;
    }
    
    // Get node ID from combo box data
    size_t nodeId = m_nodeCombo->currentData().toULongLong();
    
    // Get the main window to access the truss object
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (mainWindow && mainWindow->getTruss()) {
        try {
            truss::core::Force2D force{fx, fy};
            mainWindow->getTruss()->applyForce(nodeId, force);
            
            emit loadAdded();
            
            double magnitude = std::sqrt(fx*fx + fy*fy);
            QMessageBox::information(this, "Success", 
                QString("Load added to node %1\nForce: (%2, %3) N\nMagnitude: %4 N")
                .arg(nodeId)
                .arg(fx, 0, 'f', 2).arg(fy, 0, 'f', 2)
                .arg(magnitude, 0, 'f', 2));
                
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to add load: %1").arg(e.what()));
        }
    }
}

void LoadInputWidget::clearInputs() {
    m_nodeCombo->setCurrentIndex(-1);
    m_fxEdit->setText("0.0");
    m_fyEdit->setText("0.0");
}

void LoadInputWidget::updateNodeList() {
    m_nodeCombo->clear();
    
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (mainWindow && mainWindow->getTruss()) {
        const auto& nodes = mainWindow->getTruss()->getNodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& node = nodes[i];
            size_t nodeId = node->getId();
            QString nodeText = QString("Node %1 (%2, %3)")
                .arg(nodeId)
                .arg(node->getPosition().x, 0, 'f', 2)
                .arg(node->getPosition().y, 0, 'f', 2);
            
            m_nodeCombo->addItem(nodeText, static_cast<qulonglong>(nodeId));
        }
    }
}

} // namespace truss::gui
