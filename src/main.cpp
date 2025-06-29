/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis Application
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
 * @date 2025
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
#include <memory>

#include "gui/MainWindow.hpp"
#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "database/DatabaseManager.hpp"

/**
 * @brief Configure application settings and directories
 */
void setupApplication() {
    QApplication::setApplicationName("TrussAnalysis2D");
    QApplication::setApplicationVersion("2.0.0");
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

/**
 * @brief Initialize application components
 */
bool initializeApplication() {
    try {
        // Initialize logging system
        if (!truss::core::Logger::initialize()) {
            QMessageBox::critical(nullptr, "Error", 
                "Failed to initialize logging system");
            return false;
        }
        
        // Initialize database
        auto& dbManager = truss::database::DatabaseManager::getInstance();
        if (!dbManager.initialize()) {
            QMessageBox::critical(nullptr, "Database Error", 
                "Failed to initialize database system");
            return false;
        }
        
        // Initialize core application
        auto& coreApp = truss::core::Application::getInstance();
        if (!coreApp.initialize()) {
            QMessageBox::critical(nullptr, "Application Error", 
                "Failed to initialize core application");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
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
    
    if (!initializeApplication()) {
        return -1;
    }
    
    // Create and show main window
    splash.showMessage("Starting application...", 
                      Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    
    try {
        auto mainWindow = std::make_unique<truss::gui::MainWindow>();
        mainWindow->show();
        
        // Close splash screen after a brief delay
        QTimer::singleShot(1000, [&splash, &mainWindow]() {
            splash.finish(mainWindow.get());
        });
        
        // Start event loop
        int result = app.exec();
        
        // Cleanup
        truss::core::Logger::shutdown();
        
        return result;
        
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Application Error", 
            QString("Failed to start application: %1").arg(e.what()));
        return -1;
    }
}
