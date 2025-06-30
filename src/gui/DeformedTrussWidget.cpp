/**
 * @file DeformedTrussWidget.cpp
 * @brief Implementation of deformed truss visualization widget
 * @version 2.1.0
 */

#include "DeformedTrussWidget.hpp"
#include <QPaintEvent>
#include <QApplication>
#include <cmath>

namespace truss::gui {

DeformedTrussWidget::DeformedTrussWidget(QWidget* parent)
    : QWidget(parent),
      m_truss(nullptr),
      m_hasResults(false),
      m_viewCenter(0, 0),
      m_viewScale(1.0),
      m_panning(false),
      m_deformationScale(DEFAULT_DEFORMATION_SCALE),
      m_showOriginal(true),
      m_showDeformed(true),
      m_showMemberForces(true),
      m_showDisplacements(false),
      m_showReactions(false),
      m_forceVisualizationMode(0),
      m_maxDisplacement(0.0),
      m_maxForce(0.0) {
    
    setMinimumSize(400, 300);
    setMouseTracking(true);
    setupControls();
}

void DeformedTrussWidget::setupControls() {
    // Create control panel layout
    auto* controlLayout = new QVBoxLayout();
    
    // Deformation scale control
    auto* scaleGroup = new QGroupBox("Deformation Scale", this);
    auto* scaleLayout = new QVBoxLayout(scaleGroup);
    
    m_scaleSlider = new QSlider(Qt::Horizontal, this);
    m_scaleSlider->setRange(static_cast<int>(MIN_SCALE * 10), static_cast<int>(MAX_SCALE * 10));
    m_scaleSlider->setValue(static_cast<int>(DEFAULT_DEFORMATION_SCALE * 10));
    
    m_scaleLabel = new QLabel(QString("Scale: %1x").arg(DEFAULT_DEFORMATION_SCALE, 0, 'f', 1), this);
    
    scaleLayout->addWidget(m_scaleLabel);
    scaleLayout->addWidget(m_scaleSlider);
    
    connect(m_scaleSlider, &QSlider::valueChanged, this, [this](int value) {
        double scale = static_cast<double>(value) / 10.0;
        setDeformationScale(scale);
        m_scaleLabel->setText(QString("Scale: %1x").arg(scale, 0, 'f', 1));
    });
    
    // Display options
    auto* displayGroup = new QGroupBox("Display Options", this);
    auto* displayLayout = new QVBoxLayout(displayGroup);
    
    m_originalCheckBox = new QCheckBox("Show Original Structure", this);
    m_originalCheckBox->setChecked(m_showOriginal);
    connect(m_originalCheckBox, &QCheckBox::toggled, this, &DeformedTrussWidget::showOriginal);
    
    m_deformedCheckBox = new QCheckBox("Show Deformed Structure", this);
    m_deformedCheckBox->setChecked(m_showDeformed);
    connect(m_deformedCheckBox, &QCheckBox::toggled, this, &DeformedTrussWidget::showDeformed);
    
    m_forcesCheckBox = new QCheckBox("Show Member Forces", this);
    m_forcesCheckBox->setChecked(m_showMemberForces);
    connect(m_forcesCheckBox, &QCheckBox::toggled, this, &DeformedTrussWidget::showMemberForces);
    
    m_displacementsCheckBox = new QCheckBox("Show Displacements", this);
    m_displacementsCheckBox->setChecked(m_showDisplacements);
    connect(m_displacementsCheckBox, &QCheckBox::toggled, this, &DeformedTrussWidget::showDisplacements);
    
    m_reactionsCheckBox = new QCheckBox("Show Reactions", this);
    m_reactionsCheckBox->setChecked(m_showReactions);
    connect(m_reactionsCheckBox, &QCheckBox::toggled, this, &DeformedTrussWidget::showReactions);
    
    // Force visualization mode
    m_forceVisualizationCombo = new QComboBox(this);
    m_forceVisualizationCombo->addItems({"Color Coding", "Line Thickness", "Color + Thickness"});
    connect(m_forceVisualizationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeformedTrussWidget::setForceVisualizationMode);
    
    displayLayout->addWidget(m_originalCheckBox);
    displayLayout->addWidget(m_deformedCheckBox);
    displayLayout->addWidget(m_forcesCheckBox);
    displayLayout->addWidget(m_displacementsCheckBox);
    displayLayout->addWidget(m_reactionsCheckBox);
    displayLayout->addWidget(new QLabel("Force Visualization:"));
    displayLayout->addWidget(m_forceVisualizationCombo);
    
    controlLayout->addWidget(scaleGroup);
    controlLayout->addWidget(displayGroup);
    controlLayout->addStretch();
    
    // Position controls on the right side
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addLayout(controlLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
}

void DeformedTrussWidget::setTruss(truss::core::Truss* truss) {
    m_truss = truss;
    calculateBounds();
    update();
}

void DeformedTrussWidget::setAnalysisResults(const truss::core::AnalysisResults& results) {
    m_results = results;
    m_hasResults = true;
    
    // Calculate maximum displacement and force for scaling
    m_maxDisplacement = 0.0;
    m_maxForce = 0.0;
    
    if (m_truss) {
        for (const auto& node : m_truss->getNodes()) {
            if (node->hasResults()) {
                auto displacement = node->getResults().displacement;
                double totalDisp = std::sqrt(displacement.dx * displacement.dx + displacement.dy * displacement.dy);
                m_maxDisplacement = std::max(m_maxDisplacement, totalDisp);
            }
        }
        
        for (const auto& member : m_truss->getMembers()) {
            if (member->hasResults()) {
                double force = std::abs(member->getResults().axialForce);
                m_maxForce = std::max(m_maxForce, force);
            }
        }
    }
    
    calculateBounds();
    resetView();
    update();
}

void DeformedTrussWidget::clear() {
    m_truss = nullptr;
    m_hasResults = false;
    m_maxDisplacement = 0.0;
    m_maxForce = 0.0;
    update();
}

void DeformedTrussWidget::resetView() {
    if (!m_truss || m_truss->getNodes().empty()) {
        return;
    }
    
    calculateBounds();
    
    // Center the view
    m_viewCenter = m_structureBounds.center();
    
    // Scale to fit with some padding
    double scaleX = (width() - 50) / m_structureBounds.width();
    double scaleY = (height() - 50) / m_structureBounds.height();
    m_viewScale = std::min(scaleX, scaleY) * 0.8;
    
    if (m_viewScale <= 0) {
        m_viewScale = 1.0;
    }
    
    update();
}

void DeformedTrussWidget::setDeformationScale(double scale) {
    m_deformationScale = std::clamp(scale, MIN_SCALE, MAX_SCALE);
    update();
}

void DeformedTrussWidget::showOriginal(bool show) {
    m_showOriginal = show;
    update();
}

void DeformedTrussWidget::showDeformed(bool show) {
    m_showDeformed = show;
    update();
}

void DeformedTrussWidget::showMemberForces(bool show) {
    m_showMemberForces = show;
    update();
}

void DeformedTrussWidget::showDisplacements(bool show) {
    m_showDisplacements = show;
    update();
}

void DeformedTrussWidget::showReactions(bool show) {
    m_showReactions = show;
    update();
}

void DeformedTrussWidget::setForceVisualizationMode(int mode) {
    m_forceVisualizationMode = mode;
    update();
}

void DeformedTrussWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), Qt::white);
    
    if (!m_truss || !m_hasResults) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "No analysis results to display.\nRun analysis first.");
        return;
    }
    
    // Draw grid
    drawGrid(painter);
    
    // Draw structures in order (original first, then deformed)
    if (m_showOriginal) {
        drawOriginalTruss(painter);
    }
    
    if (m_showDeformed) {
        drawDeformedTruss(painter);
    }
    
    if (m_showMemberForces) {
        drawMemberForces(painter);
    }
    
    if (m_showDisplacements) {
        drawDisplacementVectors(painter);
    }
    
    if (m_showReactions) {
        drawSupportReactions(painter);
    }
    
    // Draw legend
    drawLegend(painter);
}

