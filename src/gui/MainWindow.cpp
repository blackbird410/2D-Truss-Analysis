/**
 * @file MainWindow.cpp
 * @brief Implementation of the main window for the 2D Truss Analysis GUI
 */

#include "MainWindow.hpp"
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>
#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtGui/QCloseEvent>

namespace truss::gui {

MainWindow::MainWindow(
    application::TrussApplicationService& trussService,
    application::AnalysisApplicationService& analysisService,
    truss_controllers::AnalysisController& analysisController,
    truss_controllers::ProjectController& projectController,
    truss_presenters::AnalysisResultsPresenter& analysisPresenter,
    truss_presenters::TrussDataPresenter& trussDataPresenter,
    truss_presenters::ValidationPresenter& validationPresenter,
    QWidget* parent) 
    : QMainWindow(parent),
      m_centralWidget(new QWidget(this)),
      m_mainSplitter(new QSplitter(Qt::Vertical, this)),
      m_drawingWidget(new InteractiveDrawingWidget(this)),
      m_resultsTabWidget(new QTabWidget(this)),
      m_resultsWidget(new ResultsWidget(this)),
      m_deformedTrussWidget(new DeformedTrussWidget(this)),
      m_logTextEdit(new QTextEdit(this)),
      m_analyzeButton(new QPushButton("Analyze Structure", this)),
      m_clearButton(new QPushButton("Clear All", this)),
      m_statusLabel(new QLabel(this)),
      m_coordinateLabel(new QLabel(this)),
      m_trussService(trussService),
      m_analysisService(analysisService),
      m_analysisController(analysisController),
      m_projectController(projectController),
      m_analysisPresenter(analysisPresenter),
      m_trussDataPresenter(trussDataPresenter),
      m_validationPresenter(validationPresenter),
      m_lastResultsHandle(0),
      m_hasResults(false) {
          
    setupUI();
    connectSignals();
    
    // Set window properties
    setWindowTitle("2D Truss Analysis - Interactive Design");
    
    // Configure window for Linux display compatibility
    setupWindowProperties();
    
    // Enable analysis when there's something to analyze
    enableAnalysis(false);
}

void MainWindow::setupUI() {
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    setCentralWidget(m_centralWidget);
    
    // Create main layout
    auto* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Add main splitter
    mainLayout->addWidget(m_mainSplitter, 1);
    
    // Add interactive drawing widget to main area
    m_mainSplitter->addWidget(m_drawingWidget);
    
    // Setup results area
    m_resultsTabWidget->addTab(m_resultsWidget, "Analysis Results");
    m_resultsTabWidget->addTab(m_deformedTrussWidget, "Deformed Structure");
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumHeight(150);
    m_resultsTabWidget->addTab(m_logTextEdit, "Analysis Log");
    
    m_mainSplitter->addWidget(m_resultsTabWidget);
    m_mainSplitter->setSizes({700, 200});
    m_mainSplitter->setCollapsible(1, false);
    
    // Analysis controls
    auto* controlLayout = new QHBoxLayout();
    m_analyzeButton->setMinimumHeight(35);
    m_analyzeButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    m_clearButton->setMinimumHeight(35);
    
    controlLayout->addWidget(m_analyzeButton);
    controlLayout->addWidget(m_clearButton);
    controlLayout->addStretch();
    
    mainLayout->addLayout(controlLayout);
}

void MainWindow::setupMenuBar() {
    auto* menuBar = this->menuBar();
    
    // File menu
    auto* fileMenu = menuBar->addMenu("&File");
    auto* newAction = fileMenu->addAction("&New Project");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::requestNewProject);
    
    auto* openAction = fileMenu->addAction("&Open...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::requestOpenProject);
    
    auto* saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::requestSaveProject);
    
