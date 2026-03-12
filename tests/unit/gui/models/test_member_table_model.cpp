/**
 * @file test_member_table_model.cpp
 * @brief Unit tests for MemberTableModel.
 *
 * Phase 3: Qt Item Models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/member_table_model.hpp"

#include <QBrush>
#include <QColor>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <gtest/gtest.h>
#include <numbers>
#include <string>
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

// ---------------------------------------------------------------------------
// Minimal ITrussView stub
// ---------------------------------------------------------------------------
using truss::core::interfaces::ITrussView;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;

class StubTrussView final : public ITrussView {
public:
    explicit StubTrussView(std::vector<MemberView> members = {}) : m_members(std::move(members)) {}

    [[nodiscard]] std::vector<NodeView> getNodeViews() const override { return {}; }
    [[nodiscard]] std::vector<MemberView> getMemberViews() const override { return m_members; }
    [[nodiscard]] size_t getNodeCount() const override { return 0; }
    [[nodiscard]] size_t getMemberCount() const override { return m_members.size(); }
    [[nodiscard]] const std::string& getName() const override { return m_name; }
    [[nodiscard]] size_t getTotalDofs() const override { return 0; }
    [[nodiscard]] size_t getFreeDofs() const override { return 0; }
    [[nodiscard]] size_t getConstrainedDofs() const override { return 0; }

private:
    std::vector<MemberView> m_members;
    std::string m_name{"stub"};
};

// Helper to build a minimal MemberView
MemberView makeMember(truss::core::MemberId id = 1,
                      double young = 200e9,
                      double area = 1e-4,
                      double length = 1.0,
                      double angle = 0.0,
                      double force = 0.0,
                      double stress = 0.0,
                      double ratio = 0.0,
                      bool inTension = false,
                      bool yielded = false) {
    MemberView m;
    m.id = id;
    m.startNodeId = 1;
    m.endNodeId = 2;
    m.label = "Steel";
    m.youngModulus = young;
    m.yieldStrength = 250e6;
    m.density = 7850.0;
    m.area = area;
    m.length = length;
    m.angle = angle;
    m.axialForce = force;
    m.axialStress = stress;
    m.utilizationRatio = ratio;
    m.inTension = inTension;
    m.yielded = yielded;
    return m;
}

}  // namespace

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class MemberTableModelTest : public ::testing::Test {
protected:
    void SetUp() override { ensureQApp(); }

    truss::gui::model::MemberTableModel model;
};

// ---------------------------------------------------------------------------
// Basic structure
// ---------------------------------------------------------------------------

TEST_F(MemberTableModelTest, InitialRowCountIsZero) {
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(MemberTableModelTest, ColumnCountIsTwelve) {
    EXPECT_EQ(model.columnCount(), 12);
    EXPECT_EQ(model.columnCount(), truss::gui::model::MemberTableModel::kColumnCount);
}

TEST_F(MemberTableModelTest, HorizontalHeaderLabels) {
    const std::vector<std::string> expected = {
        "ID",
        "Start",
        "End",
        "Material",
        "E [GPa]",
        "A [cm²]",
        "Length [m]",
        "Angle [°]",
        "Force [kN]",
        "Stress [MPa]",
        "Util. Ratio",
        "State",
    };

    for (int i = 0; i < 12; ++i) {
        const auto hdr = model.headerData(i, Qt::Horizontal, Qt::DisplayRole);
        ASSERT_TRUE(hdr.isValid()) << "Column " << i;
        EXPECT_EQ(hdr.toString().toStdString(), expected[static_cast<size_t>(i)]) << "Column " << i;
    }
}

// ---------------------------------------------------------------------------
// refresh()
// ---------------------------------------------------------------------------

TEST_F(MemberTableModelTest, RefreshUpdatesRowCount) {
    StubTrussView view({makeMember(1), makeMember(2), makeMember(3)});
    model.refresh(view);
    EXPECT_EQ(model.rowCount(), 3);
}

TEST_F(MemberTableModelTest, RefreshEmitsModelResetSignal) {
    QSignalSpy spy(&model, &QAbstractItemModel::modelReset);
    StubTrussView view({makeMember()});
    model.refresh(view);
    EXPECT_EQ(spy.count(), 1);
}

// ---------------------------------------------------------------------------
// DisplayRole
// ---------------------------------------------------------------------------

TEST_F(MemberTableModelTest, DisplayRole_E_InGPa) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, /*young=*/210e9)});
    model.refresh(view);

    EXPECT_EQ(model.data(model.index(0, M::kColE)).toString(), QStringLiteral("210.0"));
}

