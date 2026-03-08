/**
 * @file truss_canvas_widget.cpp
 * @brief 7-step rendering pipeline for TrussCanvasWidget.
 *
 * Full QPainter rendering pipeline (background, grid, members, nodes,
 * force arrows, reactions, overlay text).  Y-axis flip via QTransform.
 * Mouse/keyboard interaction is handled by the EventFilter layer.
 *
 * Coordinate convention:
 *   - World space: x → right, y ↑ up (structural engineering convention, metres).
 *   - Screen space: x → right, y ↓ down (Qt convention, pixels).
 *   - m_worldToScreen: affine matrix with scale(s, -s) for Y-flip + translation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/widgets/truss_canvas_widget.hpp"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QFont>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QResizeEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace truss::gui {

// ============================================================
// Local colour palette
// ============================================================
namespace {

// Background / structural chrome
constexpr int kBgR = 0x14, kBgG = 0x16, kBgB = 0x1C;                    // #14161C
constexpr int kGridR = 0x2A, kGridG = 0x2E, kGridB = 0x3A;              // #2A2E3A
constexpr int kMemberR = 0x8A, kMemberG = 0xB4, kMemberB = 0xF8;        // #8AB4F8 steel-blue
constexpr int kNodeR = 0xE8, kNodeG = 0xEA, kNodeB = 0xED;              // #E8EAED light
constexpr int kSupportR = 0x5B, kSupportG = 0x9B, kSupportB = 0xD5;     // #5B9BD5 accent
constexpr int kLoadR = 0xFF, kLoadG = 0xC1, kLoadB = 0x07;              // #FFC107 amber
constexpr int kReactionR = 0x34, kReactionG = 0xA8, kReactionB = 0x53;  // #34A853 green

// Stress-ratio colour stops: green(0) → amber(0.5) → red(1)
constexpr int kGreenR = 0x34, kGreenG = 0xA8, kGreenB = 0x53;  // #34A853
constexpr int kAmberR = 0xFF, kAmberG = 0xC1, kAmberB = 0x07;  // #FFC107
constexpr int kRedR = 0xEA, kRedG = 0x43, kRedB = 0x35;        // #EA4335

/// Grid snapping step for node placement (metres).
/// Matches typical structural grid spacing; keeps coordinates engineer-friendly.
constexpr double kGridSnapStep = 0.25;

// ── Deformed-shape display scaling ───────────────────────────────────────────
//
// Professional structural analysis tools (SAP2000, ABAQUS/CAE, RFEM) render
// deformed shapes with an amplification factor that achieves a target *visual*
// displacement equal to ~10 % of the model's bounding-box span.  To prevent
// both:
//   (a) misleading hyperbolic exaggeration when displacements are tiny (very
//       stiff structures or early-stage pre-analysis checks), and
//   (b) visual shrinkage when real displacements already exceed the target,
// the computed factor is clamped to the range [kMinDispScale, kMaxDispScale].
//
// kMaxDispScale = 500 matches the practical upper limit in ABAQUS/CAE and is
// consistent with the "Auto" deformation limit recommended in SAP2000 for steel
// structures (200–500×).  kMinDispScale = 1.0 ensures the deformed shape is
// never depicted as smaller than the true structural response.

/// Target: render the maximum nodal displacement as this fraction of the span.
constexpr double kDispVisualFraction = 0.10;

/// Floor: never shrink real displacements that already exceed the visual target.
constexpr double kMinDispScale = 1.0;

/// Cap: prevents misleading exaggeration for very stiff / lightly loaded
/// structures.  Consistent with ABAQUS/CAE auto-scale upper limit.
constexpr double kMaxDispScale = 500.0;

/// 3-stop colour interpolation: green(0) → amber(0.5) → red(1).
QColor lerpStressColour(double t) {
    t = std::clamp(t, 0.0, 1.0);
    auto lerp = [](int a, int b, double f) -> int {
        return static_cast<int>(std::round(a + (b - a) * f));
    };
    if (t <= 0.5) {
        const double f = t * 2.0;
        return QColor(
            lerp(kGreenR, kAmberR, f), lerp(kGreenG, kAmberG, f), lerp(kGreenB, kAmberB, f));
    } else {
        const double f = (t - 0.5) * 2.0;
        return QColor(lerp(kAmberR, kRedR, f), lerp(kAmberG, kRedG, f), lerp(kAmberB, kRedB, f));
    }
}

/// Snap @p v to the nearest multiple of @p step.
inline double snapToGrid(double v, double step) {
    return std::round(v / step) * step;
}

}  // anonymous namespace

// ============================================================
// Construction
// ============================================================

TrussCanvasWidget::TrussCanvasWidget(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("trussCanvas"));
    setMinimumSize(400, 300);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);           // receive mouseMoveEvent without button held
    setFocusPolicy(Qt::StrongFocus);  // receive keyPressEvent

    // Determine initial theme from the application palette
    const QColor windowColor = QApplication::palette().color(QPalette::Window);
    m_isDark = (windowColor.lightness() < 128);

    rebuildTransform();
}

// ============================================================
// Public slots
// ============================================================

void TrussCanvasWidget::refresh(const core::interfaces::ITrussView* view, DisplayMode mode) {
    m_view = view;
    m_mode = mode;
    update();
}

void TrussCanvasWidget::setViewport(const QRectF& worldBounds) {
    if (!worldBounds.isEmpty()) {
        m_worldBounds = worldBounds;
        rebuildTransform();
    }
    update();
}

void TrussCanvasWidget::clearCanvas() {
    m_view = nullptr;
    m_mode = DisplayMode::Geometry;
    m_pendingMemberStart.reset();
    m_selectedNodeId = 0;
    m_selectedMemberId = 0;
    update();
}

void TrussCanvasWidget::zoomToFit() {
    if (!m_view || m_view->getNodeCount() == 0) {
        // No geometry: restore a sensible default viewport
        setViewport(QRectF(-1.0, -1.0, 7.0, 7.0));
        return;
    }

    // Compute tight bounding box over all node positions
    double minX = +1e30, minY = +1e30;
    double maxX = -1e30, maxY = -1e30;
    for (const auto& nv : m_view->getNodeViews()) {
        minX = std::min(minX, nv.x);
        minY = std::min(minY, nv.y);
        maxX = std::max(maxX, nv.x);
        maxY = std::max(maxY, nv.y);
    }

    // Guard against degenerate single-point / collinear geometry
    if (maxX - minX < 1e-6) {
        minX -= 1.0;
        maxX += 1.0;
    }
    if (maxY - minY < 1e-6) {
        minY -= 1.0;
        maxY += 1.0;
    }

    // 15 % padding + at least 0.5 m so tiny trusses aren't crowded
    const double span = std::max(maxX - minX, maxY - minY);
    const double pad = span * 0.15 + 0.5;

    setViewport(
        QRectF(minX - pad, minY - pad, (maxX - minX) + 2.0 * pad, (maxY - minY) + 2.0 * pad));
}

// ============================================================
// Event overrides
// ============================================================

void TrussCanvasWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuildTransform();
}

void TrussCanvasWidget::changeEvent(QEvent* event) {
    // Fallback: react to OS-level palette changes (e.g. macOS system dark-mode
    // toggle).  Use this->palette(), not QApplication::palette() — the widget's
    // own palette is already updated before PaletteChange is dispatched.
    if (event->type() == QEvent::PaletteChange) {
        const QColor windowColor = palette().color(QPalette::Window);
        m_isDark = (windowColor.lightness() < 128);
        update();
    }
    QWidget::changeEvent(event);
}

void TrussCanvasWidget::setColorTheme(bool isDark) {
    if (m_isDark == isDark)
        return;
    m_isDark = isDark;
    update();
}

void TrussCanvasWidget::setDisplayMode(DisplayMode mode) {
    if (m_mode == mode)
        return;
    m_mode = mode;
    update();
}

TrussCanvasWidget::DisplayMode TrussCanvasWidget::displayMode() const noexcept {
    return m_mode;
}

void TrussCanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 7-step pipeline
    drawBackground(p);   // Step 1: clear to canvas background colour
    drawGrid(p);         // Step 2: draw world-space reference grid
    drawMembers(p);      // Step 3: draw member lines, coloured by mode
    drawNodes(p);        // Step 4: draw nodes with support symbols
    drawForceArrows(p);  // Step 5: draw applied load arrows
    drawReactions(p);    // Step 6: draw reaction arrows (post-analysis only)
    drawOverlayText(p);  // Step 7: stats bar, mode label, empty-state hint
}

// ============================================================
// Step 1 — Background
// ============================================================

void TrussCanvasWidget::drawBackground(QPainter& p) const {
    p.fillRect(rect(), m_isDark ? QColor(kBgR, kBgG, kBgB) : QColor(0xFA, 0xFA, 0xFA));
}

// ============================================================
// Step 2 — Grid
// ============================================================

void TrussCanvasWidget::drawGrid(QPainter& p) const {
    const QColor gridCol = m_isDark ? QColor(kGridR, kGridG, kGridB) : QColor(0xCC, 0xD0, 0xD8);
    p.setPen(QPen(gridCol, 0.5, Qt::SolidLine));

    const double span = std::max(m_worldBounds.width(), m_worldBounds.height());
    if (span <= 0.0)
        return;

    // Choose a nicely rounded grid step (~span/10, snapped to 1/2/5 × 10^n)
    const double rawStep = span / 10.0;
    const double mag = std::pow(10.0, std::floor(std::log10(rawStep)));
    double step = mag;
    if (rawStep / mag >= 5.0)
        step = mag * 5.0;
    else if (rawStep / mag >= 2.0)
        step = mag * 2.0;

    // World extents (top() = structural yMin, bottom() = structural yMax)
    const double xMin = m_worldBounds.left();
    const double xMax = m_worldBounds.right();
    const double yMin = m_worldBounds.top();     // structural minimum y
    const double yMax = m_worldBounds.bottom();  // structural maximum y

    const double x0 = std::floor(xMin / step) * step;
    const double y0 = std::floor(yMin / step) * step;

    for (double x = x0; x <= xMax + step * 0.5; x += step) {
        p.drawLine(toScreen(x, yMin), toScreen(x, yMax));
    }
    for (double y = y0; y <= yMax + step * 0.5; y += step) {
        p.drawLine(toScreen(xMin, y), toScreen(xMax, y));
    }
}

// ============================================================
// Step 3 — Members
// ============================================================

void TrussCanvasWidget::drawMembers(QPainter& p) const {
    if (!m_view)
        return;

    const auto nodes = m_view->getNodeViews();
    const auto members = m_view->getMemberViews();

    // Build a fast id→NodeView lookup
    std::unordered_map<core::NodeId, const core::interfaces::NodeView*> nodeMap;
    nodeMap.reserve(nodes.size());
    for (const auto& n : nodes) {
        nodeMap[n.id] = &n;
    }

    const bool showDeformed = (m_mode == DisplayMode::DeformedShape);
    const double dispScale = showDeformed ? autoDispScale() : 0.0;

    // ---- Pass 1 (DeformedShape only): original structure as semi-transparent ghost ----
    if (showDeformed) {
        // Ghost colour: semi-transparent — visible but clearly subordinate.
        const QColor ghost = m_isDark ? QColor(0x8A, 0xB4, 0xF8, 60) : QColor(0x78, 0x90, 0x9C, 90);
        p.setPen(QPen(ghost, kMemberWidth * 0.6, Qt::DashLine, Qt::RoundCap));
        for (const auto& mv : members) {
            const auto itA = nodeMap.find(mv.startNodeId);
            const auto itB = nodeMap.find(mv.endNodeId);
            if (itA == nodeMap.end() || itB == nodeMap.end())
                continue;
            p.drawLine(toScreen(itA->second->x, itA->second->y),
                       toScreen(itB->second->x, itB->second->y));
        }
    }

    // ---- Pass 2: member lines at displaced (or original) positions ----
    for (const auto& mv : members) {
        const auto itA = nodeMap.find(mv.startNodeId);
        const auto itB = nodeMap.find(mv.endNodeId);
        if (itA == nodeMap.end() || itB == nodeMap.end())
            continue;

        QPointF sA, sB;
        if (showDeformed && dispScale > 0.0) {
            // Displaced position: x' = x + scale * dx,  y' = y + scale * dy
            sA = toScreen(itA->second->x + dispScale * itA->second->dx,
                          itA->second->y + dispScale * itA->second->dy);
            sB = toScreen(itB->second->x + dispScale * itB->second->dx,
                          itB->second->y + dispScale * itB->second->dy);
        } else {
            sA = toScreen(itA->second->x, itA->second->y);
            sB = toScreen(itB->second->x, itB->second->y);
        }

        const QColor col = memberColour(mv);
        p.setPen(QPen(col, kMemberWidth, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(sA, sB);

        // Member ID label at midpoint
        const QPointF mid = (sA + sB) / 2.0;
        p.setPen(QPen(col.lighter(160)));
        {
            QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            f.setPointSize(8);
            p.setFont(f);
        }
        p.drawText(mid + QPointF(4.0, -3.0), QString::number(mv.id));
    }

    // ---- Pass 3: deformation scale annotation ──────────────────────────────
    //
    // Two values are displayed so the user can relate the visual exaggeration to
    // the physical response:
    //   • Disp. scale  — the amplification factor applied to δ in the drawing
    //   • Max δ        — the true maximum nodal displacement in mm
    //
    // E.g. "Disp. scale: 45.0×   Max δ: 1.234 mm" tells the engineer that the
    // drawn deflection is 45× larger than reality, and the real peak deflection
    // is 1.234 mm — enough context to judge serviceability at a glance.
    if (showDeformed && dispScale > 0.0) {
        // Re-use the 'nodes' vector already obtained at the top of this function.
        double maxDisp = 0.0;
        for (const auto& nv : nodes)
            maxDisp = std::max(maxDisp, std::hypot(nv.dx, nv.dy));
        const double maxDispMm = maxDisp * 1000.0;  // convert m → mm

        const QColor infoCol = m_isDark ? QColor(kSupportR, kSupportG, kSupportB)
                                        : QColor(0x15, 0x65, 0xC0);  // #1565C0 navy
        p.setPen(QPen(infoCol));
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setPointSize(9);
        p.setFont(f);
        p.drawText(QRect(8, 28, width() - 16, 20),
                   Qt::AlignLeft | Qt::AlignTop,
                   QStringLiteral("Disp. scale: %1×   Max δ: %2 mm")
                       .arg(dispScale, 0, 'g', 4)
                       .arg(maxDispMm, 0, 'g', 4));
    }
}

// ============================================================
// Step 4 — Nodes
// ============================================================

void TrussCanvasWidget::drawNodes(QPainter& p) const {
    if (!m_view)
        return;

    // Deformed-shape mode: render nodes at displaced positions.
    const bool showDeformed = (m_mode == DisplayMode::DeformedShape);
    const double dispScale = showDeformed ? autoDispScale() : 0.0;

    for (const auto& nv : m_view->getNodeViews()) {
        // World position: original or displaced
        const double drawX = (showDeformed && dispScale > 0.0) ? (nv.x + dispScale * nv.dx) : nv.x;
        const double drawY = (showDeformed && dispScale > 0.0) ? (nv.y + dispScale * nv.dy) : nv.y;
        const QPointF sc = toScreen(drawX, drawY);

        // Draw support symbol beneath the node circle
        if (nv.support != core::SupportType::Free) {
            drawSupportSymbol(p, sc, nv.support);
        }

        // Choose fill colour
        const bool hasLoad = std::abs(nv.fx) > 1e-10 || std::abs(nv.fy) > 1e-10;
        QColor fill;
        if (nv.support != core::SupportType::Free) {
            fill = QColor(kSupportR, kSupportG, kSupportB);
        } else if (hasLoad) {
            fill = QColor(kLoadR, kLoadG, kLoadB);
        } else {
            // Light theme: white fill gives clear contrast against the near-white canvas.
            // Dark theme: #E8EAED light-grey on the dark canvas.
            fill = m_isDark ? QColor(kNodeR, kNodeG, kNodeB) : QColor(0xFF, 0xFF, 0xFF);
        }

        // Border: light theme uses a dark slate outline for contrast (fill.lighter(130)
        // on white would be nearly invisible).  Dark theme keeps lighter-fill trick.
        const QColor border = m_isDark ? fill.lighter(130)
                                       : QColor(0x37, 0x47, 0x4F);  // #37474F blue-grey-700
        p.setPen(QPen(border, m_isDark ? 1.5 : 2.0));
        p.setBrush(fill);
        p.drawEllipse(sc, kNodeRadius, kNodeRadius);

        // Node ID label — needs sufficient contrast against the node fill.
        // Dark theme: use the dark canvas bg colour as text (dark on light fill).
        // Light theme: use blue-grey-900 for legible text on white fill.
        const QColor textCol = m_isDark ? QColor(kBgR, kBgG, kBgB)
                                        : QColor(0x26, 0x32, 0x38);  // #263238 blue-grey-900
        p.setPen(QPen(textCol));
        {
            QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            f.setPointSize(7);
            f.setWeight(QFont::Bold);
            p.setFont(f);
        }
        const QRectF labelRect(
            sc.x() - kNodeRadius, sc.y() - kNodeRadius, kNodeRadius * 2.0, kNodeRadius * 2.0);
        p.drawText(labelRect, Qt::AlignCenter, QString::number(nv.id));
    }
}

// ============================================================
// Step 5 — Force arrows (applied loads)
// ============================================================

void TrussCanvasWidget::drawForceArrows(QPainter& p) const {
    if (!m_view)
        return;

    for (const auto& nv : m_view->getNodeViews()) {
        if (std::abs(nv.fx) < 1e-10 && std::abs(nv.fy) < 1e-10)
            continue;
        drawForceArrow(p, toScreen(nv.x, nv.y), nv.fx, nv.fy, QColor(kLoadR, kLoadG, kLoadB));
    }
}

// ============================================================
// Step 6 — Reaction arrows
// ============================================================

void TrussCanvasWidget::drawReactions(QPainter& p) const {
    if (!m_view)
        return;
    // Reactions are only meaningful after analysis (non-Geometry mode)
    if (m_mode == DisplayMode::Geometry)
        return;

    for (const auto& nv : m_view->getNodeViews()) {
        if (std::abs(nv.rx) < 1e-10 && std::abs(nv.ry) < 1e-10)
            continue;
        // Sign convention: nv.rx / nv.ry are the reaction force components in the
        // global structural coordinate system (X+ right, Y+ up), exactly as the
        // FEM solver produces them via R = K·u at constrained DOFs.
        //
        // drawForceArrow() draws the arrow WITH ITS HEAD AT the node position,
        // pointing from the tail (external) toward the node — i.e. it visualises
        // the force vector acting ON the node.  Since a positive vertical reaction
        // (+ry) is an upward force on the structure, passing nv.rx / nv.ry without
        // sign modification correctly renders an upward green arrow for an upward
        // reaction.  No sign inversion is needed or correct here.
        drawForceArrow(
            p, toScreen(nv.x, nv.y), nv.rx, nv.ry, QColor(kReactionR, kReactionG, kReactionB));
    }
}

// ============================================================
// Step 7 — Overlay text
// ============================================================

void TrussCanvasWidget::drawOverlayText(QPainter& p) const {
    if (!m_view) {
        // Empty-state hint
        p.setPen(QPen(QColor(0x9A, 0xA0, 0xA6)));
        p.setFont(QFont(QStringLiteral("Arial"), 12));
        p.drawText(rect(),
                   Qt::AlignCenter,
                   QStringLiteral("No model loaded\n"
                                  "Use File → Open or build a new model"));
        return;
    }

    // Statistics bar — top-left
    const QString stats = QStringLiteral("Nodes: %1   Members: %2")
                              .arg(static_cast<int>(m_view->getNodeCount()))
                              .arg(static_cast<int>(m_view->getMemberCount()));
    p.setPen(QPen(QColor(0x9A, 0xA0, 0xA6)));
    {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setPointSize(9);
        p.setFont(f);
    }
    p.drawText(QRect(8, 8, width() - 16, 20), Qt::AlignLeft | Qt::AlignTop, stats);

    // Mode label — top-right
    QString modeLabel;
    switch (m_mode) {
        case DisplayMode::StressRatio:
            modeLabel = QStringLiteral("[ Stress Ratio ]");
            break;
        case DisplayMode::DeformedShape:
            modeLabel = QStringLiteral("[ Deformed Shape ]");
            break;
        default:
            break;
    }
    if (!modeLabel.isEmpty()) {
        p.setPen(QPen(QColor(kSupportR, kSupportG, kSupportB)));
        p.drawText(QRect(8, 8, width() - 16, 20), Qt::AlignRight | Qt::AlignTop, modeLabel);
    }
}

// ============================================================
// Helpers
// ============================================================

QPointF TrussCanvasWidget::toScreen(double wx, double wy) const {
    return m_worldToScreen.map(QPointF(wx, wy));
}

double TrussCanvasWidget::autoDispScale() const {
    // Computes an amplification factor so that the maximum nodal displacement
    // is rendered as kDispVisualFraction (10 %) of the current visible world span,
    // then clamps the result to [kMinDispScale, kMaxDispScale].
    //
    // Clamping rationale (see constant definitions above):
    //   Upper cap (500×): A 4 m truss with δ_max = 0.01 mm would otherwise yield
    //     scale = 0.10 × 4 / 0.00001 = 40 000×, rendering sub-micron elastic
    //     deformations as metre-scale displacements — physically misleading.
    //   Lower floor (1×): When δ_max > 10 % of the model span the structure
    //     undergoes large displacements; showing them at 1:1 is accurate and
    //     avoids artefactually shrinking the deformed geometry.
    //
    // Returns 0.0 when no displacement data are available (pre-analysis or
    // all-zero DOF results).
    if (!m_view)
        return 0.0;
    double maxDisp = 0.0;
    for (const auto& nv : m_view->getNodeViews())
        maxDisp = std::max(maxDisp, std::hypot(nv.dx, nv.dy));
    if (maxDisp < 1e-12)
        return 0.0;
    const double span = std::max(m_worldBounds.width(), m_worldBounds.height());
    const double ideal = kDispVisualFraction * span / maxDisp;
    return std::clamp(ideal, kMinDispScale, kMaxDispScale);
}

void TrussCanvasWidget::rebuildTransform() {
    const int w = (width() > 0) ? width() : 400;
    const int h = (height() > 0) ? height() : 300;

    const double ww = m_worldBounds.width();
    const double wh = m_worldBounds.height();

    if (ww <= 0.0 || wh <= 0.0) {
        m_worldToScreen = QTransform();
        return;
    }

    const double usableW = w * (1.0 - 2.0 * kMarginFraction);
    const double usableH = h * (1.0 - 2.0 * kMarginFraction);

    // Uniform fit-to-view scale  (pixels per metre)
    const double scale = std::min(usableW / ww, usableH / wh);

    // Centre the geometry in the available usable area
    const double centreOffsetX = (usableW - ww * scale) / 2.0;
    const double centreOffsetY = (usableH - wh * scale) / 2.0;

    const double leftMarginPx = w * kMarginFraction + centreOffsetX;
    const double topMarginPx = h * kMarginFraction + centreOffsetY;

    // Build: screen_x =  scale * wx + dx
    //        screen_y = -scale * wy + dy
    // where structural bottom (worldBounds.top()) maps to screen bottom,
    // and structural top (worldBounds.bottom()) maps to screen top.
    const double dx = leftMarginPx - m_worldBounds.left() * scale;
    const double dy = topMarginPx + m_worldBounds.bottom() * scale;

    //  QTransform(m11, m12, m21, m22, dx, dy)
    m_worldToScreen = QTransform(scale, 0.0, 0.0, -scale, dx, dy);
}

QColor TrussCanvasWidget::memberColour(const core::interfaces::MemberView& mv) const {
    if (m_mode == DisplayMode::StressRatio)
        return lerpStressColour(mv.utilizationRatio);

    // Geometry / DeformedShape: colour by mechanical state.
    // Architecture colour tokens (GUI-MODERNIZATION-ARCHITECTURE.md §5.4.1):
    //   Yield       dark:#FF1744  light:#B71C1C
    //   Tension     dark:#4FC3F7  light:#0288D1
    //   Compression dark:#FF7043  light:#D84315
    //   Pre-analysis / neutral: steel-blue

    if (mv.yielded)
        return m_isDark ? QColor(0xFF, 0x17, 0x44)   // #FF1744 — yielded (dark)
                        : QColor(0xB7, 0x1C, 0x1C);  // #B71C1C — yielded (light)

    if (mv.inTension)
        return m_isDark ? QColor(0x4F, 0xC3, 0xF7)   // #4FC3F7 — tension cyan (dark)
                        : QColor(0x02, 0x88, 0xD1);  // #0288D1 — tension blue (light)

    // Compression: axialForce < 0 distinguishes post-analysis from pre-analysis zero-force
    if (mv.axialForce < -1e-10)
        return m_isDark ? QColor(0xFF, 0x70, 0x43)   // #FF7043 — compression orange-red (dark)
                        : QColor(0xD8, 0x43, 0x15);  // #D84315 — compression deep-orange (light)

    // Pre-analysis or zero-force member: neutral steel-blue
    return m_isDark ? QColor(kMemberR, kMemberG, kMemberB)  // #8AB4F8 (dark)
                    : QColor(0x54, 0x7A, 0xC4);             // #547AC4 subdued blue (light)
}

void TrussCanvasWidget::drawSupportSymbol(QPainter& p,
                                          const QPointF& screenPos,
                                          core::SupportType support) const {
    const double sz = kSupportSize;

    // Downward-pointing triangle: apex at node, base below
    QPainterPath tri;
    tri.moveTo(screenPos);
    tri.lineTo(screenPos + QPointF(-sz * 0.65, sz));
    tri.lineTo(screenPos + QPointF(sz * 0.65, sz));
    tri.closeSubpath();

    p.setBrush(QColor(kSupportR, kSupportG, kSupportB, 140));
    p.setPen(QPen(QColor(kSupportR, kSupportG, kSupportB), 1.5));
    p.drawPath(tri);

    // Hatching below triangle (global ground)
    const double baseY = screenPos.y() + sz + 2.0;
    p.setPen(QPen(QColor(kSupportR, kSupportG, kSupportB), 1.5));

    if (support == core::SupportType::Pinned) {
        // Horizontal line = fixed in both X and Y
        p.drawLine(QPointF(screenPos.x() - sz, baseY), QPointF(screenPos.x() + sz, baseY));
    } else if (support == core::SupportType::RollerX) {
        // Small circles on a horizontal line = free to roll in X (fixed Y)
        p.drawLine(QPointF(screenPos.x() - sz, baseY), QPointF(screenPos.x() + sz, baseY));
        for (double dx = -sz * 0.6; dx <= sz * 0.6 + 0.1; dx += sz * 0.4) {
            p.drawEllipse(QPointF(screenPos.x() + dx, baseY + 4.0), 3.0, 3.0);
        }
    } else if (support == core::SupportType::RollerY) {
        // Vertical line on the right = free to roll in Y (fixed X)
        const double rightX = screenPos.x() + sz * 0.65 + 3.0;
        p.drawLine(QPointF(rightX, screenPos.y()), QPointF(rightX, screenPos.y() + sz));
    }
}

void TrussCanvasWidget::drawForceArrow(
    QPainter& p, const QPointF& headPos, double fx, double fy, const QColor& colour) const {
    const double magnitude = std::sqrt(fx * fx + fy * fy);
    if (magnitude < 1e-10)
        return;

    // Unit direction in world space; flip fy for screen-space Y (Y+ downward)
    const double nx = fx / magnitude;
    const double ny = -fy / magnitude;  // Y-axis flip

    // Arrow tail → head; head is at the node
    const QPointF tail = headPos - QPointF(nx, ny) * kArrowLength;
    const QPointF head = headPos;

    p.setPen(QPen(colour, 2.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(tail, head);

    // Arrowhead triangle
    constexpr double aw = 6.0;    // half-width of arrowhead base
    constexpr double al = 11.0;   // length of arrowhead
    const QPointF perp(ny, -nx);  // unit perpendicular in screen space

    QPainterPath arrowHead;
    arrowHead.moveTo(head);
    arrowHead.lineTo(head - QPointF(nx, ny) * al + perp * aw);
    arrowHead.lineTo(head - QPointF(nx, ny) * al - perp * aw);
    arrowHead.closeSubpath();
    p.fillPath(arrowHead, colour);

    // Force magnitude label
    p.setPen(QPen(colour));
    {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setPointSize(8);
        p.setFont(f);
    }
    const QString label = QStringLiteral("%1 kN").arg(magnitude / 1000.0, 0, 'f', 1);
    p.drawText(tail + QPointF(4.0, -4.0), label);
}

// ============================================================
// Interaction layer
// ============================================================

// -------------------------------------------------------------------
// Public slot: setMode
// -------------------------------------------------------------------

void TrussCanvasWidget::setMode(ToolMode mode) {
    if (m_toolMode == mode)
        return;
    m_toolMode = mode;
    m_pendingMemberStart.reset();  // cancel any in-progress member draw
    updateCursorForMode();
    update();  // repaint pending-member highlight
}

// -------------------------------------------------------------------
// Public method: screenToWorld
// -------------------------------------------------------------------

core::Point2D TrussCanvasWidget::screenToWorld(QPoint screenPos) const {
    bool ok = false;
    const QTransform inv = m_worldToScreen.inverted(&ok);
    if (!ok)
        return {0.0, 0.0};
    const QPointF w = inv.map(QPointF(screenPos));
    return {w.x(), w.y()};
}

// -------------------------------------------------------------------
// Mouse / key event handlers
// -------------------------------------------------------------------

void TrussCanvasWidget::mousePressEvent(QMouseEvent* event) {
    // ---- Middle button: start pan ----
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    // ---- Left button: tool-dependent action ----
    const QPoint pos = event->pos();

    switch (m_toolMode) {
        case ToolMode::Select: {
            if (auto nodeId = findNodeAt(pos)) {
                m_selectedNodeId = *nodeId;
                m_selectedMemberId = 0;
                emit nodeSelectionChanged(*nodeId);
            } else if (auto memberId = findMemberAt(pos)) {
                m_selectedMemberId = *memberId;
                m_selectedNodeId = 0;
                emit memberSelectionChanged(*memberId);
            } else {
                m_selectedNodeId = 0;
                m_selectedMemberId = 0;
                emit selectionCleared();
            }
            // Explicitly reclaim keyboard focus so the Delete key shortcut
            // (handled in keyPressEvent) remains available immediately after
            // selection, even if a Qt signal handler shifted focus elsewhere.
            setFocus(Qt::MouseFocusReason);
            update();
            break;
        }

        case ToolMode::AddNode: {
            // Only drop on empty space (prevent overlapping nodes)
            if (!findNodeAt(pos)) {
                core::Point2D worldPos = screenToWorld(pos);
                // Snap to nearest grid point (0.25 m default) for engineer-friendly
                // coordinates and clean structural geometry.
                worldPos.x = snapToGrid(worldPos.x, kGridSnapStep);
                worldPos.y = snapToGrid(worldPos.y, kGridSnapStep);
                emit nodeDropRequested(worldPos, core::SupportType::Free);
            }
            break;
        }

        case ToolMode::AddMember: {
            const auto nodeId = findNodeAt(pos);
            if (!nodeId)
                break;  // must click on a node

            if (!m_pendingMemberStart) {
                // First click — record start node
                m_pendingMemberStart = *nodeId;
                update();  // highlight pending start
            } else if (*m_pendingMemberStart != *nodeId) {
                // Second click on a different node — emit
                emit memberDrawRequested(*m_pendingMemberStart, *nodeId);
                m_pendingMemberStart.reset();
                update();
            }
            // If same node clicked again, cancel
            else {
                m_pendingMemberStart.reset();
                update();
            }
            break;
        }

        case ToolMode::Delete: {
            if (auto nodeId = findNodeAt(pos)) {
                emit nodeDeleteRequested(*nodeId);
            } else if (auto memberId = findMemberAt(pos)) {
                emit memberDeleteRequested(*memberId);
            }
            break;
        }

    }  // switch

    event->accept();
}

void TrussCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    // ---- Pan ----
    if (m_isPanning && (event->buttons() & Qt::MiddleButton)) {
        const QPoint delta = event->pos() - m_lastPanPos;
        m_lastPanPos = event->pos();

        if (width() > 0 && height() > 0) {
            const double scaleX = m_worldBounds.width() / width();
            const double scaleY = m_worldBounds.height() / height();
            // drag right → viewport shifts left (shows more right content)
            m_worldBounds.translate(-delta.x() * scaleX, -delta.y() * scaleY);
            rebuildTransform();
            update();
        }
        event->accept();
        return;
    }

    // ---- Emit cursor world coordinate ----
    emit cursorPositionChanged(screenToWorld(event->pos()));

    QWidget::mouseMoveEvent(event);
}

void TrussCanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton && m_isPanning) {
        m_isPanning = false;
        updateCursorForMode();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void TrussCanvasWidget::wheelEvent(QWheelEvent* event) {
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QWidget::wheelEvent(event);
        return;
    }

    const double zoomFactor = (delta > 0) ? (1.0 / kZoomStep) : kZoomStep;

    // World position under cursor (stays fixed during zoom)
    const core::Point2D worldCursor = screenToWorld(event->position().toPoint());

    const double newW = m_worldBounds.width() * zoomFactor;
    const double newH = m_worldBounds.height() * zoomFactor;

    // Clamp to sensible span limits
    if (newW < kMinWorldSpan || newW > kMaxWorldSpan || newH < kMinWorldSpan ||
        newH > kMaxWorldSpan) {
        event->accept();
        return;
    }

    // Keep cursor at the same world position
    const double newLeft = worldCursor.x - (worldCursor.x - m_worldBounds.left()) * zoomFactor;
    const double newTop = worldCursor.y - (worldCursor.y - m_worldBounds.top()) * zoomFactor;

    m_worldBounds = QRectF(newLeft, newTop, newW, newH);
    rebuildTransform();
    update();
    event->accept();
}

void TrussCanvasWidget::triggerDeleteSelected() {
    if (m_selectedNodeId != 0) {
        emit nodeDeleteRequested(m_selectedNodeId);
        m_selectedNodeId = 0;
        update();
    } else if (m_selectedMemberId != 0) {
        emit memberDeleteRequested(m_selectedMemberId);
        m_selectedMemberId = 0;
        update();
    }
}

void TrussCanvasWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        triggerDeleteSelected();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

// -------------------------------------------------------------------
// Interaction helpers
// -------------------------------------------------------------------

std::optional<core::NodeId> TrussCanvasWidget::findNodeAt(QPoint p) const {
    if (!m_view)
        return std::nullopt;
    const QPointF qp(p);
    for (const auto& n : m_view->getNodeViews()) {
        const QPointF sp = toScreen(n.x, n.y);
        const QPointF d = qp - sp;
        if (std::hypot(d.x(), d.y()) <= kHitRadius) {
            return n.id;
        }
    }
    return std::nullopt;
}

std::optional<core::MemberId> TrussCanvasWidget::findMemberAt(QPoint p) const {
    if (!m_view)
        return std::nullopt;

    const auto nodes = m_view->getNodeViews();
    const auto members = m_view->getMemberViews();

    std::unordered_map<core::NodeId, QPointF> screenMap;
    screenMap.reserve(nodes.size());
    for (const auto& n : nodes) {
        screenMap[n.id] = toScreen(n.x, n.y);
    }

    const QPointF qp(p);

    for (const auto& m : members) {
        const auto it1 = screenMap.find(m.startNodeId);
        const auto it2 = screenMap.find(m.endNodeId);
        if (it1 == screenMap.end() || it2 == screenMap.end())
            continue;

        const QPointF a = it1->second;
        const QPointF b = it2->second;
        const QPointF ab = b - a;
        const QPointF ap = qp - a;

        const double lenSq = ab.x() * ab.x() + ab.y() * ab.y();
        if (lenSq < 1e-10)
            continue;

        const double t = std::clamp((ap.x() * ab.x() + ap.y() * ab.y()) / lenSq, 0.0, 1.0);
        const QPointF proj = a + t * ab;
        const QPointF diff = qp - proj;

        if (std::hypot(diff.x(), diff.y()) <= kMemberHitTol) {
            return m.id;
        }
    }
    return std::nullopt;
}

void TrussCanvasWidget::updateCursorForMode() {
    switch (m_toolMode) {
        case ToolMode::Select:
            setCursor(Qt::ArrowCursor);
            break;
        case ToolMode::AddNode:
            setCursor(Qt::CrossCursor);
            break;
        case ToolMode::AddMember:
            setCursor(Qt::CrossCursor);
            break;
        case ToolMode::Delete:
            setCursor(Qt::ForbiddenCursor);
            break;
    }
}

}  // namespace truss::gui
