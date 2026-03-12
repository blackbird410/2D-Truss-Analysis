/**
 * @file test_theme_loader.cpp
 * @brief Unit tests for ThemeLoader static utility.
 *
 * Exercises:
 *  - savedThemePath() returns the dark-theme default when no setting exists.
 *  - applyTheme() with a nonexistent resource path returns false.
 *  - applyTheme() with the built-in dark QSS resource returns true.
 *  - applyTheme() with the built-in light QSS resource returns true.
 *  - applyTheme() persists the chosen path to QSettings.
 *  - restoreLastTheme() does not crash when called with a valid QApplication.
 *
 * These tests require QApplication (not just QCoreApplication) because
 * applyTheme() calls QApplication::setStyleSheet() and app.setPalette().
 * They are compiled into unit_tests_gui_widgets which runs under the
 * offscreen platform plugin.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/theme_loader.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QString>

#include <gtest/gtest.h>

// ============================================================
// QApplication bootstrap
// ============================================================

namespace {

QApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* e = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return e;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

/// Remove the "theme/path" key so each test starts from a clean state.
void clearThemeSetting() {
    QSettings settings(QStringLiteral("TrussAnalysis"), QStringLiteral("GUI"));
    settings.remove(QStringLiteral("theme/path"));
    settings.sync();
}

}  // namespace

// ============================================================
// Fixture
// ============================================================

class ThemeLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        // Force Qt resource registration from TrussGui's static library.
        // Q_INIT_RESOURCE must be called outside any anonymous namespace;
        // class member functions satisfy that requirement.
        Q_INIT_RESOURCE(resources);
        clearThemeSetting();
    }
    void TearDown() override { clearThemeSetting(); }
};

// ============================================================
// savedThemePath — default behaviour
// ============================================================

TEST_F(ThemeLoaderTest, SavedThemePath_DefaultsToBuiltInDarkPath) {
    const QString path = truss::gui::ThemeLoader::savedThemePath();
    EXPECT_EQ(path, QString::fromLatin1(truss::gui::ThemeLoader::kDarkThemePath));
}

// ============================================================
// applyTheme — invalid resource
// ============================================================

TEST_F(ThemeLoaderTest, ApplyTheme_NonExistentResource_ReturnsFalse) {
    auto& app = ensureQApp();
    EXPECT_FALSE(
        truss::gui::ThemeLoader::applyTheme(app, QStringLiteral(":/nonexistent_theme.qss")));
}

// ============================================================
// applyTheme — built-in resources
// ============================================================

TEST_F(ThemeLoaderTest, ApplyTheme_DarkTheme_ReturnsTrue) {
    auto& app = ensureQApp();
    const bool ok = truss::gui::ThemeLoader::applyTheme(
        app, QString::fromLatin1(truss::gui::ThemeLoader::kDarkThemePath));
    EXPECT_TRUE(ok);
}

TEST_F(ThemeLoaderTest, ApplyTheme_LightTheme_ReturnsTrue) {
    auto& app = ensureQApp();
    const bool ok = truss::gui::ThemeLoader::applyTheme(
        app, QString::fromLatin1(truss::gui::ThemeLoader::kLightThemePath));
    EXPECT_TRUE(ok);
}

// ============================================================
// applyTheme — persists selection to QSettings
// ============================================================

TEST_F(ThemeLoaderTest, ApplyTheme_PersistsLightThemeToSettings) {
    auto& app = ensureQApp();
    truss::gui::ThemeLoader::applyTheme(
        app, QString::fromLatin1(truss::gui::ThemeLoader::kLightThemePath));

    const QString stored = truss::gui::ThemeLoader::savedThemePath();
    EXPECT_EQ(stored, QString::fromLatin1(truss::gui::ThemeLoader::kLightThemePath));
}

TEST_F(ThemeLoaderTest, ApplyTheme_PersistsDarkThemeToSettings) {
    auto& app = ensureQApp();
    // First apply light, then switch back to dark.
    truss::gui::ThemeLoader::applyTheme(
        app, QString::fromLatin1(truss::gui::ThemeLoader::kLightThemePath));
    truss::gui::ThemeLoader::applyTheme(
        app, QString::fromLatin1(truss::gui::ThemeLoader::kDarkThemePath));

    const QString stored = truss::gui::ThemeLoader::savedThemePath();
    EXPECT_EQ(stored, QString::fromLatin1(truss::gui::ThemeLoader::kDarkThemePath));
}

// ============================================================
// restoreLastTheme — smoke test / no crash
// ============================================================

TEST_F(ThemeLoaderTest, RestoreLastTheme_WithDarkDefault_DoesNotCrash) {
    auto& app = ensureQApp();
    // No setting → falls through to kDarkThemePath at ThemeLoader::restoreLastTheme
    ASSERT_NO_FATAL_FAILURE(truss::gui::ThemeLoader::restoreLastTheme(app));
}

TEST_F(ThemeLoaderTest, RestoreLastTheme_WithStoredLightTheme_AppliesLightTheme) {
    auto& app = ensureQApp();
    // Store light theme path, then restore should pick it up.
    truss::gui::ThemeLoader::applyTheme(
        app, QString::fromLatin1(truss::gui::ThemeLoader::kLightThemePath));
    ASSERT_NO_FATAL_FAILURE(truss::gui::ThemeLoader::restoreLastTheme(app));
    // After restore, the applied theme should still be the light path.
    EXPECT_EQ(truss::gui::ThemeLoader::savedThemePath(),
              QString::fromLatin1(truss::gui::ThemeLoader::kLightThemePath));
}
