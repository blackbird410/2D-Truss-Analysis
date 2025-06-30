/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis GUI application
 */

#include <QtWidgets/QApplication>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QLoggingCategory>
#include <iostream>

#include "MainWindow.hpp"

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
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Civil Engineering Software Solutions");
    app.setOrganizationDomain("truss-analysis.com");
    
    // Set application icon if available
    // app.setWindowIcon(QIcon(":/icons/app-icon.png"));
    
    try {
        // Create and show the main window
        truss::gui::MainWindow mainWindow;
        mainWindow.show();
        
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
