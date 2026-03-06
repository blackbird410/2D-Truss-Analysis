/**
 * @file truss_canvas_widget.cpp
 * @brief 7-step rendering pipeline for TrussCanvasWidget (Phase 4).
 *
 * Phase 4: Full QPainter rendering pipeline (background, grid, members,
 *          nodes, force arrows, reactions, overlay text).  Y-axis flip via
 *          QTransform.  No interaction handling (Phase 6).
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

#include <QColor>
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
constexpr int kBgR = 0x14, kBgG = 0x16, kBgB = 0x1C;          // #14161C
constexpr int kGridR = 0x2A, kGridG = 0x2E, kGridB = 0x3A;     // #2A2E3A
constexpr int kMemberR = 0x8A, kMemberG = 0xB4, kMemberB = 0xF8; // #8AB4F8 steel-blue
constexpr int kNodeR = 0xE8, kNodeG = 0xEA, kNodeB = 0xED;     // #E8EAED light
constexpr int kSupportR = 0x5B, kSupportG = 0x9B, kSupportB = 0xD5; // #5B9BD5 accent
constexpr int kLoadR = 0xFF, kLoadG = 0xC1, kLoadB = 0x07;     // #FFC107 amber
constexpr int kReactionR = 0x34, kReactionG = 0xA8, kReactionB = 0x53; // #34A853 green

// Stress-ratio colour stops: green(0) → amber(0.5) → red(1)
constexpr int kGreenR = 0x34, kGreenG = 0xA8, kGreenB = 0x53; // #34A853
constexpr int kAmberR = 0xFF, kAmberG = 0xC1, kAmberB = 0x07; // #FFC107
constexpr int kRedR   = 0xEA, kRedG   = 0x43, kRedB   = 0x35; // #EA4335

/// Grid snapping step for node placement (metres).
/// Matches typical structural grid spacing; keeps coordinates engineer-friendly.
constexpr double kGridSnapStep = 0.25;

/// 3-stop colour interpolation: green(0) → amber(0.5) → red(1).
QColor lerpStressColour(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    auto lerp = [](int a, int b, double f) -> int {
        return static_cast<int>(std::round(a + (b - a) * f));
    };
    if (t <= 0.5) {
        const double f = t * 2.0;
        return QColor(lerp(kGreenR, kAmberR, f),
                      lerp(kGreenG, kAmberG, f),
                      lerp(kGreenB, kAmberB, f));
    } else {
        const double f = (t - 0.5) * 2.0;
        return QColor(lerp(kAmberR, kRedR, f),
                      lerp(kAmberG, kRedG, f),
                      lerp(kAmberB, kRedB, f));
    }
}

/// Snap @p v to the nearest multiple of @p step.
inline double snapToGrid(double v, double step)
{
    return std::round(v / step) * step;
}

}  // anonymous namespace

// ============================================================
// Construction
// ============================================================

TrussCanvasWidget::TrussCanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("trussCanvas"));
    setMinimumSize(400, 300);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);          // receive mouseMoveEvent without button held
    setFocusPolicy(Qt::StrongFocus); // receive keyPressEvent
    rebuildTransform();
}

// ============================================================
// Public slots
// ============================================================

void TrussCanvasWidget::refresh(const core::interfaces::ITrussView* view, DisplayMode mode)
{
    m_view = view;
    m_mode = mode;
    update();
}

void TrussCanvasWidget::setViewport(const QRectF& worldBounds)
{
    if (!worldBounds.isEmpty()) {
        m_worldBounds = worldBounds;
        rebuildTransform();
    }
    update();
}

void TrussCanvasWidget::clearCanvas()
{
    m_view = nullptr;
    m_mode = DisplayMode::Geometry;
    m_pendingMemberStart.reset();
    m_selectedNodeId   = 0;
    m_selectedMemberId = 0;
    update();
}

// ============================================================
// Event overrides
// ============================================================

void TrussCanvasWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    rebuildTransform();
}

void TrussCanvasWidget::paintEvent(QPaintEvent* /*event*/)
{
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

void TrussCanvasWidget::drawBackground(QPainter& p) const
{
    p.fillRect(rect(), QColor(kBgR, kBgG, kBgB));
}

// ============================================================
// Step 2 — Grid
// ============================================================

void TrussCanvasWidget::drawGrid(QPainter& p) const
{
    p.setPen(QPen(QColor(kGridR, kGridG, kGridB), 0.5, Qt::SolidLine));

    const double span = std::max(m_worldBounds.width(), m_worldBounds.height());
    if (span <= 0.0) return;

    // Choose a nicely rounded grid step (~span/10, snapped to 1/2/5 × 10^n)
    const double rawStep = span / 10.0;
    const double mag     = std::pow(10.0, std::floor(std::log10(rawStep)));
    double step = mag;
    if      (rawStep / mag >= 5.0) step = mag * 5.0;
    else if (rawStep / mag >= 2.0) step = mag * 2.0;

    // World extents (top() = structural yMin, bottom() = structural yMax)
    const double xMin = m_worldBounds.left();
    const double xMax = m_worldBounds.right();
    const double yMin = m_worldBounds.top();    // structural minimum y
    const double yMax = m_worldBounds.bottom(); // structural maximum y

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

void TrussCanvasWidget::drawMembers(QPainter& p) const
{
    if (!m_view) return;

    const auto nodes   = m_view->getNodeViews();
    const auto members = m_view->getMemberViews();

    // Build a fast id→NodeView lookup
    std::unordered_map<core::NodeId, const core::interfaces::NodeView*> nodeMap;
    nodeMap.reserve(nodes.size());
    for (const auto& n : nodes) {
        nodeMap[n.id] = &n;
    }

    for (const auto& mv : members) {
        const auto itA = nodeMap.find(mv.startNodeId);
        const auto itB = nodeMap.find(mv.endNodeId);
        if (itA == nodeMap.end() || itB == nodeMap.end()) continue;

        const QPointF sA = toScreen(itA->second->x, itA->second->y);
        const QPointF sB = toScreen(itB->second->x, itB->second->y);
        const QColor  col = memberColour(mv);

        p.setPen(QPen(col, kMemberWidth, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(sA, sB);

        // Member ID label at midpoint
        const QPointF mid = (sA + sB) / 2.0;
        p.setPen(QPen(col.lighter(160)));
        { QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont); f.setPointSize(8); p.setFont(f); }
        p.drawText(mid + QPointF(4.0, -3.0), QString::number(mv.id));
    }
}

// ============================================================
// Step 4 — Nodes
// ============================================================

void TrussCanvasWidget::drawNodes(QPainter& p) const
{
    if (!m_view) return;

    for (const auto& nv : m_view->getNodeViews()) {
        const QPointF sc = toScreen(nv.x, nv.y);

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
            fill = QColor(kNodeR, kNodeG, kNodeB);
        }

        p.setPen(QPen(fill.lighter(130), 1.5));
        p.setBrush(fill);
        p.drawEllipse(sc, kNodeRadius, kNodeRadius);

        // Node ID label inside the circle
        p.setPen(QPen(QColor(kBgR, kBgG, kBgB)));
        { QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont); f.setPointSize(7); f.setWeight(QFont::Bold); p.setFont(f); }
        const QRectF labelRect(sc.x() - kNodeRadius, sc.y() - kNodeRadius,
                               kNodeRadius * 2.0, kNodeRadius * 2.0);
        p.drawText(labelRect, Qt::AlignCenter, QString::number(nv.id));
    }
}

// ============================================================
// Step 5 — Force arrows (applied loads)
// ============================================================

void TrussCanvasWidget::drawForceArrows(QPainter& p) const
{
    if (!m_view) return;

    for (const auto& nv : m_view->getNodeViews()) {
        if (std::abs(nv.fx) < 1e-10 && std::abs(nv.fy) < 1e-10) continue;
        drawForceArrow(p, toScreen(nv.x, nv.y), nv.fx, nv.fy,
                       QColor(kLoadR, kLoadG, kLoadB));
    }
}

// ============================================================
// Step 6 — Reaction arrows
// ============================================================

void TrussCanvasWidget::drawReactions(QPainter& p) const
{
    if (!m_view) return;
    // Reactions are only meaningful after analysis (non-Geometry mode)
    if (m_mode == DisplayMode::Geometry) return;

    for (const auto& nv : m_view->getNodeViews()) {
        if (std::abs(nv.rx) < 1e-10 && std::abs(nv.ry) < 1e-10) continue;
        // Reactions are drawn in the opposite direction (pointing away from node)
        drawForceArrow(p, toScreen(nv.x, nv.y), -nv.rx, -nv.ry,
                       QColor(kReactionR, kReactionG, kReactionB));
    }
}

// ============================================================
// Step 7 — Overlay text
// ============================================================

void TrussCanvasWidget::drawOverlayText(QPainter& p) const
{
    if (!m_view) {
        // Empty-state hint
        p.setPen(QPen(QColor(0x9A, 0xA0, 0xA6)));
        p.setFont(QFont(QStringLiteral("Arial"), 12));
        p.drawText(rect(), Qt::AlignCenter,
                   QStringLiteral("No model loaded\n"
                                  "Use File → Open or build a new model"));
        return;
    }

    // Statistics bar — top-left
    const QString stats =
        QStringLiteral("Nodes: %1   Members: %2")
            .arg(static_cast<int>(m_view->getNodeCount()))
            .arg(static_cast<int>(m_view->getMemberCount()));
    p.setPen(QPen(QColor(0x9A, 0xA0, 0xA6)));
    { QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont); f.setPointSize(9); p.setFont(f); }
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
        p.drawText(QRect(8, 8, width() - 16, 20),
                   Qt::AlignRight | Qt::AlignTop, modeLabel);
    }
}

