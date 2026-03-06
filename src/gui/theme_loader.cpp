/**
 * @file theme_loader.cpp
 * @brief Utility for loading and applying QSS themes from Qt resources.
 *
 * Phase 2: Qt Infrastructure Setup
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "theme_loader.hpp"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QPalette>
#include <QSettings>
#include <QString>
#include <QTextStream>

namespace truss::gui {

// ---------------------------------------------------------------------------
// Public static methods
// ---------------------------------------------------------------------------

bool ThemeLoader::applyTheme(QApplication& app, const QString& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    const QString qss = in.readAll();
    file.close();

    if (qss.trimmed().isEmpty()) {
        return false;
    }

    app.setStyleSheet(qss);

    // Keep the application palette aligned with the theme so that widgets
    // which read QPalette colours (e.g. TrussCanvasWidget) stay in sync.
    // setStyleSheet() alone does NOT update QPalette, so we set it explicitly.
    // This also propagates QEvent::PaletteChange to every widget, triggering
    // repaints that depend on QPalette::Window lightness.
    const bool isDark = resourcePath.contains(QLatin1String("dark"),
                                               Qt::CaseInsensitive);
    QPalette pal = QApplication::palette();
    pal.setColor(QPalette::Window,
                 isDark ? QColor(0x1E, 0x20, 0x28)   // #1E2028 dark bg
                        : QColor(0xF5, 0xF5, 0xF5));  // #F5F5F5 light bg
    pal.setColor(QPalette::WindowText,
                 isDark ? QColor(0xE8, 0xEA, 0xED)   // #E8EAED dark text
                        : QColor(0x1A, 0x1A, 0x1A));  // #1A1A1A light text
    app.setPalette(pal);

    // Persist the user's choice.
    QSettings settings(QString::fromLatin1(kSettingsOrg),
                       QString::fromLatin1(kSettingsApp));
    settings.setValue(QString::fromLatin1(kSettingsKey), resourcePath);

    return true;
}

void ThemeLoader::restoreLastTheme(QApplication& app)
{
    const QString path = savedThemePath();
    if (!applyTheme(app, path)) {
        // Fallback: try the hard-coded default dark theme.
        if (path != QString::fromLatin1(kDarkThemePath)) {
            applyTheme(app, QString::fromLatin1(kDarkThemePath));
        }
    }
}

QString ThemeLoader::savedThemePath()
{
    QSettings settings(QString::fromLatin1(kSettingsOrg),
                       QString::fromLatin1(kSettingsApp));
    return settings.value(QString::fromLatin1(kSettingsKey),
                          QString::fromLatin1(kDarkThemePath)).toString();
}

} // namespace truss::gui