    auto* saveAsAction = fileMenu->addAction("Save &As...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::requestSaveProjectAs);
    
    fileMenu->addSeparator();
    
    auto* exportAction = fileMenu->addAction("&Export Results...");
    connect(exportAction, &QAction::triggered, this, &MainWindow::requestExportResults);
    
    fileMenu->addSeparator();
    
    auto* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    
    // Analysis menu
    auto* analysisMenu = menuBar->addMenu("&Analysis");
    auto* analyzeAction = analysisMenu->addAction("&Analyze Structure");
    analyzeAction->setShortcut(QKeySequence("F5"));
    connect(analyzeAction, &QAction::triggered, this, &MainWindow::requestAnalyze);
    
    auto* clearAction = analysisMenu->addAction("&Clear All");
    clearAction->setShortcut(QKeySequence("Ctrl+Del"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::requestClearAll);
    
    // Help menu
    auto* helpMenu = menuBar->addMenu("&Help");
    auto* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupToolBar() {
    // Main toolbar is handled by the interactive drawing widget
    // We can add additional actions here if needed
}

void MainWindow::setupStatusBar() {
    auto* statusBar = this->statusBar();
    statusBar->addWidget(m_statusLabel, 1);
    statusBar->addPermanentWidget(m_coordinateLabel);
    
    m_statusLabel->setText("Ready - Use toolbar to start designing your truss structure");
    m_coordinateLabel->setText("Coordinates: (0.000, 0.000)");
}

void MainWindow::setupWindowProperties() {
    // Get primary screen information
    QScreen* primaryScreen = QApplication::primaryScreen();
    if (!primaryScreen) {
        // Fallback for older Qt versions or unusual setups
        setMinimumSize(800, 600);
        resize(1200, 800);
        return;
    }
    
    QRect screenGeometry = primaryScreen->availableGeometry();
    QSize screenSize = screenGeometry.size();
    
    // Calculate window size based on screen resolution
    int windowWidth, windowHeight;
    
    // For high-resolution displays (>= 2K), use larger window
    if (screenSize.width() >= 2560 || screenSize.height() >= 1440) {
        windowWidth = static_cast<int>(screenSize.width() * 0.85);
        windowHeight = static_cast<int>(screenSize.height() * 0.85);
    }
    // For standard HD displays
    else if (screenSize.width() >= 1920 || screenSize.height() >= 1080) {
        windowWidth = static_cast<int>(screenSize.width() * 0.80);
        windowHeight = static_cast<int>(screenSize.height() * 0.80);
    }
    // For smaller displays
    else {
        windowWidth = static_cast<int>(screenSize.width() * 0.90);
        windowHeight = static_cast<int>(screenSize.height() * 0.85);
    }
    
    // Set minimum size constraints
    setMinimumSize(800, 600);
    
    // Set initial window size
    resize(windowWidth, windowHeight);
    
    // Center the window on screen
    int x = (screenSize.width() - windowWidth) / 2 + screenGeometry.x();
    int y = (screenSize.height() - windowHeight) / 2 + screenGeometry.y();
    move(x, y);
    
    // Enable proper window resizing and state management
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Load saved window geometry if available
    QSettings settings;
    if (settings.contains("MainWindow/geometry")) {
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    }
    
    if (settings.contains("MainWindow/windowState")) {
        restoreState(settings.value("MainWindow/windowState").toByteArray());
    }
    
    // Add fullscreen support with F11 key
    auto* fullscreenAction = new QAction("Toggle Fullscreen", this);
    fullscreenAction->setShortcut(QKeySequence("F11"));
    connect(fullscreenAction, &QAction::triggered, this, [this]() {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    });
    addAction(fullscreenAction);
    
    // Handle high-DPI scaling
    // Note: High-DPI support is enabled by default in Qt6
    
    // Set window icon if available
    // setWindowIcon(QIcon(":/icons/app-icon.png"));
}

MainWindow::~MainWindow() {
    // Save window geometry and state
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Save window geometry and state before closing
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());
    
    // Accept the close event
    event->accept();
}

void MainWindow::connectSignals() {
    // Connect drawing widget signals
    connect(m_drawingWidget, &InteractiveDrawingWidget::trussModified,
            this, &MainWindow::onTrussModified);
    connect(m_drawingWidget, &InteractiveDrawingWidget::statusMessage,
            this, &MainWindow::updateStatusMessage);
    
    // Connect control buttons
    connect(m_analyzeButton, &QPushButton::clicked, this, &MainWindow::requestAnalyze);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::requestClearAll);
    
    // Connect AnalysisController signals
    connect(&m_analysisController, &truss_controllers::AnalysisController::analysisCompleted,
            this, &MainWindow::onAnalysisCompleted);
    connect(&m_analysisController, &truss_controllers::AnalysisController::analysisFailed,
            this, &MainWindow::onAnalysisFailed);
    connect(&m_analysisController, &truss_controllers::AnalysisController::validationFailed,
            this, &MainWindow::onValidationFailed);
    
    // Connect ProjectController signals
    connect(&m_projectController, &truss_controllers::ProjectController::projectOpened,
            this, &MainWindow::onProjectOpened);
    connect(&m_projectController, &truss_controllers::ProjectController::projectSaved,
            this, &MainWindow::onProjectSaved);
    connect(&m_projectController, &truss_controllers::ProjectController::projectClosed,
            this, &MainWindow::onProjectClosed);
    connect(&m_projectController, &truss_controllers::ProjectController::operationFailed,
            this, &MainWindow::onOperationFailed);
}

application::TrussHandle MainWindow::getCurrentTrussHandle() const {
    return m_projectController.getCurrentTruss();
}

// Temporary legacy accessor for old widgets
truss::core::Truss* MainWindow::getTruss() const {
    return m_drawingWidget->getTruss();
}

void MainWindow::requestAnalyze() {
    auto handle = getCurrentTrussHandle();
    if (handle == 0) {
        showErrorMessage("No truss structure available for analysis.");
        return;
    }
    
    m_statusLabel->setText("Analyzing structure...");
    m_logTextEdit->append("Starting structural analysis...");
    QApplication::processEvents(); // Update UI
    
    // Delegate to AnalysisController
    m_analysisController.onAnalyzeRequested(handle);
}

void MainWindow::requestClearAll() {
    // Clear drawing widget
    m_drawingWidget->clearTruss();
    
    // Clear results via controller
    m_analysisController.onClearResults();
    
    // Clear UI
    m_resultsWidget->clearResults();
    m_logTextEdit->clear();
    m_hasResults = false;
    m_lastResultsHandle = 0;
    
    m_statusLabel->setText("Project cleared - Ready to design new structure");
    enableAnalysis(false);
}

void MainWindow::requestNewProject() {
    requestClearAll();
    m_projectController.onNewProject();
}

void MainWindow::requestOpenProject() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Truss Project", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Truss Project Files (*.truss);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        m_projectController.onOpenProject(fileName);
    }
}

