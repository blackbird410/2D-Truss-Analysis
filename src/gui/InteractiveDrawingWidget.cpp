/**
 * @file InteractiveDrawingWidget.cpp
 * @brief Implementation of interactive drawing widget for truss design
 */

#include "InteractiveDrawingWidget.hpp"
#include <QtCore/QDebug>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPixmap>
#include <QtCore/QStandardPaths>
#include <cmath>

namespace truss::gui {

//=============================================================================
// DrawingCanvas Implementation
//=============================================================================

DrawingCanvas::DrawingCanvas(QWidget* parent)
    : QWidget(parent),
      m_drawingMode(DrawingMode::Select),
      m_truss(std::make_unique<truss::core::Truss>()),
      m_scale(DEFAULT_SCALE),
      m_offset(0.0, 0.0),
      m_gridVisible(true),
      m_snapToGrid(true),
      m_gridSpacing(GRID_SPACING_DEFAULT),
      m_currentMaterial("Steel", 200e9, 7850, 250e6),
      m_currentSection("Default", 0.002),
      m_isDragging(false),
      m_isPanning(false),
      m_currentMouseWorld(0.0, 0.0),
      m_memberStartNode(0),
      m_isCreatingMember(false) {
    
    setMinimumSize(600, 400);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Initialize visual settings
    m_gridPen = QPen(QColor(200, 200, 200), 1, Qt::DotLine);
    m_nodePen = QPen(QColor(50, 100, 200), 2);
    m_memberPen = QPen(QColor(100, 100, 100), 2);
    m_selectionPen = QPen(QColor(255, 100, 100), 3);
    m_loadPen = QPen(QColor(200, 50, 50), 2);
    m_supportPen = QPen(QColor(50, 150, 50), 2);
    
    m_nodeBrush = QBrush(QColor(50, 100, 200));
    m_selectedBrush = QBrush(QColor(255, 100, 100));
    
    m_labelFont = QFont("Arial", 9);
    m_statusFont = QFont("Arial", 10);
    
    updateViewport();
}

void DrawingCanvas::setDrawingMode(DrawingMode mode) {
    if (m_drawingMode != mode) {
        m_drawingMode = mode;
        m_isCreatingMember = false;
        m_memberStartNode = 0;
        clearSelection();
        updateCursor();
        update();
        
        QString modeText;
        switch (mode) {
            case DrawingMode::Select:
                modeText = "Selection Mode - Click to select, drag to move";
                break;
            case DrawingMode::AddNode:
                modeText = "Add Node Mode - Click to place nodes";
                break;
            case DrawingMode::AddMember:
                modeText = "Add Member Mode - Click and drag between nodes";
                break;
            case DrawingMode::AddLoad:
                modeText = "Add Load Mode - Click nodes to apply loads";
                break;
            case DrawingMode::SetSupport:
                modeText = "Set Support Mode - Click nodes to set support conditions";
                break;
        }
        emit statusMessage(modeText);
    }
}

void DrawingCanvas::zoomIn() {
    m_scale = std::min(m_scale * 1.2, MAX_SCALE);
    update();
}

void DrawingCanvas::zoomOut() {
    m_scale = std::max(m_scale / 1.2, MIN_SCALE);
    update();
}

void DrawingCanvas::zoomToFit() {
    if (!m_truss || m_truss->getNodes().empty()) {
        m_scale = DEFAULT_SCALE;
        m_offset = truss::core::Point2D(0.0, 0.0);
        update();
        return;
    }
    
    // Calculate bounding box
    const auto& nodes = m_truss->getNodes();
    double minX = nodes[0]->getPosition().x;
    double maxX = nodes[0]->getPosition().x;
    double minY = nodes[0]->getPosition().y;
    double maxY = nodes[0]->getPosition().y;
    
    for (const auto& node : nodes) {
        const auto& pos = node->getPosition();
        minX = std::min(minX, pos.x);
        maxX = std::max(maxX, pos.x);
        minY = std::min(minY, pos.y);
        maxY = std::max(maxY, pos.y);
    }
    
    // Add margins
    double margin = 0.1;
    double rangeX = maxX - minX;
    double rangeY = maxY - minY;
    if (rangeX < 0.1) rangeX = 1.0;
    if (rangeY < 0.1) rangeY = 1.0;
    
    minX -= rangeX * margin;
    maxX += rangeX * margin;
    minY -= rangeY * margin;
    maxY += rangeY * margin;
    
    // Calculate scale to fit
    double scaleX = width() / (maxX - minX);
    double scaleY = height() / (maxY - minY);
    m_scale = std::min(scaleX, scaleY);
    m_scale = std::max(MIN_SCALE, std::min(m_scale, MAX_SCALE));
    
    // Center the view
    m_offset.x = (minX + maxX) / 2.0 - width() / (2.0 * m_scale);
    m_offset.y = (minY + maxY) / 2.0 - height() / (2.0 * m_scale);
    
    update();
}

void DrawingCanvas::resetView() {
    m_scale = DEFAULT_SCALE;
    m_offset = truss::core::Point2D(0.0, 0.0);
    update();
}

void DrawingCanvas::setGridVisible(bool visible) {
    m_gridVisible = visible;
    update();
}

void DrawingCanvas::setSnapToGrid(bool snap) {
    m_snapToGrid = snap;
}

void DrawingCanvas::setGridSpacing(double spacing) {
    m_gridSpacing = std::max(0.1, spacing);
    update();
}

void DrawingCanvas::setCurrentMaterial(const MaterialPreset& material) {
    m_currentMaterial = material;
}

void DrawingCanvas::setCurrentSection(const SectionPreset& section) {
    m_currentSection = section;
}

void DrawingCanvas::setTruss(std::unique_ptr<truss::core::Truss> truss) {
    m_truss = std::move(truss);
    clearSelection();
    updateViewport();
    update();
    emit trussModified();
}

void DrawingCanvas::clearTruss() {
    m_truss = std::make_unique<truss::core::Truss>();
    clearSelection();
    updateViewport();
    update();
    emit trussModified();
}

truss::core::Point2D DrawingCanvas::screenToWorld(const QPoint& screenPoint) const {
    double worldX = m_offset.x + screenPoint.x() / m_scale;
    double worldY = m_offset.y + (height() - screenPoint.y()) / m_scale;
    return truss::core::Point2D(worldX, worldY);
}

QPoint DrawingCanvas::worldToScreen(const truss::core::Point2D& worldPoint) const {
    int screenX = static_cast<int>((worldPoint.x - m_offset.x) * m_scale);
    int screenY = static_cast<int>(height() - (worldPoint.y - m_offset.y) * m_scale);
    return QPoint(screenX, screenY);
}

void DrawingCanvas::clearSelection() {
    m_selectedNodes.clear();
    m_selectedMembers.clear();
    update();
}

void DrawingCanvas::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QBrush(Qt::white));
    
    // Draw grid
    if (m_gridVisible) {
        drawGrid(painter);
    }
    
    // Draw coordinate system
    drawCoordinateSystem(painter);
    
    // Draw truss elements
    if (m_truss) {
        drawTruss(painter);
    }
    
    // Draw current operation
    drawCurrentOperation(painter);
    
    // Draw status overlay
    drawStatusOverlay(painter);
}

