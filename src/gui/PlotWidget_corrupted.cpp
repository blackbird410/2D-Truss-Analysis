/**
 * @file PlotWidget.cpp
 * @brief Implementation of the plot widget
 */

#include "MainWindow.hpp"
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtCore/QTimer>
#include <cmath>

namespace truss::gui {

PlotWidget::PlotWidget(QWidget *parent)
    : QWidget(parent),
      m_showDeformed(false),
      m_showForces(false),
      m_scaleFactor(1.0),
      m_minBounds(0.0, 0.0),
      m_maxBounds(1.0, 1.0) {
    
    setMinimumSize(400, 300);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PlotWidget::updatePlot() {
    calculateViewport();
    update(); // Triggers paintEvent
}

void PlotWidget::clearPlot() {
    m_minBounds = truss::core::Point2D{0.0, 0.0};
    m_maxBounds = truss::core::Point2D{1.0, 1.0};
    m_scaleFactor = 1.0;
    update();
}

void PlotWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QBrush(Qt::white));
    
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow-getTruss()) {
        // Draw empty state message
        painter.setPen(QPen(Qt::gray));
        painter.drawText(rect(), Qt::AlignCenter, No truss data to display);
        return;
    }
    
    calculateViewport();
    drawTruss(painter);
}

void PlotWidget::drawTruss(QPainter &painter) {
    drawMembers(painter);
    drawNodes(painter);
    drawSupports(painter);
    drawLoads(painter);
    
    if (m_showDeformed) {
        drawDeformedShape(painter);
    }
}

void PlotWidget::drawNodes(QPainter &painter) {
    MainWindow* mainWindow = qobject_castMainWindow*(window());
    if (!mainWindow || !mainWindow-getTruss()) return;
    
    const auto nodes = mainWindow-getTruss()-getNodes();
    
    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(QBrush(Qt::blue));
    
    for (size_t i = 0; i  nodes.size();  i) {
        const auto node = nodes[i];
        QPoint screenPos = worldToScreen(node-getPosition());
        
        // Draw node as circle
        painter.drawEllipse(screenPos, 4, 4);
        
        // Draw node ID
        painter.setPen(QPen(Qt::black));
        painter.drawText(screenPos.AddDays  QPoint(8, -8), QString::number(node-getId()));
        painter.setPen(QPen(Qt::blue, 2));
    }
}

void PlotWidget::drawMembers(QPainter &painter) {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow-getTruss()) return;
    
    const auto members = mainWindow-getTruss()-getMembers();
    const auto nodes = mainWindow-getTruss()-getNodes();
    
    painter.setPen(QPen(Qt::black, 2));
    
    for (size_t i = 0; i  members.size();  i) {
        const auto member = members[i];
        auto startNode = member-getStartNode();
        auto endNode = member-getEndNode();
        
        if (startNode  endNode) {
            QPoint startPos = worldToScreen(startNode-getPosition());
            QPoint endPos = worldToScreen(endNode-getPosition());
            
            painter.drawLine(startPos, endPos);
            
            // Draw member ID at midpoint
            QPoint midPoint = (startPos + endPos) / 2;
            painter.setPen(QPen(Qt::darkGray));
            painter.drawText(midPoint + QPoint(5, 5), QString::number(member->getId()));
            painter.setPen(QPen(Qt::black, 2));
        }
    }
}

void PlotWidget::drawLoads(QPainter &painter) {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->getTruss()) return;
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    
    painter.setPen(QPen(Qt::red, 2));
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (!node->hasAppliedForce()) continue;
        
        QPoint nodePos = worldToScreen(node->getPosition());
        
        // Calculate force vector for display (scaled)
        double forceScale = 50.0; // Pixels per unit force (adjust as needed)
        truss::core::Force2D force = node->getAppliedForce();
        double magnitude = std::sqrt(force.fx*force.fx + force.fy*force.fy);
        
        if (magnitude > 0) {
            QPoint forceVec(
                static_cast<int>(force.fx * forceScale / magnitude * std::min(50.0, magnitude)),
                static_cast<int>(-force.fy * forceScale / magnitude * std::min(50.0, magnitude)) // Negative Y for screen coords
            );
            
            QPoint arrowEnd = nodePos + forceVec;
            
            // Draw force vector
            painter.drawLine(nodePos, arrowEnd);
            
            // Draw arrowhead
            QPoint arrowHead1 = arrowEnd + QPoint(-5, -3);
            QPoint arrowHead2 = arrowEnd + QPoint(-5, 3);
            painter.drawLine(arrowEnd, arrowHead1);
            painter.drawLine(arrowEnd, arrowHead2);
            
            // Draw force magnitude
            painter.setPen(QPen(Qt::red));
            painter.drawText(arrowEnd + QPoint(5, 5), 
                QString("F=%1N").arg(magnitude, 0, 'f', 1));
            painter.setPen(QPen(Qt::red, 2));
        }
    }
}

