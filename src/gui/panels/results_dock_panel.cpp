/**
 * @file results_dock_panel.cpp
 * @brief ResultsDockPanel implementation (Phase 5).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/panels/results_dock_panel.hpp"

#include "core/interfaces/ianalysis_results_view.hpp"
#include "gui/models/member_table_model.hpp"
#include "gui/models/node_table_model.hpp"
#include "gui/models/results_table_model.hpp"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

namespace truss::gui {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ResultsDockPanel::ResultsDockPanel(model::NodeTableModel*    nodeModel,
                                    model::MemberTableModel*  memberModel,
                                    model::ResultsTableModel* resultsModel,
                                    QWidget*                  parent)
    : QWidget{parent}
    , m_nodeModel{nodeModel}
    , m_memberModel{memberModel}
    , m_resultsModel{resultsModel}
{
    setObjectName(QStringLiteral("resultsDockPanel"));

    auto* vbox = new QVBoxLayout{this};
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    m_tabs = new QTabWidget{this};
    m_tabs->setObjectName(QStringLiteral("resultsTabs"));
    m_tabs->setDocumentMode(true);
    vbox->addWidget(m_tabs, 1);

    buildNodeResultsTab();
    buildMemberResultsTab();
    buildSystemSummaryTab();
    buildStiffnessMatrixTab();

    connect(m_tabs, &QTabWidget::currentChanged, this, &ResultsDockPanel::onTabChanged);
}

// ---------------------------------------------------------------------------
// Tab builders — each tab wraps a content widget in a scroll-friendly layout
// with an Export button at the bottom.
// ---------------------------------------------------------------------------

static QWidget* makeTabWrapper(QWidget* parent, QWidget* content,
                                const QString& exportLabel,
                                truss::ExportFormat format,
                                ResultsDockPanel* panel)
{
    auto* wrapper = new QWidget{parent};
    auto* vbox    = new QVBoxLayout{wrapper};
    vbox->setContentsMargins(0, 0, 0, 4);
    vbox->setSpacing(0);
    vbox->addWidget(content, 1);

    auto* btnRow  = new QWidget{wrapper};
    auto* hbox    = new QHBoxLayout{btnRow};
    hbox->setContentsMargins(8, 4, 8, 4);
    hbox->addStretch();
    auto* exportBtn = new QPushButton{exportLabel, btnRow};
    exportBtn->setObjectName(QStringLiteral("results_exportBtn_") + exportLabel);
    hbox->addWidget(exportBtn);
    vbox->addWidget(btnRow);

    QObject::connect(exportBtn, &QPushButton::clicked, panel, [panel, format] {
        emit panel->exportRequested(format, QStringLiteral("results"));
    });

    return wrapper;
}

void ResultsDockPanel::buildNodeResultsTab()
{
    m_nodeTableView = new QTableView{this};
    m_nodeTableView->setObjectName(QStringLiteral("nodeResultsTable"));
    m_nodeTableView->setModel(m_nodeModel);
    m_nodeTableView->setAlternatingRowColors(true);
    m_nodeTableView->horizontalHeader()->setStretchLastSection(true);
    m_nodeTableView->verticalHeader()->hide();
    m_nodeTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto* tab = makeTabWrapper(this, m_nodeTableView,
                               QStringLiteral("Export CSV"),
                               truss::ExportFormat::CSV, this);
    m_tabs->addTab(tab, QStringLiteral("Node Results"));
}

void ResultsDockPanel::buildMemberResultsTab()
{
    m_memberTableView = new QTableView{this};
    m_memberTableView->setObjectName(QStringLiteral("memberResultsTable"));
    m_memberTableView->setModel(m_memberModel);
    m_memberTableView->setAlternatingRowColors(true);
    m_memberTableView->horizontalHeader()->setStretchLastSection(true);
    m_memberTableView->verticalHeader()->hide();
    m_memberTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto* tab = makeTabWrapper(this, m_memberTableView,
                               QStringLiteral("Export CSV"),
                               truss::ExportFormat::CSV, this);
    m_tabs->addTab(tab, QStringLiteral("Member Results"));
}

void ResultsDockPanel::buildSystemSummaryTab()
{
    m_resultsTableView = new QTableView{this};
    m_resultsTableView->setObjectName(QStringLiteral("systemSummaryTable"));
    m_resultsTableView->setModel(m_resultsModel);
    m_resultsTableView->setAlternatingRowColors(false);
    m_resultsTableView->horizontalHeader()->setStretchLastSection(true);
    m_resultsTableView->verticalHeader()->hide();

    auto* tab = makeTabWrapper(this, m_resultsTableView,
                               QStringLiteral("Export JSON"),
                               truss::ExportFormat::JSON, this);
    m_tabs->addTab(tab, QStringLiteral("System Summary"));
}

void ResultsDockPanel::buildStiffnessMatrixTab()
{
    // Start with an empty QTableView; actual matrix data is loaded lazily
    // on the first tab activation via onTabChanged / setResultsView().
    m_stiffnessTableView = new QTableView{this};
    m_stiffnessTableView->setObjectName(QStringLiteral("stiffnessMatrixTable"));
    m_stiffnessTableView->setAlternatingRowColors(true);
    m_stiffnessTableView->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_stiffnessTableView->verticalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_stiffnessTableView->setSelectionBehavior(QAbstractItemView::SelectItems);

    // Placeholder model (0 rows, 0 cols) until analysis results are available
    m_stiffnessTableView->setModel(new QStandardItemModel(0, 0, m_stiffnessTableView));

    auto* tab = makeTabWrapper(this, m_stiffnessTableView,
                               QStringLiteral("Export JSON"),
                               truss::ExportFormat::JSON, this);
    m_tabs->addTab(tab, QStringLiteral("Stiffness Matrix"));
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void ResultsDockPanel::onStateChanged(const truss::gui::state::WorkspaceState& state)
{
    const bool hasResults = state.hasResults();
    m_nodeTableView->setEnabled(hasResults);
    m_memberTableView->setEnabled(hasResults);
    m_resultsTableView->setEnabled(hasResults);
}

void ResultsDockPanel::onTabChanged(int index)
{
    // Lazy stiffness matrix population on first activation (tab index 3)
    constexpr int kStiffnessTab = 3;
    if (index == kStiffnessTab && !m_stiffnessPopulated) {
        m_stiffnessPopulated = true;
        // Phase 6 TODO: populate stiffness matrix widget from results view.

void ResultsDockPanel::setResultsView(
    const truss::core::interfaces::IAnalysisResultsView* results)
{
    m_resultsView = results;
    m_stiffnessPopulated = false;  // mark dirty so it re-populates on next activation

    if (results && m_tabs->currentIndex() == 3)
        populateStiffnessMatrix();  // already on the tab — populate immediately
}

void ResultsDockPanel::populateStiffnessMatrix()
{
    if (!m_resultsView || !m_stiffnessTableView) return;

    const auto& matrix = m_resultsView->getStiffnessMatrix();
    const int n = static_cast<int>(matrix.size());

    // Swap in a new model (delete old one to avoid leaks)
    QAbstractItemModel* oldModel = m_stiffnessTableView->model();
    m_stiffnessTableView->setModel(nullptr);
    delete oldModel;

    auto* model = new QStandardItemModel(n, n, m_stiffnessTableView);

    // Column / row headers: DOF index labels (u0, u1, …)
    for (int i = 0; i < n; ++i) {
        model->setHorizontalHeaderItem(i, new QStandardItem(
            QStringLiteral("u%1").arg(i)));
        model->setVerticalHeaderItem(i, new QStandardItem(
            QStringLiteral("u%1").arg(i)));
    }

    // Populate cells.  Values are in N/m (Pa × m / m); display in kN/m for
    // readability.  Diagonal entries are typically much larger than off-diagonal.
    for (int row = 0; row < n; ++row) {
        const auto& rowData = matrix[static_cast<size_t>(row)];
        for (int col = 0; col < n && col < static_cast<int>(rowData.size()); ++col) {
            const double val = rowData[static_cast<size_t>(col)] / 1.0e3;  // → kN/m
            auto* item = new QStandardItem(
                QString::number(val, 'g', 4));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);  // read-only
            model->setItem(row, col, item);
        }
    }

    m_stiffnessTableView->setModel(model);
    m_stiffnessPopulated = true;
}

}  // namespace truss::gui
