/**
 * @file deformed_truss_widget.hpp
 * @brief Widget for visualizing deformed truss geometry and analysis results.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "application/analysis_application_service.hpp"
#include "core/interfaces/ianalysis_results_view.hpp"
#include "core/interfaces/itruss_view.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

#include <memory>

namespace truss::gui {

using truss::application::ResultsHandle;
using truss::core::interfaces::IAnalysisResultsView;
using truss::core::interfaces::ITrussView;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;

/**
 * @brief Widget for displaying the deformed shape of a truss structure
 *
 * This widget provides visualization of:
 * - Original structure (undeformed)
 * - Deformed structure with displacements
 * - Member forces (compression/tension)
 * - Displacement vectors
 * - Support reactions
 */
class DeformedTrussWidget : public QWidget {
    Q_OBJECT

public:
    explicit DeformedTrussWidget(QWidget* parent = nullptr);

    /**
     * @brief Set the truss and analysis results for display
     * @param trussView Read-only view of truss geometry
     * @param resultsView Read-only view of analysis results
     * @param resultsHandle Handle to results (for future operations)
     */
    void setData(const ITrussView& trussView,
                 const IAnalysisResultsView& resultsView,
                 ResultsHandle resultsHandle);

    /**
     * @brief Clear the display
     */
    void clear();

    /**
     * @brief Reset view to fit all content
     */
    void resetView();

public slots:
    /**
     * @brief Set the deformation scale factor
     * @param scale Scale factor (0.1 to 10.0)
     */
    void setDeformationScale(double scale);

    /**
     * @brief Toggle display of original structure
     * @param show Whether to show original structure
     */
    void showOriginal(bool show);

    /**
     * @brief Toggle display of deformed structure
     * @param show Whether to show deformed structure
     */
    void showDeformed(bool show);

    /**
     * @brief Toggle display of member forces
     * @param show Whether to show member forces
     */
    void showMemberForces(bool show);

    /**
     * @brief Toggle display of displacement vectors
     * @param show Whether to show displacement vectors
     */
    void showDisplacements(bool show);

    /**
     * @brief Toggle display of support reactions
     * @param show Whether to show support reactions
     */
    void showReactions(bool show);

    /**
     * @brief Set the force visualization mode
     * @param mode Visualization mode
     */
    void setForceVisualizationMode(int mode);

signals:
    /**
     * @brief Emitted when status message should be shown
     * @param message Status message
     */
    void statusMessage(const QString& message);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief Setup the control panel
     */
    void setupControls();

    /**
     * @brief Draw the original truss structure
     * @param painter QPainter instance
     */
    void drawOriginalTruss(QPainter& painter);

    /**
     * @brief Draw the deformed truss structure
     * @param painter QPainter instance
     */
    void drawDeformedTruss(QPainter& painter);

    /**
     * @brief Draw member force visualization
     * @param painter QPainter instance
     */
    void drawMemberForces(QPainter& painter);

    /**
     * @brief Draw displacement vectors
     * @param painter QPainter instance
     */
    void drawDisplacementVectors(QPainter& painter);

    /**
     * @brief Draw support reactions
     * @param painter QPainter instance
     */
    void drawSupportReactions(QPainter& painter);

    /**
     * @brief Draw coordinate grid
     * @param painter QPainter instance
     */
    void drawGrid(QPainter& painter);

    /**
     * @brief Draw legend
     * @param painter QPainter instance
     */
    void drawLegend(QPainter& painter);

    /**
     * @brief Convert world coordinates to screen coordinates
     * @param worldPoint Point in world coordinates
     * @return Point in screen coordinates
     */
    QPointF worldToScreen(const QPointF& worldPoint) const;

    /**
     * @brief Convert screen coordinates to world coordinates
     * @param screenPoint Point in screen coordinates
     * @return Point in world coordinates
     */
    QPointF screenToWorld(const QPointF& screenPoint) const;

    /**
     * @brief Get deformed position of a node
     * @param nodeView Node view data
     * @return Deformed position
     */
    QPointF getDeformedPosition(const NodeView& nodeView) const;

    /**
     * @brief Get color for member force visualization
     * @param force Axial force in the member
     * @param maxForce Maximum force in the structure
     * @return Color for the member
     */
    QColor getMemberForceColor(double force, double maxForce) const;

    /**
     * @brief Calculate bounds of the structure
     */
    void calculateBounds();

    /**
     * @brief Update view transformation
     */
    void updateViewTransform();

    // Data members (cached views from interfaces)
    std::vector<NodeView> m_nodeViews;
    std::vector<MemberView> m_memberViews;
    ResultsHandle m_resultsHandle;
    bool m_hasResults;

    // Analysis result data (cached from IAnalysisResultsView)
    std::vector<truss::core::Real> m_displacements;
    std::vector<truss::core::Real> m_reactions;
    std::vector<truss::core::Real> m_memberForces;

    // View transformation
    QPointF m_viewCenter;
    double m_viewScale;
    QPointF m_panStart;
    bool m_panning;

    // Visualization options
    double m_deformationScale;
    bool m_showOriginal;
    bool m_showDeformed;
    bool m_showMemberForces;
    bool m_showDisplacements;
    bool m_showReactions;
    int m_forceVisualizationMode;  // 0: Color, 1: Thickness, 2: Both

    // Bounds
    QRectF m_structureBounds;
    double m_maxDisplacement;
    double m_maxForce;

    // Control widgets
    QWidget* m_controlPanel;
    QSlider* m_scaleSlider;
    QLabel* m_scaleLabel;
    QCheckBox* m_originalCheckBox;
    QCheckBox* m_deformedCheckBox;
    QCheckBox* m_forcesCheckBox;
    QCheckBox* m_displacementsCheckBox;
    QCheckBox* m_reactionsCheckBox;
    QComboBox* m_forceVisualizationCombo;

    // Constants
    static constexpr double MIN_SCALE = 0.1;
    static constexpr double MAX_SCALE = 10.0;
    static constexpr double DEFAULT_DEFORMATION_SCALE = 1.0;
    static constexpr int LEGEND_WIDTH = 200;
    static constexpr int LEGEND_HEIGHT = 150;
};

}  // namespace truss::gui
