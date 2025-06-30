/**
 * @file MainWindow.cpp
 * @brief Implementation of the main window for the 2D Truss Analysis GUI
 */

#include "MainWindow.hpp"
#include "ProjectFileManager.hpp"
#include "ResultsExporter.hpp"
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>
#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtGui/QCloseEvent>

namespace truss::gui {

MainWindow::MainWindow(QWidget* parent) 
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
      m_analysisEngine(std::make_unique<truss::core::AnalysisEngine>()),
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
    connect(newAction, &QAction::triggered, this, &MainWindow::newProject);
    
    auto* openAction = fileMenu->addAction("&Open...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openProject);
    
    auto* saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveProject);
    
    auto* saveAsAction = fileMenu->addAction("Save &As...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveProjectAs);
    
    fileMenu->addSeparator();
    
    auto* exportAction = fileMenu->addAction("&Export Results...");
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportResults);
    
    fileMenu->addSeparator();
    
    auto* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    
    // Analysis menu
    auto* analysisMenu = menuBar->addMenu("&Analysis");
    auto* analyzeAction = analysisMenu->addAction("&Analyze Structure");
    analyzeAction->setShortcut(QKeySequence("F5"));
    connect(analyzeAction, &QAction::triggered, this, &MainWindow::analyze);
    
    auto* clearAction = analysisMenu->addAction("&Clear All");
    clearAction->setShortcut(QKeySequence("Ctrl+Del"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearAll);
    
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
    connect(m_analyzeButton, &QPushButton::clicked, this, &MainWindow::analyze);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearAll);
}

truss::core::Truss* MainWindow::getTruss() const {
    return m_drawingWidget->getTruss();
}

void MainWindow::analyze() {
    auto* truss = getTruss();
    if (!truss) {
        showErrorMessage("No truss structure available for analysis.");
        return;
    }
    
    // Check if we have at least some nodes and members
    if (truss->getNodes().empty()) {
        showErrorMessage("Cannot analyze: No nodes defined.\nUse the drawing tools to create your truss structure.");
        return;
    }
    
    if (truss->getMembers().empty()) {
        showErrorMessage("Cannot analyze: No members defined.\nAdd members to connect your nodes.");
        return;
    }
    
    // Check if there are any supports
    bool hasSupports = false;
    for (const auto& node : truss->getNodes()) {
        if (node->getSupportType() != truss::core::SupportType::Free) {
            hasSupports = true;
            break;
        }
    }
    if (!hasSupports) {
        showErrorMessage("Cannot analyze: No supports defined.\nAdd support conditions to prevent rigid body motion.");
        return;
    }
    
    // Check if there are any loads
    bool hasLoads = false;
    for (const auto& node : truss->getNodes()) {
        if (node->hasAppliedForce()) {
            hasLoads = true;
            break;
        }
    }
    if (!hasLoads) {
        showErrorMessage("Cannot analyze: No loads defined.\nApply loads to your structure for analysis.");
        return;
    }
    
    try {
        m_statusLabel->setText("Analyzing structure...");
        m_logTextEdit->append("Starting structural analysis...");
        QApplication::processEvents(); // Update UI
        
        // Perform analysis
        auto analysisResults = m_analysisEngine->analyze(*truss);
        
        if (analysisResults.converged) {
            m_hasResults = true;
            updateResultsDisplay();
            
            QString message = QString("Analysis completed successfully!\n")
                + QString("Nodes: %1, Members: %2\n").arg(truss->getNodeCount()).arg(truss->getMemberCount())
                + QString("Max displacement: %1 mm\n").arg(analysisResults.maxDisplacement * 1000, 0, 'f', 3)
                + QString("Max stress: %1 MPa").arg(analysisResults.maxStress / 1e6, 0, 'f', 2);
            
            m_logTextEdit->append("Analysis completed successfully!");
            m_logTextEdit->append(QString("Max displacement: %1 mm").arg(analysisResults.maxDisplacement * 1000, 0, 'f', 3));
            m_logTextEdit->append(QString("Max stress: %1 MPa").arg(analysisResults.maxStress / 1e6, 0, 'f', 2));
            
            showInfoMessage(message);
            m_statusLabel->setText("Analysis complete - View results in the results tab");
        } else {
            showErrorMessage("Analysis failed to converge. Check your structure and loads.");
            m_logTextEdit->append("ERROR: Analysis failed to converge");
            m_statusLabel->setText("Analysis failed");
        }
        
    } catch (const std::exception& e) {
        showErrorMessage(QString("Analysis error: %1").arg(e.what()));
        m_logTextEdit->append(QString("ERROR: %1").arg(e.what()));
        m_statusLabel->setText("Analysis error");
    }
}

void MainWindow::clearAll() {
    m_drawingWidget->clearTruss();
    m_hasResults = false;
    
    // Clear results
    m_resultsWidget->clearResults();
    m_logTextEdit->clear();
    
    m_statusLabel->setText("Project cleared - Ready to design new structure");
    enableAnalysis(false);
}

