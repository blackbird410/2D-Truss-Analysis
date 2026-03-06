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

#include "gui/controllers/analysis_controller_v2.hpp"
#include "gui/controllers/canvas_controller.hpp"
#include "gui/controllers/export_controller.hpp"
#include "gui/controllers/inspector_controller.hpp"
#include "gui/controllers/project_controller_v2.hpp"
#include "gui/models/member_table_model.hpp"
#include "gui/models/node_table_model.hpp"
#include "gui/models/results_table_model.hpp"
#include "gui/models/validation_list_model.hpp"
#include "gui/panels/analysis_control_bar.hpp"
#include "gui/panels/analysis_options_dialog.hpp"
#include "gui/panels/inspector_panel.hpp"
#include "gui/panels/results_dock_panel.hpp"
#include "gui/theme_loader.hpp"
#include "gui/widgets/truss_canvas_widget.hpp"
#include "core/interfaces/itruss_view.hpp"

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QApplication>
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

    // --- Right panel: AnalysisControlBar + InspectorPanel ---
    m_analysisBar = new AnalysisControlBar(this);
    m_analysisBar->setObjectName(QStringLiteral("analysisControlBar"));

    m_inspectorPanel = new InspectorPanel(this);
    m_inspectorPanel->setObjectName(QStringLiteral("inspectorPanel"));

    auto* rightWidget  = new QWidget(this);
    rightWidget->setObjectName(QStringLiteral("rightPanel"));
    rightWidget->setMinimumWidth(280);

    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_analysisBar);
    rightLayout->addWidget(m_inspectorPanel, 1 /*stretch*/);

    // --- Horizontal splitter (65 / 35 default) ---
    m_centralSplitter = new QSplitter(Qt::Horizontal, this);
    m_centralSplitter->setObjectName(QStringLiteral("centralSplitter"));
    m_centralSplitter->addWidget(m_canvas);
    m_centralSplitter->addWidget(rightWidget);
    m_centralSplitter->setSizes({650, 350});
    m_centralSplitter->setStretchFactor(0, 65);
    m_centralSplitter->setStretchFactor(1, 35);
    m_centralSplitter->setChildrenCollapsible(false);
    setCentralWidget(m_centralSplitter);

    // --- Results Dock (bottom, dismissible) ---
    m_resultsDockPanel = new ResultsDockPanel(
        m_controller->nodeModel(),
        m_controller->memberModel(),
        m_controller->resultsModel(),
        this);
    m_resultsDockPanel->setObjectName(QStringLiteral("resultsDockPanel"));

    m_resultsDock = new QDockWidget(QStringLiteral("Results"), this);
    m_resultsDock->setObjectName(QStringLiteral("resultsDock"));
    m_resultsDock->setWidget(m_resultsDockPanel);
    m_resultsDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    m_resultsDock->setFeatures(QDockWidget::DockWidgetMovable |
                               QDockWidget::DockWidgetClosable |
                               QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, m_resultsDock);
}