TEST_F(MemberTableModelTest, DisplayRole_A_InCm2) {
    using M = truss::gui::model::MemberTableModel;
    // 1e-4 m² = 1.0000 cm²
    StubTrussView view({makeMember(1, 200e9, 1e-4)});
    model.refresh(view);

    EXPECT_EQ(model.data(model.index(0, M::kColA)).toString(), QStringLiteral("1.0000"));
}

TEST_F(MemberTableModelTest, DisplayRole_Angle_InDegrees) {
    using M = truss::gui::model::MemberTableModel;
    // 90° = π/2 radians
    const double angle90 = std::numbers::pi / 2.0;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, angle90)});
    model.refresh(view);

    const auto s = model.data(model.index(0, M::kColAngle)).toString();
    EXPECT_EQ(s, QStringLiteral("90.00"));
}

// ---------------------------------------------------------------------------
// Results columns show "—" before setHasResults
// ---------------------------------------------------------------------------

TEST_F(MemberTableModelTest, ResultsColumnsShowDashBeforeResults) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 50000.0, 500e6, 0.8, true, false)});
    model.refresh(view);

    for (int col : {M::kColForce, M::kColStress, M::kColRatio, M::kColState}) {
        EXPECT_EQ(model.data(model.index(0, col)).toString(), QStringLiteral("—")) << "Col " << col;
    }
}

TEST_F(MemberTableModelTest, ResultsColumnsShowValuesAfterSetHasResults) {
    using M = truss::gui::model::MemberTableModel;
    // force=50000 N → 50.000 kN, stress=500e6 Pa → 500.000 MPa
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 50000.0, 500e6, 0.8, true, false)});
    model.refresh(view);
    model.setHasResults(true);

    EXPECT_EQ(model.data(model.index(0, M::kColForce)).toString(), QStringLiteral("50.000"));
    EXPECT_EQ(model.data(model.index(0, M::kColStress)).toString(), QStringLiteral("500.000"));
    EXPECT_EQ(model.data(model.index(0, M::kColRatio)).toString(), QStringLiteral("0.800"));
    EXPECT_EQ(model.data(model.index(0, M::kColState)).toString(), QStringLiteral("Tension"));
}

TEST_F(MemberTableModelTest, StateString_Yielded) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 0, 0, 1.2, true, /*yielded=*/true)});
    model.refresh(view);
    model.setHasResults(true);

    EXPECT_EQ(model.data(model.index(0, M::kColState)).toString(), QStringLiteral("Yielded"));
}

TEST_F(MemberTableModelTest, StateString_Compression) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 0, 0, 0.5, /*inTension=*/false)});
    model.refresh(view);
    model.setHasResults(true);

    EXPECT_EQ(model.data(model.index(0, M::kColState)).toString(), QStringLiteral("Compression"));
}

// ---------------------------------------------------------------------------
// ForegroundRole — colour interpolation
// ---------------------------------------------------------------------------

TEST_F(MemberTableModelTest, ForegroundRole_GreenForZeroRatio) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 0, 0, 0.0)});
    model.refresh(view);
    model.setHasResults(true);

    const auto fg = model.data(model.index(0, M::kColRatio), Qt::ForegroundRole);
    ASSERT_TRUE(fg.isValid());

    const auto colour = fg.value<QColor>();
    EXPECT_EQ(colour.red(), 0x34);
    EXPECT_EQ(colour.green(), 0xA8);
    EXPECT_EQ(colour.blue(), 0x53);
}