void DrawingCanvas::drawGrid(QPainter& painter) {
    painter.setPen(m_gridPen);
    
    // Calculate grid bounds
    truss::core::Point2D topLeft = screenToWorld(QPoint(0, 0));
    truss::core::Point2D bottomRight = screenToWorld(QPoint(width(), height()));
    
    // Snap to grid
    double startX = std::floor(topLeft.x / m_gridSpacing) * m_gridSpacing;
    double endX = std::ceil(bottomRight.x / m_gridSpacing) * m_gridSpacing;
    double startY = std::floor(bottomRight.y / m_gridSpacing) * m_gridSpacing;
    double endY = std::ceil(topLeft.y / m_gridSpacing) * m_gridSpacing;
    
    // Draw vertical lines
    for (double x = startX; x <= endX; x += m_gridSpacing) {
        QPoint start = worldToScreen(truss::core::Point2D(x, startY));
        QPoint end = worldToScreen(truss::core::Point2D(x, endY));
        painter.drawLine(start, end);
    }
    
    // Draw horizontal lines
    for (double y = startY; y <= endY; y += m_gridSpacing) {
        QPoint start = worldToScreen(truss::core::Point2D(startX, y));
        QPoint end = worldToScreen(truss::core::Point2D(endX, y));
        painter.drawLine(start, end);
    }
}

void DrawingCanvas::drawTruss(QPainter& painter) {
    // Draw members first (behind nodes)
    drawMembers(painter);
    
    // Draw supports
    drawSupports(painter);
    
    // Draw loads
    drawLoads(painter);
    
    // Draw nodes (on top)
    drawNodes(painter);
    
    // Draw selection highlights
    drawSelection(painter);
}

void DrawingCanvas::drawNodes(QPainter& painter) {
    if (!m_truss) return;
    
    const auto& nodes = m_truss->getNodes();
    painter.setFont(m_labelFont);
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        QPoint screenPos = worldToScreen(node->getPosition());
        
        // Check if selected
        bool isSelected = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), i) != m_selectedNodes.end();
        
        painter.setPen(m_nodePen);
        painter.setBrush(isSelected ? m_selectedBrush : m_nodeBrush);
        
        // Draw node circle
        painter.drawEllipse(screenPos - QPoint(NODE_RADIUS, NODE_RADIUS), 
                           2 * NODE_RADIUS, 2 * NODE_RADIUS);
        
        // Draw node ID
        painter.setPen(QPen(Qt::black));
        painter.drawText(screenPos + QPoint(NODE_RADIUS + 2, -NODE_RADIUS), 
                        QString::number(node->getId()));
    }
}

void DrawingCanvas::drawMembers(QPainter& painter) {
    if (!m_truss) return;
    
    const auto& members = m_truss->getMembers();
    painter.setFont(m_labelFont);
    
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        auto startNode = member->getStartNode();
        auto endNode = member->getEndNode();
        
        if (!startNode || !endNode) continue;
        
        QPoint startPos = worldToScreen(startNode->getPosition());
        QPoint endPos = worldToScreen(endNode->getPosition());
        
        // Check if selected
        bool isSelected = std::find(m_selectedMembers.begin(), m_selectedMembers.end(), i) != m_selectedMembers.end();
        
        painter.setPen(isSelected ? m_selectionPen : m_memberPen);
        painter.drawLine(startPos, endPos);
        
        // Draw member ID at midpoint
        QPoint midPoint = (startPos + endPos) / 2;
        painter.setPen(QPen(Qt::darkGray));
        painter.drawText(midPoint + QPoint(3, 3), QString("M%1").arg(member->getId()));
    }
}

void DrawingCanvas::drawLoads(QPainter& painter) {
    if (!m_truss) return;
    
    const auto& nodes = m_truss->getNodes();
    painter.setPen(m_loadPen);
    
    for (const auto& node : nodes) {
        if (!node->hasAppliedForce()) continue;
        
        QPoint nodePos = worldToScreen(node->getPosition());
        truss::core::Force2D force = node->getAppliedForce();
        
        // Scale force for display
        double magnitude = std::sqrt(force.fx * force.fx + force.fy * force.fy);
        if (magnitude < 1e-6) continue;
        
        double scale = 50.0; // pixels per kN
        double maxLength = 60.0;
        double length = std::min(scale * magnitude / 1000.0, maxLength);
        
        QPoint forceEnd(
            nodePos.x() + static_cast<int>(length * force.fx / magnitude),
            nodePos.y() - static_cast<int>(length * force.fy / magnitude)
        );
        
        // Draw force arrow
        painter.drawLine(nodePos, forceEnd);
        
        // Draw arrowhead
        QVector<QPoint> arrowHead;
        double angle = std::atan2(-force.fy, force.fx);
        int arrowSize = 8;
        
        arrowHead << forceEnd
                  << forceEnd + QPoint(
                      static_cast<int>(-arrowSize * std::cos(angle - M_PI/6)),
                      static_cast<int>(arrowSize * std::sin(angle - M_PI/6)))
                  << forceEnd + QPoint(
                      static_cast<int>(-arrowSize * std::cos(angle + M_PI/6)),
                      static_cast<int>(arrowSize * std::sin(angle + M_PI/6)));
        
        painter.setBrush(m_loadPen.brush());
        painter.drawPolygon(arrowHead);
        
        // Draw magnitude
        painter.setPen(QPen(Qt::red));
        painter.setFont(m_labelFont);
        painter.drawText(forceEnd + QPoint(5, 5), QString("%1 kN").arg(magnitude/1000.0, 0, 'f', 1));
    }
}

