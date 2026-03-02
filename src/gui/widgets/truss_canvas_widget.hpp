/**
 * @file truss_canvas_widget.hpp
 * @brief Interactive canvas widget for 2D truss geometry visualisation.
 *
 * Phase 1 stub — class declaration only.
 * Full rendering pipeline and interaction logic are implemented in Phase 4.
 * Full interaction (pan, zoom, node drop, member draw, select) in Phase 6.
 *
 * @note Q_OBJECT and paintEvent are added in Phase 4.
 *       Do NOT add Q_OBJECT to this stub — it would trigger unnecessary MOC runs.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QWidget>

// Forward declarations — full includes deferred until Phase 4 implementation
namespace truss::core::interfaces { class ITrussView; }

namespace truss::gui {

/**
 * @brief Renders a 2D truss model with optional result overlays.
 *
 * Responsibilities (Phase 4+):
 *  - Renders nodes, members, support symbols, force arrows
 *  - Supports three DisplayModes: Geometry, StressRatio, DeformedShape
 *  - Accepts mouse/keyboard events for model editing (Phase 6)
 *  - Emits signals for node/member creation and selection changes
 *
 * Coordinate system: world coordinates use structural convention
 * (Y+ upward). The worldToScreen QTransform applies scale(1, -1)
 * to convert to Qt's screen space (Y+ downward).
 *
 * @todo Phase 4: Add Q_OBJECT macro, implement paintEvent pipeline,
 *       add DisplayMode enum, add refresh(ITrussView*, DisplayMode) slot.
 * @todo Phase 6: Add mouse/keyboard interaction and all interaction signals.
 */
class TrussCanvasWidget : public QWidget {
public:
    explicit TrussCanvasWidget(QWidget* parent = nullptr) : QWidget(parent) {}

    // TODO Phase 4: DisplayMode enum { Geometry, StressRatio, DeformedShape }
    // TODO Phase 4: void refresh(const core::interfaces::ITrussView* view, DisplayMode mode)
    // TODO Phase 4: void setViewport(QRectF worldBounds)
    // TODO Phase 6: signals: nodeDropRequested, memberDrawRequested, selectionChanged,
    //                         deleteRequested, cursorPositionChanged
};

}  // namespace truss::gui
