/**
 * @file InteractiveDrawingWidget.hpp
 * @brief Interactive drawing widget for designing truss structures
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSplitter>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <memory>
#include <vector>

#include "../core/model/Truss.hpp"
#include "../core/analysis/AnalysisOrchestrator.hpp"

namespace truss::gui {

class MainWindow; // Forward declaration

/**
 * @brief Drawing modes for the interactive widget
 */
enum class DrawingMode {
    Select,      ///< Selection and modification mode
    AddNode,     ///< Add nodes by clicking
    AddMember,   ///< Add members by clicking and dragging
    AddLoad,     ///< Add loads by clicking nodes
    SetSupport   ///< Set support conditions
};

/**
 * @brief Material properties for quick selection
 */
struct MaterialPreset {
    QString name;
    double youngModulus;    ///< Pa
    double density;         ///< kg/m³
    double yieldStrength;   ///< Pa
    
    MaterialPreset(const QString& n, double E, double rho, double fy)
        : name(n), youngModulus(E), density(rho), yieldStrength(fy) {}
};

/**
 * @brief Section properties for quick selection
 */
struct SectionPreset {
    QString name;
    double area;            ///< m²
    
    SectionPreset(const QString& n, double A) : name(n), area(A) {}
};

/**
 * @brief Interactive drawing canvas for truss design
 */
class DrawingCanvas : public QWidget {
    Q_OBJECT

public:
    explicit DrawingCanvas(QWidget* parent = nullptr);
    ~DrawingCanvas() override = default;

    // Drawing mode management
    void setDrawingMode(DrawingMode mode);
    DrawingMode getDrawingMode() const { return m_drawingMode; }
    
    // View management
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void resetView();
    
    // Grid and snap settings
    void setGridVisible(bool visible);
    void setSnapToGrid(bool snap);
    void setGridSpacing(double spacing);
    
    // Material and section settings
    void setCurrentMaterial(const MaterialPreset& material);
    void setCurrentSection(const SectionPreset& section);
    
    // Truss access
    truss::core::Truss* getTruss() const { return m_truss.get(); }
    void setTruss(std::unique_ptr<truss::core::Truss> truss);
    void clearTruss();
    
    // Coordinate conversion
    truss::core::Point2D screenToWorld(const QPoint& screenPoint) const;
    QPoint worldToScreen(const truss::core::Point2D& worldPoint) const;
    
    // Selection management
    void clearSelection();
    const std::vector<size_t>& getSelectedNodes() const { return m_selectedNodes; }
    const std::vector<size_t>& getSelectedMembers() const { return m_selectedMembers; }

signals:
    void trussModified();
    void nodeSelected(size_t nodeId);
    void memberSelected(size_t memberId);
    void coordinatesChanged(double x, double y);
    void statusMessage(const QString& message);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void updateCursor();

private:
    // Drawing functions
    void drawGrid(QPainter& painter);
    void drawTruss(QPainter& painter);
    void drawNodes(QPainter& painter);
    void drawMembers(QPainter& painter);
    void drawLoads(QPainter& painter);
    void drawSupports(QPainter& painter);
    void drawSelection(QPainter& painter);
    void drawCurrentOperation(QPainter& painter);
    void drawCoordinateSystem(QPainter& painter);
    void drawStatusOverlay(QPainter& painter);
    
    // Helper functions
    void updateViewport();
    void updateCursorPosition(const QPoint& pos);
    truss::core::Point2D snapToGrid(const truss::core::Point2D& point) const;
    size_t findNodeAt(const QPoint& screenPos, double tolerance = 10.0) const;
    size_t findMemberAt(const QPoint& screenPos, double tolerance = 5.0) const;
    bool isNearLine(const QPoint& point, const QPoint& lineStart, const QPoint& lineEnd, double tolerance) const;
    void addNodeAtPosition(const truss::core::Point2D& position);
    void addMemberBetweenNodes(size_t startNodeId, size_t endNodeId);
    void applyLoadToNode(size_t nodeId, const truss::core::Force2D& force);
    void setSupportType(size_t nodeId, truss::core::SupportType supportType);
    void deleteSelectedElements();
    
    // State variables
    DrawingMode m_drawingMode;
    std::unique_ptr<truss::core::Truss> m_truss;
    
    // View transformation
    double m_scale;
    truss::core::Point2D m_offset;
    QRect m_viewportRect;
    truss::core::Point2D m_worldBounds[2]; // min, max
    
    // Grid settings
    bool m_gridVisible;
    bool m_snapToGrid;
    double m_gridSpacing;
    
    // Current material and section
    MaterialPreset m_currentMaterial;
    SectionPreset m_currentSection;
    