void MainWindow::exitApplication() {
    close();
}

void MainWindow::newProject() {
    clearAll();
    m_currentFileName.clear();
    setWindowTitle("2D Truss Analysis - Interactive Design");
}

void MainWindow::openProject() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Truss Project", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Truss Project Files (*.truss);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        auto loadedTruss = ProjectFileManager::loadProject(fileName);
        
        if (loadedTruss) {
            // Clear current project
            clearAll();
            
            // Load the truss into the drawing widget
            m_drawingWidget->setTruss(std::move(loadedTruss));
            
            // Update UI
            m_currentFileName = fileName;
            QFileInfo fileInfo(fileName);
            setWindowTitle(QString("2D Truss Analysis - %1").arg(fileInfo.baseName()));
            
            onTrussModified();
            m_statusLabel->setText(QString("Project loaded: %1").arg(fileInfo.fileName()));
            
            showInfoMessage("Project loaded successfully!");
        } else {
            showErrorMessage(QString("Failed to load project:\n%1").arg(ProjectFileManager::getLastError()));
            m_statusLabel->setText("Failed to load project");
        }
    }
}

void MainWindow::saveProject() {
    if (m_currentFileName.isEmpty()) {
        saveProjectAs();
    } else {
        auto* truss = getTruss();
        if (!truss) {
            showErrorMessage("No project to save.");
            return;
        }
        
        if (ProjectFileManager::saveProject(*truss, m_currentFileName)) {
            QFileInfo fileInfo(m_currentFileName);
            m_statusLabel->setText(QString("Project saved: %1").arg(fileInfo.fileName()));
            showInfoMessage("Project saved successfully!");
        } else {
            showErrorMessage(QString("Failed to save project:\n%1").arg(ProjectFileManager::getLastError()));
            m_statusLabel->setText("Failed to save project");
        }
    }
}

void MainWindow::saveProjectAs() {
    auto* truss = getTruss();
    if (!truss) {
        showErrorMessage("No project to save.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Truss Project", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Truss Project Files (*.truss);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        // Ensure .truss extension
        if (!fileName.endsWith(".truss", Qt::CaseInsensitive)) {
            fileName += ".truss";
        }
        
        if (ProjectFileManager::saveProject(*truss, fileName)) {
            m_currentFileName = fileName;
            QFileInfo fileInfo(fileName);
            setWindowTitle(QString("2D Truss Analysis - %1").arg(fileInfo.baseName()));
            m_statusLabel->setText(QString("Project saved: %1").arg(fileInfo.fileName()));
            showInfoMessage("Project saved successfully!");
        } else {
            showErrorMessage(QString("Failed to save project:\n%1").arg(ProjectFileManager::getLastError()));
            m_statusLabel->setText("Failed to save project");
        }
    }
}

void MainWindow::exportResults() {
    if (!m_hasResults) {
        showErrorMessage("No analysis results to export. Run analysis first.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Analysis Results", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV Files (*.csv);;TSV Files (*.tsv);;JSON Files (*.json);;XML Files (*.xml);;Text Files (*.txt);;LaTeX Files (*.tex);;HTML Files (*.html);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        truss::core::ResultsExporter exporter;
        truss::core::AnalysisResults results = m_analysisEngine->getLastResults();
        truss::core::ExportOptions options;
        options.includeGeometry = true;
        options.includeDisplacements = true;
        options.includeMemberForces = true;
        options.includeReactions = true;
        options.includeMetadata = true;
        options.precision = 6;

        // Determine format from file extension
        truss::core::ExportFormat format = truss::core::ResultsExporter::detectFormat(fileName.toStdString());
        
        if (exporter.exportResults(*getTruss(), results, fileName.toStdString(), format, options)) {
            QFileInfo fileInfo(fileName);
            m_statusLabel->setText(QString("Results exported: %1").arg(fileInfo.fileName()));
            showInfoMessage(QString("Results exported successfully to %1!").arg(fileInfo.fileName()));
        } else {
            showErrorMessage(QString("Failed to export results: %1").arg(QString::fromStdString(exporter.getLastError())));
            m_statusLabel->setText("Failed to export results");
        }
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

void MainWindow::onTrussModified() {
    auto* truss = getTruss();
    bool hasStructure = truss && !truss->getNodes().empty() && !truss->getMembers().empty();
    enableAnalysis(hasStructure);
    
    if (hasStructure) {
        m_statusLabel->setText(QString("Structure updated - %1 nodes, %2 members")
                             .arg(truss->getNodeCount()).arg(truss->getMemberCount()));
    }
}

void MainWindow::updateStatusMessage(const QString& message) {
    m_statusLabel->setText(message);
}

void MainWindow::updateResultsDisplay() {
    m_resultsWidget->updateResults();
    
    // Update deformed truss visualization
    auto* truss = getTruss();
    if (truss && m_hasResults) {
        m_deformedTrussWidget->setTruss(truss);
        m_deformedTrussWidget->setAnalysisResults(m_analysisEngine->getLastResults());
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

