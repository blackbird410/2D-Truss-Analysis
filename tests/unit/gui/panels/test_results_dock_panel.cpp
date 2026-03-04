/**
 * @file test_results_dock_panel.cpp
 * @brief Unit tests for ResultsDockPanel (Phase 5).
 *
 * Verifies:
 *  - Widget constructs without crashing (even with nullptr models).
 *  - TabWidget has exactly 4 tabs.
 *  - Tab labels match the expected names.
 *  - onStateChanged(Empty) disables the result table views.
 *  - onStateChanged(ResultsReady) enables the result table views.
 *  - Clicking the Export button on tab 0 emits exportRequested.
 *
 * @note Models are passed as nullptr; QAbstractItemView::setModel(nullptr)
 *       is valid in Qt and does not crash.  This simplifies fixture setup by
 *       avoiding the need to construct heavyweight Phase 3 models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/panels/results_dock_panel.hpp"

#include "gui/state/workspace_state.hpp"
#include "truss/export/export_format.hpp"

#include <QApplication>
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QPushButton>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTest>

#include <gtest/gtest.h>

using truss::gui::ResultsDockPanel;
using truss::gui::state::WorkspacePhase;
using truss::gui::state::WorkspaceState;

// ============================================================
// QApplication bootstrap
// ============================================================

namespace {
QApplication& ensureQApp()
{
    static int   s_argc    = 1;
    static char  s_argv0[] = "unit_tests";
    static char* s_argv[]  = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* e = qobject_cast<QApplication*>(QCoreApplication::instance())) return e;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

WorkspaceState makeState(WorkspacePhase phase, std::size_t handle = 0,
                          std::size_t resultsHandle = 0)
{
    WorkspaceState s;
    s.phase = phase;
    s.trussHandle = handle;
    s.resultsHandle = resultsHandle;
    return s;
}
}  // namespace

// ============================================================
// Fixture
// ============================================================

class ResultsDockPanelTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ensureQApp();
        // Pass nullptr for all three models — valid per Qt documentation.
        panel = std::make_unique<ResultsDockPanel>(nullptr, nullptr, nullptr);
    }
    void TearDown() override { panel.reset(); }

    std::unique_ptr<ResultsDockPanel> panel;
};

// ============================================================
// Construction / structure
// ============================================================

TEST_F(ResultsDockPanelTest, ConstructsWithoutCrash) { SUCCEED(); }

TEST_F(ResultsDockPanelTest, HasFourTabs)
{
    auto* tabs = panel->findChild<QTabWidget*>();
    ASSERT_NE(tabs, nullptr) << "Expected a QTabWidget inside ResultsDockPanel";
    EXPECT_EQ(tabs->count(), 4);
}

TEST_F(ResultsDockPanelTest, TabNamesAreCorrect)
{
    auto* tabs = panel->findChild<QTabWidget*>();
    ASSERT_NE(tabs, nullptr);

    EXPECT_EQ(tabs->tabText(0), QString{"Node Results"});
    EXPECT_EQ(tabs->tabText(1), QString{"Member Results"});
    EXPECT_EQ(tabs->tabText(2), QString{"System Summary"});
    EXPECT_EQ(tabs->tabText(3), QString{"Stiffness Matrix"});
}

// ============================================================
// State changes
// ============================================================

TEST_F(ResultsDockPanelTest, StateChanged_NoResults_DisablesTableViews)
{
    panel->onStateChanged(makeState(WorkspacePhase::Empty));
    QApplication::processEvents();

    // All QTableView widgets should be disabled when there are no results.
    for (auto* view : panel->findChildren<QAbstractItemView*>()) {
        EXPECT_FALSE(view->isEnabled())
            << "TableView '" << view->objectName().toStdString()
            << "' should be disabled in Empty state";
    }
}

TEST_F(ResultsDockPanelTest, StateChanged_ResultsReady_EnablesTableViews)
{
    // First disable views.
    panel->onStateChanged(makeState(WorkspacePhase::Empty));

    // Then bring results
    panel->onStateChanged(makeState(WorkspacePhase::ResultsReady, 1, 1));
    QApplication::processEvents();

    // All QTableView widgets should now be enabled.
    for (auto* view : panel->findChildren<QAbstractItemView*>()) {
        EXPECT_TRUE(view->isEnabled())
            << "TableView '" << view->objectName().toStdString()
            << "' should be enabled in ResultsReady state";
    }
}

// ============================================================
// Export signal
// ============================================================

TEST_F(ResultsDockPanelTest, ExportButtonOnTab0_EmitsExportRequested)
{
    QSignalSpy spy{panel.get(), &ResultsDockPanel::exportRequested};

    // Find the first export button (tab 0).
    auto btns = panel->findChildren<QPushButton*>();
    auto it = std::find_if(btns.begin(), btns.end(), [](QPushButton* b) {
        return b->text().contains(QLatin1String("Export"), Qt::CaseInsensitive);
    });
    ASSERT_NE(it, btns.end()) << "Expected at least one Export button";

    QTest::mouseClick(*it, Qt::LeftButton);
    QApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);
}