void DeformedTrussWidget::drawGrid(QPainter& painter) {
    painter.setPen(QPen(QColor(230, 230, 230), 1));
    
    // Calculate grid spacing
    double gridSpacing = 1.0; // meters
    while (gridSpacing * m_viewScale < 20) {
        gridSpacing *= 2;
    }
    while (gridSpacing * m_viewScale > 100) {
        gridSpacing /= 2;
    }
    
    // Draw vertical lines
    for (double x = std::floor(m_structureBounds.left() / gridSpacing) * gridSpacing; 
         x <= m_structureBounds.right(); x += gridSpacing) {
        QPointF top = worldToScreen(QPointF(x, m_structureBounds.top()));
        QPointF bottom = worldToScreen(QPointF(x, m_structureBounds.bottom()));
        painter.drawLine(top, bottom);
    }
    
    // Draw horizontal lines
    for (double y = std::floor(m_structureBounds.top() / gridSpacing) * gridSpacing; 
         y <= m_structureBounds.bottom(); y += gridSpacing) {
        QPointF left = worldToScreen(QPointF(m_structureBounds.left(), y));
        QPointF right = worldToScreen(QPointF(m_structureBounds.right(), y));
        painter.drawLine(left, right);
    }
}

void DeformedTrussWidget::drawOriginalTruss(QPainter& painter) {
    painter.setPen(QPen(QColor(150, 150, 150), 2, Qt::DashLine));
    
    // Draw members
    for (const auto& member : m_truss->getMembers()) {
        QPointF start = worldToScreen(QPointF(member->getStartNode()->getX(), member->getStartNode()->getY()));
        QPointF end = worldToScreen(QPointF(member->getEndNode()->getX(), member->getEndNode()->getY()));
        painter.drawLine(start, end);
    }
    
    // Draw nodes
    painter.setPen(QPen(Qt::gray, 2));
    painter.setBrush(Qt::white);
    
    for (const auto& node : m_truss->getNodes()) {
        QPointF pos = worldToScreen(QPointF(node->getX(), node->getY()));
        painter.drawEllipse(pos, 4, 4);
    }
}

