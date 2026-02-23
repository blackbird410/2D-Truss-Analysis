/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis GUI application
 */

#include "application/analysis_application_service.hpp"
#include "application/truss_application_service.hpp"
#include "controllers/analysis_controller.hpp"
#include "controllers/project_controller.hpp"
#include "controllers/truss_edit_controller.hpp"
#include "infrastructure/logging/logger_factory.hpp"
#include "main_window.hpp"
#include "presenters/analysis_results_presenter.hpp"
#include "presenters/truss_data_presenter.hpp"
#include "presenters/validation_presenter.hpp"

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>

#include <filesystem>
#include <memory>

namespace {
truss::infrastructure::logging::LoggerPtr createGuiLogger() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    std::filesystem::path logPath = std::filesystem::path(appDataPath.toStdString()) /
                                    "TrussAnalysis2D.log";
    return truss::infrastructure::logging::LoggerFactory::createDefaultLogger(logPath);
}
}  // namespace

int main(int argc, char* argv[]) {
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

    auto logger = createGuiLogger();

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
            &trussService, &analysisService, analysisPresenter, validationPresenter);

        truss_controllers::ProjectController projectController(&trussService);

        truss_controllers::TrussEditController trussEditController(&trussService,
                                                                   trussDataPresenter);

        // Create and show the main window with dependency injection
        auto mainWindow = std::make_unique<truss::gui::MainWindow>(trussService,
                                                                   analysisService,
                                                                   analysisController,
                                                                   projectController,
                                                                   trussEditController,
                                                                   analysisPresenter,
                                                                   trussDataPresenter,
                                                                   validationPresenter);

        mainWindow->show();

        // Start the event loop
        return app.exec();

    } catch (const std::exception& e) {
        logger->error(std::string("Application error: ") + e.what());
        return 1;
    } catch (...) {
        logger->critical("Unknown application error occurred");
        return 1;
    }
}
