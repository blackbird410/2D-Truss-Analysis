/**
 * @file truss_canvas_widget.hpp
 * @brief Interactive canvas widget for 2D truss geometry visualisation.
 *
 * Phase 4: Full rendering pipeline — Q_OBJECT, paintEvent (7-step pipeline),
 *          DisplayMode, refresh slot, viewport, Y-axis coordinate transform.
 * Phase 6: Full interaction layer — pan (middle-button drag), zoom (scroll wheel),
 *          node drop, member draw, select, delete.  Tool modes drive left-click behaviour.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"

#include <QPoint>
#include <QRectF>
#include <QTransform>
#include <QWidget>

#include <optional>
#include <unordered_map>

namespace truss::gui {

/**
 * @brief Interactive canvas widget for rendering and editing a 2D truss.
 *
 * TrussCanvasWidget owns a @c QTransform m_worldToScreen that maps structural
 * world coordinates (Y+ upward, metres) to Qt screen coordinates (Y+ downward,
 * pixels).  The mapping is rebuilt on resize and after pan/zoom operations.
 *
 * @par Coordinate system
 * @code
 *   world:  x → right,  y ↑ up
 *   screen: x → right,  y ↓ down
 *   transform:  screen_x =  scale * (wx - world_left) + left_margin_px
 *               screen_y = -scale * (wy - world_top_structural) + top_margin_px
 * @endcode
 *
 * @par Display modes (overlay rendering)
 * - @c Geometry      — plain geometry, support symbols, force arrows
 * - @c StressRatio   — members colour-mapped by utilisation ratio (green→amber→red)
 * - @c DeformedShape — exaggerated deformation overlay in ghost colour (post-analysis)
 *
 * @par Tool modes (left-click behaviour)
 * - @c Select       — click to select a node or member; click empty space to deselect
 * - @c AddNode      — click on canvas to drop a new node at that world coordinate
 * - @c AddMember    — first click picks start node; second picks end node; emits memberDrawRequested
 * - @c Delete       — click on a node or member to delete it
 */
class TrussCanvasWidget : public QWidget {
    Q_OBJECT

public:
    /// @brief Mutually exclusive display overlay modes.
    enum class DisplayMode {
        Geometry,      ///< Plain geometry (always available)
        StressRatio,   ///< Member colour by utilisation ratio (post-analysis)
        DeformedShape  ///< Deformed shape overlay (post-analysis)
    };

    /// @brief Active editing tool; governs left-click / left-drag behaviour.
    enum class ToolMode {
        Select,     ///< Click to select node or member; empty space clears selection
        AddNode,    ///< Click on canvas space to drop a new node
        AddMember,  ///< Click two nodes in sequence to draw a member between them
        Delete      ///< Click a node or member to request its deletion
    };

    explicit TrussCanvasWidget(QWidget* parent = nullptr);
    ~TrussCanvasWidget() override = default;

    /**
     * @brief Convert a screen pixel position to a structural world coordinate.
     *
     * Uses the inverse of @c m_worldToScreen.  Returns {0, 0} if the transform
     * is not invertible.
     *
     * @param screenPos  Mouse position in widget (pixel) coordinates.
     * @return           Corresponding world-space point (metres, Y+ upward).
     */
    [[nodiscard]] truss::core::Point2D screenToWorld(QPoint screenPos) const;

public slots:
    /**
     * @brief Update the data source and trigger a repaint.
     *
     * Does @em not take ownership of @p view; the pointer must remain valid
     * until the next call to refresh() or clearCanvas().
     *
     * @param view  Live view of the current truss.  May be nullptr → empty canvas.
     * @param mode  Which overlay to render.
     */
    void refresh(const core::interfaces::ITrussView* view,
                 DisplayMode mode = DisplayMode::Geometry);

    /**
     * @brief Set the visible extent of world-space shown in the canvas.
     *
     * Rebuilds the world→screen transform immediately.  Call this before
     * refresh() when loading a new model to frame the geometry correctly.
     *
     * @param worldBounds  Visible area in world coordinates (metres, Y+ upward),
     *                     expressed as a QRectF with positive width and height.
     *                     A zero-area rect is ignored.
     */
    void setViewport(const QRectF& worldBounds);

    /// @brief Detach any current view and repaint to show the empty canvas hint.
    void clearCanvas();

    /**
     * @brief Switch the active editing tool mode.
     * @param mode  The new tool mode.
     */
    void setMode(ToolMode mode);

signals:
    // -----------------------------------------------------------------------
    // Model-mutation requests (wired to CanvasController)
    // -----------------------------------------------------------------------

    /// @brief Emitted in AddNode mode when the user clicks on empty canvas space.
    void nodeDropRequested(truss::core::Point2D worldPos,
                           truss::core::SupportType defaultSupport);

    /// @brief Emitted in AddMember mode when the user clicks on two distinct nodes.
    void memberDrawRequested(truss::core::NodeId startId,
                             truss::core::NodeId endId);

    /// @brief Emitted in Delete mode when the user clicks on a node.
    void nodeDeleteRequested(truss::core::NodeId nodeId);

    /// @brief Emitted in Delete mode when the user clicks on a member.
    void memberDeleteRequested(truss::core::MemberId memberId);

    // -----------------------------------------------------------------------
    // Selection signals (wired to InspectorController)
    // -----------------------------------------------------------------------

