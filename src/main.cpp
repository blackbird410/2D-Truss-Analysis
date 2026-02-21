/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis Application
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026
 * 
 * Professional 2D Truss Analysis Application built with modern C++ and Qt6.
 * Provides comprehensive structural analysis capabilities for civil engineers.
 */

#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <filesystem>
#include <memory>

#include "gui/MainWindow.hpp"
#include "application/TrussApplicationService.hpp"
#include "application/AnalysisApplicationService.hpp"
#include "gui/controllers/AnalysisController.hpp"
#include "gui/controllers/ProjectController.hpp"
#include "gui/controllers/TrussEditController.hpp"
#include "gui/presenters/AnalysisResultsPresenter.hpp"
#include "gui/presenters/TrussDataPresenter.hpp"
#include "gui/presenters/ValidationPresenter.hpp"
#include "core/Application.hpp"
#include "database/DatabaseManager.hpp"
#include "infrastructure/logging/logger_factory.hpp"

/**
 * @brief Configure application settings and directories
 */
void setupApplication() {
    QApplication::setApplicationName("TrussAnalysis2D");
    QApplication::setApplicationVersion("3.0.0");
    QApplication::setApplicationDisplayName("2D Truss Analysis");
    QApplication::setOrganizationName("Civil Engineering Software Solutions");
    QApplication::setOrganizationDomain("truss-analysis.com");
    
    // Set up application data directory
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
}

/**
 * @brief Apply modern application styling
 */
void applyModernStyle(QApplication& app) {
    // Set modern style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply dark theme palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
}

truss::infrastructure::logging::LoggerPtr createAppLogger() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    std::filesystem::path logPath = std::filesystem::path(appDataPath.toStdString()) / "TrussAnalysis2D.log";
    return truss::infrastructure::logging::LoggerFactory::createDefaultLogger(logPath);
}

/**
 * @brief Initialize application components
 */
bool initializeApplication(truss::infrastructure::logging::ILogger& logger) {
    try {
        // Initialize database
        auto& dbManager = truss::database::DatabaseManager::getInstance();
        if (!dbManager.initialize()) {
            logger.error("Failed to initialize database system");
            QMessageBox::critical(nullptr, "Database Error", 
                "Failed to initialize database system");
            return false;
        }
        
        // Initialize core application
        auto& coreApp = truss::core::Application::getInstance();
        if (!coreApp.initialize()) {
            logger.error("Failed to initialize core application");
            QMessageBox::critical(nullptr, "Application Error", 
                "Failed to initialize core application");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        logger.error(std::string("Application initialization failed: ") + e.what());
        QMessageBox::critical(nullptr, "Initialization Error", 
            QString("Application initialization failed: %1").arg(e.what()));
        return false;
    }
}

/**
 * @brief Main application entry point
 */
int main(int argc, char *argv[]) {
    // Enable high DPI support
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    
    // Setup application configuration
    setupApplication();

    auto logger = createAppLogger();
    
    // Apply modern styling
    applyModernStyle(app);
    
    // Show splash screen
    QPixmap splashPixmap(400, 300);
    splashPixmap.fill(QColor(42, 130, 218));
    
    QSplashScreen splash(splashPixmap);
    splash.show();
    splash.showMessage("Loading 2D Truss Analysis...", 
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    
    app.processEvents();
    
    // Initialize application components
    splash.showMessage("Initializing components...", 
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    
    if (!initializeApplication(*logger)) {
        return -1;
    }
    
    // Create and show main window
    splash.showMessage("Starting application...", 
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    
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
        
        // Create MainWindow with dependency injection
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
        
        // Close splash screen after a brief delay
        QTimer::singleShot(1000, [&splash, &mainWindow]() {
            splash.finish(mainWindow.get());
        });
        
        // Start event loop
        int result = app.exec();
        
        return result;
        
    } catch (const std::exception& e) {
        logger->error(std::string("Failed to start application: ") + e.what());
        QMessageBox::critical(nullptr, "Application Error", 
            QString("Failed to start application: %1").arg(e.what()));
        return -1;
    }
}
