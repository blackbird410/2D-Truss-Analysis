/**
 * @file ResultsWidget.cpp
 * @brief Implementation of the results widget
 */

#include "MainWindow.hpp"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QHeaderView>

namespace truss::gui {

ResultsWidget::ResultsWidget(QWidget *parent)
    : QWidget(parent),
      m_displacementsTable(new QTableWidget(this)),
      m_forcesTable(new QTableWidget(this)),
      m_reactionsTable(new QTableWidget(this)),
      m_stiffnessTable(new QTableWidget(this)),
      m_summaryText(new QTextEdit(this)) {
    
    setupUI();
}

void ResultsWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    auto* tabWidget = new QTabWidget(this);
    
    // Set up displacements table
    m_displacementsTable->setColumnCount(3);
    QStringList displHeaders;
    displHeaders << "Node ID" << "Displacement X (m)" << "Displacement Y (m)";
    m_displacementsTable->setHorizontalHeaderLabels(displHeaders);
    m_displacementsTable->horizontalHeader()->setStretchLastSection(true);
    m_displacementsTable->setAlternatingRowColors(true);
    m_displacementsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Set up forces table
    m_forcesTable->setColumnCount(3);
    QStringList forceHeaders;
    forceHeaders << "Member ID" << "Axial Force (N)" << "Stress (Pa)";
    m_forcesTable->setHorizontalHeaderLabels(forceHeaders);
    m_forcesTable->horizontalHeader()->setStretchLastSection(true);
    m_forcesTable->setAlternatingRowColors(true);
    m_forcesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Set up reactions table
    m_reactionsTable->setColumnCount(3);
    QStringList reactionHeaders;
    reactionHeaders << "Node ID" << "Reaction X (N)" << "Reaction Y (N)";
    m_reactionsTable->setHorizontalHeaderLabels(reactionHeaders);
    m_reactionsTable->horizontalHeader()->setStretchLastSection(true);
    m_reactionsTable->setAlternatingRowColors(true);
    m_reactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Set up stiffness matrix table
    m_stiffnessTable->setAlternatingRowColors(true);
    m_stiffnessTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_stiffnessTable->horizontalHeader()->setDefaultSectionSize(80);
    m_stiffnessTable->verticalHeader()->setDefaultSectionSize(25);
    
    // Set up summary text
    m_summaryText->setReadOnly(true);
    m_summaryText->setMaximumHeight(150);
    
    // Add tables to tabs
    tabWidget->addTab(m_displacementsTable, "Displacements");
    tabWidget->addTab(m_forcesTable, "Member Forces");
    tabWidget->addTab(m_reactionsTable, "Reactions");
    tabWidget->addTab(m_stiffnessTable, "Stiffness Matrix");
    tabWidget->addTab(m_summaryText, "Summary");
    
    layout->addWidget(tabWidget);
}

void ResultsWidget::updateResults() {
    updateDisplacementsTable();
    updateForcesTable();
    updateReactionsTable();
    updateStiffnessTable();
    updateSummary();
}

void ResultsWidget::clearResults() {
    m_displacementsTable->setRowCount(0);
    m_forcesTable->setRowCount(0);
    m_reactionsTable->setRowCount(0);
    m_stiffnessTable->setRowCount(0);
    m_stiffnessTable->setColumnCount(0);
    m_summaryText->clear();
}

void ResultsWidget::updateDisplacementsTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->hasResults()) {
        m_displacementsTable->setRowCount(0);
        return;
    }
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    m_displacementsTable->setRowCount(nodes.size());
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        
        // Node ID
        auto* idItem = new QTableWidgetItem(QString::number(node->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_displacementsTable->setItem(i, 0, idItem);
        
        // Displacement X (from node results)
        auto* dxItem = new QTableWidgetItem(QString::number(node->getDisplacement().x, 'e', 6));
        dxItem->setFlags(dxItem->flags() & ~Qt::ItemIsEditable);
        m_displacementsTable->setItem(i, 1, dxItem);
        
        // Displacement Y (from node results)
        auto* dyItem = new QTableWidgetItem(QString::number(node->getDisplacement().y, 'e', 6));
        dyItem->setFlags(dyItem->flags() & ~Qt::ItemIsEditable);
        m_displacementsTable->setItem(i, 2, dyItem);
    }
    
    m_displacementsTable->resizeColumnsToContents();
}

void ResultsWidget::updateForcesTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->hasResults()) {
        m_forcesTable->setRowCount(0);
        return;
    }
    
    const auto& members = mainWindow->getTruss()->getMembers();
    m_forcesTable->setRowCount(members.size());
    
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        
        // Member ID
        auto* idItem = new QTableWidgetItem(QString::number(member->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_forcesTable->setItem(i, 0, idItem);
        
        // Axial force (from member results)
        auto* forceItem = new QTableWidgetItem(QString::number(member->getAxialForce(), 'e', 3));
        forceItem->setFlags(forceItem->flags() & ~Qt::ItemIsEditable);
        m_forcesTable->setItem(i, 1, forceItem);
        
        // Stress (from member results)
        auto* stressItem = new QTableWidgetItem(QString::number(member->getAxialStress(), 'e', 3));
        stressItem->setFlags(stressItem->flags() & ~Qt::ItemIsEditable);
        m_forcesTable->setItem(i, 2, stressItem);
    }
    
    m_forcesTable->resizeColumnsToContents();
}