// ============================================================
// Helpers
// ============================================================

QPointF TrussCanvasWidget::toScreen(double wx, double wy) const
{
    return m_worldToScreen.map(QPointF(wx, wy));
}

void TrussCanvasWidget::rebuildTransform()
{
    const int w = (width()  > 0) ? width()  : 400;
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
    const double topMarginPx  = h * kMarginFraction + centreOffsetY;

    // Build: screen_x =  scale * wx + dx
    //        screen_y = -scale * wy + dy
    // where structural bottom (worldBounds.top()) maps to screen bottom,
    // and structural top (worldBounds.bottom()) maps to screen top.
    const double dx = leftMarginPx - m_worldBounds.left()   * scale;
    const double dy = topMarginPx  + m_worldBounds.bottom() * scale;

    //  QTransform(m11, m12, m21, m22, dx, dy)
    m_worldToScreen = QTransform(scale, 0.0,
                                 0.0, -scale,
                                 dx, dy);
}

QColor TrussCanvasWidget::memberColour(const core::interfaces::MemberView& mv) const
{
    if (m_mode == DisplayMode::StressRatio) {
        return lerpStressColour(mv.utilizationRatio);
    }
    // Geometry / DeformedShape: colour by mechanical state
    if (mv.yielded)   return QColor(0xFF, 0x17, 0x44);              // red — yielded
    if (mv.inTension) return QColor(0x4F, 0xC3, 0xF7);              // light blue — tension
    return QColor(kMemberR, kMemberG, kMemberB);                    // steel blue — compression
}

