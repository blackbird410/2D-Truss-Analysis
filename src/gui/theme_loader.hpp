/**
 * @file theme_loader.hpp
 * @brief Utility for loading and applying QSS themes from Qt resources.
 *
 * Usage:
 * @code
 *   // Apply a named theme and persist the choice:
 *   truss::gui::ThemeLoader::applyTheme(app, ":/themes/dark.qss");
 *
 *   // On next launch, restore the user's last selection:
 *   truss::gui::ThemeLoader::restoreLastTheme(app);
 * @endcode
 *
 * Available built-in resource paths:
 *   ":/themes/dark.qss"
 *   ":/themes/light.qss"
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QString>

class QApplication;

namespace truss::gui {

/**
 * @brief Static helper class that loads QSS stylesheets from Qt resource paths
 *        and applies them to the running QApplication.
 *
 * Theme selections are persisted via QSettings so the user's preference
 * survives application restarts.
 *
 * QSettings scope:
 *   Organisation: "TrussAnalysis"
 *   Application:  "GUI"
 *   Key:          "theme/path"  (value: resource path string)
 */
class ThemeLoader final {
public:
    ThemeLoader() = delete;  ///< Static utility — not instantiable.

    /**
     * @brief Load a QSS file from a Qt resource path, apply it to @p app,
     *        and persist the path to QSettings.
     *
     * @param app          The running QApplication instance.
     * @param resourcePath Qt resource path (e.g. ":/themes/dark.qss").
     * @return @c true on success; @c false if the resource could not be read.
     */
    static bool applyTheme(QApplication& app, const QString& resourcePath);

    /**
     * @brief Restore the theme that was last persisted by applyTheme().
     *        Falls back to ":/themes/dark.qss" if no preference is stored or
     *        if the stored resource path cannot be loaded.
     *
     * @param app The running QApplication instance.
     */
    static void restoreLastTheme(QApplication& app);

    /**
     * @brief Return the resource path stored in QSettings, or the default
     *        dark theme path if no preference has been saved yet.
     */
    [[nodiscard]] static QString savedThemePath();

    /// Resource path of the built-in dark theme.
    static constexpr const char* kDarkThemePath = ":/themes/dark.qss";
    /// Resource path of the built-in light theme.
    static constexpr const char* kLightThemePath = ":/themes/light.qss";

private:
    static constexpr const char* kSettingsOrg = "TrussAnalysis";
    static constexpr const char* kSettingsApp = "GUI";
    static constexpr const char* kSettingsKey = "theme/path";
};

}  // namespace truss::gui
