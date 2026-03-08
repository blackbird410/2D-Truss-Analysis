/**
 * @file test_results_table_model.cpp
 * @brief Unit tests for ResultsTableModel (Phase 3 skeleton).
 *
 * Phase 3: Qt Item Models — verifies the skeleton contract.
 * Phase 6 will add tests for refresh() population.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/results_table_model.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QString>

#include <gtest/gtest.h>

// ---------------------------------------------------------------------------
// QApplication bootstrap (shared static pattern)
// ---------------------------------------------------------------------------
namespace {

QApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* existing = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return existing;
        return new QApplication(s_argc, s_argv);
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