void TrussCanvasWidget::drawSupportSymbol(QPainter& p,
                                          const QPointF& screenPos,
                                          core::SupportType support) const
{
    const double sz = kSupportSize;

    // Downward-pointing triangle: apex at node, base below
    QPainterPath tri;
    tri.moveTo(screenPos);
    tri.lineTo(screenPos + QPointF(-sz * 0.65, sz));
    tri.lineTo(screenPos + QPointF( sz * 0.65, sz));
    tri.closeSubpath();

    p.setBrush(QColor(kSupportR, kSupportG, kSupportB, 140));
    p.setPen(QPen(QColor(kSupportR, kSupportG, kSupportB), 1.5));
    p.drawPath(tri);

    // Hatching below triangle (global ground)
    const double baseY = screenPos.y() + sz + 2.0;
    p.setPen(QPen(QColor(kSupportR, kSupportG, kSupportB), 1.5));

    if (support == core::SupportType::Pinned) {
        // Horizontal line = fixed in both X and Y
        p.drawLine(QPointF(screenPos.x() - sz, baseY),
                   QPointF(screenPos.x() + sz, baseY));
    } else if (support == core::SupportType::RollerX) {
        // Small circles on a horizontal line = free to roll in X (fixed Y)
        p.drawLine(QPointF(screenPos.x() - sz, baseY),
                   QPointF(screenPos.x() + sz, baseY));
        for (double dx = -sz * 0.6; dx <= sz * 0.6 + 0.1; dx += sz * 0.4) {
            p.drawEllipse(QPointF(screenPos.x() + dx, baseY + 4.0), 3.0, 3.0);
        }
    } else if (support == core::SupportType::RollerY) {
        // Vertical line on the right = free to roll in Y (fixed X)
        const double rightX = screenPos.x() + sz * 0.65 + 3.0;
        p.drawLine(QPointF(rightX, screenPos.y()),
                   QPointF(rightX, screenPos.y() + sz));
    }
}

