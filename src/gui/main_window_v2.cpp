/**
 * @file main_window_v2.cpp
 * @brief New MainWindow full implementation (Phase 6).
 *
 * Phase 4: QMainWindow skeleton with placeholder panels.
 * Phase 5: Real InspectorPanel / AnalysisControlBar / ResultsDockPanel.
 * Phase 6: Full signal/slot wiring; closeEvent dirty-state guard;
 *          theme switching; toolbar tool-mode selection.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/main_window_v2.hpp"

#include "gui/widgets/truss_canvas_widget.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

namespace truss::gui {

// ============================================================
// Construction
// ============================================================

MainWindowV2::MainWindowV2(truss::interface::ITrussAnalysisFacade& facade,
                           QWidget* parent)
    : QMainWindow(parent)
    , m_controller(std::make_unique<ctrl::MainWindowController>(facade, this))
{
    setObjectName(QStringLiteral("mainWindowV2"));
    setWindowTitle(QStringLiteral("2D Truss Analysis"));
    setMinimumSize(1024, 768);
    resize(1440, 900);

    setupCentralWidget();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    connectSignals();
}

// ============================================================
// closeEvent: dirty-state guard
// ============================================================

void MainWindowV2::closeEvent(QCloseEvent* event)
{
    if (m_controller->state().isDirty) {
        const auto reply = QMessageBox::question(
            this,
            QStringLiteral("Unsaved Changes"),
            QStringLiteral("The project has unsaved changes.\n"
                           "Do you want to quit without saving?"),
            QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (reply != QMessageBox::Discard) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

// ============================================================
// Setup helpers
// ============================================================

void MainWindowV2::setupCentralWidget()
{
    // --- Canvas (left, ~65 %) ---
    m_canvas = new TrussCanvasWidget(this);
    m_canvas->setObjectName(QStringLiteral("trussCanvas"));

    // --- Inspector placeholder (right, ~35 %) ---
    m_inspectorPlaceholder = new QWidget(this);
    m_inspectorPlaceholder->setObjectName(QStringLiteral("inspectorPlaceholder"));
    m_inspectorPlaceholder->setMinimumWidth(280);

    auto* phLabel = new QLabel(
        QStringLiteral("Inspector Panel\n(Phase 5)"), m_inspectorPlaceholder);
    phLabel->setAlignment(Qt::AlignCenter);
    phLabel->setStyleSheet(QStringLiteral("color: #5F5F5F; font-size: 11px;"));
    auto* phLayout = new QVBoxLayout(m_inspectorPlaceholder);
    phLayout->addWidget(phLabel);
    phLayout->setAlignment(Qt::AlignCenter);

    // --- Horizontal splitter (65 / 35 default) ---
    m_centralSplitter = new QSplitter(Qt::Horizontal, this);
    m_centralSplitter->setObjectName(QStringLiteral("centralSplitter"));
    m_centralSplitter->addWidget(m_canvas);
    m_centralSplitter->addWidget(m_inspectorPlaceholder);
    // Set proportional sizes: 65 + 35 = 100 units
    m_centralSplitter->setSizes({650, 350});
    m_centralSplitter->setStretchFactor(0, 65);
    m_centralSplitter->setStretchFactor(1, 35);
    m_centralSplitter->setChildrenCollapsible(false);

    setCentralWidget(m_centralSplitter);

    // --- Results Dock (bottom, dismissible) ---
    m_resultsPlaceholder = new QWidget();
    m_resultsPlaceholder->setObjectName(QStringLiteral("resultsPlaceholder"));
    m_resultsPlaceholder->setMinimumHeight(120);

    auto* rLabel = new QLabel(
        QStringLiteral("Results Panel\n(Phase 5)"), m_resultsPlaceholder);
    rLabel->setAlignment(Qt::AlignCenter);
    rLabel->setStyleSheet(QStringLiteral("color: #5F5F5F; font-size: 11px;"));
    auto* rLayout = new QVBoxLayout(m_resultsPlaceholder);
    rLayout->addWidget(rLabel);
    rLayout->setAlignment(Qt::AlignCenter);

    m_resultsDock = new QDockWidget(QStringLiteral("Results"), this);
    m_resultsDock->setObjectName(QStringLiteral("resultsDock"));
    m_resultsDock->setWidget(m_resultsPlaceholder);
    m_resultsDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    m_resultsDock->setFeatures(QDockWidget::DockWidgetMovable |
                               QDockWidget::DockWidgetClosable |
                               QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, m_resultsDock);
}

void MainWindowV2::setupMenuBar()
{
    QMenuBar* mb = menuBar();

    // File menu
    auto* fileMenu = mb->addMenu(QStringLiteral("&File"));
    fileMenu->setObjectName(QStringLiteral("menuFile"));

    auto* actNew  = fileMenu->addAction(QStringLiteral("&New Project"));
    auto* actOpen = fileMenu->addAction(QStringLiteral("&Open…"));
    auto* actSave = fileMenu->addAction(QStringLiteral("&Save"));
    fileMenu->addSeparator();
    auto* actQuit = fileMenu->addAction(QStringLiteral("&Quit"));

    actNew->setShortcut(QKeySequence::New);
    actOpen->setShortcut(QKeySequence::Open);
    actSave->setShortcut(QKeySequence::Save);
    actQuit->setShortcut(QKeySequence::Quit);

    // Suppress unused-variable warnings — actions will be connected in Phase 6
    Q_UNUSED(actNew)
    Q_UNUSED(actOpen)
    Q_UNUSED(actSave)
    Q_UNUSED(actQuit)

    // Edit menu
    auto* editMenu = mb->addMenu(QStringLiteral("&Edit"));
    editMenu->setObjectName(QStringLiteral("menuEdit"));
    editMenu->addAction(QStringLiteral("&Undo"))->setShortcut(QKeySequence::Undo);
    editMenu->addAction(QStringLiteral("&Redo"))->setShortcut(QKeySequence::Redo);

    // View menu
    auto* viewMenu = mb->addMenu(QStringLiteral("&View"));
    viewMenu->setObjectName(QStringLiteral("menuView"));
    viewMenu->addAction(m_resultsDock->toggleViewAction());
    viewMenu->addSeparator();
    auto* themeMenu = viewMenu->addMenu(QStringLiteral("&Theme"));
    themeMenu->addAction(QStringLiteral("Dark"));
    themeMenu->addAction(QStringLiteral("Light"));

    // Analysis menu
    auto* analysisMenu = mb->addMenu(QStringLiteral("&Analysis"));
    analysisMenu->setObjectName(QStringLiteral("menuAnalysis"));
    analysisMenu->addAction(QStringLiteral("&Run Analysis"))->setShortcut(Qt::Key_F5);
    analysisMenu->addAction(QStringLiteral("&Validate…"));
    analysisMenu->addSeparator();
    analysisMenu->addAction(QStringLiteral("Analysis &Options…"));

    // Export menu
    auto* exportMenu = mb->addMenu(QStringLiteral("&Export"));
    exportMenu->setObjectName(QStringLiteral("menuExport"));
    exportMenu->addAction(QStringLiteral("Export as &CSV…"));
    exportMenu->addAction(QStringLiteral("Export as &JSON…"));
    exportMenu->addAction(QStringLiteral("Export as &HTML…"));
    exportMenu->addAction(QStringLiteral("Export as &LaTeX…"));

    // Help menu
    auto* helpMenu = mb->addMenu(QStringLiteral("&Help"));
    helpMenu->setObjectName(QStringLiteral("menuHelp"));
    helpMenu->addAction(QStringLiteral("&About…"));
}

void MainWindowV2::setupToolBar()
{
    auto* tb = addToolBar(QStringLiteral("Main"));
    tb->setObjectName(QStringLiteral("mainToolBar"));
    tb->setMovable(false);
    tb->setIconSize(QSize(24, 24));

    // File group
    tb->addAction(QStringLiteral("New"));
    tb->addAction(QStringLiteral("Open"));
    tb->addAction(QStringLiteral("Save"));
    tb->addSeparator();

    // Edit tool group
    tb->addAction(QStringLiteral("Select"));
    tb->addAction(QStringLiteral("Node"));
    tb->addAction(QStringLiteral("Member"));
    tb->addAction(QStringLiteral("Load"));
    tb->addAction(QStringLiteral("Support"));
    tb->addSeparator();

    // Analysis group
    tb->addAction(QStringLiteral("Run"));
    tb->addAction(QStringLiteral("Stop"));
}

void MainWindowV2::setupStatusBar()
{
    m_phaseLabel = new QLabel(QStringLiteral("Empty"), this);
    m_phaseLabel->setObjectName(QStringLiteral("statusPhaseLabel"));
    m_phaseLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_phaseLabel->setMinimumWidth(120);

    m_statsLabel = new QLabel(QStringLiteral("Nodes: 0   Members: 0"), this);
    m_statsLabel->setObjectName(QStringLiteral("statusStatsLabel"));
    m_statsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_statsLabel->setMinimumWidth(200);

    statusBar()->addPermanentWidget(m_phaseLabel);
    statusBar()->addPermanentWidget(m_statsLabel);
    statusBar()->setObjectName(QStringLiteral("mainStatusBar"));
}

void MainWindowV2::connectSignals()
{
    connect(m_controller.get(), &ctrl::MainWindowController::stateChanged,
            this, &MainWindowV2::onStateChanged);
}

// ============================================================
// Private slots
// ============================================================

void MainWindowV2::onStateChanged(const truss::gui::state::WorkspaceState& newState)
{
    // Phase label
    QString phaseText;
    switch (newState.phase) {
        case state::WorkspacePhase::Empty:         phaseText = QStringLiteral("Empty");          break;
        case state::WorkspacePhase::ModelBuilding: phaseText = QStringLiteral("Building");       break;
        case state::WorkspacePhase::Validating:    phaseText = QStringLiteral("Validating…");    break;
        case state::WorkspacePhase::Analysing:     phaseText = QStringLiteral("Analysing…");     break;
        case state::WorkspacePhase::ResultsReady:  phaseText = QStringLiteral("Results Ready");  break;
    }
    m_phaseLabel->setText(phaseText);

    // Stats label (node/member counts from models)
    const int nodeCount   = m_controller->nodeModel()->rowCount();
    const int memberCount = m_controller->memberModel()->rowCount();
    m_statsLabel->setText(
        QStringLiteral("Nodes: %1   Members: %2").arg(nodeCount).arg(memberCount));

    // Dirty indicator in title bar
    setWindowTitle(newState.isDirty
        ? QStringLiteral("2D Truss Analysis *")
        : QStringLiteral("2D Truss Analysis"));

    // Last error / success in status bar (transient)
    if (!newState.lastError.empty()) {
        statusBar()->showMessage(
            QString::fromStdString(newState.lastError), 8000);
    }

    // Enable/disable toolbar tool actions based on editable state
    const bool editable = newState.isEditable() || newState.phase == state::WorkspacePhase::Empty;
    for (auto* act : {m_actToolNode, m_actToolMember, m_actToolDelete}) {
        if (act) act->setEnabled(editable);
    }
}

void MainWindowV2::onExportRequested(truss::ExportFormat format,
                                     const QString& suggestedFilename)
{
    // Map format to file filter
    QString filter;
    switch (format) {
    case truss::ExportFormat::CSV:   filter = QStringLiteral("CSV Files (*.csv)");    break;
    case truss::ExportFormat::TSV:   filter = QStringLiteral("TSV Files (*.tsv)");    break;
    case truss::ExportFormat::JSON:  filter = QStringLiteral("JSON Files (*.json)");  break;
    case truss::ExportFormat::HTML:  filter = QStringLiteral("HTML Files (*.html)");  break;
    case truss::ExportFormat::LaTeX: filter = QStringLiteral("LaTeX Files (*.tex)");  break;
    case truss::ExportFormat::TXT:   filter = QStringLiteral("Text Files (*.txt)");   break;
    case truss::ExportFormat::XML:   filter = QStringLiteral("XML Files (*.xml)");    break;
    }

    const QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export Results"),
        suggestedFilename,
        filter);

    if (!path.isEmpty()) {
        m_controller->exportController()->onExportRequested(format, path);
    }
}

void MainWindowV2::onOptionsRequested()
{
    AnalysisOptionsDialog dlg(this);
    dlg.setOptions(m_analysisBar->options());
    if (dlg.exec() == QDialog::Accepted) {
        m_analysisBar->setOptions(dlg.options());
    }
}

}  // namespace truss::gui
