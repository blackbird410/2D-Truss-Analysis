/**
 * @file DataTableWidget.cpp
 * @brief Implementation of the data table widget
 */

#include "MainWindow.hpp"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QHeaderView>

namespace truss::gui {

DataTableWidget::DataTableWidget(QWidget *parent)
    : QWidget(parent),
      m_nodesTable(new QTableWidget(this)),
      m_membersTable(new QTableWidget(this)),
      m_loadsTable(new QTableWidget(this)) {
    
    setupUI();
}

void DataTableWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    auto* tabWidget = new QTabWidget(this);
    
    // Set up nodes table
    m_nodesTable->setColumnCount(4);
    QStringList nodeHeaders;
    nodeHeaders << "Node ID" << "X (m)" << "Y (m)" << "Support Type";
    m_nodesTable->setHorizontalHeaderLabels(nodeHeaders);
    m_nodesTable->horizontalHeader()->setStretchLastSection(true);
    m_nodesTable->setAlternatingRowColors(true);
    m_nodesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Set up members table
    m_membersTable->setColumnCount(7);
    QStringList memberHeaders;
    memberHeaders << "Member ID" << "Start Node" << "End Node" 
                 << "Area (m²)" << "E (Pa)" << "Density (kg/m³)" << "Yield (Pa)";
    m_membersTable->setHorizontalHeaderLabels(memberHeaders);
    m_membersTable->horizontalHeader()->setStretchLastSection(true);
    m_membersTable->setAlternatingRowColors(true);
    m_membersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Set up loads table
    m_loadsTable->setColumnCount(4);
    QStringList loadHeaders;
    loadHeaders << "Load ID" << "Node ID" << "Fx (N)" << "Fy (N)";
    m_loadsTable->setHorizontalHeaderLabels(loadHeaders);
    m_loadsTable->horizontalHeader()->setStretchLastSection(true);
    m_loadsTable->setAlternatingRowColors(true);
    m_loadsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Add tables to tabs
    tabWidget->addTab(m_nodesTable, "Nodes");
    tabWidget->addTab(m_membersTable, "Members");
    tabWidget->addTab(m_loadsTable, "Loads");
    
    layout->addWidget(tabWidget);
}

void DataTableWidget::updateTables() {
    updateNodesTable();
    updateMembersTable();
    updateLoadsTable();
}

void DataTableWidget::updateNodesTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->getTruss()) {
        m_nodesTable->setRowCount(0);
        return;
    }
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    m_nodesTable->setRowCount(nodes.size());
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        
        // Node ID
        auto* idItem = new QTableWidgetItem(QString::number(node->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 0, idItem);
        
        // X coordinate
        auto* xItem = new QTableWidgetItem(QString::number(node->getPosition().x, 'f', 6));
        xItem->setFlags(xItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 1, xItem);
        
        // Y coordinate
        auto* yItem = new QTableWidgetItem(QString::number(node->getPosition().y, 'f', 6));
        yItem->setFlags(yItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 2, yItem);
        
        // Support type
        QString supportTypeStr;
        switch (node->getSupportType()) {
            case truss::core::SupportType::Free: 
                supportTypeStr = "Free"; 
                break;
            case truss::core::SupportType::Pinned: 
                supportTypeStr = "Pinned"; 
                break;
            case truss::core::SupportType::PinnedX: 
                supportTypeStr = "Pinned X"; 
                break;
            case truss::core::SupportType::PinnedY: 
                supportTypeStr = "Pinned Y"; 
                break;
            case truss::core::SupportType::RollerX: 
                supportTypeStr = "Roller X"; 
                break;
            case truss::core::SupportType::RollerY: 
                supportTypeStr = "Roller Y"; 
                break;
        }
        
        auto* supportItem = new QTableWidgetItem(supportTypeStr);
        supportItem->setFlags(supportItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 3, supportItem);
    }
    
    m_nodesTable->resizeColumnsToContents();
}

void DataTableWidget::updateMembersTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->getTruss()) {
        m_membersTable->setRowCount(0);
        return;
    }
    
    const auto& members = mainWindow->getTruss()->getMembers();
    m_membersTable->setRowCount(members.size());
    
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        
        // Member ID
        auto* idItem = new QTableWidgetItem(QString::number(member->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 0, idItem);
        
        // Start node ID
        auto* startItem = new QTableWidgetItem(QString::number(member->getStartNode()->getId()));
        startItem->setFlags(startItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 1, startItem);
        
        // End node ID
        auto* endItem = new QTableWidgetItem(QString::number(member->getEndNode()->getId()));
        endItem->setFlags(endItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 2, endItem);
        
        // Cross-sectional area
        auto* areaItem = new QTableWidgetItem(QString::number(member->getSection().area, 'e', 6));
        areaItem->setFlags(areaItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 3, areaItem);
        
        // Young's modulus
        auto* eItem = new QTableWidgetItem(QString::number(member->getMaterial().youngModulus, 'e', 3));
        eItem->setFlags(eItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 4, eItem);
        
        // Density
        auto* densityItem = new QTableWidgetItem(QString::number(member->getMaterial().density, 'f', 1));
        densityItem->setFlags(densityItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 5, densityItem);
        
        // Yield strength
        auto* yieldItem = new QTableWidgetItem(QString::number(member->getMaterial().yieldStrength, 'e', 3));
        yieldItem->setFlags(yieldItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 6, yieldItem);
    }
    
    m_membersTable->resizeColumnsToContents();
}

void DataTableWidget::updateLoadsTable() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->getTruss()) {
        m_loadsTable->setRowCount(0);
        return;
    }
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    
    // Count nodes with applied forces
    int loadCount = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (node->hasAppliedForce()) {
            loadCount++;
        }
    }
    
    m_loadsTable->setRowCount(loadCount);
    
    int row = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (!node->hasAppliedForce()) continue;
        
        const auto& force = node->getAppliedForce();
        
        // Load index (not really an ID since they're stored in nodes)
        auto* idItem = new QTableWidgetItem(QString::number(row + 1));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 0, idItem);
        
        // Node ID
        auto* nodeItem = new QTableWidgetItem(QString::number(node->getId()));
        nodeItem->setFlags(nodeItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 1, nodeItem);
        
        // Force X
        auto* fxItem = new QTableWidgetItem(QString::number(force.fx, 'f', 6));
        fxItem->setFlags(fxItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 2, fxItem);
        
        // Force Y
        auto* fyItem = new QTableWidgetItem(QString::number(force.fy, 'f', 6));
        fyItem->setFlags(fyItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 3, fyItem);
        
        row++;
    }
    
    m_loadsTable->resizeColumnsToContents();
}

} // namespace truss::gui