void ResultsWidget::updateReactionsTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->hasResults()) {
        m_reactionsTable->setRowCount(0);
        return;
    }
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    
    // Count nodes with supports
    int supportNodeCount = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (node->getSupportType() != truss::core::SupportType::Free) {
            supportNodeCount++;
        }
    }
    
    m_reactionsTable->setRowCount(supportNodeCount);
    
    int row = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (node->getSupportType() == truss::core::SupportType::Free) {
            continue;
        }
        
        // Node ID
        auto* idItem = new QTableWidgetItem(QString::number(node->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_reactionsTable->setItem(row, 0, idItem);
        
        // Reaction X (from node results)
        auto* rxItem = new QTableWidgetItem(QString::number(node->getReaction().fx, 'e', 3));
        rxItem->setFlags(rxItem->flags() & ~Qt::ItemIsEditable);
        m_reactionsTable->setItem(row, 1, rxItem);
        
        // Reaction Y (from node results)
        auto* ryItem = new QTableWidgetItem(QString::number(node->getReaction().fy, 'e', 3));
        ryItem->setFlags(ryItem->flags() & ~Qt::ItemIsEditable);
        m_reactionsTable->setItem(row, 2, ryItem);
        
        row++;
    }
    
    m_reactionsTable->resizeColumnsToContents();
}

void ResultsWidget::updateSummary() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->hasResults()) {
        m_summaryText->clear();
        return;
    }
    
    QString summary;
    summary += "Analysis Summary\n";
    summary += "================\n\n";
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    const auto& members = mainWindow->getTruss()->getMembers();
    
    summary += QString("Nodes: %1\n").arg(nodes.size());
    summary += QString("Members: %1\n").arg(members.size());
    
    // Count applied loads
    int loadCount = 0;
    double totalFx = 0.0, totalFy = 0.0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (node->hasAppliedForce()) {
            loadCount++;
            const auto& force = node->getAppliedForce();
            totalFx += force.fx;
            totalFy += force.fy;
        }
    }
    
    summary += QString("Loads: %1\n\n").arg(loadCount);
    
    summary += QString("Total Applied Load:\n");
    summary += QString("  Fx = %1 N\n").arg(totalFx, 0, 'f', 2);
    summary += QString("  Fy = %1 N\n").arg(totalFy, 0, 'f', 2);
    summary += QString("  Magnitude = %1 N\n\n").arg(std::sqrt(totalFx*totalFx + totalFy*totalFy), 0, 'f', 2);
    
    // Get analysis results if available
    if (mainWindow->hasResults()) {
        const auto& results = mainWindow->getLastResults();
        summary += QString("Maximum Displacement: %1 m\n").arg(results.maxDisplacement, 0, 'e', 3);
        summary += QString("Maximum Stress: %1 Pa\n").arg(results.maxStress, 0, 'e', 3);
        summary += QString("Analysis converged in %1 iterations\n").arg(results.iterations);
    }
    
    m_summaryText->setPlainText(summary);
}

void ResultsWidget::updateStiffnessTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->hasResults()) {
        m_stiffnessTable->setRowCount(0);
        m_stiffnessTable->setColumnCount(0);
        return;
    }
    
    const auto& results = mainWindow->getLastResults();
    const auto& stiffnessMatrix = results.stiffnessMatrix;
    
    if (stiffnessMatrix.empty()) {
        m_stiffnessTable->setRowCount(0);
        m_stiffnessTable->setColumnCount(0);
        return;
    }
    
    int matrixSize = static_cast<int>(stiffnessMatrix.size());
    m_stiffnessTable->setRowCount(matrixSize);
    m_stiffnessTable->setColumnCount(matrixSize);
    
    // Set headers for DOF numbers
    QStringList headers;
    for (int i = 0; i < matrixSize; ++i) {
        headers << QString("DOF %1").arg(i);
    }
    m_stiffnessTable->setHorizontalHeaderLabels(headers);
    m_stiffnessTable->setVerticalHeaderLabels(headers);
    
    // Fill the table with stiffness matrix values
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < static_cast<int>(stiffnessMatrix[i].size()); ++j) {
            double value = stiffnessMatrix[i][j];
            
            // Format values for better readability
            QString valueStr;
            if (std::abs(value) < 1e-10) {
                valueStr = "0";
            } else if (std::abs(value) < 1e3) {
                valueStr = QString::number(value, 'f', 2);
            } else {
                valueStr = QString::number(value, 'e', 2);
            }
            
            auto* item = new QTableWidgetItem(valueStr);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            
            // Color code diagonal elements
            if (i == j && std::abs(value) > 1e-10) {
                item->setBackground(QColor(240, 248, 255)); // Light blue for diagonal
            } else if (std::abs(value) < 1e-10) {
                item->setBackground(QColor(248, 248, 248)); // Light gray for zeros
            }
            
            m_stiffnessTable->setItem(i, j, item);
        }
    }
    
    // Adjust column widths for better display
    m_stiffnessTable->resizeColumnsToContents();
    
    // Ensure minimum column width for readability
    for (int col = 0; col < matrixSize; ++col) {
        if (m_stiffnessTable->columnWidth(col) < 80) {
            m_stiffnessTable->setColumnWidth(col, 80);
        }
    }
}

} // namespace truss::gui