void DrawingCanvas::drawSupports(QPainter& painter) {
    if (!m_truss) return;
    
    const auto& nodes = m_truss->getNodes();
    painter.setPen(m_supportPen);
    painter.setBrush(QBrush(m_supportPen.color()));
    
    for (const auto& node : nodes) {
        if (node->getSupportType() == truss::core::SupportType::Free) continue;
        
        QPoint nodePos = worldToScreen(node->getPosition());
        
        switch (node->getSupportType()) {
            case truss::core::SupportType::Pinned: {
                // Draw triangle
                QVector<QPoint> triangle;
                triangle << nodePos + QPoint(-10, 10)
                        << nodePos + QPoint(10, 10)
                        << nodePos;
                painter.drawPolygon(triangle);
                break;
            }
            
            case truss::core::SupportType::RollerX: {
                // Draw circle
                painter.drawEllipse(nodePos + QPoint(-8, 2), 16, 16);
                // Draw ground line
                painter.drawLine(nodePos + QPoint(-12, 18), nodePos + QPoint(12, 18));
                break;
            }
            
            case truss::core::SupportType::RollerY: {
                // Draw circle to the side
                painter.drawEllipse(nodePos + QPoint(-18, -8), 16, 16);
                // Draw ground line
                painter.drawLine(nodePos + QPoint(-18, -12), nodePos + QPoint(-18, 12));
                break;
            }
            
            default:
                break;
        }
    }
}

void DrawingCanvas::drawSelection(QPainter& /*painter*/) {
    // Selection is already handled in drawNodes and drawMembers
}

void DrawingCanvas::drawCurrentOperation(QPainter& painter) {
    if (m_drawingMode == DrawingMode::AddMember && m_isCreatingMember && m_memberStartNode > 0) {
        // Draw line from start node to current mouse position
        auto startNode = m_truss->getNode(m_memberStartNode);
        if (startNode) {
            QPoint startPos = worldToScreen(startNode->getPosition());
            QPoint mousePos = worldToScreen(m_currentMouseWorld);
            
            painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            painter.drawLine(startPos, mousePos);
        }
    }
}

void DrawingCanvas::drawCoordinateSystem(QPainter& painter) {
    // Draw coordinate system in bottom-left corner
    int margin = 20;
    QPoint origin(margin, height() - margin);
    int axisLength = 40;
    
    painter.setPen(QPen(Qt::black, 2));
    
    // X-axis (red)
    painter.setPen(QPen(Qt::red, 2));
    painter.drawLine(origin, origin + QPoint(axisLength, 0));
    painter.drawText(origin + QPoint(axisLength + 5, 5), "X");
    
    // Y-axis (green)
    painter.setPen(QPen(Qt::green, 2));
    painter.drawLine(origin, origin + QPoint(0, -axisLength));
    painter.drawText(origin + QPoint(5, -axisLength - 5), "Y");
}

void DrawingCanvas::drawStatusOverlay(QPainter& painter) {
    // Draw coordinate display in top-right corner
    painter.setFont(m_statusFont);
    painter.setPen(QPen(Qt::black));
    
    QString coordText = QString("X: %1 m, Y: %2 m")
                       .arg(m_currentMouseWorld.x, 0, 'f', 3)
                       .arg(m_currentMouseWorld.y, 0, 'f', 3);
    
    QRect textRect = painter.fontMetrics().boundingRect(coordText);
    QPoint textPos(width() - textRect.width() - 10, 20);
    
    painter.fillRect(textRect.translated(textPos).adjusted(-5, -2, 5, 2), 
                    QBrush(QColor(255, 255, 255, 200)));
    painter.drawText(textPos, coordText);
}

// Continue with mouse event handlers...
void DrawingCanvas::mousePressEvent(QMouseEvent* event) {
    m_lastMousePos = event->pos();
    m_dragStartPos = event->pos();
    m_currentMouseWorld = screenToWorld(event->pos());
    
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    
    if (event->button() != Qt::LeftButton) return;
    
    switch (m_drawingMode) {
        case DrawingMode::Select: {
            // Find what was clicked
            size_t nodeId = findNodeAt(event->pos());
            size_t memberId = findMemberAt(event->pos());
            
            if (nodeId > 0) {
                if (!(event->modifiers() & Qt::ControlModifier)) {
                    clearSelection();
                }
                auto it = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), nodeId - 1);
                if (it == m_selectedNodes.end()) {
                    m_selectedNodes.push_back(nodeId - 1);
                } else {
                    m_selectedNodes.erase(it);
                }
                emit nodeSelected(nodeId - 1);
            } else if (memberId > 0) {
                if (!(event->modifiers() & Qt::ControlModifier)) {
                    clearSelection();
                }
                auto it = std::find(m_selectedMembers.begin(), m_selectedMembers.end(), memberId - 1);
                if (it == m_selectedMembers.end()) {
                    m_selectedMembers.push_back(memberId - 1);
                } else {
                    m_selectedMembers.erase(it);
                }
                emit memberSelected(memberId - 1);
            } else {
                if (!(event->modifiers() & Qt::ControlModifier)) {
                    clearSelection();
                }
            }
            break;
        }
        
        case DrawingMode::AddNode: {
            truss::core::Point2D position = m_snapToGrid ? 
                snapToGrid(m_currentMouseWorld) : m_currentMouseWorld;
            addNodeAtPosition(position);
            break;
        }
        
        case DrawingMode::AddMember: {
            size_t nodeId = findNodeAt(event->pos());
            if (nodeId > 0) {
                if (!m_isCreatingMember) {
                    m_memberStartNode = nodeId;
                    m_isCreatingMember = true;
                } else {
                    if (nodeId != m_memberStartNode) {
                        addMemberBetweenNodes(m_memberStartNode - 1, nodeId - 1);
                    }
                    m_isCreatingMember = false;
                    m_memberStartNode = 0;
                }
            }
            break;
        }
        
        case DrawingMode::AddLoad: {
            size_t nodeId = findNodeAt(event->pos());
            if (nodeId > 0) {
                // Show load input dialog or apply default load
                truss::core::Force2D defaultLoad(0.0, -10000.0); // 10 kN downward
                applyLoadToNode(nodeId - 1, defaultLoad);
            }
            break;
        }
        
        case DrawingMode::SetSupport: {
            size_t nodeId = findNodeAt(event->pos());
            if (nodeId > 0) {
                // Cycle through support types
                auto node = m_truss->getNode(nodeId);
                if (node) {
                    auto currentType = node->getSupportType();
                    truss::core::SupportType newType;
                    
                    switch (currentType) {
                        case truss::core::SupportType::Free:
                            newType = truss::core::SupportType::Pinned;
                            break;
                        case truss::core::SupportType::Pinned:
                            newType = truss::core::SupportType::RollerX;
                            break;
                        case truss::core::SupportType::RollerX:
                            newType = truss::core::SupportType::RollerY;
                            break;
                        default:
                            newType = truss::core::SupportType::Free;
                            break;
                    }
                    
                    setSupportType(nodeId - 1, newType);
                }
            }
            break;
        }
    }
    
    update();
}

