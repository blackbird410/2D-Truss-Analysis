/**
 * @file test_results_table_model.cpp
 * @brief Unit tests for ResultsTableModel.
 *
 * Covers:
 *  - Phase 3 skeleton contract (structure, empty state).
 *  - Phase 6: refresh() population from IAnalysisResultsView,
 *    text/alignment roles, clear(), and signal emission.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "core/interfaces/ianalysis_results_view.hpp"
#include "gui/models/results_table_model.hpp"

#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <gtest/gtest.h>
#include <vector>

// ---------------------------------------------------------------------------
// QCoreApplication bootstrap
// QAbstractItemModel lives in QtCore and does not interact with the display
// system. QCoreApplication is sufficient; no platform plugin is loaded, so
// this binary runs without a display server in CI.
// ---------------------------------------------------------------------------
namespace {

QCoreApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QCoreApplication* s_app = []() -> QCoreApplication* {
        if (QCoreApplication::instance())
            return QCoreApplication::instance();
        return new QCoreApplication(s_argc, s_argv);
    }();
    return *s_app;
}

}  // namespace

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class ResultsTableModelTest : public ::testing::Test {
protected:
    void SetUp() override { ensureQApp(); }

    truss::gui::model::ResultsTableModel model;
};

// ---------------------------------------------------------------------------
// Phase 3 skeleton contract
// ---------------------------------------------------------------------------

TEST_F(ResultsTableModelTest, InitialRowCountIsZero) {
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(ResultsTableModelTest, ColumnCountIsAlwaysTwo) {
    EXPECT_EQ(model.columnCount(), 2);
    EXPECT_EQ(model.columnCount(), truss::gui::model::ResultsTableModel::kColumnCount);
}

TEST_F(ResultsTableModelTest, HorizontalHeaders_PropertyAndValue) {
    const auto key = model.headerData(
        truss::gui::model::ResultsTableModel::kColKey, Qt::Horizontal, Qt::DisplayRole);
    ASSERT_TRUE(key.isValid());
    EXPECT_EQ(key.toString(), QStringLiteral("Property"));

    const auto val = model.headerData(
        truss::gui::model::ResultsTableModel::kColValue, Qt::Horizontal, Qt::DisplayRole);
    ASSERT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QStringLiteral("Value"));
}

TEST_F(ResultsTableModelTest, InvalidIndexReturnsInvalidVariant) {
    EXPECT_FALSE(model.data(QModelIndex{}).isValid());
    EXPECT_FALSE(model.data(model.index(0, 0)).isValid());  // row 0 doesn't exist
}

TEST_F(ResultsTableModelTest, OutOfRangeSectionHeaderIsInvalid) {
    EXPECT_FALSE(model.headerData(99, Qt::Horizontal, Qt::DisplayRole).isValid());
}

TEST_F(ResultsTableModelTest, ClearOnEmptyModelDoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(model.clear());
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(ResultsTableModelTest, ParentIndexGuard) {
    // For a flat table model the standard tree guard is:
    //   rowCount(validParent) == 0, columnCount(validParent) == 0
    // A valid parent can only exist in a non-empty model.
    // When the model is empty, index(0,0) is itself invalid, so we
    // verify the default (root) path returns the correct counts instead.
    EXPECT_EQ(model.rowCount(QModelIndex{}), 0);
    EXPECT_EQ(model.columnCount(QModelIndex{}), 2);
}

// ---------------------------------------------------------------------------
// Stub IAnalysisResultsView — returns fixed, controllable values.
// All 16 pure virtuals are implemented so the stub is concrete.
// ---------------------------------------------------------------------------
namespace {

using truss::core::Real;
using truss::core::interfaces::IAnalysisResultsView;

class StubAnalysisResultsView final : public IAnalysisResultsView {
public:
    // Configurable fields; set them directly from tests.
    bool converged{true};
    int iterations{3};
    Real residualNorm{1.23e-8};
    Real conditionNumber{42.5};
    Real maxDisplacement{0.005};  // 5 mm
    Real maxStress{150.0e6};      // 150 MPa
    Real totalStrain{0.75};       // 0.75 J
    std::size_t totalDofs{6};
    std::size_t freeDofs{2};
    std::size_t constrainedDofs{4};

    // Vectors (unused by ResultsTableModel::refresh but required as pure-virtual overrides)
    const std::vector<Real>& getDisplacements() const override { return m_empty; }
    const std::vector<Real>& getReactions() const override { return m_empty; }
    const std::vector<Real>& getMemberForces() const override { return m_empty; }
    const std::vector<Real>& getMemberStresses() const override { return m_empty; }
    const std::vector<Real>& getUtilizationRatios() const override { return m_empty; }
    const std::vector<std::vector<Real>>& getStiffnessMatrix() const override { return m_matrix; }

    // Scalars used directly by ResultsTableModel::refresh()
    bool hasConverged() const override { return converged; }
    int getIterations() const override { return iterations; }
    Real getResidualNorm() const override { return residualNorm; }
    Real getConditionNumber() const override { return conditionNumber; }
    std::size_t getTotalDofs() const override { return totalDofs; }
    std::size_t getFreeDofs() const override { return freeDofs; }
    std::size_t getConstrainedDofs() const override { return constrainedDofs; }
    Real getTotalStrain() const override { return totalStrain; }
    Real getMaxDisplacement() const override { return maxDisplacement; }
    Real getMaxStress() const override { return maxStress; }

private:
    std::vector<Real> m_empty;
    std::vector<std::vector<Real>> m_matrix;
};

}  // namespace

// ---------------------------------------------------------------------------
// Tests — refresh() population
// ---------------------------------------------------------------------------

TEST_F(ResultsTableModelTest, Refresh_PopulatesExpectedNumberOfRows) {
    // ResultsTableModel::refresh() adds 10 fixed rows regardless of view values.
    StubAnalysisResultsView view;
    model.refresh(view);
    EXPECT_EQ(model.rowCount(), 10);
}

TEST_F(ResultsTableModelTest, Refresh_ConvergedTrue_ShowsYesCheckmark) {
    StubAnalysisResultsView view;
    view.converged = true;
    model.refresh(view);

    const auto val = model.data(model.index(0, truss::gui::model::ResultsTableModel::kColValue));
    ASSERT_TRUE(val.isValid());
    EXPECT_TRUE(val.toString().startsWith(QStringLiteral("Yes")));
}

TEST_F(ResultsTableModelTest, Refresh_ConvergedFalse_ShowsNoCross) {
    StubAnalysisResultsView view;
    view.converged = false;
    model.refresh(view);

    const auto val = model.data(model.index(0, truss::gui::model::ResultsTableModel::kColValue));
    ASSERT_TRUE(val.isValid());
    EXPECT_TRUE(val.toString().startsWith(QStringLiteral("No")));
}

TEST_F(ResultsTableModelTest, Refresh_DisplaysKeyForFirstRow) {
    StubAnalysisResultsView view;
    model.refresh(view);

    const auto key = model.data(model.index(0, truss::gui::model::ResultsTableModel::kColKey));
    ASSERT_TRUE(key.isValid());
    EXPECT_EQ(key.toString(), QStringLiteral("Converged"));
}

TEST_F(ResultsTableModelTest, Refresh_DisplaysIterationCount) {
    StubAnalysisResultsView view;
    view.iterations = 7;
    model.refresh(view);

    // Row 1 is "Iterations"
    const auto val = model.data(model.index(1, truss::gui::model::ResultsTableModel::kColValue));
    ASSERT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QStringLiteral("7"));
}

TEST_F(ResultsTableModelTest, Refresh_DisplaysMaxDisplacementInMillimetres) {
    // getMaxDisplacement() = 0.005 m → "5.0000 mm"
    StubAnalysisResultsView view;
    view.maxDisplacement = 0.005;
    model.refresh(view);

    // Row 4 is "Max Displacement"
    const auto val = model.data(model.index(4, truss::gui::model::ResultsTableModel::kColValue));
    ASSERT_TRUE(val.isValid());
    EXPECT_TRUE(val.toString().contains(QStringLiteral("mm")));
    EXPECT_TRUE(val.toString().contains(QStringLiteral("5.")));
}

TEST_F(ResultsTableModelTest, Refresh_DisplaysMaxStressInMPa) {
    // getMaxStress() = 150e6 Pa → "150.000 MPa"
    StubAnalysisResultsView view;
    view.maxStress = 150.0e6;
    model.refresh(view);

    // Row 5 is "Max Stress"
    const auto val = model.data(model.index(5, truss::gui::model::ResultsTableModel::kColValue));
    ASSERT_TRUE(val.isValid());
    EXPECT_TRUE(val.toString().contains(QStringLiteral("MPa")));
    EXPECT_TRUE(val.toString().contains(QStringLiteral("150")));
}

TEST_F(ResultsTableModelTest, Refresh_DisplaysDofCounts) {
    StubAnalysisResultsView view;
    view.totalDofs = 6;
    view.freeDofs = 2;
    view.constrainedDofs = 4;
    model.refresh(view);

    using M = truss::gui::model::ResultsTableModel;
    // Rows 7, 8, 9 are Total DOFs, Free DOFs, Constrained DOFs
    const auto total = model.data(model.index(7, M::kColValue));
    const auto free = model.data(model.index(8, M::kColValue));
    const auto constrained = model.data(model.index(9, M::kColValue));

    ASSERT_TRUE(total.isValid());
    ASSERT_TRUE(free.isValid());
    ASSERT_TRUE(constrained.isValid());

    EXPECT_EQ(total.toString(), QStringLiteral("6"));
    EXPECT_EQ(free.toString(), QStringLiteral("2"));
    EXPECT_EQ(constrained.toString(), QStringLiteral("4"));
}

TEST_F(ResultsTableModelTest, Refresh_EmitsModelResetSignal) {
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);
    StubAnalysisResultsView view;
    model.refresh(view);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ResultsTableModelTest, Refresh_TextAlignmentRole_KeyColumnIsLeftAligned) {
    StubAnalysisResultsView view;
    model.refresh(view);

    using M = truss::gui::model::ResultsTableModel;
    const auto align = model.data(model.index(0, M::kColKey), Qt::TextAlignmentRole);
    ASSERT_TRUE(align.isValid());
    const int flags = align.toInt();
    EXPECT_NE(flags & Qt::AlignLeft, 0);
}

TEST_F(ResultsTableModelTest, Refresh_TextAlignmentRole_ValueColumnIsRightAligned) {
    StubAnalysisResultsView view;
    model.refresh(view);

    using M = truss::gui::model::ResultsTableModel;
    const auto align = model.data(model.index(0, M::kColValue), Qt::TextAlignmentRole);
    ASSERT_TRUE(align.isValid());
    const int flags = align.toInt();
    EXPECT_NE(flags & Qt::AlignRight, 0);
}

TEST_F(ResultsTableModelTest, Clear_AfterRefresh_ResetsToZeroRows) {
    StubAnalysisResultsView view;
    model.refresh(view);
    ASSERT_EQ(model.rowCount(), 10);

    model.clear();
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(ResultsTableModelTest, Clear_EmitsModelResetWhenNonEmpty) {
    StubAnalysisResultsView view;
    model.refresh(view);

    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);
    model.clear();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ResultsTableModelTest, Clear_OnAlreadyEmpty_DoesNotCrash) {
    // clear() guards with `if (m_rows.empty()) return;` — must be a silent no-op.
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);
    ASSERT_NO_FATAL_FAILURE(model.clear());
    EXPECT_EQ(spy.count(), 0);  // guard: no signal emitted for already-empty model
}

TEST_F(ResultsTableModelTest, Refresh_VerticalHeader_ReturnsRowNumber) {
    StubAnalysisResultsView view;
    model.refresh(view);

    // Vertical header returns 1-based row number
    const auto hdr = model.headerData(0, Qt::Vertical, Qt::DisplayRole);
    ASSERT_TRUE(hdr.isValid());
    EXPECT_EQ(hdr.toInt(), 1);
}
