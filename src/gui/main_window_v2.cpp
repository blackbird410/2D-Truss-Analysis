/**
 * @file main_window_v2.cpp
 * @brief New MainWindow skeleton implementation (Phase 4).
 *
 * Constructs a workspace-centric layout: horizontal QSplitter (65/35),
 * bottom QDockWidget, menu bar stubs, QToolBar stubs, QStatusBar with
 * phase + stats labels.  All secondary panels are QWidget placeholders
 * pending Phase 5.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/main_window_v2.hpp"

#include "gui/widgets/truss_canvas_widget.hpp"

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
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
// Slots
// ============================================================

void MainWindowV2::onStateChanged(const truss::gui::state::WorkspaceState& newState)
{
    // Update phase label
    QString phaseText;
    switch (newState.phase) {
        case state::WorkspacePhase::Empty:
            phaseText = QStringLiteral("Empty");
            break;
        case state::WorkspacePhase::ModelBuilding:
            phaseText = QStringLiteral("Building");
            break;
        case state::WorkspacePhase::Validating:
            phaseText = QStringLiteral("Validating");
            break;
        case state::WorkspacePhase::Analysing:
            phaseText = QStringLiteral("Analysing…");
            break;
        case state::WorkspacePhase::ResultsReady:
            phaseText = QStringLiteral("Results Ready");
            break;
    }
    m_phaseLabel->setText(phaseText);

    // Dirty indicator
    if (newState.isDirty) {
        setWindowTitle(QStringLiteral("2D Truss Analysis *"));
    } else {
        setWindowTitle(QStringLiteral("2D Truss Analysis"));
    }

    // Show last error in the status bar (transient)
    if (!newState.lastError.empty()) {
        statusBar()->showMessage(
            QString::fromStdString(newState.lastError), 5000);
    }

    // TODO Phase 6: Enable/disable toolbar actions based on newState.phase.
}

}  // namespace truss::gui