void DrawingCanvas::mouseMoveEvent(QMouseEvent* event) {
    m_currentMouseWorld = screenToWorld(event->pos());
    emit coordinatesChanged(m_currentMouseWorld.x, m_currentMouseWorld.y);
    
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_offset.x -= delta.x() / m_scale;
        m_offset.y += delta.y() / m_scale;
        update();
    }
    
    m_lastMousePos = event->pos();
    
    if (m_drawingMode == DrawingMode::AddMember && m_isCreatingMember) {
        update(); // Redraw to show the preview line
    }
}

void DrawingCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        updateCursor();
    }
}

void DrawingCanvas::wheelEvent(QWheelEvent* event) {
    // Zoom at mouse position
    QPoint mousePos = event->position().toPoint();
    truss::core::Point2D worldPos = screenToWorld(mousePos);
    
    double scaleFactor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    double newScale = std::max(MIN_SCALE, std::min(m_scale * scaleFactor, MAX_SCALE));
    
    if (newScale != m_scale) {
        // Adjust offset to zoom at mouse position
        m_offset.x = worldPos.x - mousePos.x() / newScale;
        m_offset.y = worldPos.y - (height() - mousePos.y()) / newScale;
        m_scale = newScale;
        update();
    }
}

void DrawingCanvas::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            deleteSelectedElements();
            break;
        case Qt::Key_Escape:
            if (m_isCreatingMember) {
                m_isCreatingMember = false;
                m_memberStartNode = 0;
                update();
            } else {
                clearSelection();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void DrawingCanvas::leaveEvent(QEvent*) {
    // Hide coordinate display when mouse leaves widget
    m_currentMouseWorld = truss::core::Point2D(0.0, 0.0);
    emit coordinatesChanged(0.0, 0.0);
}

void DrawingCanvas::updateCursor() {
    if (m_isPanning) {
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    
    switch (m_drawingMode) {
        case DrawingMode::Select:
            setCursor(Qt::ArrowCursor);
            break;
        case DrawingMode::AddNode:
            setCursor(Qt::CrossCursor);
            break;
        case DrawingMode::AddMember:
            setCursor(Qt::PointingHandCursor);
            break;
        case DrawingMode::AddLoad:
            setCursor(Qt::CrossCursor);
            break;
        case DrawingMode::SetSupport:
            setCursor(Qt::PointingHandCursor);
            break;
    }
}

// Helper function implementations
void DrawingCanvas::updateViewport() {
    m_viewportRect = rect();
    update();
}

void DrawingCanvas::updateCursorPosition(const QPoint& pos) {
    m_currentMouseWorld = screenToWorld(pos);
    emit coordinatesChanged(m_currentMouseWorld.x, m_currentMouseWorld.y);
}

truss::core::Point2D DrawingCanvas::snapToGrid(const truss::core::Point2D& point) const {
    if (!m_snapToGrid) return point;
    
    double snappedX = std::round(point.x / m_gridSpacing) * m_gridSpacing;
    double snappedY = std::round(point.y / m_gridSpacing) * m_gridSpacing;
    
    return truss::core::Point2D(snappedX, snappedY);
}

size_t DrawingCanvas::findNodeAt(const QPoint& screenPos, double tolerance) const {
    if (!m_truss) return 0;
    
    const auto& nodes = m_truss->getNodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
        QPoint nodeScreen = worldToScreen(nodes[i]->getPosition());
        double distance = std::sqrt(std::pow(screenPos.x() - nodeScreen.x(), 2) + 
                                  std::pow(screenPos.y() - nodeScreen.y(), 2));
        if (distance <= tolerance) {
            return nodes[i]->getId();
        }
    }
    return 0;
}

size_t DrawingCanvas::findMemberAt(const QPoint& screenPos, double tolerance) const {
    if (!m_truss) return 0;
    
    const auto& members = m_truss->getMembers();
    for (size_t i = 0; i < members.size(); ++i) {
        const auto& member = members[i];
        auto startNode = member->getStartNode();
        auto endNode = member->getEndNode();
        
        if (!startNode || !endNode) continue;
        
        QPoint startScreen = worldToScreen(startNode->getPosition());
        QPoint endScreen = worldToScreen(endNode->getPosition());
        
        if (isNearLine(screenPos, startScreen, endScreen, tolerance)) {
            return member->getId();
        }
    }
    return 0;
}

bool DrawingCanvas::isNearLine(const QPoint& point, const QPoint& lineStart, 
                              const QPoint& lineEnd, double tolerance) const {
    // Calculate distance from point to line segment
    QPoint A = lineStart;
    QPoint B = lineEnd;
    QPoint P = point;
    
    QPoint AB = B - A;
    QPoint AP = P - A;
    
    double AB_squared = QPoint::dotProduct(AB, AB);
    if (AB_squared == 0) {
        // Line is actually a point
        return std::sqrt(QPoint::dotProduct(AP, AP)) <= tolerance;
    }
    
    double t = QPoint::dotProduct(AP, AB) / AB_squared;
    t = std::max(0.0, std::min(1.0, t)); // Clamp to line segment
    
    QPoint closest = A + QPoint(static_cast<int>(t * AB.x()), static_cast<int>(t * AB.y()));
    QPoint distance_vec = P - closest;
    
    return std::sqrt(QPoint::dotProduct(distance_vec, distance_vec)) <= tolerance;
}

void DrawingCanvas::addNodeAtPosition(const truss::core::Point2D& position) {
    if (!m_truss) return;
    
    try {
        auto node = m_truss->addNode(position);
        emit trussModified();
        emit statusMessage(QString("Node added at (%1, %2)").arg(position.x, 0, 'f', 3).arg(position.y, 0, 'f', 3));
    } catch (const std::exception& e) {
        emit statusMessage(QString("Failed to add node: %1").arg(e.what()));
    }
}

void DrawingCanvas::addMemberBetweenNodes(size_t startNodeId, size_t endNodeId) {
    if (!m_truss) return;
    
    try {
        const auto& nodes = m_truss->getNodes();
        if (startNodeId >= nodes.size() || endNodeId >= nodes.size()) return;
        
        auto startNode = nodes[startNodeId];
        auto endNode = nodes[endNodeId];
        
        // Create material and section properties
        truss::core::MaterialProperties material{
            m_currentMaterial.youngModulus,
            m_currentMaterial.density,
            m_currentMaterial.yieldStrength,
            m_currentMaterial.yieldStrength * 1.6, // Estimate ultimate strength
            "Steel"
        };
        
        truss::core::SectionProperties section{m_currentSection.area, 1e-8, m_currentSection.area, "Default"};
        
        auto member = m_truss->addMember(startNode, endNode, material, section);
        
        emit trussModified();
        emit statusMessage(QString("Member added between nodes %1 and %2")
                          .arg(startNode->getId()).arg(endNode->getId()));
    } catch (const std::exception& e) {
        emit statusMessage(QString("Failed to add member: %1").arg(e.what()));
    }
}

void DrawingCanvas::applyLoadToNode(size_t nodeId, const truss::core::Force2D& force) {
    if (!m_truss) return;
    
    try {
        const auto& nodes = m_truss->getNodes();
        if (nodeId >= nodes.size()) return;
        
        auto node = nodes[nodeId];
        node->setAppliedForce(force);
        
        emit trussModified();
        emit statusMessage(QString("Load applied to node %1: Fx=%2 kN, Fy=%3 kN")
                          .arg(node->getId())
                          .arg(force.fx / 1000.0, 0, 'f', 1)
                          .arg(force.fy / 1000.0, 0, 'f', 1));
    } catch (const std::exception& e) {
        emit statusMessage(QString("Failed to apply load: %1").arg(e.what()));
    }
}

void DrawingCanvas::setSupportType(size_t nodeId, truss::core::SupportType supportType) {
    if (!m_truss) return;
    
    try {
        const auto& nodes = m_truss->getNodes();
        if (nodeId >= nodes.size()) return;
        
        auto node = nodes[nodeId];
        node->setSupportType(supportType);
        
        emit trussModified();
        
        QString supportName;
        switch (supportType) {
            case truss::core::SupportType::Free: supportName = "Free"; break;
            case truss::core::SupportType::Pinned: supportName = "Pinned"; break;
            case truss::core::SupportType::RollerX: supportName = "Roller X"; break;
            case truss::core::SupportType::RollerY: supportName = "Roller Y"; break;
            default: supportName = "Unknown"; break;
        }
        
        emit statusMessage(QString("Node %1 support set to %2").arg(node->getId()).arg(supportName));
    } catch (const std::exception& e) {
        emit statusMessage(QString("Failed to set support: %1").arg(e.what()));
    }
}

void DrawingCanvas::deleteSelectedElements() {
    if (!m_truss) return;
    
    bool modified = false;
    
    // Delete selected members first
    for (auto it = m_selectedMembers.rbegin(); it != m_selectedMembers.rend(); ++it) {
        size_t memberId = *it;
        const auto& members = m_truss->getMembers();
        if (memberId < members.size()) {
            auto member = members[memberId];
            m_truss->removeMember(member->getId());
            modified = true;
        }
    }
    
    // Delete selected nodes
    for (auto it = m_selectedNodes.rbegin(); it != m_selectedNodes.rend(); ++it) {
        size_t nodeId = *it;
        const auto& nodes = m_truss->getNodes();
        if (nodeId < nodes.size()) {
            auto node = nodes[nodeId];
            m_truss->removeNode(node->getId());
            modified = true;
        }
    }
    
    if (modified) {
        clearSelection();
        emit trussModified();
        emit statusMessage("Selected elements deleted");
    }
}

//=============================================================================
// PropertyPanel Implementation
//=============================================================================

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent), m_canvas(nullptr) {
    setupUI();
    updateMaterialPresets();
    updateSectionPresets();
}