    // Mouse interaction state
    bool m_isDragging;
    bool m_isPanning;
    QPoint m_lastMousePos;
    QPoint m_dragStartPos;
    truss::core::Point2D m_currentMouseWorld;
    
    // Selection
    std::vector<size_t> m_selectedNodes;
    std::vector<size_t> m_selectedMembers;
    
    // Current operation state
    size_t m_memberStartNode;  // For member creation
    bool m_isCreatingMember;
    
    // Visual settings
    static constexpr double DEFAULT_SCALE = 100.0;  // pixels per meter
    static constexpr double MIN_SCALE = 10.0;
    static constexpr double MAX_SCALE = 1000.0;
    static constexpr double GRID_SPACING_DEFAULT = 1.0; // meters
    static constexpr int NODE_RADIUS = 4;
    static constexpr int SELECTION_TOLERANCE = 10;
    
    // Visual elements
    QPen m_gridPen;
    QPen m_nodePen;
    QPen m_memberPen;
    QPen m_selectionPen;
    QPen m_loadPen;
    QPen m_supportPen;
    
    QBrush m_nodeBrush;
    QBrush m_selectedBrush;
    
    QFont m_labelFont;
    QFont m_statusFont;
};

/**
 * @brief Property panel for editing truss elements
 */
class PropertyPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget* parent = nullptr);
    
    void setDrawingCanvas(DrawingCanvas* canvas);
    void updateFromSelection();

signals:
    void propertiesChanged();

private slots:
    void onMaterialChanged();
    void onSectionChanged();
    void onLoadChanged();
    void onSupportChanged();
    void onNodePositionChanged();

private:
    void setupUI();
    void updateMaterialPresets();
    void updateSectionPresets();
    
    DrawingCanvas* m_canvas;
    
    // Material/Section controls
    QGroupBox* m_materialGroup;
    QComboBox* m_materialCombo;
    QDoubleSpinBox* m_youngModulusSpin;
    QDoubleSpinBox* m_densitySpin;
    QDoubleSpinBox* m_yieldStrengthSpin;
    
    QGroupBox* m_sectionGroup;
    QComboBox* m_sectionCombo;
    QDoubleSpinBox* m_areaSpin;
    
    // Node properties
    QGroupBox* m_nodeGroup;
    QDoubleSpinBox* m_nodeXSpin;
    QDoubleSpinBox* m_nodeYSpin;
    QComboBox* m_supportCombo;
    
    // Load properties
    QGroupBox* m_loadGroup;
    QDoubleSpinBox* m_forceXSpin;
    QDoubleSpinBox* m_forceYSpin;
    
    // Presets
    std::vector<MaterialPreset> m_materialPresets;
    std::vector<SectionPreset> m_sectionPresets;
};

/**
 * @brief Main interactive drawing widget combining canvas and controls
 */
class InteractiveDrawingWidget : public QWidget {
    Q_OBJECT

public:
    explicit InteractiveDrawingWidget(QWidget* parent = nullptr);
    ~InteractiveDrawingWidget() override = default;
    
    // Access to components
    DrawingCanvas* getCanvas() const { return m_canvas; }
    PropertyPanel* getPropertyPanel() const { return m_propertyPanel; }
    
    // Truss access
    truss::core::Truss* getTruss() const { return m_canvas->getTruss(); }
    void setTruss(std::unique_ptr<truss::core::Truss> truss);
    void clearTruss();

signals:
    void trussModified();
    void statusMessage(const QString& message);

private slots:
    void onDrawingModeChanged();
    void onViewChanged();
    void onGridToggled();
    void onSnapToggled();

private:
    void setupUI();
    void setupToolbar();
    void connectSignals();
    
    // UI Components
    QSplitter* m_mainSplitter;
    DrawingCanvas* m_canvas;
    PropertyPanel* m_propertyPanel;
    
    // Toolbar
    QToolBar* m_toolbar;
    QActionGroup* m_drawingModeActions;
    QAction* m_selectAction;
    QAction* m_addNodeAction;
    QAction* m_addMemberAction;
    QAction* m_addLoadAction;
    QAction* m_setSupportAction;
    
    // View controls
    QAction* m_zoomInAction;
    QAction* m_zoomOutAction;
    QAction* m_zoomFitAction;
    QAction* m_resetViewAction;
    
    // Grid controls
    QCheckBox* m_gridCheckBox;
    QCheckBox* m_snapCheckBox;
    QDoubleSpinBox* m_gridSpacingSpin;
    
    // Status display
    QLabel* m_coordinateLabel;
    QLabel* m_statusLabel;
};

} // namespace truss::gui
