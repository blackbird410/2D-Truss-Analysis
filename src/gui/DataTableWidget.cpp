/**
 * @file DataTableWidget.cpp
 * @brief Implementation of the data table widget
 */

#include "DataTableWidget.hpp"

#include "core/interfaces/ITrussView.hpp"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

#include <algorithm>

using namespace truss::application;
using namespace truss::infrastructure;

namespace truss::gui {

DataTableWidget::DataTableWidget(application::TrussApplicationService& trussService,
                                 QWidget* parent)
    : QWidget(parent), m_trussService(trussService), m_currentTrussHandle(0),
      m_nodesTable(new QTableWidget(this)), m_membersTable(new QTableWidget(this)),
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

[[maybe_unused]] void DataTableWidget::updateTables(application::TrussHandle trussHandle) {
    m_currentTrussHandle = trussHandle;
    updateNodesTable(trussHandle);
    updateMembersTable(trussHandle);
    updateLoadsTable(trussHandle);
}

[[maybe_unused]] void DataTableWidget::clearTables() {
    m_currentTrussHandle = 0;
    m_nodesTable->setRowCount(0);
    m_membersTable->setRowCount(0);
    m_loadsTable->setRowCount(0);
}

void DataTableWidget::updateNodesTable(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_nodesTable->setRowCount(0);
        return;
    }

    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto nodeViews = trussView.getNodeViews();
    m_nodesTable->setRowCount(nodeViews.size());

    for (size_t i = 0; i < nodeViews.size(); ++i) {
        const auto& nodeView = nodeViews[i];

        // Node ID
        auto* idItem = new QTableWidgetItem(QString::number(nodeView.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 0, idItem);

        // X coordinate
        auto* xItem = new QTableWidgetItem(QString::number(nodeView.x, 'f', 6));
        xItem->setFlags(xItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 1, xItem);

        // Y coordinate
        auto* yItem = new QTableWidgetItem(QString::number(nodeView.y, 'f', 6));
        yItem->setFlags(yItem->flags() & ~Qt::ItemIsEditable);
        m_nodesTable->setItem(i, 2, yItem);

        // Support type
        QString supportTypeStr;
        switch (nodeView.support) {
            case truss::core::SupportType::Free:
                supportTypeStr = "Free";
                break;
            case truss::core::SupportType::Pinned:
                supportTypeStr = "Pinned";
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

void DataTableWidget::updateMembersTable(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_membersTable->setRowCount(0);
        return;
    }

    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto memberViews = trussView.getMemberViews();
    m_membersTable->setRowCount(memberViews.size());

    for (size_t i = 0; i < memberViews.size(); ++i) {
        const auto& memberView = memberViews[i];

        // Member ID
        auto* idItem = new QTableWidgetItem(QString::number(memberView.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 0, idItem);

        // Start node ID
        auto* startItem = new QTableWidgetItem(QString::number(memberView.startNodeId));
        startItem->setFlags(startItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 1, startItem);

        // End node ID
        auto* endItem = new QTableWidgetItem(QString::number(memberView.endNodeId));
        endItem->setFlags(endItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 2, endItem);

        // Cross-sectional area
        auto* areaItem = new QTableWidgetItem(QString::number(memberView.area, 'e', 6));
        areaItem->setFlags(areaItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 3, areaItem);

        // Young's modulus
        auto* eItem = new QTableWidgetItem(QString::number(memberView.youngModulus, 'e', 3));
        eItem->setFlags(eItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 4, eItem);

        // Density
        auto* densityItem = new QTableWidgetItem(QString::number(0.0, 'f', 1));  // Not in view
        densityItem->setFlags(densityItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 5, densityItem);

        // Yield strength
        auto* yieldItem = new QTableWidgetItem(QString::number(0.0, 'e', 3));  // Not in view
        yieldItem->setFlags(yieldItem->flags() & ~Qt::ItemIsEditable);
        m_membersTable->setItem(i, 6, yieldItem);
    }

    m_membersTable->resizeColumnsToContents();
}

void DataTableWidget::updateLoadsTable(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_loadsTable->setRowCount(0);
        return;
    }

    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto nodeViews = trussView.getNodeViews();

    // Count nodes with applied forces
    int loadCount = static_cast<int>(
        std::count_if(nodeViews.begin(), nodeViews.end(), [](const auto& nodeView) {
            return nodeView.fx != 0.0 || nodeView.fy != 0.0;
        }));

    m_loadsTable->setRowCount(loadCount);

    int row = 0;
    for (const auto& nodeView : nodeViews) {
        if (nodeView.fx == 0.0 && nodeView.fy == 0.0)
            continue;

        // Load index (not really an ID since they're stored in nodes)
        auto* idItem = new QTableWidgetItem(QString::number(row + 1));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 0, idItem);

        // Node ID
        auto* nodeItem = new QTableWidgetItem(QString::number(nodeView.id));
        nodeItem->setFlags(nodeItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 1, nodeItem);

        // Force X
        auto* fxItem = new QTableWidgetItem(QString::number(nodeView.fx, 'f', 6));
        fxItem->setFlags(fxItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 2, fxItem);

        // Force Y
        auto* fyItem = new QTableWidgetItem(QString::number(nodeView.fy, 'f', 6));
        fyItem->setFlags(fyItem->flags() & ~Qt::ItemIsEditable);
        m_loadsTable->setItem(row, 3, fyItem);

        row++;
    }

    m_loadsTable->resizeColumnsToContents();
}

}  // namespace truss::gui
