/**
 * @file MainWindow.hpp
 * @brief Main window for the 2D Truss Analysis GUI
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtGui/QAction>
#include <QtCore/QTimer>
#include <memory>

#include "application/TrussApplicationService.hpp"
#include "application/AnalysisApplicationService.hpp"
#include "gui/controllers/AnalysisController.hpp"
#include "gui/controllers/ProjectController.hpp"
#include "gui/controllers/TrussEditController.hpp"
#include "gui/presenters/AnalysisResultsPresenter.hpp"
#include "gui/presenters/TrussDataPresenter.hpp"
#include "gui/presenters/ValidationPresenter.hpp"
#include "InteractiveDrawingWidget.hpp"
#include "DeformedTrussWidget.hpp"
#include "LoadInputWidget.hpp"

namespace truss::gui {

class NodeInputWidget;
class MemberInputWidget;
class DataTableWidget;
class ResultsWidget;
class PlotWidget;

/**
 * @brief Main application window with interactive drawing interface
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Construct MainWindow with dependency injection
     * 
     * @param trussService Application service for truss operations
     * @param analysisService Application service for analysis
     * @param analysisController Controller for analysis workflow
     * @param projectController Controller for project lifecycle
     * @param trussEditController Controller for truss editing (optional)
     * @param analysisPresenter Presenter for analysis results formatting
     * @param trussDataPresenter Presenter for truss data formatting
     * @param validationPresenter Presenter for validation messages
     * @param parent Qt parent widget
     */
    explicit MainWindow(
        application::TrussApplicationService& trussService,
        application::AnalysisApplicationService& analysisService,
        truss_controllers::AnalysisController& analysisController,
        truss_controllers::ProjectController& projectController,
        truss_controllers::TrussEditController& trussEditController,
        truss_presenters::AnalysisResultsPresenter& analysisPresenter,
        truss_presenters::TrussDataPresenter& trussDataPresenter,
        truss_presenters::ValidationPresenter& validationPresenter,
        QWidget *parent = nullptr);
    
    ~MainWindow() override;
    
protected:
    void closeEvent(QCloseEvent* event) override;
    
public:
    /**
     * @brief Get current truss handle
     * 
     * @return TrussHandle Handle to current truss
     */
    application::TrussHandle getCurrentTrussHandle() const;
    
    /**
     * @brief Check if analysis results are available
     * 
     * @return true if results exist
     */
    bool hasResults() const { return m_hasResults; }
    
    /**
     * @brief Get last results handle
     * 
     * @return ResultsHandle Handle to last analysis results
     */
    application::ResultsHandle getLastResultsHandle() const { return m_lastResultsHandle; }
    
    /**
     * @deprecated Legacy accessor for old widgets - will be removed
     * @brief Get raw truss pointer (temporary for legacy widgets)
     * 
     * @return Truss* Pointer to truss (from drawing widget)
     */
    [[deprecated("Use getCurrentTrussHandle() and Application services instead")]]
    truss::core::Truss* getTruss() const;
    
    /**
     * @deprecated Legacy accessor for old widgets - will be removed
     * @brief Get analysis service reference (temporary for legacy widgets)
     * 
     * @return AnalysisApplicationService& Reference to analysis service
     */
    [[deprecated("Use injected dependencies instead")]]
    application::AnalysisApplicationService& getAnalysisService() { return m_analysisService; }