void PropertyPanel::setDrawingCanvas(DrawingCanvas* canvas) {
    m_canvas = canvas;
    // Connect signals to update when canvas changes
    if (m_canvas) {
        connect(m_canvas, &DrawingCanvas::nodeSelected, this, &PropertyPanel::updateFromSelection);
        connect(m_canvas, &DrawingCanvas::memberSelected, this, &PropertyPanel::updateFromSelection);
        connect(m_canvas, &DrawingCanvas::trussModified, this, &PropertyPanel::updateFromSelection);
    }
}

void PropertyPanel::updateFromSelection() {
    if (!m_canvas) return;
    
    const auto& selectedNodes = m_canvas->getSelectedNodes();
    
    // Enable/disable groups based on selection
    m_nodeGroup->setEnabled(!selectedNodes.empty());
    m_loadGroup->setEnabled(!selectedNodes.empty());
    
    // Update node properties if one node is selected
    if (selectedNodes.size() == 1) {
        const auto& nodes = m_canvas->getTruss()->getNodes();
        if (selectedNodes[0] < nodes.size()) {
            auto node = nodes[selectedNodes[0]];
            const auto& pos = node->getPosition();
            
            m_nodeXSpin->blockSignals(true);
            m_nodeYSpin->blockSignals(true);
            m_nodeXSpin->setValue(pos.x);
            m_nodeYSpin->setValue(pos.y);
            m_nodeXSpin->blockSignals(false);
            m_nodeYSpin->blockSignals(false);
            
            // Update support type
            m_supportCombo->blockSignals(true);
            m_supportCombo->setCurrentIndex(static_cast<int>(node->getSupportType()));
            m_supportCombo->blockSignals(false);
            
            // Update load values
            if (node->hasAppliedForce()) {
                auto force = node->getAppliedForce();
                m_forceXSpin->blockSignals(true);
                m_forceYSpin->blockSignals(true);
                m_forceXSpin->setValue(force.fx / 1000.0); // Convert to kN
                m_forceYSpin->setValue(force.fy / 1000.0);
                m_forceXSpin->blockSignals(false);
                m_forceYSpin->blockSignals(false);
            }
        }
    }
}