    /// @brief Emitted in Select mode when the user clicks a node.
    void nodeSelectionChanged(truss::core::NodeId nodeId);

    /// @brief Emitted in Select mode when the user clicks a member.
    void memberSelectionChanged(truss::core::MemberId memberId);

    /// @brief Emitted in Select mode when the user clicks on empty canvas space.
    void selectionCleared();

    // -----------------------------------------------------------------------
    // Auxiliary signals
    // -----------------------------------------------------------------------

    /// @brief Emitted on every mouse move to show world coordinates in status bar.
    void cursorPositionChanged(truss::core::Point2D worldPos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    /// React to application-wide palette/style changes to keep colours consistent.
    void changeEvent(QEvent* event) override;

private:
    // -------------------------------------------------------------------
    // 7-step paint pipeline
    // -------------------------------------------------------------------
    void drawBackground(QPainter& p) const;
    void drawGrid(QPainter& p) const;
    void drawMembers(QPainter& p) const;
    void drawNodes(QPainter& p) const;
    void drawForceArrows(QPainter& p) const;
    void drawReactions(QPainter& p) const;
    void drawOverlayText(QPainter& p) const;

    // -------------------------------------------------------------------
    // Rendering helpers
    // -------------------------------------------------------------------

    /// @brief Convert world (metres, Y+ up) to screen (pixels, Y+ down).
    [[nodiscard]] QPointF toScreen(double wx, double wy) const;

    /// @brief Rebuild @c m_worldToScreen from @c m_worldBounds and widget size.
    void rebuildTransform();

    /// @brief Return the draw colour for a member given the current display mode.
    [[nodiscard]] QColor memberColour(const core::interfaces::MemberView& mv) const;

    /// @brief Draw a structural support symbol at a screen position.
    void drawSupportSymbol(QPainter& p,
                           const QPointF& screenPos,
                           core::SupportType support) const;

    /// @brief Draw a force arrow with arrowhead.  headPos is the screen node position.
    void drawForceArrow(QPainter& p,
                        const QPointF& headPos,
                        double fx,
                        double fy,
                        const QColor& colour) const;

    // -------------------------------------------------------------------
    // Interaction helpers
    // -------------------------------------------------------------------

    /**
     * @brief Find a node within hit-testing radius at screen position @p p.
     * @return Node id, or std::nullopt if nothing is close enough.
     */
    [[nodiscard]] std::optional<core::NodeId> findNodeAt(QPoint p) const;

    /**
     * @brief Find a member whose line passes within tolerance of screen position @p p.
     * @return Member id, or std::nullopt if nothing is close enough.
     */
    [[nodiscard]] std::optional<core::MemberId> findMemberAt(QPoint p) const;

    /// @brief Update the cursor shape to reflect the current tool mode.
    void updateCursorForMode();

    // -------------------------------------------------------------------
    // Rendering state
    // -------------------------------------------------------------------

    /// Non-owning pointer to the current truss view; may be nullptr.
    const core::interfaces::ITrussView* m_view{nullptr};

    DisplayMode m_mode{DisplayMode::Geometry};

    /// Maps world coordinates → screen pixels.
    /// Built by rebuildTransform(); m12 = 0, m22 = -scale for Y-axis flip.
    QTransform m_worldToScreen;

    /// Visible world-space rectangle (metres, Y+ upward; stored as QRectF
    /// so top() = structural yMin, bottom() = structural yMax).
    /// Default: [−1, −1, 7, 7] m  (6 m × 6 m with 1 m padding each side).
    QRectF m_worldBounds{-1.0, -1.0, 7.0, 7.0};

    /// True when the application palette is a dark theme; drives colour selection.
    bool m_isDark{true};

    // -------------------------------------------------------------------
    // Interaction state
    // -------------------------------------------------------------------

    ToolMode m_toolMode{ToolMode::Select};

    /// True while the middle mouse button is held for panning.
    bool   m_isPanning{false};
    QPoint m_lastPanPos;

    /// AddMember mode: set after the user clicks the first node.
    std::optional<core::NodeId> m_pendingMemberStart;

    /// Currently selected node (0 = none).
    core::NodeId   m_selectedNodeId{0};
    /// Currently selected member (0 = none).
    core::MemberId m_selectedMemberId{0};

    // -------------------------------------------------------------------
    // Visual constants
    // -------------------------------------------------------------------
    static constexpr double kNodeRadius       = 6.0;   ///< Node circle radius (px)
    static constexpr double kMemberWidth      = 2.0;   ///< Member stroke width (px)
    static constexpr double kSupportSize      = 14.0;  ///< Support symbol half-size (px)
    static constexpr double kArrowLength      = 40.0;  ///< Force arrow length (px)
    static constexpr double kMarginFraction   = 0.12;  ///< Canvas margin (fraction of dimension)
    static constexpr double kHitRadius        = 10.0;  ///< Node hit-test radius (px)
    static constexpr double kMemberHitTol     = 6.0;   ///< Member hit-test tolerance (px)
    static constexpr double kZoomStep         = 1.15;  ///< Zoom factor per scroll step
    static constexpr double kMinWorldSpan     = 0.1;   ///< Minimum world-space span (m)
    static constexpr double kMaxWorldSpan     = 1000.0;///< Maximum world-space span (m)
};

}  // namespace truss::gui
