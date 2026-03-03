/**
 * @file test_node_table_model.cpp
 * @brief Unit tests for NodeTableModel.
 *
 * Phase 3: Qt Item Models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/node_table_model.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <gtest/gtest.h>

#include <string>
#include <vector>

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

// ---------------------------------------------------------------------------
// Minimal ITrussView stub for tests
// ---------------------------------------------------------------------------
using truss::core::interfaces::ITrussView;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;
using truss::core::SupportType;

class StubTrussView final : public ITrussView {
public:
    explicit StubTrussView(std::vector<NodeView> nodes = {})
        : m_nodes(std::move(nodes)) {}

    [[nodiscard]] std::vector<NodeView>   getNodeViews()   const override { return m_nodes;   }
    [[nodiscard]] std::vector<MemberView> getMemberViews() const override { return {};         }
    [[nodiscard]] size_t getNodeCount()        const override { return m_nodes.size(); }
    [[nodiscard]] size_t getMemberCount()      const override { return 0;              }
    [[nodiscard]] const std::string& getName() const override { return m_name;         }
    [[nodiscard]] size_t getTotalDofs()        const override { return 0;              }
    [[nodiscard]] size_t getFreeDofs()         const override { return 0;              }
    [[nodiscard]] size_t getConstrainedDofs()  const override { return 0;              }

private:
    std::vector<NodeView> m_nodes;
    std::string           m_name{"stub"};
};

} // namespace

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class NodeTableModelTest : public ::testing::Test {
protected:
    void SetUp() override { ensureQApp(); }

    truss::gui::model::NodeTableModel model;
};

// ---------------------------------------------------------------------------
// Basic structure
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, InitialRowCountIsZero)
{
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(NodeTableModelTest, ColumnCountIsAlwaysTen)
{
    EXPECT_EQ(model.columnCount(), 10);
    EXPECT_EQ(model.columnCount(), truss::gui::model::NodeTableModel::kColumnCount);
}

TEST_F(NodeTableModelTest, HorizontalHeaderLabels)
{
    const std::vector<std::string> expected = {
        "ID", "X [m]", "Y [m]", "Support",
        "Fx [kN]", "Fy [kN]", "dx [mm]", "dy [mm]",
        "Rx [kN]", "Ry [kN]",
    };

    for (int i = 0; i < 10; ++i) {
        const auto hdr = model.headerData(i, Qt::Horizontal, Qt::DisplayRole);
        ASSERT_TRUE(hdr.isValid())   << "Column " << i << " header is invalid";
        EXPECT_EQ(hdr.toString().toStdString(), expected[static_cast<size_t>(i)])
            << "Column " << i;
    }
}

// ---------------------------------------------------------------------------
// refresh() populates rows
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, RefreshUpdatesRowCount)
{
    NodeView n1{1, 0.0, 0.0, SupportType::Pinned};
    NodeView n2{2, 1.0, 0.0, SupportType::Free};
    NodeView n3{3, 0.5, 1.0, SupportType::Free, /*fx=*/10000.0};

    StubTrussView view({n1, n2, n3});
    model.refresh(view);

    EXPECT_EQ(model.rowCount(), 3);
}

TEST_F(NodeTableModelTest, RefreshEmitsModelResetSignal)
{
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);

    NodeView n{1, 0.0, 0.0, SupportType::Free};
    StubTrussView view({n});
    model.refresh(view);

    EXPECT_EQ(spy.count(), 1);
}

// ---------------------------------------------------------------------------
// DisplayRole values
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, DisplayRole_Id)
{
    NodeView n{42u, 1.5, -2.3, SupportType::Free};
    StubTrussView view({n});
    model.refresh(view);

    const auto idx = model.index(0, truss::gui::model::NodeTableModel::kColId);
    EXPECT_EQ(model.data(idx).toString(), QStringLiteral("42"));
}

TEST_F(NodeTableModelTest, DisplayRole_X)
{
    NodeView n{1, 1.23456, 0.0, SupportType::Free};
    StubTrussView view({n});
    model.refresh(view);

    const auto idx = model.index(0, truss::gui::model::NodeTableModel::kColX);
    EXPECT_EQ(model.data(idx).toString(), QStringLiteral("1.2346"));
}

TEST_F(NodeTableModelTest, DisplayRole_SupportPinned)
{
    NodeView n{1, 0.0, 0.0, SupportType::Pinned};
    StubTrussView view({n});
    model.refresh(view);

    const auto idx = model.index(0, truss::gui::model::NodeTableModel::kColSupport);
    EXPECT_EQ(model.data(idx).toString(), QStringLiteral("Pinned (X+Y)"));
}

