/**
 * @file DeformedTrussWidget.hpp
 * @brief Widget for visualizing deformed truss structure with analysis results
 * @version 2.1.0
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <memory>
#include "../core/Truss.hpp"
#include "../core/AnalysisEngine.hpp"

namespace truss::gui {

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
     * @brief Set the truss to display
     * @param truss Pointer to the analyzed truss
     */
    void setTruss(truss::core::Truss* truss);

    /**
     * @brief Set the analysis results
     * @param results The analysis results containing displacements and forces
     */
    void setAnalysisResults(const truss::core::AnalysisResults& results);

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
     * @param node Pointer to the node
     * @return Deformed position
     */
    QPointF getDeformedPosition(const truss::core::Node* node) const;

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

    // Data members
    truss::core::Truss* m_truss;
    truss::core::AnalysisResults m_results;
    bool m_hasResults;

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
    int m_forceVisualizationMode; // 0: Color, 1: Thickness, 2: Both

    // Bounds
    QRectF m_structureBounds;
    double m_maxDisplacement;
    double m_maxForce;

    // Control widgets
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

} // namespace truss::gui