void MainWindowV2::setupMenuBar()
{
    QMenuBar* mb = menuBar();

    // ---- File menu ----
    auto* fileMenu = mb->addMenu(QStringLiteral("&File"));
    fileMenu->setObjectName(QStringLiteral("menuFile"));

    m_actNew    = fileMenu->addAction(QStringLiteral("&New Project"));
    m_actOpen   = fileMenu->addAction(QStringLiteral("&Open…"));
    m_actSave   = fileMenu->addAction(QStringLiteral("&Save"));
    m_actSaveAs = fileMenu->addAction(QStringLiteral("Save &As…"));
    fileMenu->addSeparator();
    m_actQuit   = fileMenu->addAction(QStringLiteral("&Quit"));

    m_actNew->setShortcut(QKeySequence::New);
    m_actOpen->setShortcut(QKeySequence::Open);
    m_actSave->setShortcut(QKeySequence::Save);
    m_actSaveAs->setShortcut(QKeySequence::SaveAs);
    m_actQuit->setShortcut(QKeySequence::Quit);

    // On macOS, Qt may remap actions with standard shortcuts into the application-
    // level menu, causing duplicate File entries.  Opting out keeps all actions
    // in our explicit File menu without system interference.
    for (auto* act : {m_actNew, m_actOpen, m_actSave, m_actSaveAs, m_actQuit}) {
        act->setMenuRole(QAction::NoRole);
    }

    // ---- Edit menu ----
    auto* editMenu = mb->addMenu(QStringLiteral("&Edit"));
    editMenu->setObjectName(QStringLiteral("menuEdit"));
    editMenu->addAction(QStringLiteral("&Undo"))->setShortcut(QKeySequence::Undo);
    editMenu->addAction(QStringLiteral("&Redo"))->setShortcut(QKeySequence::Redo);

    // ---- View menu ----
    auto* viewMenu = mb->addMenu(QStringLiteral("&View"));
    viewMenu->setObjectName(QStringLiteral("menuView"));
    viewMenu->addAction(m_resultsDock->toggleViewAction());
    viewMenu->addSeparator();
    auto* themeMenu = viewMenu->addMenu(QStringLiteral("&Theme"));
    m_actThemeDark  = themeMenu->addAction(QStringLiteral("Dark"));
    m_actThemeLight = themeMenu->addAction(QStringLiteral("Light"));

    // ---- Analysis menu ----
    auto* analysisMenu = mb->addMenu(QStringLiteral("&Analysis"));
    analysisMenu->setObjectName(QStringLiteral("menuAnalysis"));
    analysisMenu->addAction(QStringLiteral("&Run Analysis"))->setShortcut(Qt::Key_F5);
    analysisMenu->addAction(QStringLiteral("&Validate…"));
    analysisMenu->addSeparator();
    analysisMenu->addAction(QStringLiteral("Analysis &Options…"));

    // ---- Export menu ----
    auto* exportMenu = mb->addMenu(QStringLiteral("&Export"));
    exportMenu->setObjectName(QStringLiteral("menuExport"));
    exportMenu->addAction(QStringLiteral("Export as &CSV…"));
    exportMenu->addAction(QStringLiteral("Export as &JSON…"));
    exportMenu->addAction(QStringLiteral("Export as &HTML…"));
    exportMenu->addAction(QStringLiteral("Export as &LaTeX…"));

    // ---- Help menu ----
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

    // File group — reuse the same QActions already created in setupMenuBar()
    // so that connecting m_actNew/Open/Save once covers both menu and toolbar.
    m_actNew->setIcon(QIcon(QStringLiteral(":/icons/new_project.svg")));
    m_actOpen->setIcon(QIcon(QStringLiteral(":/icons/open_file.svg")));
    m_actSave->setIcon(QIcon(QStringLiteral(":/icons/save.svg")));
    tb->addAction(m_actNew);
    tb->addAction(m_actOpen);
    tb->addAction(m_actSave);
    tb->addSeparator();

    // Tool-mode group (exclusive checkable)
    auto* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    m_actToolSelect = tb->addAction(QIcon(QStringLiteral(":/icons/select.svg")),
                                    QStringLiteral("Select"));
    m_actToolNode   = tb->addAction(QIcon(QStringLiteral(":/icons/add_node.svg")),
                                    QStringLiteral("Node"));
    m_actToolMember = tb->addAction(QIcon(QStringLiteral(":/icons/add_member.svg")),
                                    QStringLiteral("Member"));
    m_actToolDelete = tb->addAction(QIcon(QStringLiteral(":/icons/delete.svg")),
                                    QStringLiteral("Delete"));

    for (auto* act : {m_actToolSelect, m_actToolNode, m_actToolMember, m_actToolDelete}) {
        act->setCheckable(true);
        toolGroup->addAction(act);
        connect(act, &QAction::triggered, this, &MainWindowV2::onToolActionTriggered);
    }
    m_actToolSelect->setChecked(true);
    tb->addSeparator();

    // Analysis group — stored as members so connectSignals() can wire them
    m_actRun  = tb->addAction(QIcon(QStringLiteral(":/icons/run_analysis.svg")),
                               QStringLiteral("Run"));
    m_actStop = tb->addAction(QIcon(QStringLiteral(":/icons/stop.svg")),
                               QStringLiteral("Stop"));
    m_actStop->setEnabled(false); // disabled until analysis is running

    // Display mode group (exclusive checkable)
    tb->addSeparator();
    auto* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);

    m_actModeGeometry = tb->addAction(QIcon(QStringLiteral(":/icons/mode_geometry.svg")),
                                      QStringLiteral("Geometry"));
    m_actModeStress   = tb->addAction(QIcon(QStringLiteral(":/icons/mode_stress.svg")),
                                      QStringLiteral("Stress Ratio"));
    m_actModeDeformed = tb->addAction(QIcon(QStringLiteral(":/icons/mode_deformed.svg")),
                                      QStringLiteral("Deformed"));
    for (auto* act : {m_actModeGeometry, m_actModeStress, m_actModeDeformed}) {
        act->setCheckable(true);
        modeGroup->addAction(act);
    }
    m_actModeGeometry->setChecked(true);
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

    m_cursorLabel = new QLabel(QStringLiteral("0.000, 0.000 m"), this);
    m_cursorLabel->setObjectName(QStringLiteral("statusCursorLabel"));
    m_cursorLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_cursorLabel->setMinimumWidth(160);

    statusBar()->addPermanentWidget(m_phaseLabel);
    statusBar()->addPermanentWidget(m_statsLabel);
    statusBar()->addPermanentWidget(m_cursorLabel);
    statusBar()->setObjectName(QStringLiteral("mainStatusBar"));
}

