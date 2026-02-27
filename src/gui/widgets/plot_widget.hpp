/**
 * @file plot_widget.hpp
 * @brief Widget for plotting truss geometry.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/truss_application_service.hpp"
#include "core/model/types.hpp"

#include <QtGui/QPainter>
#include <QtWidgets/QWidget>

namespace truss::gui {

/**
 * @brief Widget for visualizing truss structure
 *
 * This widget follows Clean Architecture principles:
 * - Uses dependency injection for services
 * - Does not own or directly access Domain entities
 * - Renders via ITrussView interface (read-only)
 * - No mutations, only visualization
 */
class PlotWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct PlotWidget with dependency injection
     *
     * @param trussService Application service for truss operations
     * @param parent Parent widget
     */
    explicit PlotWidget(truss::application::TrussApplicationService& trussService,
                        QWidget* parent = nullptr);

public slots:
    /**
     * @brief Update plot from current truss
     *
     * @param trussHandle Handle to truss to render
     */
    void updatePlot(truss::application::TrussHandle trussHandle);

    /**
     * @brief Clear plot display
     */
    void clearPlot();

protected:
    /**
     * @brief Paint event handler
     */
    void paintEvent(QPaintEvent* event) override;

private:
    /**
     * @brief Draw complete truss structure
     */
    void drawTruss(QPainter& painter);

    /**
     * @brief Draw nodes
     */
    void drawNodes(QPainter& painter);

    /**
     * @brief Draw members
     */
    void drawMembers(QPainter& painter);

    /**
     * @brief Draw applied loads
     */
    void drawLoads(QPainter& painter);

    /**
     * @brief Draw support conditions
     */
    void drawSupports(QPainter& painter);

    /**
     * @brief Draw deformed shape (if enabled)
     */
    void drawDeformedShape(QPainter& painter);

    /**
     * @brief Convert world coordinates to screen coordinates
     */
    QPoint worldToScreen(const truss::core::Point2D& point) const;

    /**
     * @brief Calculate viewport bounds
     */
    void calculateViewport();

    // Dependencies (Clean Architecture)
    truss::application::TrussApplicationService& m_trussService;
    truss::application::TrussHandle m_currentTrussHandle;

    // Display state
    bool m_showDeformed;
    bool m_showForces;
    double m_scaleFactor;
    QRect m_plotArea;
    truss::core::Point2D m_minBounds;
    truss::core::Point2D m_maxBounds;
};

}  // namespace truss::gui