void DeformedTrussWidget::drawDeformedTruss(QPainter& painter) {
    painter.setPen(QPen(Qt::black, 3));
    
    // Draw deformed members
    for (const auto& member : m_truss->getMembers()) {
        QPointF start = getDeformedPosition(member->getStartNode().get());
        QPointF end = getDeformedPosition(member->getEndNode().get());
        painter.drawLine(worldToScreen(start), worldToScreen(end));
    }
    
    // Draw deformed nodes
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::red);
    
    for (const auto& node : m_truss->getNodes()) {
        QPointF pos = worldToScreen(getDeformedPosition(node.get()));
        painter.drawEllipse(pos, 5, 5);
    }
}

void DeformedTrussWidget::drawMemberForces(QPainter& painter) {
    if (m_maxForce <= 0) return;
    
    for (const auto& member : m_truss->getMembers()) {
        if (!member->hasResults()) continue;
        
        double force = member->getResults().axialForce;
        QColor color = getMemberForceColor(force, m_maxForce);
        
        // Determine line width based on visualization mode
        int lineWidth = 3;
        if (m_forceVisualizationMode == 1 || m_forceVisualizationMode == 2) {
            lineWidth = static_cast<int>(2 + 8 * std::abs(force) / m_maxForce);
        }
        
        // Use color if in color mode
        if (m_forceVisualizationMode == 0 || m_forceVisualizationMode == 2) {
            painter.setPen(QPen(color, lineWidth));
        } else {
            painter.setPen(QPen(Qt::black, lineWidth));
        }
        
        QPointF start = getDeformedPosition(member->getStartNode().get());
        QPointF end = getDeformedPosition(member->getEndNode().get());
        painter.drawLine(worldToScreen(start), worldToScreen(end));
    }
}

