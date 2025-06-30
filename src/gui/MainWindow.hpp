/**
 * @file MainWindow.hpp
 * @brief Main window for the 2D Truss Analysis GUI
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
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

#include "Truss.hpp"
#include "AnalysisEngine.hpp"
#include "InteractiveDrawingWidget.hpp"
#include "DeformedTrussWidget.hpp"

namespace truss::gui {

class NodeInputWidget;
class MemberInputWidget;
class LoadInputWidget;
class DataTableWidget;
class ResultsWidget;
class PlotWidget;

/**
 * @brief Main application window with interactive drawing interface
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    
protected:
    void closeEvent(QCloseEvent* event) override;
    
public:
    // Public accessors for widgets
    truss::core::Truss* getTruss() const;
    truss::core::AnalysisEngine* getAnalysisEngine() const { return m_analysisEngine.get(); }
    bool hasResults() const { return m_hasResults; }
    const truss::core::AnalysisResults& getLastResults() const { return m_analysisEngine->getLastResults(); }

private slots:
    void analyze();
    void clearAll();
    void exitApplication();
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void exportResults();
    void showAbout();
    void onTrussModified();
    void updateStatusMessage(const QString& message);

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
    
    // Data model
    std::unique_ptr<truss::core::AnalysisEngine> m_analysisEngine;
    bool m_hasResults;
    QString m_currentFileName;
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
 * @brief Widget for load input
 */
class LoadInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoadInputWidget(QWidget *parent = nullptr);

signals:
    void loadAdded();

public slots:
    void addLoad();
    void clearInputs();
    void updateNodeList();

private:
    void setupUI();
    
    QComboBox* m_nodeCombo;
    QLineEdit* m_fxEdit;
    QLineEdit* m_fyEdit;
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