TEST_F(NodeTableModelTest, DisplayRole_ForceInKN)
{
    NodeView n{1, 0.0, 0.0, SupportType::Free, /*fx=*/5000.0, /*fy=*/-3000.0};
    StubTrussView view({n});
    model.refresh(view);

    const auto idxFx = model.index(0, truss::gui::model::NodeTableModel::kColFx);
    EXPECT_EQ(model.data(idxFx).toString(), QStringLiteral("5.000"));

    const auto idxFy = model.index(0, truss::gui::model::NodeTableModel::kColFy);
    EXPECT_EQ(model.data(idxFy).toString(), QStringLiteral("-3.000"));
}

// ---------------------------------------------------------------------------
// Results columns show "—" before setHasResults
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, ResultsColumnShowDashBeforeResults)
{
    using M = truss::gui::model::NodeTableModel;
    NodeView n{1, 0.0, 0.0, SupportType::Pinned, 0, 0, 0.001, 0.002, 100.0, 200.0};
    StubTrussView view({n});
    model.refresh(view);

    for (int col : {M::kColDx, M::kColDy, M::kColRx, M::kColRy}) {
        const auto idx = model.index(0, col);
        EXPECT_EQ(model.data(idx).toString(), QStringLiteral("—"))
            << "Col " << col << " should be '—' before setHasResults";
    }
}

TEST_F(NodeTableModelTest, ResultsColumnShowValuesAfterSetHasResults)
{
    using M = truss::gui::model::NodeTableModel;
    // dx=0.001 m → 1.0 mm, dy=0.002 m → 2.0 mm
    NodeView n{1, 0.0, 0.0, SupportType::Pinned, 0, 0, 0.001, 0.002, 1000.0, 2000.0};
    StubTrussView view({n});
    model.refresh(view);
    model.setHasResults(true);

    // dx in mm
    EXPECT_EQ(model.data(model.index(0, M::kColDx)).toString(), QStringLiteral("1.0000"));
    // Rx: 1000 N / 1000 = 1.000 kN
    EXPECT_EQ(model.data(model.index(0, M::kColRx)).toString(), QStringLiteral("1.000"));
}

// ---------------------------------------------------------------------------
// BackgroundRole — loaded nodes
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, BackgroundRole_LoadedNodeHasAmberTint)
{
    NodeView n{1, 0.0, 0.0, SupportType::Free, /*fx=*/1000.0};
    StubTrussView view({n});
    model.refresh(view);

    const auto idx = model.index(0, truss::gui::model::NodeTableModel::kColId);
    const auto bg  = model.data(idx, Qt::BackgroundRole);
    ASSERT_TRUE(bg.isValid());

    const auto colour = bg.value<QColor>();
    // Amber R=255, check at least that it's not default
    EXPECT_GT(colour.alpha(), 0);
    EXPECT_EQ(colour.red(), 255);   // Material Amber R
}

TEST_F(NodeTableModelTest, BackgroundRole_UnloadedNodeHasNoBackground)
{
    NodeView n{1, 0.0, 0.0, SupportType::Free};
    StubTrussView view({n});
    model.refresh(view);

    const auto bg = model.data(model.index(0, 0), Qt::BackgroundRole);
    EXPECT_FALSE(bg.isValid());
}

// ---------------------------------------------------------------------------
// ToolTipRole for support column
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, ToolTipRole_SupportColumn)
{
    using M = truss::gui::model::NodeTableModel;

    NodeView n{1, 0.0, 0.0, SupportType::RollerX};
    StubTrussView view({n});
    model.refresh(view);

    const auto tip = model.data(model.index(0, M::kColSupport), Qt::ToolTipRole);
    ASSERT_TRUE(tip.isValid());
    EXPECT_EQ(tip.toString(), QStringLiteral("Roller X"));
}

// ---------------------------------------------------------------------------
// Invalid index guard
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, InvalidIndexReturnsInvalidVariant)
{
    EXPECT_FALSE(model.data(QModelIndex{}).isValid());
    EXPECT_FALSE(model.data(model.index(99, 0)).isValid());
}

// ---------------------------------------------------------------------------
// setHasResults idempotency
// ---------------------------------------------------------------------------

TEST_F(NodeTableModelTest, SetHasResultsIdempotent)
{
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);
    model.setHasResults(false); // no change, no signal
    EXPECT_EQ(spy.count(), 0);

    model.setHasResults(true);  // change — one reset
    EXPECT_EQ(spy.count(), 1);

    model.setHasResults(true);  // no change — no additional signal
    EXPECT_EQ(spy.count(), 1);
}
