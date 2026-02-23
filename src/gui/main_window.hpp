/**
 * @file MainWindow.hpp
 * @brief Main window for the 2D Truss Analysis GUI
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "data_table_widget.hpp"
#include "deformed_truss_widget.hpp"
#include "interactive_drawing_widget.hpp"
#include "load_input_widget.hpp"
#include "member_input_widget.hpp"
#include "node_input_widget.hpp"
#include "plot_widget.hpp"
#include "results_widget.hpp"
#include "application/analysis_application_service.hpp"
#include "application/truss_application_service.hpp"
#include "gui/controllers/analysis_controller.hpp"
#include "gui/controllers/project_controller.hpp"
#include "gui/controllers/truss_edit_controller.hpp"
#include "gui/presenters/analysis_results_presenter.hpp"
#include "gui/presenters/truss_data_presenter.hpp"
#include "gui/presenters/validation_presenter.hpp"

#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <memory>

namespace truss::gui {

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
    explicit MainWindow(application::TrussApplicationService& trussService,
                        application::AnalysisApplicationService& analysisService,
                        truss_controllers::AnalysisController& analysisController,
                        truss_controllers::ProjectController& projectController,
                        truss_controllers::TrussEditController& trussEditController,
                        truss_presenters::AnalysisResultsPresenter& analysisPresenter,
                        truss_presenters::TrussDataPresenter& trussDataPresenter,
                        truss_presenters::ValidationPresenter& validationPresenter,
                        QWidget* parent = nullptr);

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
    [[maybe_unused]] bool hasResults() const { return m_hasResults; }

    /**
     * @brief Get last results handle
     *
     * @return ResultsHandle Handle to last analysis results
     */
    [[maybe_unused]] application::ResultsHandle getLastResultsHandle() const {
        return m_lastResultsHandle;
    }

private slots:
    void onAnalysisCompleted(size_t resultsHandle);
    void onAnalysisFailed(const QString& errorMessage);
    void
    onValidationFailed(const truss_presenters::ValidationPresenter::ValidationDisplay& display);
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
    static void setupToolBar();
    void setupStatusBar();
    void setupWindowProperties();
    void initializeEmptyProject();  // Initialize application with empty project
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

}  // namespace truss::gui
