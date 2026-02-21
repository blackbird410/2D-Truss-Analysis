/**
 * @file MemberInputWidget.cpp
 * @brief Implementation of the member input widget
 */

#include "MemberInputWidget.hpp"
#include "controllers/TrussEditController.hpp"
#include "application/TrussEditDTOs.hpp"
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDoubleValidator>

namespace truss::gui {

MemberInputWidget::MemberInputWidget(
    truss::application::TrussApplicationService& trussService,
    truss_controllers::TrussEditController& editController,
    QWidget *parent)
    : QWidget(parent),
      m_trussService(trussService),
      m_editController(editController),
      m_currentTrussHandle(0),
      m_startNodeCombo(new QComboBox(this)),
      m_endNodeCombo(new QComboBox(this)),
      m_areaEdit(new QLineEdit(this)),
      m_youngModulusEdit(new QLineEdit(this)),
      m_densityEdit(new QLineEdit(this)),
      m_yieldStrengthEdit(new QLineEdit(this)),
      m_addButton(new QPushButton("Add Member", this)),
      m_clearButton(new QPushButton("Clear", this)) {
    
    setupUI();
    
    // Connect UI signals
    connect(m_addButton, &QPushButton::clicked, this, &MemberInputWidget::addMember);
    connect(m_clearButton, &QPushButton::clicked, this, &MemberInputWidget::clearInputs);
    
    // Connect controller signals
    connect(&m_editController, &truss_controllers::TrussEditController::memberAdded,
            this, &MemberInputWidget::onMemberAdded);
    connect(&m_editController, &truss_controllers::TrussEditController::operationFailed,
            this, &MemberInputWidget::onOperationFailed);
}

void MemberInputWidget::setupUI() {
    auto* layout = new QFormLayout(this);
    
    // Set up validators for numeric inputs
    auto* positiveDoubleValidator = new QDoubleValidator(0.0, 1e12, 6, this);
    m_areaEdit->setValidator(positiveDoubleValidator);
    m_youngModulusEdit->setValidator(positiveDoubleValidator);
    m_densityEdit->setValidator(positiveDoubleValidator);
    m_yieldStrengthEdit->setValidator(positiveDoubleValidator);
    
    // Add to layout
    layout->addRow("Start Node:", m_startNodeCombo);
    layout->addRow("End Node:", m_endNodeCombo);
    layout->addRow("Cross-sectional Area (m²):", m_areaEdit);
    layout->addRow("Young's Modulus (Pa):", m_youngModulusEdit);
    layout->addRow("Density (kg/m³):", m_densityEdit);
    layout->addRow("Yield Strength (Pa):", m_yieldStrengthEdit);
    
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    
    layout->addRow(buttonLayout);
    
    // Set default values
    m_areaEdit->setText("0.01");  // 1 cm²
    m_youngModulusEdit->setText("200000000000");  // 200 GPa (steel)
    m_densityEdit->setText("7850");  // kg/m³ (steel)
    m_yieldStrengthEdit->setText("250000000");  // 250 MPa (steel)
}

void MemberInputWidget::addMember() {
    int startNodeIndex = m_startNodeCombo->currentIndex();
    int endNodeIndex = m_endNodeCombo->currentIndex();
    
    if (startNodeIndex < 0 || endNodeIndex < 0) {
        QMessageBox::warning(this, "Invalid Input", "Please select both start and end nodes.");
        return;
    }
    
    if (startNodeIndex == endNodeIndex) {
        QMessageBox::warning(this, "Invalid Input", "Start and end nodes must be different.");
        return;
    }
    
    bool areaOk, youngOk, densityOk, yieldOk;
    double area = m_areaEdit->text().toDouble(&areaOk);
    double youngModulus = m_youngModulusEdit->text().toDouble(&youngOk);
    double density = m_densityEdit->text().toDouble(&densityOk);
    double yieldStrength = m_yieldStrengthEdit->text().toDouble(&yieldOk);
    
    if (!areaOk || !youngOk || !densityOk || !yieldOk) {
        QMessageBox::warning(this, "Invalid Input", "Please enter valid material properties.");
        return;
    }
    
    if (area <= 0 || youngModulus <= 0 || density <= 0 || yieldStrength <= 0) {
        QMessageBox::warning(this, "Invalid Input", "All material properties must be positive.");
        return;
    }
    
    // Get node IDs from combo box data
    size_t startNodeId = m_startNodeCombo->currentData().toULongLong();
    size_t endNodeId = m_endNodeCombo->currentData().toULongLong();
    
    // Delegate to controller (Clean Architecture) - Using Application DTOs
    truss::application::MaterialSpec material{youngModulus, "Steel"};
    truss::application::SectionSpec section{area, "Custom"};
    
    m_editController.onMemberAddRequested(startNodeId, endNodeId, material, section);
}

void MemberInputWidget::clearInputs() {
    m_startNodeCombo->setCurrentIndex(-1);
    m_endNodeCombo->setCurrentIndex(-1);
    m_areaEdit->setText("0.01");
    m_youngModulusEdit->setText("200000000000");
    m_densityEdit->setText("7850");
    m_yieldStrengthEdit->setText("250000000");
}

void MemberInputWidget::updateNodeList(truss::application::TrussHandle trussHandle) {
    m_currentTrussHandle = trussHandle;
    m_startNodeCombo->clear();
    m_endNodeCombo->clear();
    
    if (m_currentTrussHandle == 0) return;
    
    // Use ITrussView interface (Clean Architecture)
    const auto& trussView = m_trussService.getTrussView(m_currentTrussHandle);
    auto nodeViews = trussView.getNodeViews();
    
    for (const auto& nodeView : nodeViews) {
        QString nodeText = QString("Node %1 (%2, %3)")
            .arg(nodeView.id)
            .arg(nodeView.x, 0, 'f', 2)
            .arg(nodeView.y, 0, 'f', 2);
        
        m_startNodeCombo->addItem(nodeText, static_cast<qulonglong>(nodeView.id));
        m_endNodeCombo->addItem(nodeText, static_cast<qulonglong>(nodeView.id));
    }
}

void MemberInputWidget::onMemberAdded(truss::core::MemberId memberId) {
    emit memberAdded();
    
    size_t startNodeId = m_startNodeCombo->currentData().toULongLong();
    size_t endNodeId = m_endNodeCombo->currentData().toULongLong();
    
    QMessageBox::information(this, "Success", 
        QString("Member %1 added between nodes %2 and %3")
        .arg(memberId).arg(startNodeId).arg(endNodeId));
}

void MemberInputWidget::onOperationFailed(const QString& errorMessage) {
    QMessageBox::critical(this, "Error", 
        QString("Failed to add member: %1").arg(errorMessage));
}

} // namespace truss::gui
