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
 *  - setResultsView(nullptr) does not crash.
 *  - setResultsView(ptr) + switching to stiffness-matrix tab populates the table.
 *  - onTabChanged(3) with results view populates the stiffness matrix model.
 *
 * @note Models are passed as nullptr; QAbstractItemView::setModel(nullptr)
 *       is valid in Qt and does not crash.  This simplifies fixture setup by
 *       avoiding the need to construct heavyweight Phase 3 models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "core/interfaces/ianalysis_results_view.hpp"
#include "gui/panels/results_dock_panel.hpp"
#include "gui/state/workspace_state.hpp"
#include "truss/export/export_format.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QCoreApplication>
#include <QPushButton>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTableView>
#include <QTest>

#include <gtest/gtest.h>

using truss::gui::ResultsDockPanel;
using truss::gui::state::WorkspacePhase;
using truss::gui::state::WorkspaceState;

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

WorkspaceState
makeState(WorkspacePhase phase, std::size_t handle = 0, std::size_t resultsHandle = 0) {
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
    void SetUp() override {
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

TEST_F(ResultsDockPanelTest, ConstructsWithoutCrash) {
    SUCCEED();
}

TEST_F(ResultsDockPanelTest, HasFourTabs) {
    auto* tabs = panel->findChild<QTabWidget*>();
    ASSERT_NE(tabs, nullptr) << "Expected a QTabWidget inside ResultsDockPanel";
    EXPECT_EQ(tabs->count(), 4);
}

TEST_F(ResultsDockPanelTest, TabNamesAreCorrect) {
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

TEST_F(ResultsDockPanelTest, StateChanged_NoResults_DisablesTableViews) {
    panel->onStateChanged(makeState(WorkspacePhase::Empty));
    QApplication::processEvents();

    // All QTableView widgets should be disabled when there are no results.
    for (auto* view : panel->findChildren<QAbstractItemView*>()) {
        EXPECT_FALSE(view->isEnabled()) << "TableView '" << view->objectName().toStdString()
                                        << "' should be disabled in Empty state";
    }
}

TEST_F(ResultsDockPanelTest, StateChanged_ResultsReady_EnablesTableViews) {
    // First disable views.
    panel->onStateChanged(makeState(WorkspacePhase::Empty));

    // Then bring results
    panel->onStateChanged(makeState(WorkspacePhase::ResultsReady, 1, 1));
    QApplication::processEvents();

    // All QTableView widgets should now be enabled.
    for (auto* view : panel->findChildren<QAbstractItemView*>()) {
        EXPECT_TRUE(view->isEnabled()) << "TableView '" << view->objectName().toStdString()
                                       << "' should be enabled in ResultsReady state";
    }
}

// ============================================================
// Export signal
// ============================================================

TEST_F(ResultsDockPanelTest, ExportButtonOnTab0_EmitsExportRequested) {
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

// ============================================================
// Minimal IAnalysisResultsView stub
// ============================================================

namespace {

using truss::core::Real;

class StubResultsView final : public truss::core::interfaces::IAnalysisResultsView {
public:
    void setMatrix(std::vector<std::vector<Real>> m) { m_matrix = std::move(m); }

    const std::vector<Real>& getDisplacements() const override { return m_empty; }
    const std::vector<Real>& getReactions() const override { return m_empty; }
    const std::vector<Real>& getMemberForces() const override { return m_empty; }
    const std::vector<Real>& getMemberStresses() const override { return m_empty; }
    const std::vector<Real>& getUtilizationRatios() const override { return m_empty; }
    const std::vector<std::vector<Real>>& getStiffnessMatrix() const override { return m_matrix; }
    bool hasConverged() const override { return true; }
    int getIterations() const override { return 1; }
    Real getResidualNorm() const override { return 0.0; }
    Real getConditionNumber() const override { return 1.0; }
    size_t getTotalDofs() const override { return 0; }
    size_t getFreeDofs() const override { return 0; }
    size_t getConstrainedDofs() const override { return 0; }
    Real getTotalStrain() const override { return 0.0; }
    Real getMaxDisplacement() const override { return 0.0; }
    Real getMaxStress() const override { return 0.0; }

private:
    std::vector<Real> m_empty;
    std::vector<std::vector<Real>> m_matrix;
};

}  // namespace

// ============================================================
// setResultsView
// ============================================================

TEST_F(ResultsDockPanelTest, SetResultsView_Nullptr_DoesNotCrash) {
    ASSERT_NO_FATAL_FAILURE(panel->setResultsView(nullptr));
}

TEST_F(ResultsDockPanelTest, SetResultsView_ValidPtr_DoesNotCrash) {
    StubResultsView stub;
    ASSERT_NO_FATAL_FAILURE(panel->setResultsView(&stub));
}

// ============================================================
// Stiffness-matrix tab — lazy population
// ============================================================

TEST_F(ResultsDockPanelTest, StiffnessTab_WithResults_PopulatesModel) {
    // Build a 2×2 stiffness matrix stub.
    StubResultsView stub;
    stub.setMatrix({{1.0e6, -1.0e6}, {-1.0e6, 1.0e6}});
    panel->setResultsView(&stub);

    // Switch to the stiffness-matrix tab (index 3).
    auto* tabs = panel->findChild<QTabWidget*>();
    ASSERT_NE(tabs, nullptr);
    tabs->setCurrentIndex(3);
    QApplication::processEvents();

    // Find the stiffness table view and check its model was swapped in.
    auto* stiffTable = panel->findChild<QTableView*>("stiffnessMatrixTable");
    ASSERT_NE(stiffTable, nullptr);
    ASSERT_NE(stiffTable->model(), nullptr);
    EXPECT_EQ(stiffTable->model()->rowCount(), 2);
    EXPECT_EQ(stiffTable->model()->columnCount(), 2);
}

TEST_F(ResultsDockPanelTest, StiffnessTab_WithoutResults_DoesNotPopulate) {
    // No results view set — switching to tab 3 must not crash and leaves model empty.
    auto* tabs = panel->findChild<QTabWidget*>();
    ASSERT_NE(tabs, nullptr);
    tabs->setCurrentIndex(3);
    QApplication::processEvents();

    auto* stiffTable = panel->findChild<QTableView*>("stiffnessMatrixTable");
    ASSERT_NE(stiffTable, nullptr);
    // The placeholder model should have 0 rows.
    EXPECT_EQ(stiffTable->model()->rowCount(), 0);
}

TEST_F(ResultsDockPanelTest, SetResultsView_WhileOnTab3_PopulatesImmediately) {
    auto* tabs = panel->findChild<QTabWidget*>();
    ASSERT_NE(tabs, nullptr);
    tabs->setCurrentIndex(3);  // navigate there first
    QApplication::processEvents();

    // Now provide results — should populate immediately.
    StubResultsView stub;
    stub.setMatrix({{4.0e6, -4.0e6}, {-4.0e6, 4.0e6}});
    panel->setResultsView(&stub);
    QApplication::processEvents();

    auto* stiffTable = panel->findChild<QTableView*>("stiffnessMatrixTable");
    ASSERT_NE(stiffTable, nullptr);
    EXPECT_EQ(stiffTable->model()->rowCount(), 2);
}