void MainWindow::requestSaveProject() {
    m_projectController.onSaveProject();
}

void MainWindow::requestSaveProjectAs() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Truss Project", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Truss Project Files (*.truss);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        // Ensure .truss extension
        if (!fileName.endsWith(".truss", Qt::CaseInsensitive)) {
            fileName += ".truss";
        }
        
        m_projectController.onSaveProjectAs(fileName);
    }
}

void MainWindow::requestExportResults() {
    if (!m_hasResults) {
        showErrorMessage("No analysis results to export. Run analysis first.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Analysis Results", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV Files (*.csv);;TSV Files (*.tsv);;JSON Files (*.json);;XML Files (*.xml);;Text Files (*.txt);;LaTeX Files (*.tex);;HTML Files (*.html);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        m_analysisController.onExportRequested(m_lastResultsHandle, fileName);
    }
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "About 2D Truss Analysis",
        "<h3>2D Truss Analysis v2.1.1</h3>"
        "<p>Professional structural analysis software for 2D truss structures with advanced visualization capabilities.</p>"
        "<p><b>New Features in v2.1.1:</b></p>"
        "<ul>"
        "<li>Deformed structure visualization with interactive controls</li>"
        "<li>Color-coded member force display (tension/compression)</li>"
        "<li>Displacement vector arrows with configurable scaling</li>"
        "<li>Support reaction force visualization</li>"
        "<li>Complete project save/load functionality with JSON format</li>"
        "<li>Member intersection detection and geometric analysis</li>"
        "<li>Professional pan/zoom controls with grid overlay</li>"
        "</ul>"
        "<p><b>Core Features:</b></p>"
        "<ul>"
        "<li>Interactive drawing interface with real-time feedback</li>"
        "<li>Advanced finite element analysis engine</li>"
        "<li>Material and section property management</li>"
        "<li>Comprehensive results analysis and reporting</li>"
        "<li>Professional-grade accuracy and performance</li>"
        "</ul>"
        "<p><b>© 2024 Civil Engineering Software Solutions</b></p>");
}