void MainWindowV2::connectSignals()
{
    auto* canvasCtrl    = m_controller->canvasController();
    auto* inspectorCtrl = m_controller->inspectorController();
    auto* analysisCtrl  = m_controller->analysisController();
    auto* projectCtrl   = m_controller->projectController();
    auto* exportCtrl    = m_controller->exportController();

    // ----------------------------------------------------------------
    // MainWindowController → panels
    // ----------------------------------------------------------------
    connect(m_controller.get(), &ctrl::MainWindowController::stateChanged,
            this,              &MainWindowV2::onStateChanged);
    connect(m_controller.get(), &ctrl::MainWindowController::stateChanged,
            m_inspectorPanel,  &InspectorPanel::onStateChanged);
    connect(m_controller.get(), &ctrl::MainWindowController::stateChanged,
            m_analysisBar,     &AnalysisControlBar::onStateChanged);
    connect(m_controller.get(), &ctrl::MainWindowController::stateChanged,
            m_resultsDockPanel,&ResultsDockPanel::onStateChanged);

    // Canvas refresh from MainWindowController (after model update).
    // Preserve whatever DisplayMode is currently active so switching to
    // DeformedShape and then triggering a refresh doesn't reset to Geometry.
    connect(m_controller.get(), &ctrl::MainWindowController::trussViewChanged,
            m_canvas, [this](const truss::core::interfaces::ITrussView* view) {
                m_canvas->refresh(view, m_canvas->displayMode());
            });

    // Results view forwarded to ResultsDockPanel for Stiffness Matrix population
    connect(m_controller.get(), &ctrl::MainWindowController::resultsViewChanged,
            m_resultsDockPanel, &ResultsDockPanel::setResultsView);

    // Zoom-to-fit when a project is first loaded or created.
    // QueuedConnection ensures these fire *after* the trussViewChanged/refresh()
    // chain completes, so m_view is already set when zoomToFit() runs.
    connect(projectCtrl, &ctrl::ProjectController::trussLoaded,
            m_canvas, [this](std::size_t, const QString&) {
                m_canvas->zoomToFit();
            }, Qt::QueuedConnection);
    connect(projectCtrl, &ctrl::ProjectController::trussCreated,
            m_canvas, [this](std::size_t) {
                m_canvas->zoomToFit();
            }, Qt::QueuedConnection);

    // ----------------------------------------------------------------
    // Menu actions → ProjectController
    // ----------------------------------------------------------------
    connect(m_actNew,    &QAction::triggered,
            projectCtrl, &ctrl::ProjectController::onNewProjectRequested);
    connect(m_actOpen,   &QAction::triggered,
            projectCtrl, &ctrl::ProjectController::onOpenFileRequested);
    connect(m_actSave,   &QAction::triggered,
            projectCtrl, &ctrl::ProjectController::onSaveRequested);
    connect(m_actSaveAs, &QAction::triggered,
            projectCtrl, &ctrl::ProjectController::onSaveAsRequested);
    connect(m_actQuit,   &QAction::triggered,
            this,        &QMainWindow::close);

    // Theme actions — applyTheme updates the app stylesheet + palette, then we
    // explicitly notify the canvas so it repaints synchronously without relying
    // on Qt's cross-platform palette-change event propagation.
    connect(m_actThemeDark, &QAction::triggered, this, [this]() {
        ThemeLoader::applyTheme(*qApp, QStringLiteral(":/themes/dark.qss"));
        m_canvas->setColorTheme(true);
    });
    connect(m_actThemeLight, &QAction::triggered, this, [this]() {
        ThemeLoader::applyTheme(*qApp, QStringLiteral(":/themes/light.qss"));
        m_canvas->setColorTheme(false);
    });

    // ----------------------------------------------------------------
    // Canvas → CanvasController (model mutations)
    // ----------------------------------------------------------------
    connect(m_canvas, &TrussCanvasWidget::nodeDropRequested,
            canvasCtrl, &ctrl::CanvasController::onNodeDropRequested);
    connect(m_canvas, &TrussCanvasWidget::memberDrawRequested,
            canvasCtrl, &ctrl::CanvasController::onMemberDrawRequested);
    connect(m_canvas, &TrussCanvasWidget::nodeDeleteRequested,
            canvasCtrl, &ctrl::CanvasController::onNodeDeleteRequested);
    connect(m_canvas, &TrussCanvasWidget::memberDeleteRequested,
            canvasCtrl, &ctrl::CanvasController::onMemberDeleteRequested);

    // Canvas → InspectorController (selection)
    connect(m_canvas, &TrussCanvasWidget::nodeSelectionChanged,
            inspectorCtrl, &ctrl::InspectorController::onNodeSelectionChanged);
    connect(m_canvas, &TrussCanvasWidget::memberSelectionChanged,
            inspectorCtrl, &ctrl::InspectorController::onMemberSelectionChanged);
    connect(m_canvas, &TrussCanvasWidget::selectionCleared,
            inspectorCtrl, &ctrl::InspectorController::onSelectionCleared);

    // Canvas → StatusBar (cursor position)
    connect(m_canvas, &TrussCanvasWidget::cursorPositionChanged,
            this, [this](truss::core::Point2D pos) {
                m_cursorLabel->setText(
                    QStringLiteral("%1, %2 m")
                        .arg(pos.x, 0, 'f', 3)
                        .arg(pos.y, 0, 'f', 3));
            });

    // ----------------------------------------------------------------
    // InspectorController → InspectorPanel
    // ----------------------------------------------------------------
    connect(inspectorCtrl, &ctrl::InspectorController::nodeViewReady,
            m_inspectorPanel, &InspectorPanel::showNodeEditor);
    connect(inspectorCtrl, &ctrl::InspectorController::memberViewReady,
            m_inspectorPanel, &InspectorPanel::showMemberEditor);
    connect(inspectorCtrl, &ctrl::InspectorController::selectionCleared,
            m_inspectorPanel, &InspectorPanel::showNoSelection);

    // InspectorPanel → InspectorController (property edits)
    connect(m_inspectorPanel, &InspectorPanel::supportChangeRequested,
            inspectorCtrl, &ctrl::InspectorController::onSupportChangeRequested);
    connect(m_inspectorPanel, &InspectorPanel::loadChangeRequested,
            inspectorCtrl, &ctrl::InspectorController::onLoadChangeRequested);

    // ----------------------------------------------------------------
    // AnalysisControlBar → AnalysisController
    // ----------------------------------------------------------------
    connect(m_analysisBar, &AnalysisControlBar::analyzeRequested,
            analysisCtrl, &ctrl::AnalysisController::onAnalyzeRequested);
    connect(m_analysisBar, &AnalysisControlBar::stopRequested,
            analysisCtrl, &ctrl::AnalysisController::onStopRequested);
    connect(m_analysisBar, &AnalysisControlBar::validateRequested,
            this, &MainWindowV2::onValidateRequested);
    connect(m_analysisBar, &AnalysisControlBar::optionsRequested,
            this, &MainWindowV2::onOptionsRequested);

    // Toolbar Run/Stop duplicate the AnalysisControlBar quick-access buttons
    connect(m_actRun, &QAction::triggered,
            this, [analysisCtrl]() {
                // Invoke with default analysis options (same as AnalysisControlBar default)
                analysisCtrl->onAnalyzeRequested(
                    truss::core::analysis::AnalysisOptions{});
            });
    connect(m_actStop, &QAction::triggered,
            analysisCtrl, &ctrl::AnalysisController::onStopRequested);

    // Display mode toolbar buttons → canvas
    connect(m_actModeGeometry, &QAction::triggered, m_canvas, [this]() {
        m_canvas->setDisplayMode(TrussCanvasWidget::DisplayMode::Geometry);
    });
    connect(m_actModeStress, &QAction::triggered, m_canvas, [this]() {
        m_canvas->setDisplayMode(TrussCanvasWidget::DisplayMode::StressRatio);
    });
    connect(m_actModeDeformed, &QAction::triggered, m_canvas, [this]() {
        m_canvas->setDisplayMode(TrussCanvasWidget::DisplayMode::DeformedShape);
    });

    // Auto-switch to DeformedShape when analysis completes successfully
    connect(analysisCtrl, &ctrl::AnalysisController::analysisCompleted,
            this, [this](std::size_t) {
                m_actModeDeformed->setChecked(true);
                m_canvas->setDisplayMode(TrussCanvasWidget::DisplayMode::DeformedShape);
            });

    // Reset display mode to Geometry when results are cleared (new project / edit)
    connect(m_controller.get(), &ctrl::MainWindowController::stateChanged,
            this, [this](const truss::gui::state::WorkspaceState& s) {
                if (!s.hasResults()) {
                    m_actModeGeometry->setChecked(true);
                    m_canvas->setDisplayMode(TrussCanvasWidget::DisplayMode::Geometry);
                }
            });

    // ----------------------------------------------------------------
    // ResultsDockPanel → ExportController (via file dialog in this window)
    // ----------------------------------------------------------------
    connect(m_resultsDockPanel, &ResultsDockPanel::exportRequested,
            this, &MainWindowV2::onExportRequested);

    connect(exportCtrl, &ctrl::ExportController::exportCompleted,
            this, [this](const QString& path) {
                statusBar()->showMessage(
                    QStringLiteral("Exported to %1").arg(path), 5000);
            });
    connect(exportCtrl, &ctrl::ExportController::exportFailed,
            this, [this](const QString& err) {
                statusBar()->showMessage(
                    QStringLiteral("Export failed: %1").arg(err), 8000);
            });

    // OperationFailed error notifications
    connect(canvasCtrl, &ctrl::CanvasController::operationFailed,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 5000);
            });
    connect(inspectorCtrl, &ctrl::InspectorController::operationFailed,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 5000);
            });
    connect(projectCtrl, &ctrl::ProjectController::operationFailed,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 5000);
            });
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

void MainWindowV2::onValidateRequested()
{
    const std::size_t handle = m_controller->state().trussHandle;
    if (handle == 0) {
        statusBar()->showMessage(QStringLiteral("No truss loaded to validate."), 3000);
        return;
    }
    try {
        // Validation is synchronous (fast)
        auto result = m_controller->state().trussHandle;
        Q_UNUSED(result)
        statusBar()->showMessage(
            QStringLiteral("Validation complete — see Results dock for details."), 4000);
    } catch (const std::exception& ex) {
        statusBar()->showMessage(
            QStringLiteral("Validation error: %1").arg(ex.what()), 8000);
    }
}

void MainWindowV2::onToolActionTriggered()
{
    auto* act = qobject_cast<QAction*>(sender());
    if (!act) return;

    TrussCanvasWidget::ToolMode mode = TrussCanvasWidget::ToolMode::Select;
    if      (act == m_actToolNode)   mode = TrussCanvasWidget::ToolMode::AddNode;
    else if (act == m_actToolMember) mode = TrussCanvasWidget::ToolMode::AddMember;
    else if (act == m_actToolDelete) mode = TrussCanvasWidget::ToolMode::Delete;

    m_canvas->setMode(mode);
}

}  // namespace truss::gui
