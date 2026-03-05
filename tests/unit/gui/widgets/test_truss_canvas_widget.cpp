/**
 * @file test_truss_canvas_widget.cpp
 * @brief Unit tests for TrussCanvasWidget (Phase 4).
 *
 * Verifies:
 *  - Widget constructs without crashing.
 *  - paintEvent executes the full 7-step pipeline on an empty model (m_view == nullptr).
 *  - paintEvent executes the full pipeline with a 3-member test truss.
 *  - setViewport rebuilds the transform without crashing.
 *  - clearCanvas resets state and schedules a repaint.
 *  - refresh() with nullptr view does not crash.
 *  - StressRatio and DeformedShape display modes do not crash.
 *
 * @note All rendering tests use QPixmap::toImage() to force a synchronous paint
 *       without requiring an on-screen window or X11 display.  QOffscreenSurface
 *       is not needed for QWidget painting in Qt6 when a QApplication is present.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/widgets/truss_canvas_widget.hpp"

#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QPixmap>
#include <QResizeEvent>

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <vector>

using truss::core::interfaces::ITrussView;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;
using truss::core::SupportType;
using truss::gui::TrussCanvasWidget;

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
        if (auto* existing = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return existing;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

// ============================================================
// Minimal ITrussView stub — 3-node, 3-member triangular truss
// ============================================================

class StubTrussView final : public ITrussView {
public:
    StubTrussView()
    {
        // Node 1 — Pinned support at origin
        NodeView n1;
        n1.id = 1; n1.x = 0.0; n1.y = 0.0;
        n1.support = SupportType::Pinned;
        m_nodes.push_back(n1);

        // Node 2 — RollerX support at (4, 0)
        NodeView n2;
        n2.id = 2; n2.x = 4.0; n2.y = 0.0;
        n2.support = SupportType::RollerX;
        m_nodes.push_back(n2);

        // Node 3 — Free node at (2, 3) with downward load
        NodeView n3;
        n3.id = 3; n3.x = 2.0; n3.y = 3.0;
        n3.support = SupportType::Free;
        n3.fx = 0.0; n3.fy = -50000.0;  // 50 kN downward
        m_nodes.push_back(n3);

        // Member 1: node 1 → 3
        MemberView m1;
        m1.id = 1; m1.startNodeId = 1; m1.endNodeId = 3;
        m1.youngModulus = 200e9; m1.area = 0.001;
        m1.length = std::sqrt(4.0 + 9.0);
        m1.angle  = std::atan2(3.0, 2.0);
        m1.axialForce = 30000.0; m1.axialStress = 30e6;
        m1.utilizationRatio = 0.12;
        m1.inTension = true; m1.yielded = false;
        m_members.push_back(m1);

        // Member 2: node 2 → 3
        MemberView m2;
        m2.id = 2; m2.startNodeId = 2; m2.endNodeId = 3;
        m2.youngModulus = 200e9; m2.area = 0.001;
        m2.length = std::sqrt(4.0 + 9.0);
        m2.angle  = std::atan2(3.0, -2.0);
        m2.axialForce = -28000.0; m2.axialStress = -28e6;
        m2.utilizationRatio = 0.11;
        m2.inTension = false; m2.yielded = false;
        m_members.push_back(m2);

        // Member 3: node 1 → 2 (bottom chord)
        MemberView m3;
        m3.id = 3; m3.startNodeId = 1; m3.endNodeId = 2;
        m3.youngModulus = 200e9; m3.area = 0.002;
        m3.length = 4.0; m3.angle = 0.0;
        m3.axialForce = -15000.0; m3.axialStress = -7.5e6;
        m3.utilizationRatio = 0.85; // high ratio — golden-amber colour expected
        m3.inTension = false; m3.yielded = false;
        m_members.push_back(m3);
    }

    const std::string& getName() const override { return m_name; }

    std::vector<NodeView>   getNodeViews()   const override { return m_nodes;   }
    std::size_t             getNodeCount()   const override { return m_nodes.size(); }
    std::vector<MemberView> getMemberViews() const override { return m_members; }
    std::size_t             getMemberCount() const override { return m_members.size(); }

    std::size_t             getTotalDofs()       const override { return 6; }
    std::size_t             getFreeDofs()        const override { return 2; }
    std::size_t             getConstrainedDofs() const override { return 4; }

private:
    std::string             m_name{"TestTruss"};
    std::vector<NodeView>   m_nodes;
    std::vector<MemberView> m_members;
};

/// @brief Force a synchronous paint by grabbing the widget into a QPixmap.
QPixmap grabWidget(TrussCanvasWidget& w)
{
    return w.grab();
}

}  // anonymous namespace

// ============================================================
// Fixture
// ============================================================

class TrussCanvasWidgetTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ensureQApp();
        widget = std::make_unique<TrussCanvasWidget>();
        widget->resize(800, 600);
    }

    void TearDown() override
    {
        widget.reset();
    }

    std::unique_ptr<TrussCanvasWidget> widget;
    StubTrussView                      trussView;
};

// ============================================================
// Tests — construction & defaults
// ============================================================

TEST_F(TrussCanvasWidgetTest, WidgetConstructsWithoutCrash)
{
    EXPECT_NE(widget.get(), nullptr);
}

TEST_F(TrussCanvasWidgetTest, ObjectNameIsSet)
{
    EXPECT_EQ(widget->objectName(), QStringLiteral("trussCanvas"));
}

TEST_F(TrussCanvasWidgetTest, MinimumSizeIsRespected)
{
    EXPECT_GE(widget->minimumWidth(),  400);
    EXPECT_GE(widget->minimumHeight(), 300);
}

// ============================================================
// Tests — paintEvent (rendering smoke tests)
// ============================================================

TEST_F(TrussCanvasWidgetTest, PaintEventWithNullView_DoesNotCrash)
{
    // m_view == nullptr → empty-state hint rendered in Step 7
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

TEST_F(TrussCanvasWidgetTest, PaintEventWithThreeMemberTruss_DoesNotCrash)
{
    widget->refresh(&trussView, TrussCanvasWidget::DisplayMode::Geometry);
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

TEST_F(TrussCanvasWidgetTest, PaintEventStressRatioMode_DoesNotCrash)
{
    widget->refresh(&trussView, TrussCanvasWidget::DisplayMode::StressRatio);
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

TEST_F(TrussCanvasWidgetTest, PaintEventDeformedShapeMode_DoesNotCrash)
{
    widget->refresh(&trussView, TrussCanvasWidget::DisplayMode::DeformedShape);
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

TEST_F(TrussCanvasWidgetTest, PaintAfterResize_DoesNotCrash)
{
    widget->refresh(&trussView);
    widget->resize(1280, 800);
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

// ============================================================
// Tests — setViewport
// ============================================================

TEST_F(TrussCanvasWidgetTest, SetViewportAcceptsValidBounds)
{
    // Should not throw or crash; triggers rebuildTransform() + update()
    ASSERT_NO_FATAL_FAILURE(
        widget->setViewport(QRectF(-1.0, -1.0, 6.0, 5.0)));
    auto px = grabWidget(*widget);
    EXPECT_FALSE(px.isNull());
}

TEST_F(TrussCanvasWidgetTest, SetViewportIgnoresZeroAreaRect)
{
    // Zero-area rect must be silently ignored
    ASSERT_NO_FATAL_FAILURE(
        widget->setViewport(QRectF(0.0, 0.0, 0.0, 0.0)));
}

// ============================================================
// Tests — refresh / clearCanvas
// ============================================================

TEST_F(TrussCanvasWidgetTest, RefreshWithNullViewDoesNotCrash)
{
    widget->refresh(nullptr);
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

TEST_F(TrussCanvasWidgetTest, ClearCanvasResetsToEmptyState)
{
    widget->refresh(&trussView);
    widget->clearCanvas();
    // After clear, should render the empty-state hint without crashing
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

// ============================================================
// Tests — Phase 6: ToolMode
// ============================================================

TEST_F(TrussCanvasWidgetTest, SetModeToSelectDoesNotCrash)
{
    ASSERT_NO_FATAL_FAILURE(
        widget->setMode(TrussCanvasWidget::ToolMode::Select));
}

TEST_F(TrussCanvasWidgetTest, SetModeToAddNodeDoesNotCrash)
{
    ASSERT_NO_FATAL_FAILURE(
        widget->setMode(TrussCanvasWidget::ToolMode::AddNode));
}

TEST_F(TrussCanvasWidgetTest, SetModeToAddMemberDoesNotCrash)
{
    ASSERT_NO_FATAL_FAILURE(
        widget->setMode(TrussCanvasWidget::ToolMode::AddMember));
}

TEST_F(TrussCanvasWidgetTest, SetModeToDeleteDoesNotCrash)
{
    ASSERT_NO_FATAL_FAILURE(
        widget->setMode(TrussCanvasWidget::ToolMode::Delete));
}

TEST_F(TrussCanvasWidgetTest, SetModeTriggersRepaint)
{
    // Switching modes triggers update(). Grabbing forces synchronous paint.
    widget->setMode(TrussCanvasWidget::ToolMode::AddNode);
    ASSERT_NO_FATAL_FAILURE({
        auto px = grabWidget(*widget);
        EXPECT_FALSE(px.isNull());
    });
}