// Controller signal handlers
void MainWindow::onAnalysisCompleted(size_t resultsHandle) {
    m_lastResultsHandle = resultsHandle;
    m_hasResults = true;
    
    // Get results view and truss view
    const auto& results = m_analysisService.getResultsView(resultsHandle);
    const auto& truss = m_trussService.getTrussView(getCurrentTrussHandle());
    
    // Format results using Presenter
    auto displayData = m_analysisPresenter.formatResults(results, truss);
    
    // Update display
    updateResultsDisplay();
    
    // Update log
    m_logTextEdit->append("Analysis completed successfully!");
    m_logTextEdit->append(displayData.maxDisplacementText);
    m_logTextEdit->append(displayData.maxStressText);
    
    // Show summary
    showInfoMessage(displayData.summaryMessage);
    m_statusLabel->setText("Analysis complete - View results in the results tab");
}

void MainWindow::onAnalysisFailed(const QString& errorMessage) {
    showErrorMessage(QString("Analysis failed: %1").arg(errorMessage));
    m_logTextEdit->append(QString("ERROR: %1").arg(errorMessage));
    m_statusLabel->setText("Analysis failed");
}

void MainWindow::onValidationFailed(const truss_presenters::ValidationPresenter::ValidationDisplay& display) {
    showErrorMessage(display.summaryMessage);
    
    // Log detailed errors
    for (const auto& error : display.fatalErrors) {
        m_logTextEdit->append(QString("FATAL: %1").arg(error));
    }
    for (const auto& error : display.errors) {
        m_logTextEdit->append(QString("ERROR: %1").arg(error));
    }
    for (const auto& warning : display.warnings) {
        m_logTextEdit->append(QString("WARNING: %1").arg(warning));
    }
    
    m_statusLabel->setText("Validation failed - fix errors and try again");
}

void MainWindow::onProjectOpened(application::TrussHandle, const QString& filepath) {
    // Update drawing widget with loaded truss
    // Note: InteractiveDrawingWidget will be refactored in Phase 3D
    // For now, use temporary workaround
    QFileInfo fileInfo(filepath);
    setWindowTitle(QString("2D Truss Analysis - %1").arg(fileInfo.baseName()));
    
    onTrussModified();
    m_statusLabel->setText(QString("Project loaded: %1").arg(fileInfo.fileName()));
    showInfoMessage("Project loaded successfully!");
}

void MainWindow::onProjectSaved(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    setWindowTitle(QString("2D Truss Analysis - %1").arg(fileInfo.baseName()));
    m_statusLabel->setText(QString("Project saved: %1").arg(fileInfo.fileName()));
    showInfoMessage("Project saved successfully!");
}

void MainWindow::onProjectClosed() {
    setWindowTitle("2D Truss Analysis - Interactive Design");
    m_statusLabel->setText("Ready - Use toolbar to start designing your truss structure");
}

void MainWindow::onOperationFailed(const QString& errorMessage) {
    showErrorMessage(errorMessage);
    m_statusLabel->setText("Operation failed");
}

void MainWindow::exitApplication() {
    close();
}

void MainWindow::onTrussModified() {
    auto handle = getCurrentTrussHandle();
    if (handle == 0) {
        enableAnalysis(false);
        return;
    }
    
    const auto& truss = m_trussService.getTrussView(handle);
    bool hasStructure = truss.getNodeCount() > 0 && truss.getMemberCount() > 0;
    enableAnalysis(hasStructure);
    
    if (hasStructure) {
        // Format status message using Presenter
        auto statusData = m_trussDataPresenter.formatStatus(truss);
        m_statusLabel->setText(statusData.statusMessage);
    }
    
    // Mark project as modified
    m_projectController.markAsModified();
}

void MainWindow::updateStatusMessage(const QString& message) {
    m_statusLabel->setText(message);
}

void MainWindow::updateResultsDisplay() {
    m_resultsWidget->updateResults();
    
    // Update deformed truss visualization
    if (m_hasResults) {
        // TODO Phase 3E: Refactor DeformedTrussWidget to use IAnalysisResultsView
        auto& results = m_analysisService.getResults(m_lastResultsHandle);
        m_deformedTrussWidget->setTruss(m_drawingWidget->getTruss());
        m_deformedTrussWidget->setAnalysisResults(results);
    }
}

void MainWindow::showErrorMessage(const QString& message) {
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::showInfoMessage(const QString& message) {
    QMessageBox::information(this, "Analysis Complete", message);
}

void MainWindow::enableAnalysis(bool enable) {
    m_analyzeButton->setEnabled(enable);
    if (enable) {
        m_analyzeButton->setToolTip("Run structural analysis on the current design");
    } else {
        m_analyzeButton->setToolTip("Create a structure with nodes, members, supports, and loads to enable analysis");
    }
}

} // namespace truss::gui