void PropertyPanel::setupUI() {
    setMaximumWidth(300);
    setMinimumWidth(250);
    
    auto* layout = new QVBoxLayout(this);
    
    // Material Group
    m_materialGroup = new QGroupBox("Material Properties", this);
    auto* materialLayout = new QGridLayout(m_materialGroup);
    
    materialLayout->addWidget(new QLabel("Material:"), 0, 0);
    m_materialCombo = new QComboBox();
    materialLayout->addWidget(m_materialCombo, 0, 1);
    
    materialLayout->addWidget(new QLabel("Young's Modulus (GPa):"), 1, 0);
    m_youngModulusSpin = new QDoubleSpinBox();
    m_youngModulusSpin->setRange(1, 1000);
    m_youngModulusSpin->setSuffix(" GPa");
    m_youngModulusSpin->setValue(200);
    materialLayout->addWidget(m_youngModulusSpin, 1, 1);
    
    materialLayout->addWidget(new QLabel("Density (kg/m³):"), 2, 0);
    m_densitySpin = new QDoubleSpinBox();
    m_densitySpin->setRange(100, 20000);
    m_densitySpin->setSuffix(" kg/m³");
    m_densitySpin->setValue(7850);
    materialLayout->addWidget(m_densitySpin, 2, 1);
    
    materialLayout->addWidget(new QLabel("Yield Strength (MPa):"), 3, 0);
    m_yieldStrengthSpin = new QDoubleSpinBox();
    m_yieldStrengthSpin->setRange(10, 2000);
    m_yieldStrengthSpin->setSuffix(" MPa");
    m_yieldStrengthSpin->setValue(250);
    materialLayout->addWidget(m_yieldStrengthSpin, 3, 1);
    
    layout->addWidget(m_materialGroup);
    
    // Section Group
    m_sectionGroup = new QGroupBox("Section Properties", this);
    auto* sectionLayout = new QGridLayout(m_sectionGroup);
    
    sectionLayout->addWidget(new QLabel("Section:"), 0, 0);
    m_sectionCombo = new QComboBox();
    sectionLayout->addWidget(m_sectionCombo, 0, 1);
    
    sectionLayout->addWidget(new QLabel("Area (cm²):"), 1, 0);
    m_areaSpin = new QDoubleSpinBox();
    m_areaSpin->setRange(0.1, 1000);
    m_areaSpin->setSuffix(" cm²");
    m_areaSpin->setValue(20);
    materialLayout->addWidget(m_areaSpin, 1, 1);
    
    layout->addWidget(m_sectionGroup);
    
    // Node Properties Group
    m_nodeGroup = new QGroupBox("Node Properties", this);
    auto* nodeLayout = new QGridLayout(m_nodeGroup);
    
    nodeLayout->addWidget(new QLabel("X Position (m):"), 0, 0);
    m_nodeXSpin = new QDoubleSpinBox();
    m_nodeXSpin->setRange(-1000, 1000);
    m_nodeXSpin->setSuffix(" m");
    m_nodeXSpin->setDecimals(3);
    nodeLayout->addWidget(m_nodeXSpin, 0, 1);
    
    nodeLayout->addWidget(new QLabel("Y Position (m):"), 1, 0);
    m_nodeYSpin = new QDoubleSpinBox();
    m_nodeYSpin->setRange(-1000, 1000);
    m_nodeYSpin->setSuffix(" m");
    m_nodeYSpin->setDecimals(3);
    nodeLayout->addWidget(m_nodeYSpin, 1, 1);
    
    nodeLayout->addWidget(new QLabel("Support Type:"), 2, 0);
    m_supportCombo = new QComboBox();
    m_supportCombo->addItems({"Free", "Pinned", "Roller X", "Roller Y"});
    nodeLayout->addWidget(m_supportCombo, 2, 1);
    
    m_nodeGroup->setEnabled(false);
    layout->addWidget(m_nodeGroup);
    
    // Load Properties Group
    m_loadGroup = new QGroupBox("Applied Loads", this);
    auto* loadLayout = new QGridLayout(m_loadGroup);
    
    loadLayout->addWidget(new QLabel("Force X (kN):"), 0, 0);
    m_forceXSpin = new QDoubleSpinBox();
    m_forceXSpin->setRange(-10000, 10000);
    m_forceXSpin->setSuffix(" kN");
    m_forceXSpin->setDecimals(2);
    loadLayout->addWidget(m_forceXSpin, 0, 1);
    
    loadLayout->addWidget(new QLabel("Force Y (kN):"), 1, 0);
    m_forceYSpin = new QDoubleSpinBox();
    m_forceYSpin->setRange(-10000, 10000);
    m_forceYSpin->setSuffix(" kN");
    m_forceYSpin->setDecimals(2);
    loadLayout->addWidget(m_forceYSpin, 1, 1);
    
    m_loadGroup->setEnabled(false);
    layout->addWidget(m_loadGroup);
    
    layout->addStretch();
    
    // Connect signals
    connect(m_materialCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertyPanel::onMaterialChanged);
    connect(m_sectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertyPanel::onSectionChanged);
    connect(m_nodeXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onNodePositionChanged);
    connect(m_nodeYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onNodePositionChanged);
    connect(m_supportCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertyPanel::onSupportChanged);
    connect(m_forceXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onLoadChanged);
    connect(m_forceYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onLoadChanged);
}

void PropertyPanel::updateMaterialPresets() {
    m_materialPresets.clear();
    m_materialPresets.emplace_back("Steel (Structural)", 200e9, 7850, 250e6);
    m_materialPresets.emplace_back("Steel (High Strength)", 200e9, 7850, 355e6);
    m_materialPresets.emplace_back("Aluminum", 70e9, 2700, 276e6);
    m_materialPresets.emplace_back("Timber (Softwood)", 12e9, 500, 40e6);
    m_materialPresets.emplace_back("Concrete", 30e9, 2400, 30e6);
    
    m_materialCombo->clear();
    for (const auto& preset : m_materialPresets) {
        m_materialCombo->addItem(preset.name);
    }
}