TEST_F(MemberTableModelTest, ForegroundRole_RedForRatioOne) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 0, 0, 1.0)});
    model.refresh(view);
    model.setHasResults(true);

    const auto fg = model.data(model.index(0, M::kColRatio), Qt::ForegroundRole);
    const auto colour = fg.value<QColor>();
    EXPECT_EQ(colour.red(), 0xEA);
    EXPECT_EQ(colour.green(), 0x43);
    EXPECT_EQ(colour.blue(), 0x35);
}

TEST_F(MemberTableModelTest, ForegroundRole_NotShownBeforeResults) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1, 200e9, 1e-4, 1.0, 0.0, 0, 0, 0.5)});
    model.refresh(view);
    // m_hasResults stays false

    const auto fg = model.data(model.index(0, M::kColRatio), Qt::ForegroundRole);
    EXPECT_FALSE(fg.isValid());
}

// ---------------------------------------------------------------------------
// Guard
// ---------------------------------------------------------------------------

TEST_F(MemberTableModelTest, InvalidIndexReturnsInvalidVariant) {
    EXPECT_FALSE(model.data(QModelIndex{}).isValid());
}

// ---------------------------------------------------------------------------
// BackgroundRole — semi-transparent tint on kColState (col 11)
// ---------------------------------------------------------------------------

/// Yielded member: kColState must return a reddish tint (#FF1744 @ alpha ≈ 45).
TEST_F(MemberTableModelTest, BackgroundRole_Yielded_ReturnsRedTint) {
    using M = truss::gui::model::MemberTableModel;
    // yielded=true forces the first branch in BackgroundRole
    StubTrussView view({makeMember(1,
                                   200e9,
                                   1e-4,
                                   1.0,
                                   0.0,
                                   100.0,
                                   1.0e6,
                                   0.5,
                                   /*inTension=*/false,
                                   /*yielded=*/true)});
    model.refresh(view);
    model.setHasResults(true);

    const auto bg = model.data(model.index(0, M::kColState), Qt::BackgroundRole);
    ASSERT_TRUE(bg.isValid());
    const QBrush brush = bg.value<QBrush>();
    const QColor c = brush.color();
    EXPECT_EQ(c.red(), 0xFF);
    EXPECT_EQ(c.green(), 0x17);
    EXPECT_EQ(c.blue(), 0x44);
}

/// Tension member (not yielded): kColState must return a blue tint (#4FC3F7 @ alpha ≈ 45).
TEST_F(MemberTableModelTest, BackgroundRole_Tension_ReturnsBlueTint) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1,
                                   200e9,
                                   1e-4,
                                   1.0,
                                   0.0,
                                   30000.0,
                                   30.0e6,
                                   0.12,
                                   /*inTension=*/true,
                                   /*yielded=*/false)});
    model.refresh(view);
    model.setHasResults(true);

    const auto bg = model.data(model.index(0, M::kColState), Qt::BackgroundRole);
    ASSERT_TRUE(bg.isValid());
    const QBrush brush = bg.value<QBrush>();
    const QColor c = brush.color();
    EXPECT_EQ(c.red(), 0x4F);
    EXPECT_EQ(c.green(), 0xC3);
    EXPECT_EQ(c.blue(), 0xF7);
}

/// Compression member (not yielded, axialForce < 0): kColState must return an orange tint
/// (#FF7043 @ alpha ≈ 45).
TEST_F(MemberTableModelTest, BackgroundRole_Compression_ReturnsOrangeTint) {
    using M = truss::gui::model::MemberTableModel;
    // axialForce = -15000 → compression branch (axialForce < -1e-10)
    StubTrussView view({makeMember(1,
                                   200e9,
                                   1e-4,
                                   1.0,
                                   0.0,
                                   -15000.0,
                                   -15.0e6,
                                   0.06,
                                   /*inTension=*/false,
                                   /*yielded=*/false)});
    model.refresh(view);
    model.setHasResults(true);

    const auto bg = model.data(model.index(0, M::kColState), Qt::BackgroundRole);
    ASSERT_TRUE(bg.isValid());
    const QBrush brush = bg.value<QBrush>();
    const QColor c = brush.color();
    EXPECT_EQ(c.red(), 0xFF);
    EXPECT_EQ(c.green(), 0x70);
    EXPECT_EQ(c.blue(), 0x43);
}