void DeformedTrussWidget::drawDisplacementVectors(QPainter& painter) {
    if (m_maxDisplacement <= 0) return;
    
    painter.setPen(QPen(Qt::blue, 2));
    painter.setBrush(Qt::blue);
    
    for (const auto& node : m_truss->getNodes()) {
        if (!node->hasResults()) continue;
        
        auto displacement = node->getResults().displacement;
        if (std::abs(displacement.dx) < 1e-10 && std::abs(displacement.dy) < 1e-10) continue;
        
        QPointF original(node->getX(), node->getY());
        QPointF displaced = getDeformedPosition(node.get());
        
        QPointF start = worldToScreen(original);
        QPointF end = worldToScreen(displaced);
        
        // Draw displacement vector
        painter.drawLine(start, end);
        
        // Draw arrow head
        QPointF direction = end - start;
        double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
        if (length > 5) {
            direction /= length;
            QPointF perpendicular(-direction.y(), direction.x());
            
            QPointF arrowPoint1 = end - direction * 8 + perpendicular * 4;
            QPointF arrowPoint2 = end - direction * 8 - perpendicular * 4;
            
            QPolygonF arrow;
            arrow << end << arrowPoint1 << arrowPoint2;
            painter.drawPolygon(arrow);
        }
    }
}

void DeformedTrussWidget::drawSupportReactions(QPainter& painter) {
    painter.setPen(QPen(Qt::magenta, 3));
    painter.setBrush(Qt::magenta);
    
    for (const auto& node : m_truss->getNodes()) {
        if (!node->hasResults() || node->getSupportType() == truss::core::SupportType::Free) continue;
        
        auto reactions = node->getResults().reactions;
        QPointF pos = worldToScreen(getDeformedPosition(node.get()));
        
        // Draw reaction forces as arrows
        double scale = 50.0; // Pixel scale for reaction display
        
        if (std::abs(reactions.fx) > 1e-6) {
            QPointF end = pos + QPointF(reactions.fx > 0 ? scale : -scale, 0);
            painter.drawLine(pos, end);
            // Arrow head for X reaction
            QPolygonF arrow;
            if (reactions.fx > 0) {
                arrow << end << (end + QPointF(-8, 4)) << (end + QPointF(-8, -4));
            } else {
                arrow << end << (end + QPointF(8, 4)) << (end + QPointF(8, -4));
            }
            painter.drawPolygon(arrow);
        }
        
        if (std::abs(reactions.fy) > 1e-6) {
            QPointF end = pos + QPointF(0, reactions.fy > 0 ? -scale : scale); // Y is inverted in screen coordinates
            painter.drawLine(pos, end);
            // Arrow head for Y reaction
            QPolygonF arrow;
            if (reactions.fy > 0) {
                arrow << end << (end + QPointF(4, 8)) << (end + QPointF(-4, 8));
            } else {
                arrow << end << (end + QPointF(4, -8)) << (end + QPointF(-4, -8));
            }
            painter.drawPolygon(arrow);
        }
    }
}

void DeformedTrussWidget::drawLegend(QPainter& painter) {
    QRect legendRect(width() - LEGEND_WIDTH - 10, 10, LEGEND_WIDTH, LEGEND_HEIGHT);
    
    painter.setPen(Qt::black);
    painter.setBrush(QColor(255, 255, 255, 200));
    painter.drawRect(legendRect);
    
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(legendRect.adjusted(5, 5, -5, -5), Qt::AlignTop | Qt::AlignLeft, "Legend");
    
    int y = legendRect.top() + 25;
    painter.setFont(QFont("Arial", 9));
    
    // Color legend for forces
    if (m_showMemberForces && m_maxForce > 0) {
        painter.drawText(5 + legendRect.left(), y, "Member Forces:");
        y += 15;
        
        // Compression (red)
        painter.setPen(QPen(Qt::red, 3));
        painter.drawLine(legendRect.left() + 5, y, legendRect.left() + 30, y);
        painter.setPen(Qt::black);
        painter.drawText(legendRect.left() + 35, y + 4, "Compression");
        y += 15;
        
        // Tension (blue)
        painter.setPen(QPen(Qt::blue, 3));
        painter.drawLine(legendRect.left() + 5, y, legendRect.left() + 30, y);
        painter.setPen(Qt::black);
        painter.drawText(legendRect.left() + 35, y + 4, "Tension");
        y += 15;
    }
    
    // Scale information
    painter.drawText(legendRect.left() + 5, y, QString("Def. Scale: %1x").arg(m_deformationScale, 0, 'f', 1));
    y += 15;
    
    if (m_maxDisplacement > 0) {
        painter.drawText(legendRect.left() + 5, y, QString("Max Disp: %1 mm").arg(m_maxDisplacement * 1000, 0, 'f', 2));
    }
}