void PropertyPanel::updateSectionPresets() {
    m_sectionPresets.clear();
    m_sectionPresets.emplace_back("20 cm²", 0.002);
    m_sectionPresets.emplace_back("50 cm²", 0.005);
    m_sectionPresets.emplace_back("100 cm²", 0.010);
    m_sectionPresets.emplace_back("200 cm²", 0.020);
    m_sectionPresets.emplace_back("Custom", 0.002);
    
    m_sectionCombo->clear();
    for (const auto& preset : m_sectionPresets) {
        m_sectionCombo->addItem(preset.name);
    }
}

void PropertyPanel::onMaterialChanged() {
    int index = m_materialCombo->currentIndex();
    if (index >= 0 && index < static_cast<int>(m_materialPresets.size())) {
        const auto& material = m_materialPresets[index];
        
        m_youngModulusSpin->blockSignals(true);
        m_densitySpin->blockSignals(true);
        m_yieldStrengthSpin->blockSignals(true);
        
        m_youngModulusSpin->setValue(material.youngModulus / 1e9); // Convert to GPa
        m_densitySpin->setValue(material.density);
        m_yieldStrengthSpin->setValue(material.yieldStrength / 1e6); // Convert to MPa
        
        m_youngModulusSpin->blockSignals(false);
        m_densitySpin->blockSignals(false);
        m_yieldStrengthSpin->blockSignals(false);
        
        if (m_canvas) {
            MaterialPreset newMaterial(
                material.name,
                m_youngModulusSpin->value() * 1e9,
                m_densitySpin->value(),
                m_yieldStrengthSpin->value() * 1e6
            );
            m_canvas->setCurrentMaterial(newMaterial);
        }
    }
    emit propertiesChanged();
}

void PropertyPanel::onSectionChanged() {
    int index = m_sectionCombo->currentIndex();
    if (index >= 0 && index < static_cast<int>(m_sectionPresets.size())) {
        const auto& section = m_sectionPresets[index];
        
        m_areaSpin->blockSignals(true);
        m_areaSpin->setValue(section.area * 10000); // Convert to cm²
        m_areaSpin->blockSignals(false);
        
        if (m_canvas) {
            SectionPreset newSection(section.name, m_areaSpin->value() / 10000); // Convert back to m²
            m_canvas->setCurrentSection(newSection);
        }
    }
    emit propertiesChanged();
}

void PropertyPanel::onLoadChanged() {
    if (!m_canvas) return;
    
    const auto& selectedNodes = m_canvas->getSelectedNodes();
    if (selectedNodes.size() == 1) {
        const auto& nodes = m_canvas->getTruss()->getNodes();
        if (selectedNodes[0] < nodes.size()) {
            auto node = nodes[selectedNodes[0]];
            
            truss::core::Force2D force(
                m_forceXSpin->value() * 1000.0, // Convert to N
                m_forceYSpin->value() * 1000.0
            );
            
            node->setAppliedForce(force);
            m_canvas->update();
        }
    }
    emit propertiesChanged();
}

void PropertyPanel::onSupportChanged() {
    if (!m_canvas) return;
    
    const auto& selectedNodes = m_canvas->getSelectedNodes();
    if (selectedNodes.size() == 1) {
        const auto& nodes = m_canvas->getTruss()->getNodes();
        if (selectedNodes[0] < nodes.size()) {
            auto node = nodes[selectedNodes[0]];
            
            auto supportType = static_cast<truss::core::SupportType>(m_supportCombo->currentIndex());
            node->setSupportType(supportType);
            m_canvas->update();
        }
    }
    emit propertiesChanged();
}

void PropertyPanel::onNodePositionChanged() {
    if (!m_canvas) return;
    
    const auto& selectedNodes = m_canvas->getSelectedNodes();
    if (selectedNodes.size() == 1) {
        const auto& nodes = m_canvas->getTruss()->getNodes();
        if (selectedNodes[0] < nodes.size()) {
            auto node = nodes[selectedNodes[0]];
            
            truss::core::Point2D newPos(m_nodeXSpin->value(), m_nodeYSpin->value());
            node->setPosition(newPos);
            m_canvas->update();
        }
    }
    emit propertiesChanged();
}

//=============================================================================
// InteractiveDrawingWidget Implementation
//=============================================================================

InteractiveDrawingWidget::InteractiveDrawingWidget(QWidget* parent)
    : QWidget(parent) {
    setupUI();
    setupToolbar();
    connectSignals();
}

void InteractiveDrawingWidget::setTruss(std::unique_ptr<truss::core::Truss> truss) {
    m_canvas->setTruss(std::move(truss));
}

void InteractiveDrawingWidget::clearTruss() {
    m_canvas->clearTruss();
}

void InteractiveDrawingWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Create toolbar
    m_toolbar = new QToolBar("Drawing Tools", this);
    layout->addWidget(m_toolbar);
    
    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    layout->addWidget(m_mainSplitter);
    
    // Create canvas
    m_canvas = new DrawingCanvas(this);
    m_mainSplitter->addWidget(m_canvas);
    
    // Create property panel
    m_propertyPanel = new PropertyPanel(this);
    m_propertyPanel->setDrawingCanvas(m_canvas);
    m_mainSplitter->addWidget(m_propertyPanel);
    
    // Set splitter sizes
    m_mainSplitter->setSizes({800, 300});
    m_mainSplitter->setCollapsible(1, false);
    
    // Status bar
    auto* statusLayout = new QHBoxLayout();
    m_coordinateLabel = new QLabel("X: 0.000 m, Y: 0.000 m");
    m_statusLabel = new QLabel("Ready");
    statusLayout->addWidget(m_coordinateLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_statusLabel);
    layout->addLayout(statusLayout);
}