void PlotWidget::drawSupports(QPainter &painter) {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->getTruss()) return;
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    
    painter.setPen(QPen(Qt::darkGreen, 2));
    painter.setBrush(QBrush(Qt::darkGreen));
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (node->getSupportType() == truss::core::SupportType::Free) continue;
        
        QPoint nodePos = worldToScreen(node->getPosition());
        
        switch (node->getSupportType()) {
            case truss::core::SupportType::Pinned:
                // Draw triangle below node
                painter.drawPolygon(QPolygon({
                    nodePos + QPoint(-8, 8),
                    nodePos + QPoint(8, 8),
                    nodePos + QPoint(0, 0)
                }));
                break;
                
            case truss::core::SupportType::RollerX:
                // Draw circle below node
                painter.drawEllipse(nodePos + QPoint(-6, 2), 12, 12);
                break;
                
            case truss::core::SupportType::RollerY:
                // Draw circle to the left of node
                painter.drawEllipse(nodePos + QPoint(-14, -6), 12, 12);
                break;
                
            default:
                break;
        }
    }
}

void PlotWidget::drawDeformedShape(QPainter &painter) {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->hasResults()) return;
    
    // This would draw the deformed shape if analysis results are available
    // For now, just draw a placeholder message
    painter.setPen(QPen(Qt::magenta));
    painter.drawText(10, height() - 10, "Deformed shape visualization not yet implemented");
}

QPoint PlotWidget::worldToScreen(const truss::core::Point2D& point) const {
    if (m_plotArea.isEmpty()) {
        return QPoint(static_cast<int>(point.x), static_cast<int>(point.y));
    }
    
    double rangeX = m_maxBounds.x - m_minBounds.x;
    double rangeY = m_maxBounds.y - m_minBounds.y;
    
    if (rangeX <= 0) rangeX = 1.0;
    if (rangeY <= 0) rangeY = 1.0;
    
    // Add margins
    double margin = 0.1;
    double marginX = rangeX * margin;
    double marginY = rangeY * margin;
    
    double scaleX = m_plotArea.width() / (rangeX + 2 * marginX);
    double scaleY = m_plotArea.height() / (rangeY + 2 * marginY);
    
    // Use same scale for both axes to maintain aspect ratio
    double scale = std::min(scaleX, scaleY);
    
    int screenX = static_cast<int>(m_plotArea.left() + (point.x - m_minBounds.x + marginX) * scale);
    int screenY = static_cast<int>(m_plotArea.bottom() - (point.y - m_minBounds.y + marginY) * scale);
    
    return QPoint(screenX, screenY);
}

void PlotWidget::calculateViewport() {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow || !mainWindow->getTruss()) {
        m_plotArea = rect();
        return;
    }
    
    const auto& nodes = mainWindow->getTruss()->getNodes();
    if (nodes.empty()) {
        m_plotArea = rect();
        return;
    }
    
    // Calculate bounding box
    m_minBounds = nodes[0]->getPosition();
    m_maxBounds = nodes[0]->getPosition();
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        const auto& pos = node->getPosition();
        m_minBounds.x = std::min(m_minBounds.x, pos.x);
        m_minBounds.y = std::min(m_minBounds.y, pos.y);
        m_maxBounds.x = std::max(m_maxBounds.x, pos.x);
        m_maxBounds.y = std::max(m_maxBounds.y, pos.y);
    }
    
    // Set plot area (leave space for margins and controls)
    int margin = 20;
    m_plotArea = rect().adjusted(margin, margin, -margin, -margin);
}

} // namespace truss::gui
