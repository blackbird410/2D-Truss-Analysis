/**
 * @file ResultsWidget.cpp
 * @brief Implementation of the results widget
 */

#include "ResultsWidget.hpp"

#include "core/interfaces/ITrussView.hpp"

#include <algorithm>
#include <numeric>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

namespace truss::gui {

ResultsWidget::ResultsWidget(application::TrussApplicationService& trussService,
                             application::AnalysisApplicationService& analysisService,
                             QWidget* parent)
    : QWidget(parent), m_trussService(trussService), m_analysisService(analysisService),
      m_currentTrussHandle(0), m_displacementsTable(new QTableWidget(this)),
      m_forcesTable(new QTableWidget(this)), m_reactionsTable(new QTableWidget(this)),
      m_stiffnessTable(new QTableWidget(this)), m_summaryText(new QTextEdit(this)) {
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

void ResultsWidget::updateResults(application::TrussHandle trussHandle) {
    m_currentTrussHandle = trussHandle;

    updateDisplacementsTable(trussHandle);
    updateForcesTable(trussHandle);
    updateReactionsTable(trussHandle);
    updateStiffnessTable(trussHandle);
    updateSummary(trussHandle);
}

void ResultsWidget::clearResults() {
    m_currentTrussHandle = 0;
    m_displacementsTable->setRowCount(0);
    m_forcesTable->setRowCount(0);
    m_reactionsTable->setRowCount(0);
    m_stiffnessTable->setRowCount(0);
    m_stiffnessTable->setColumnCount(0);
    m_summaryText->clear();
}

void ResultsWidget::updateDisplacementsTable(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_displacementsTable->setRowCount(0);
        return;
    }

    // Use ITrussView interface (Clean Architecture)
    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto nodeViews = trussView.getNodeViews();

    m_displacementsTable->setRowCount(nodeViews.size());

    for (size_t i = 0; i < nodeViews.size(); ++i) {
        const auto& nodeView = nodeViews[i];

        // Node ID
        auto* idItem = new QTableWidgetItem(QString::number(nodeView.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_displacementsTable->setItem(i, 0, idItem);

        // Displacement X (from node view results)
        auto* dxItem = new QTableWidgetItem(QString::number(nodeView.dx, 'e', 6));
        dxItem->setFlags(dxItem->flags() & ~Qt::ItemIsEditable);
        m_displacementsTable->setItem(i, 1, dxItem);

        // Displacement Y (from node view results)
        auto* dyItem = new QTableWidgetItem(QString::number(nodeView.dy, 'e', 6));
        dyItem->setFlags(dyItem->flags() & ~Qt::ItemIsEditable);
        m_displacementsTable->setItem(i, 2, dyItem);
    }

    m_displacementsTable->resizeColumnsToContents();
}

void ResultsWidget::updateForcesTable(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_forcesTable->setRowCount(0);
        return;
    }

    // Use ITrussView interface (Clean Architecture)
    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto memberViews = trussView.getMemberViews();

    m_forcesTable->setRowCount(memberViews.size());

    for (size_t i = 0; i < memberViews.size(); ++i) {
        const auto& memberView = memberViews[i];

        // Member ID
        auto* idItem = new QTableWidgetItem(QString::number(memberView.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_forcesTable->setItem(i, 0, idItem);

        // Axial force (from member view results)
        auto* forceItem = new QTableWidgetItem(QString::number(memberView.axialForce, 'e', 3));
        forceItem->setFlags(forceItem->flags() & ~Qt::ItemIsEditable);
        m_forcesTable->setItem(i, 1, forceItem);

        // Stress (from member view results)
        auto* stressItem = new QTableWidgetItem(QString::number(memberView.axialStress, 'e', 3));
        stressItem->setFlags(stressItem->flags() & ~Qt::ItemIsEditable);
        m_forcesTable->setItem(i, 2, stressItem);
    }

    m_forcesTable->resizeColumnsToContents();
}

void ResultsWidget::updateReactionsTable(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_reactionsTable->setRowCount(0);
        return;
    }

    // Use ITrussView interface (Clean Architecture)
    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto nodeViews = trussView.getNodeViews();

    // Count nodes with reactions (non-zero reaction forces)
    const auto reactionNodeCount = static_cast<int>(
        std::count_if(nodeViews.begin(), nodeViews.end(), [](const auto& nodeView) {
            return nodeView.rx != 0.0 || nodeView.ry != 0.0;
        }));

    m_reactionsTable->setRowCount(reactionNodeCount);

    int row = 0;
    for (const auto& nodeView : nodeViews) {
        if (nodeView.rx == 0.0 && nodeView.ry == 0.0) {
            continue;
        }

        // Node ID
        auto* idItem = new QTableWidgetItem(QString::number(nodeView.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_reactionsTable->setItem(row, 0, idItem);

        // Reaction X (from node view results)
        auto* rxItem = new QTableWidgetItem(QString::number(nodeView.rx, 'e', 3));
        rxItem->setFlags(rxItem->flags() & ~Qt::ItemIsEditable);
        m_reactionsTable->setItem(row, 1, rxItem);

        // Reaction Y (from node view results)
        auto* ryItem = new QTableWidgetItem(QString::number(nodeView.ry, 'e', 3));
        ryItem->setFlags(ryItem->flags() & ~Qt::ItemIsEditable);
        m_reactionsTable->setItem(row, 2, ryItem);

        row++;
    }

    m_reactionsTable->resizeColumnsToContents();
}

void ResultsWidget::updateSummary(application::TrussHandle trussHandle) {
    if (trussHandle == 0) {
        m_summaryText->clear();
        return;
    }

    // Use ITrussView interface (Clean Architecture)
    const auto& trussView = m_trussService.getTrussView(trussHandle);
    auto nodeViews = trussView.getNodeViews();
    auto memberViews = trussView.getMemberViews();

    QString summary;
    summary += "Analysis Summary\n";
    summary += "================\n\n";

    summary += QString("Nodes: %1\n").arg(nodeViews.size());
    summary += QString("Members: %1\n").arg(memberViews.size());

    // Count applied loads
    int loadCount = 0;
    double totalFx = 0.0, totalFy = 0.0;
    for (const auto& nodeView : nodeViews) {
        if (nodeView.fx != 0.0 || nodeView.fy != 0.0) {
            loadCount++;
            totalFx += nodeView.fx;
            totalFy += nodeView.fy;
        }
    }

    summary += QString("Loads: %1\n\n").arg(loadCount);

    summary += QString("Total Applied Load:\n");
    summary += QString("  Fx = %1 N\n").arg(totalFx, 0, 'f', 2);
    summary += QString("  Fy = %1 N\n").arg(totalFy, 0, 'f', 2);
    summary += QString("  Magnitude = %1 N\n\n")
                   .arg(std::sqrt(totalFx * totalFx + totalFy * totalFy), 0, 'f', 2);

    // Calculate max displacement and stress from node/member views
    const auto maxDisplacement = std::accumulate(
        nodeViews.begin(), nodeViews.end(), 0.0, [](double maxValue, const auto& nodeView) {
            const double displacement =
                std::sqrt(nodeView.dx * nodeView.dx + nodeView.dy * nodeView.dy);
            return std::max(maxValue, displacement);
        });

    const auto maxStress = std::accumulate(
        memberViews.begin(), memberViews.end(), 0.0, [](double maxValue, const auto& memberView) {
            return std::max(maxValue, std::abs(memberView.axialStress));
        });

    summary += QString("Maximum Displacement: %1 m\n").arg(maxDisplacement, 0, 'e', 3);
    summary += QString("Maximum Stress: %1 Pa\n").arg(maxStress, 0, 'e', 3);

    m_summaryText->setPlainText(summary);
}

void ResultsWidget::updateStiffnessTable(application::TrussHandle /*trussHandle*/) {
    // Note: Stiffness matrix access requires IAnalysisResultsView interface
    // For now, show a placeholder message
    m_stiffnessTable->setRowCount(1);
    m_stiffnessTable->setColumnCount(1);

    auto* item = new QTableWidgetItem("Stiffness matrix visualization coming soon");
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    m_stiffnessTable->setItem(0, 0, item);
}

}  // namespace truss::gui