void InteractiveDrawingWidget::setupToolbar() {
    // Drawing mode actions
    m_drawingModeActions = new QActionGroup(this);
    
    m_selectAction = new QAction("Select", this);
    m_selectAction->setCheckable(true);
    m_selectAction->setChecked(true);
    m_selectAction->setToolTip("Select and modify elements");
    m_drawingModeActions->addAction(m_selectAction);
    m_toolbar->addAction(m_selectAction);
    
    m_addNodeAction = new QAction("Add Node", this);
    m_addNodeAction->setCheckable(true);
    m_addNodeAction->setToolTip("Click to add nodes");
    m_drawingModeActions->addAction(m_addNodeAction);
    m_toolbar->addAction(m_addNodeAction);
    
    m_addMemberAction = new QAction("Add Member", this);
    m_addMemberAction->setCheckable(true);
    m_addMemberAction->setToolTip("Click and drag between nodes to add members");
    m_drawingModeActions->addAction(m_addMemberAction);
    m_toolbar->addAction(m_addMemberAction);
    
    m_addLoadAction = new QAction("Add Load", this);
    m_addLoadAction->setCheckable(true);
    m_addLoadAction->setToolTip("Click nodes to apply loads");
    m_drawingModeActions->addAction(m_addLoadAction);
    m_toolbar->addAction(m_addLoadAction);
    
    m_setSupportAction = new QAction("Set Support", this);
    m_setSupportAction->setCheckable(true);
    m_setSupportAction->setToolTip("Click nodes to set support conditions");
    m_drawingModeActions->addAction(m_setSupportAction);
    m_toolbar->addAction(m_setSupportAction);
    
    m_toolbar->addSeparator();
    
    // View actions
    m_zoomInAction = new QAction("Zoom In", this);
    m_zoomInAction->setToolTip("Zoom in");
    m_toolbar->addAction(m_zoomInAction);
    
    m_zoomOutAction = new QAction("Zoom Out", this);
    m_zoomOutAction->setToolTip("Zoom out");
    m_toolbar->addAction(m_zoomOutAction);
    
    m_zoomFitAction = new QAction("Zoom Fit", this);
    m_zoomFitAction->setToolTip("Zoom to fit all elements");
    m_toolbar->addAction(m_zoomFitAction);
    
    m_resetViewAction = new QAction("Reset View", this);
    m_resetViewAction->setToolTip("Reset view to origin");
    m_toolbar->addAction(m_resetViewAction);
    
    m_toolbar->addSeparator();
    
    // Grid controls
    m_gridCheckBox = new QCheckBox("Grid");
    m_gridCheckBox->setChecked(true);
    m_gridCheckBox->setToolTip("Show/hide grid");
    m_toolbar->addWidget(m_gridCheckBox);
    
    m_snapCheckBox = new QCheckBox("Snap");
    m_snapCheckBox->setChecked(true);
    m_snapCheckBox->setToolTip("Snap to grid");
    m_toolbar->addWidget(m_snapCheckBox);
    
    m_toolbar->addWidget(new QLabel("Grid:"));
    m_gridSpacingSpin = new QDoubleSpinBox();
    m_gridSpacingSpin->setRange(0.1, 10.0);
    m_gridSpacingSpin->setSingleStep(0.5);
    m_gridSpacingSpin->setValue(1.0);
    m_gridSpacingSpin->setSuffix(" m");
    m_gridSpacingSpin->setMaximumWidth(80);
    m_toolbar->addWidget(m_gridSpacingSpin);
}

void InteractiveDrawingWidget::connectSignals() {
    // Drawing mode signals
    connect(m_selectAction, &QAction::triggered, this, &InteractiveDrawingWidget::onDrawingModeChanged);
    connect(m_addNodeAction, &QAction::triggered, this, &InteractiveDrawingWidget::onDrawingModeChanged);
    connect(m_addMemberAction, &QAction::triggered, this, &InteractiveDrawingWidget::onDrawingModeChanged);
    connect(m_addLoadAction, &QAction::triggered, this, &InteractiveDrawingWidget::onDrawingModeChanged);
    connect(m_setSupportAction, &QAction::triggered, this, &InteractiveDrawingWidget::onDrawingModeChanged);
    
    // View signals
    connect(m_zoomInAction, &QAction::triggered, m_canvas, &DrawingCanvas::zoomIn);
    connect(m_zoomOutAction, &QAction::triggered, m_canvas, &DrawingCanvas::zoomOut);
    connect(m_zoomFitAction, &QAction::triggered, m_canvas, &DrawingCanvas::zoomToFit);
    connect(m_resetViewAction, &QAction::triggered, m_canvas, &DrawingCanvas::resetView);
    
    // Grid signals
    connect(m_gridCheckBox, &QCheckBox::toggled, this, &InteractiveDrawingWidget::onGridToggled);
    connect(m_snapCheckBox, &QCheckBox::toggled, this, &InteractiveDrawingWidget::onSnapToggled);
    connect(m_gridSpacingSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            m_canvas, &DrawingCanvas::setGridSpacing);
    
    // Canvas signals
    connect(m_canvas, &DrawingCanvas::coordinatesChanged, [this](double x, double y) {
        m_coordinateLabel->setText(QString("X: %1 m, Y: %2 m").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
    });
    
    connect(m_canvas, &DrawingCanvas::statusMessage, [this](const QString& message) {
        m_statusLabel->setText(message);
    });
    
    connect(m_canvas, &DrawingCanvas::trussModified, this, &InteractiveDrawingWidget::trussModified);
    connect(m_canvas, &DrawingCanvas::statusMessage, this, &InteractiveDrawingWidget::statusMessage);
}

void InteractiveDrawingWidget::onDrawingModeChanged() {
    if (m_selectAction->isChecked()) {
        m_canvas->setDrawingMode(DrawingMode::Select);
    } else if (m_addNodeAction->isChecked()) {
        m_canvas->setDrawingMode(DrawingMode::AddNode);
    } else if (m_addMemberAction->isChecked()) {
        m_canvas->setDrawingMode(DrawingMode::AddMember);
    } else if (m_addLoadAction->isChecked()) {
        m_canvas->setDrawingMode(DrawingMode::AddLoad);
    } else if (m_setSupportAction->isChecked()) {
        m_canvas->setDrawingMode(DrawingMode::SetSupport);
    }
}

void InteractiveDrawingWidget::onViewChanged() {
    // This can be used for any view-related updates
}

void InteractiveDrawingWidget::onGridToggled() {
    m_canvas->setGridVisible(m_gridCheckBox->isChecked());
}

void InteractiveDrawingWidget::onSnapToggled() {
    m_canvas->setSnapToGrid(m_snapCheckBox->isChecked());
}

} // namespace truss::gui
