/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis GUI application.
 *
 * Composition root.  Creates a single TrussAnalysisFacade and hands it
 * to MainWindow, which internally constructs all sub-controllers, models, and
 * panels.  No presenters, adapters, or legacy controllers are created here.
 *
 * ThemeLoader::restoreLastTheme() is called after QApplication construction
 * so the previously persisted theme is applied before the window is shown.
 */

#include "gui/main_window.hpp"
#include "gui/theme_loader.hpp"
#include "infrastructure/logging/logger_factory.hpp"
#include "interface/truss_analysis_facade.hpp"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>

#include <filesystem>

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

    QApplication app(argc, argv);
    app.setApplicationName("2D Truss Analysis");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("Civil Engineering Software Solutions");
    app.setOrganizationDomain("truss-analysis.com");

    auto logger = createGuiLogger();

    try {
        // Facade owns all application services.
        // MainWindow creates all sub-controllers, models, and panels internally.
        truss::interface::TrussAnalysisFacade facade;
        truss::gui::MainWindow window(facade);

        // Restore the last-used theme (persisted via QSettings) before showing.
        truss::gui::ThemeLoader::restoreLastTheme(app);

        window.show();
        return app.exec();

    } catch (const std::exception& e) {
        logger->error(std::string("Application error: ") + e.what());
        return 1;
    } catch (...) {
        logger->critical("Unknown application error occurred");
        return 1;
    }
}