void TrussCanvasWidget::drawForceArrow(QPainter& p,
                                       const QPointF& headPos,
                                       double fx,
                                       double fy,
                                       const QColor& colour) const
{
    const double magnitude = std::sqrt(fx * fx + fy * fy);
    if (magnitude < 1e-10) return;

    // Unit direction in world space; flip fy for screen-space Y (Y+ downward)
    const double nx =  fx / magnitude;
    const double ny = -fy / magnitude;  // Y-axis flip

    // Arrow tail → head; head is at the node
    const QPointF tail = headPos - QPointF(nx, ny) * kArrowLength;
    const QPointF head = headPos;

    p.setPen(QPen(colour, 2.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(tail, head);

    // Arrowhead triangle
    constexpr double aw = 6.0;   // half-width of arrowhead base
    constexpr double al = 11.0;  // length of arrowhead
    const QPointF perp(ny, -nx); // unit perpendicular in screen space

    QPainterPath arrowHead;
    arrowHead.moveTo(head);
    arrowHead.lineTo(head - QPointF(nx, ny) * al + perp * aw);
    arrowHead.lineTo(head - QPointF(nx, ny) * al - perp * aw);
    arrowHead.closeSubpath();
    p.fillPath(arrowHead, colour);

    // Force magnitude label
    p.setPen(QPen(colour));
    { QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont); f.setPointSize(8); p.setFont(f); }
    const QString label =
        QStringLiteral("%1 kN").arg(magnitude / 1000.0, 0, 'f', 1);
    p.drawText(tail + QPointF(4.0, -4.0), label);
}

// ============================================================
// Phase 6 — Interaction layer
// ============================================================

// -------------------------------------------------------------------
// Public slot: setMode
// -------------------------------------------------------------------

void TrussCanvasWidget::setMode(ToolMode mode)
{
    if (m_toolMode == mode) return;
    m_toolMode = mode;
    m_pendingMemberStart.reset(); // cancel any in-progress member draw
    updateCursorForMode();
    update(); // repaint pending-member highlight
}

// -------------------------------------------------------------------
// Public method: screenToWorld
// -------------------------------------------------------------------

core::Point2D TrussCanvasWidget::screenToWorld(QPoint screenPos) const
{
    bool ok = false;
    const QTransform inv = m_worldToScreen.inverted(&ok);
    if (!ok) return {0.0, 0.0};
    const QPointF w = inv.map(QPointF(screenPos));
    return {w.x(), w.y()};
}

// -------------------------------------------------------------------
// Mouse / key event handlers
// -------------------------------------------------------------------

void TrussCanvasWidget::mousePressEvent(QMouseEvent* event)
{
    // ---- Middle button: start pan ----
    if (event->button() == Qt::MiddleButton) {
        m_isPanning   = true;
        m_lastPanPos  = event->pos();
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
            m_selectedNodeId   = *nodeId;
            m_selectedMemberId = 0;
            emit nodeSelectionChanged(*nodeId);
        } else if (auto memberId = findMemberAt(pos)) {
            m_selectedMemberId = *memberId;
            m_selectedNodeId   = 0;
            emit memberSelectionChanged(*memberId);
        } else {
            m_selectedNodeId   = 0;
            m_selectedMemberId = 0;
            emit selectionCleared();
        }
        update();
        break;
    }

    case ToolMode::AddNode: {
        // Only drop on empty space (prevent overlapping nodes)
        if (!findNodeAt(pos)) {
            const core::Point2D worldPos = screenToWorld(pos);
            emit nodeDropRequested(worldPos, core::SupportType::Free);
        }
        break;
    }

    case ToolMode::AddMember: {
        const auto nodeId = findNodeAt(pos);
        if (!nodeId) break;  // must click on a node

        if (!m_pendingMemberStart) {
            // First click — record start node
            m_pendingMemberStart = *nodeId;
            update(); // highlight pending start
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

void TrussCanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    // ---- Pan ----
    if (m_isPanning && (event->buttons() & Qt::MiddleButton)) {
        const QPoint delta = event->pos() - m_lastPanPos;
        m_lastPanPos = event->pos();

        if (width() > 0 && height() > 0) {
            const double scaleX = m_worldBounds.width()  / width();
            const double scaleY = m_worldBounds.height() / height();
            // drag right → viewport shifts left (shows more right content)
            m_worldBounds.translate(-delta.x() * scaleX,
                                    -delta.y() * scaleY);
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

void TrussCanvasWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton && m_isPanning) {
        m_isPanning = false;
        updateCursorForMode();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void TrussCanvasWidget::wheelEvent(QWheelEvent* event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0) { QWidget::wheelEvent(event); return; }

    const double zoomFactor = (delta > 0) ? (1.0 / kZoomStep) : kZoomStep;

    // World position under cursor (stays fixed during zoom)
    const core::Point2D worldCursor = screenToWorld(event->position().toPoint());

    const double newW = m_worldBounds.width()  * zoomFactor;
    const double newH = m_worldBounds.height() * zoomFactor;

    // Clamp to sensible span limits
    if (newW < kMinWorldSpan || newW > kMaxWorldSpan ||
        newH < kMinWorldSpan || newH > kMaxWorldSpan) {
        event->accept();
        return;
    }

    // Keep cursor at the same world position
    const double newLeft = worldCursor.x - (worldCursor.x - m_worldBounds.left())  * zoomFactor;
    const double newTop  = worldCursor.y - (worldCursor.y - m_worldBounds.top())   * zoomFactor;

    m_worldBounds = QRectF(newLeft, newTop, newW, newH);
    rebuildTransform();
    update();
    event->accept();
}

void TrussCanvasWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (m_selectedNodeId != 0) {
            emit nodeDeleteRequested(m_selectedNodeId);
            m_selectedNodeId = 0;
        } else if (m_selectedMemberId != 0) {
            emit memberDeleteRequested(m_selectedMemberId);
            m_selectedMemberId = 0;
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

// -------------------------------------------------------------------
// Interaction helpers
// -------------------------------------------------------------------

std::optional<core::NodeId> TrussCanvasWidget::findNodeAt(QPoint p) const
{
    if (!m_view) return std::nullopt;
    const QPointF qp(p);
    for (const auto& n : m_view->getNodeViews()) {
        const QPointF sp = toScreen(n.x, n.y);
        const QPointF d  = qp - sp;
        if (std::hypot(d.x(), d.y()) <= kHitRadius) {
            return n.id;
        }
    }
    return std::nullopt;
}

std::optional<core::MemberId> TrussCanvasWidget::findMemberAt(QPoint p) const
{
    if (!m_view) return std::nullopt;

    const auto nodes   = m_view->getNodeViews();
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
        if (it1 == screenMap.end() || it2 == screenMap.end()) continue;

        const QPointF a = it1->second;
        const QPointF b = it2->second;
        const QPointF ab = b - a;
        const QPointF ap = qp - a;

        const double lenSq = ab.x() * ab.x() + ab.y() * ab.y();
        if (lenSq < 1e-10) continue;

        const double t = std::clamp(
            (ap.x() * ab.x() + ap.y() * ab.y()) / lenSq, 0.0, 1.0);
        const QPointF proj = a + t * ab;
        const QPointF diff = qp - proj;

        if (std::hypot(diff.x(), diff.y()) <= kMemberHitTol) {
            return m.id;
        }
    }
    return std::nullopt;
}

void TrussCanvasWidget::updateCursorForMode()
{
    switch (m_toolMode) {
    case ToolMode::Select:    setCursor(Qt::ArrowCursor);     break;
    case ToolMode::AddNode:   setCursor(Qt::CrossCursor);     break;
    case ToolMode::AddMember: setCursor(Qt::CrossCursor);     break;
    case ToolMode::Delete:    setCursor(Qt::ForbiddenCursor); break;
    }
}

}  // namespace truss::gui
