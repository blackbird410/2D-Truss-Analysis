/**
 * @file MainWindow.cpp
 * @brief Implementation of the main window for the 2D Truss Analysis GUI
 */

#include "MainWindow.hpp"

namespace truss::gui {

MainWindow::MainWindow(QWidget* parent) 
    : QMainWindow(parent),
      m_centralWidget(new QWidget(this)),
      m_mainSplitter(new QSplitter(Qt::Horizontal, this)),
      m_inputTabWidget(new QTabWidget(this)),
      m_nodeInputWidget(new NodeInputWidget(this)),
      m_memberInputWidget(new MemberInputWidget(this)),
      m_loadInputWidget(new LoadInputWidget(this)),
      m_dataTableWidget(new DataTableWidget(this)),
      m_resultsWidget(new ResultsWidget(this)),
      m_plotWidget(new PlotWidget(this)),
      m_analyzeButton(new QPushButton("Analyze", this)),
      m_clearButton(new QPushButton("Clear All", this)),
      m_exitButton(new QPushButton("Exit", this)),
      m_statusLabel(new QLabel(this)),
      m_logTextEdit(new QTextEdit(this)),
      m_truss(std::make_unique<truss::core::Truss>()),
      m_analysisEngine(std::make_unique<truss::core::AnalysisEngine>()),
      m_hasResults(false) {
          
    setupUI();
    connectSignals();
}

void MainWindow::setupUI() {
    setWindowTitle("2D Truss Analysis");
    resize(1200, 800);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    setCentralWidget(m_centralWidget);
    
    m_inputTabWidget->addTab(m_nodeInputWidget, "Nodes");
    m_inputTabWidget->addTab(m_memberInputWidget, "Members");
    m_inputTabWidget->addTab(m_loadInputWidget, "Loads");
    
    auto* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_inputTabWidget);
    mainLayout->addWidget(m_dataTableWidget);
    mainLayout->addWidget(m_mainSplitter);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_analyzeButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_exitButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_analyzeButton, SIGNAL(clicked()), this, SLOT(analyze()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearAll()));
    connect(m_exitButton, SIGNAL(clicked()), this, SLOT(exitApplication()));

    m_logTextEdit->setReadOnly(true);
    mainLayout->addWidget(m_logTextEdit);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* fileMenu = menuBar->addMenu("File");

    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    setMenuBar(menuBar);
}

void MainWindow::setupToolBar() {
    QToolBar* toolBar = addToolBar("Main Toolbar");
    toolBar->addAction("Analyze", this, SLOT(analyze()));
    toolBar->addAction("Clear All", this, SLOT(clearAll()));
    toolBar->addAction("Exit", this, SLOT(exitApplication()));
}

void MainWindow::setupStatusBar() {
    statusBar()->addWidget(m_statusLabel);
    m_statusLabel->setText("Ready");
}

void MainWindow::connectSignals() {
    connect(m_nodeInputWidget, SIGNAL(nodeAdded()), this, SLOT(updateDataTable()));
    connect(m_nodeInputWidget, SIGNAL(nodeAdded()), m_memberInputWidget, SLOT(updateNodeList()));
    connect(m_nodeInputWidget, SIGNAL(nodeAdded()), m_loadInputWidget, SLOT(updateNodeList()));
    connect(m_memberInputWidget, SIGNAL(memberAdded()), this, SLOT(updateDataTable()));
    connect(m_loadInputWidget, SIGNAL(loadAdded()), this, SLOT(updateDataTable()));
}

void MainWindow::addNode() {
    // Implementation to add a node
    m_statusLabel->setText("Node added");
}

void MainWindow::addMember() {
    // Implementation to add a member
    m_statusLabel->setText("Member added");
}

void MainWindow::addLoad() {
    // Implementation to add a load
    m_statusLabel->setText("Load added");
}

void MainWindow::showData() {
    // Implementation to show data
}

void MainWindow::analyze() {
    if (!m_truss) {
        showErrorMessage("No truss structure available for analysis.");
        return;
    }
    
    // Check if we have at least some nodes and members
    if (m_truss->getNodes().empty()) {
        showErrorMessage("Cannot analyze: No nodes defined.");
        return;
    }
    
    if (m_truss->getMembers().empty()) {
        showErrorMessage("Cannot analyze: No members defined.");
        return;
    }
    
    // Check structural validity
    if (!m_analysisEngine->checkStructuralValidity(*m_truss)) {
        showErrorMessage("Structure is not valid for analysis. Check supports and member connectivity.");
        return;
    }
    
    try {
        m_statusLabel->setText("Analyzing...");
        QApplication::processEvents(); // Update UI
        
        // Perform analysis
        auto analysisResults = m_analysisEngine->analyze(*m_truss);
        
        if (analysisResults.converged) {
            m_hasResults = true;
            updateResultsDisplay();
            updatePlot();
            
            QString message = QString("Analysis completed successfully!\n")
                + QString("Max displacement: %1 m\n").arg(analysisResults.maxDisplacement, 0, 'e', 3)
                + QString("Max stress: %1 Pa").arg(analysisResults.maxStress, 0, 'e', 3);
            
            showInfoMessage(message);
            m_statusLabel->setText("Analysis complete");
        } else {
            showErrorMessage("Analysis failed to converge. Check your structure and loads.");
            m_statusLabel->setText("Analysis failed");
        }
        
    } catch (const std::exception& e) {
        showErrorMessage(QString("Analysis error: %1").arg(e.what()));
        m_statusLabel->setText("Analysis error");
    }
}

void MainWindow::showResults() {
    // Implementation to show results
}

void MainWindow::clearAll() {
    // Clear the truss data
    m_truss = std::make_unique<truss::core::Truss>();
    m_hasResults = false;
    
    // Clear all input widgets
    m_nodeInputWidget->clearInputs();
    m_memberInputWidget->clearInputs();
    m_loadInputWidget->clearInputs();
    
    // Update displays
    updateDataTable();
    m_resultsWidget->clearResults();
    m_plotWidget->clearPlot();
    
    m_statusLabel->setText("All inputs cleared");
}

void MainWindow::exitApplication() {
    close();
}

void MainWindow::updatePlot() {
    m_plotWidget->updatePlot();
}

void MainWindow::updateDataTable() {
    m_dataTableWidget->updateTables();
}

void MainWindow::updateResultsDisplay() {
    m_resultsWidget->updateResults();
}

void MainWindow::showErrorMessage(const QString& message) {
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::showInfoMessage(const QString& message) {
    QMessageBox::information(this, "Information", message);
}

} // namespace truss::gui