QPointF DeformedTrussWidget::getDeformedPosition(const truss::core::Node* node) const {
    QPointF original(node->getX(), node->getY());
    
    if (!node->hasResults() || m_maxDisplacement <= 0) {
        return original;
    }
    
    auto displacement = node->getResults().displacement;
    
    // Scale displacement for visualization
    double scaleFactor = m_deformationScale;
    if (m_maxDisplacement > 0) {
        // Auto-scale based on structure size
        double structureSize = std::max(m_structureBounds.width(), m_structureBounds.height());
        scaleFactor *= std::min(structureSize * 0.1 / m_maxDisplacement, 100.0);
    }
    
    return QPointF(original.x() + displacement.dx * scaleFactor,
                   original.y() + displacement.dy * scaleFactor);
}

QColor DeformedTrussWidget::getMemberForceColor(double force, double maxForce) const {
    if (maxForce <= 0) return Qt::black;
    
    double ratio = std::abs(force) / maxForce;
    
    if (force > 0) {
        // Tension - blue scale
        int intensity = static_cast<int>(255 * ratio);
        return QColor(0, 0, intensity);
    } else {
        // Compression - red scale
        int intensity = static_cast<int>(255 * ratio);
        return QColor(intensity, 0, 0);
    }
}

void DeformedTrussWidget::calculateBounds() {
    if (!m_truss || m_truss->getNodes().empty()) {
        m_structureBounds = QRectF(-1, -1, 2, 2);
        return;
    }
    
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    
    // Consider both original and deformed positions
    for (const auto& node : m_truss->getNodes()) {
        // Original position
        minX = std::min(minX, node->getX());
        maxX = std::max(maxX, node->getX());
        minY = std::min(minY, node->getY());
        maxY = std::max(maxY, node->getY());
        
        // Deformed position
        if (m_hasResults) {
            QPointF deformed = getDeformedPosition(node.get());
            minX = std::min(minX, deformed.x());
            maxX = std::max(maxX, deformed.x());
            minY = std::min(minY, deformed.y());
            maxY = std::max(maxY, deformed.y());
        }
    }
    
    // Add some padding
    double paddingX = (maxX - minX) * 0.1;
    double paddingY = (maxY - minY) * 0.1;
    
    m_structureBounds = QRectF(minX - paddingX, minY - paddingY, 
                               (maxX - minX) + 2 * paddingX, 
                               (maxY - minY) + 2 * paddingY);
}

QPointF DeformedTrussWidget::worldToScreen(const QPointF& worldPoint) const {
    QPointF translated = worldPoint - m_viewCenter;
    QPointF scaled = translated * m_viewScale;
    QPointF screenPoint = scaled + QPointF(width() / 2.0, height() / 2.0);
    
    // Flip Y coordinate (screen Y increases downward, world Y increases upward)
    screenPoint.setY(height() - screenPoint.y());
    
    return screenPoint;
}

QPointF DeformedTrussWidget::screenToWorld(const QPointF& screenPoint) const {
    // Flip Y coordinate back
    QPointF flipped(screenPoint.x(), height() - screenPoint.y());
    QPointF translated = flipped - QPointF(width() / 2.0, height() / 2.0);
    QPointF scaled = translated / m_viewScale;
    return scaled + m_viewCenter;
}

void DeformedTrussWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void DeformedTrussWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning && (event->buttons() & Qt::LeftButton)) {
        QPointF delta = event->pos() - m_panStart;
        m_viewCenter -= QPointF(delta.x() / m_viewScale, -delta.y() / m_viewScale);
        m_panStart = event->pos();
        update();
    }
}

void DeformedTrussWidget::wheelEvent(QWheelEvent* event) {
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        m_viewScale *= scaleFactor;
    } else {
        m_viewScale /= scaleFactor;
    }
    
    // Limit zoom range
    m_viewScale = std::clamp(m_viewScale, 0.1, 100.0);
    
    update();
}

void DeformedTrussWidget::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event)
    resetView();
}

} // namespace truss::gui

#include "DeformedTrussWidget.moc"
