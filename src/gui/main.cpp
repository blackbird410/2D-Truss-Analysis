/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis GUI application
 */

#include <QtWidgets/QApplication>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QLoggingCategory>
#include <iostream>
#include <memory>

#include "MainWindow.hpp"
#include "application/TrussApplicationService.hpp"
#include "application/AnalysisApplicationService.hpp"
#include "controllers/AnalysisController.hpp"
#include "controllers/ProjectController.hpp"
#include "controllers/TrussEditController.hpp"
#include "presenters/AnalysisResultsPresenter.hpp"
#include "presenters/TrussDataPresenter.hpp"
#include "presenters/ValidationPresenter.hpp"

int main(int argc, char *argv[]) {
    // Note: High-DPI support is enabled by default in Qt6
    
    // Enable better font rendering on Linux
#if defined(Q_OS_LINUX)
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, false);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, false);
#endif
    
    // Create the Qt application
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("2D Truss Analysis");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("Civil Engineering Software Solutions");
    app.setOrganizationDomain("truss-analysis.com");
    
    // Set application icon if available
    // app.setWindowIcon(QIcon(":/icons/app-icon.png"));
    
    try {
        // Create Application Services (no Qt dependencies)
        truss::application::TrussApplicationService trussService;
        truss::application::AnalysisApplicationService analysisService;
        
        // Create Presenters (formatting layer)
        truss_presenters::AnalysisResultsPresenter analysisPresenter;
        truss_presenters::TrussDataPresenter trussDataPresenter;
        truss_presenters::ValidationPresenter validationPresenter;
        
        // Create Controllers (orchestration layer)
        truss_controllers::AnalysisController analysisController(
            &trussService,
            &analysisService,
            analysisPresenter,
            validationPresenter
        );
        
        truss_controllers::ProjectController projectController(&trussService);
        
        truss_controllers::TrussEditController trussEditController(
            &trussService,
            trussDataPresenter
        );
        
        // Create and show the main window with dependency injection
        auto mainWindow = std::make_unique<truss::gui::MainWindow>(
            trussService,
            analysisService,
            analysisController,
            projectController,
            trussEditController,
            analysisPresenter,
            trussDataPresenter,
            validationPresenter
        );
        
        mainWindow->show();
        
        // Start the event loop
        return app.exec();
        
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown application error occurred" << std::endl;
        return 1;
    }
}