private slots:
    void onAnalysisCompleted(size_t resultsHandle);
    void onAnalysisFailed(const QString& errorMessage);
    void onValidationFailed(const truss_presenters::ValidationPresenter::ValidationDisplay& display);
    void onProjectOpened(application::TrussHandle handle, const QString& filepath);
    void onProjectSaved(const QString& filepath);
    void onProjectClosed();
    void onOperationFailed(const QString& errorMessage);
    void onTrussModified();
    void updateStatusMessage(const QString& message);
    void exitApplication();
    void showAbout();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupWindowProperties();
    void connectSignals();
    
    void updateResultsDisplay();
    void showErrorMessage(const QString& message);
    void showInfoMessage(const QString& message);
    void enableAnalysis(bool enable);
    
    // Menu action handlers (delegate to controllers)
    void requestAnalyze();
    void requestClearAll();
    void requestNewProject();
    void requestOpenProject();
    void requestSaveProject();
    void requestSaveProjectAs();
    void requestExportResults();

    // UI Components
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    
    // Interactive drawing widget
    InteractiveDrawingWidget* m_drawingWidget;
    
    // Results display
    QTabWidget* m_resultsTabWidget;
    ResultsWidget* m_resultsWidget;
    DeformedTrussWidget* m_deformedTrussWidget;
    QTextEdit* m_logTextEdit;
    
    // Control buttons
    QPushButton* m_analyzeButton;
    QPushButton* m_clearButton;
    
    // Status display
    QLabel* m_statusLabel;
    QLabel* m_coordinateLabel;
    
    // Injected dependencies (references, not owned)
    application::TrussApplicationService& m_trussService;
    application::AnalysisApplicationService& m_analysisService;
    truss_controllers::AnalysisController& m_analysisController;
    truss_controllers::ProjectController& m_projectController;
    truss_controllers::TrussEditController& m_trussEditController;
    truss_presenters::AnalysisResultsPresenter& m_analysisPresenter;
    truss_presenters::TrussDataPresenter& m_trussDataPresenter;
    truss_presenters::ValidationPresenter& m_validationPresenter;
    
    // State
    application::ResultsHandle m_lastResultsHandle;
    bool m_hasResults;
};

/**
 * @brief Widget for node input
 */
class NodeInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit NodeInputWidget(QWidget *parent = nullptr);

signals:
    void nodeAdded();

public slots:
    void addNode();
    void clearInputs();

private:
    void setupUI();
    
    QLineEdit* m_xCoordEdit;
    QLineEdit* m_yCoordEdit;
    QComboBox* m_supportTypeCombo;
    QPushButton* m_addButton;
    QPushButton* m_clearButton;
    
    friend class MainWindow;
};

/**
 * @brief Widget for member input
 */
class MemberInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit MemberInputWidget(QWidget *parent = nullptr);

signals:
    void memberAdded();

public slots:
    void addMember();
    void clearInputs();
    void updateNodeList();

private:
    void setupUI();
    
    QComboBox* m_startNodeCombo;
    QComboBox* m_endNodeCombo;
    QLineEdit* m_areaEdit;
    QLineEdit* m_youngModulusEdit;
    QLineEdit* m_densityEdit;
    QLineEdit* m_yieldStrengthEdit;
    QPushButton* m_addButton;
    QPushButton* m_clearButton;
    
    friend class MainWindow;
};

/**
 * @brief Widget for displaying data tables
 */
class DataTableWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataTableWidget(QWidget *parent = nullptr);

public slots:
    void updateTables();

private:
    void setupUI();
    void updateNodesTable();
    void updateMembersTable();
    void updateLoadsTable();
    
    QTableWidget* m_nodesTable;
    QTableWidget* m_membersTable;
    QTableWidget* m_loadsTable;
    
    friend class MainWindow;
};

/**
 * @brief Widget for displaying analysis results
 */
class ResultsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResultsWidget(QWidget *parent = nullptr);

public slots:
    void updateResults();
    void clearResults();

private:
    void setupUI();
    void updateDisplacementsTable();
    void updateForcesTable();
    void updateReactionsTable();
    void updateStiffnessTable();
    void updateSummary();
    
    QTableWidget* m_displacementsTable;
    QTableWidget* m_forcesTable;
    QTableWidget* m_reactionsTable;
    QTableWidget* m_stiffnessTable;
    QTextEdit* m_summaryText;
    
    friend class MainWindow;
};

/**
 * @brief Widget for plotting the truss structure
 */
class PlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlotWidget(QWidget *parent = nullptr);

public slots:
    void updatePlot();
    void clearPlot();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawTruss(QPainter &painter);
    void drawNodes(QPainter &painter);
    void drawMembers(QPainter &painter);
    void drawLoads(QPainter &painter);
    void drawSupports(QPainter &painter);
    void drawDeformedShape(QPainter &painter);
    
    QPoint worldToScreen(const truss::core::Point2D& point) const;
    void calculateViewport();
    
    bool m_showDeformed;
    bool m_showForces;
    double m_scaleFactor;
    QRect m_plotArea;
    truss::core::Point2D m_minBounds;
    truss::core::Point2D m_maxBounds;
    
    friend class MainWindow;
};

} // namespace truss::gui
