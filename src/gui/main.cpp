/**
 * @file main.cpp
 * @brief Main entry point for the 2D Truss Analysis GUI application.
 *
 * Phase 6 composition root.  Creates a single TrussAnalysisFacade and hands it
 * to MainWindowV2, which internally constructs all sub-controllers, models, and
 * panels.  No presenters, adapters, or legacy controllers are created here.
 */

#include "gui/main_window_v2.hpp"
#include "infrastructure/logging/logger_factory.hpp"
#include "interface/truss_analysis_facade.hpp"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>

#include <filesystem>

namespace {
truss::infrastructure::logging::LoggerPtr createGuiLogger()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    std::filesystem::path logPath =
        std::filesystem::path(appDataPath.toStdString()) / "TrussAnalysis2D.log";
    return truss::infrastructure::logging::LoggerFactory::createDefaultLogger(logPath);
}
}  // namespace

int main(int argc, char* argv[])
{
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
        // MainWindowV2 creates all sub-controllers, models, and panels internally.
        truss::interface::TrussAnalysisFacade facade;
        truss::gui::MainWindowV2 window(facade);
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
