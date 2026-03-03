/**
 * @file test_validation_list_model.cpp
 * @brief Unit tests for ValidationListModel.
 *
 * Phase 3: Qt Item Models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/validation_list_model.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QPixmap>
#include <QSignalSpy>
#include <QString>

#include <gtest/gtest.h>

using truss::core::validation::ValidationIssue;
using truss::core::validation::ValidationResult;
using truss::core::validation::ValidationSeverity;

// ---------------------------------------------------------------------------
// QApplication bootstrap (shared static pattern)
// ---------------------------------------------------------------------------
namespace {

QApplication& ensureQApp()
{
    static int   s_argc    = 1;
    static char  s_argv0[] = "unit_tests";
    static char* s_argv[]  = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* existing = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return existing;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

} // namespace

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class ValidationListModelTest : public ::testing::Test {
protected:
    void SetUp() override { ensureQApp(); }

    truss::gui::model::ValidationListModel model;
};

// ---------------------------------------------------------------------------
// Basic structure
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, InitialRowCountIsZero)
{
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(ValidationListModelTest, InvalidIndexReturnsInvalidVariant)
{
    EXPECT_FALSE(model.data(QModelIndex{}).isValid());
    EXPECT_FALSE(model.data(model.index(99)).isValid());
}

// ---------------------------------------------------------------------------
// refresh()
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, RefreshUpdatesRowCount)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Error, "Geometry", "Bad geometry"});
    result.addIssue({ValidationSeverity::Warning, "Material", "High stress"});

    model.refresh(result);

    EXPECT_EQ(model.rowCount(), 2);
}

TEST_F(ValidationListModelTest, RefreshEmitsModelResetSignal)
{
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);

    ValidationResult result;
    result.addIssue({ValidationSeverity::Info, "General", "Note"});
    model.refresh(result);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ValidationListModelTest, RefreshWithEmptyResultClearsRows)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Error, "X", "Err"});
    model.refresh(result);
    ASSERT_EQ(model.rowCount(), 1);

    ValidationResult empty;
    model.refresh(empty);
    EXPECT_EQ(model.rowCount(), 0);
}

// ---------------------------------------------------------------------------
// DisplayRole
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, DisplayRole_FormatIsCategoryPlusMessage)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Error, "Geometry", "Node overlap detected"});
    model.refresh(result);

    const auto display = model.data(model.index(0), Qt::DisplayRole);
    ASSERT_TRUE(display.isValid());
    EXPECT_EQ(display.toString(), QStringLiteral("[Geometry] Node overlap detected"));
}

TEST_F(ValidationListModelTest, DisplayRole_MultipleIssuesInOrder)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Info,    "Cat1", "Message1"});
    result.addIssue({ValidationSeverity::Warning, "Cat2", "Message2"});
    result.addIssue({ValidationSeverity::Fatal,   "Cat3", "Message3"});
    model.refresh(result);

    EXPECT_EQ(model.data(model.index(0)).toString(), QStringLiteral("[Cat1] Message1"));
    EXPECT_EQ(model.data(model.index(1)).toString(), QStringLiteral("[Cat2] Message2"));
    EXPECT_EQ(model.data(model.index(2)).toString(), QStringLiteral("[Cat3] Message3"));
}

// ---------------------------------------------------------------------------
// DecorationRole — small colour pixmap
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, DecorationRole_ReturnsPixmapForEachSeverity)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Info,    "A", "a"});
    result.addIssue({ValidationSeverity::Warning, "B", "b"});
    result.addIssue({ValidationSeverity::Error,   "C", "c"});
    result.addIssue({ValidationSeverity::Fatal,   "D", "d"});
    model.refresh(result);

    for (int row = 0; row < 4; ++row) {
        const auto deco = model.data(model.index(row), Qt::DecorationRole);
        ASSERT_TRUE(deco.isValid()) << "Row " << row;
        EXPECT_TRUE(deco.canConvert<QPixmap>()) << "Row " << row;
        const auto pix = deco.value<QPixmap>();
        EXPECT_FALSE(pix.isNull()) << "Row " << row;
    }
}

// ---------------------------------------------------------------------------
// ToolTipRole
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, ToolTipRole_ShownWhenTechnicalDetailPresent)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Error, "Geo", "Problem", "Technical detail here"});
    model.refresh(result);

    const auto tip = model.data(model.index(0), Qt::ToolTipRole);
    ASSERT_TRUE(tip.isValid());
    EXPECT_EQ(tip.toString(), QStringLiteral("Technical detail here"));
}

TEST_F(ValidationListModelTest, ToolTipRole_EmptyWhenNoTechnicalDetail)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Info, "X", "Y"});  // no technicalDetail
    model.refresh(result);

    const auto tip = model.data(model.index(0), Qt::ToolTipRole);
    EXPECT_FALSE(tip.isValid());
}

// ---------------------------------------------------------------------------
// UserRole — affected entity IDs
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, NodeIdRole_ReturnsFirstAffectedNodeOrZero)
{
    using M = truss::gui::model::ValidationListModel;

    ValidationResult result;
    ValidationIssue issue{ValidationSeverity::Error, "X", "Y"};
    issue.affectedNodes = {42u, 7u};
    result.addIssue(issue);
    model.refresh(result);

    const auto nodeId = model.data(model.index(0), M::kNodeIdRole);
    ASSERT_TRUE(nodeId.isValid());
    EXPECT_EQ(nodeId.toUInt(), 42u);
}

TEST_F(ValidationListModelTest, NodeIdRole_ReturnsZeroWhenNoAffectedNodes)
{
    using M = truss::gui::model::ValidationListModel;

    ValidationResult result;
    result.addIssue({ValidationSeverity::Warning, "X", "Y"});
    model.refresh(result);

    EXPECT_EQ(model.data(model.index(0), M::kNodeIdRole).toUInt(), 0u);
}

TEST_F(ValidationListModelTest, MemberIdRole_ReturnsFirstAffectedMember)
{
    using M = truss::gui::model::ValidationListModel;

    ValidationResult result;
    ValidationIssue issue{ValidationSeverity::Fatal, "X", "Y"};
    issue.affectedMembers = {99u};
    result.addIssue(issue);
    model.refresh(result);

    EXPECT_EQ(model.data(model.index(0), M::kMemberIdRole).toUInt(), 99u);
}

// ---------------------------------------------------------------------------
// clear() slot
// ---------------------------------------------------------------------------

TEST_F(ValidationListModelTest, ClearEmptiesModel)
{
    ValidationResult result;
    result.addIssue({ValidationSeverity::Error, "A", "B"});
    model.refresh(result);
    ASSERT_EQ(model.rowCount(), 1);

    model.clear();
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(ValidationListModelTest, ClearOnEmptyModelDoesNotEmitSignal)
{
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);
    model.clear();  // already empty
    EXPECT_EQ(spy.count(), 0);
}