/// BackgroundRole on a non-state column must return an empty (invalid) QVariant.
TEST_F(MemberTableModelTest, BackgroundRole_NonStateColumn_ReturnsEmpty) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1,
                                   200e9,
                                   1e-4,
                                   1.0,
                                   0.0,
                                   30000.0,
                                   30.0e6,
                                   0.12,
                                   /*inTension=*/true,
                                   /*yielded=*/false)});
    model.refresh(view);
    model.setHasResults(true);

    // kColId (col 0) is not the state column — must return empty brush
    const auto bg = model.data(model.index(0, M::kColId), Qt::BackgroundRole);
    // An empty QVariant or a default-constructed QBrush is acceptable
    if (bg.isValid()) {
        EXPECT_EQ(bg.value<QBrush>(), QBrush{});
    }
}

/// BackgroundRole before setHasResults(true) must return an invalid QVariant
/// (results not available — no tint should be shown).
TEST_F(MemberTableModelTest, BackgroundRole_BeforeResults_ReturnsEmpty) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1,
                                   200e9,
                                   1e-4,
                                   1.0,
                                   0.0,
                                   30000.0,
                                   30.0e6,
                                   0.12,
                                   /*inTension=*/true,
                                   /*yielded=*/false)});
    model.refresh(view);
    // m_hasResults stays false (setHasResults not called)

    const auto bg = model.data(model.index(0, M::kColState), Qt::BackgroundRole);
    // Must be invalid — no tint before analysis results arrive
    EXPECT_FALSE(bg.isValid());
}

// ---------------------------------------------------------------------------
// ToolTipRole
// ---------------------------------------------------------------------------

/// ToolTipRole on kColState after results: must return the stateString (e.g. "Tension").
TEST_F(MemberTableModelTest, ToolTipRole_StateColumn_AfterResults_ReturnsStateString) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1,
                                   200e9,
                                   1e-4,
                                   1.0,
                                   0.0,
                                   30000.0,
                                   30.0e6,
                                   0.12,
                                   /*inTension=*/true,
                                   /*yielded=*/false)});
    model.refresh(view);
    model.setHasResults(true);

    const auto tip = model.data(model.index(0, M::kColState), Qt::ToolTipRole);
    ASSERT_TRUE(tip.isValid());
    EXPECT_FALSE(tip.toString().isEmpty());
}

/// ToolTipRole on kColE must return a non-empty description ("Young's Modulus").
TEST_F(MemberTableModelTest, ToolTipRole_EColumn_ReturnsDescription) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1)});
    model.refresh(view);

    const auto tip = model.data(model.index(0, M::kColE), Qt::ToolTipRole);
    ASSERT_TRUE(tip.isValid());
    EXPECT_EQ(tip.toString(), QStringLiteral("Young's Modulus"));
}

/// ToolTipRole on kColA must return "Cross-sectional area".
TEST_F(MemberTableModelTest, ToolTipRole_AColumn_ReturnsDescription) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1)});
    model.refresh(view);

    const auto tip = model.data(model.index(0, M::kColA), Qt::ToolTipRole);
    ASSERT_TRUE(tip.isValid());
    EXPECT_EQ(tip.toString(), QStringLiteral("Cross-sectional area"));
}

/// ToolTipRole on an unhandled column (e.g. kColId) must return an empty/invalid QVariant.
TEST_F(MemberTableModelTest, ToolTipRole_UnhandledColumn_ReturnsEmpty) {
    using M = truss::gui::model::MemberTableModel;
    StubTrussView view({makeMember(1)});
    model.refresh(view);

    const auto tip = model.data(model.index(0, M::kColId), Qt::ToolTipRole);
    // Either invalid or empty string is acceptable
    if (tip.isValid()) {
        EXPECT_TRUE(tip.toString().isEmpty());
    }
}
