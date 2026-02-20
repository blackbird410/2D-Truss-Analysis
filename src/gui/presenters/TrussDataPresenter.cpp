#include "TrussDataPresenter.hpp"
#include <sstream>

namespace truss_presenters {

TrussDataPresenter::StatusData 
TrussDataPresenter::formatStatus(const truss::core::interfaces::ITrussView& truss) const {
    StatusData data;
    
    // Status message
    data.statusMessage = QString("%1, %2")
        .arg(formatNodeCount(truss.getNodeCount()))
        .arg(formatMemberCount(truss.getMemberCount()));
    
    // Statistics
    data.statisticsText = generateStatistics(truss);
    
    return data;
}

QString TrussDataPresenter::formatCoordinate(
    const truss::core::Point2D& point,
    const QString& unit) const 
{
    return QString("(%1, %2) %3")
        .arg(point.x, 0, 'f', COORDINATE_PRECISION)
        .arg(point.y, 0, 'f', COORDINATE_PRECISION)
        .arg(unit);
}

QString TrussDataPresenter::formatNodeCount(size_t count) const {
    return QString("%1 node%2").arg(count).arg(count == 1 ? "" : "s");
}

QString TrussDataPresenter::formatMemberCount(size_t count) const {
    return QString("%1 member%2").arg(count).arg(count == 1 ? "" : "s");
}

QString TrussDataPresenter::generateStatistics(
    const truss::core::interfaces::ITrussView& truss) const 
{
    size_t supportCount = 0;
    size_t loadCount = 0;
    
    // Get node views
    auto nodeViews = truss.getNodeViews();
    
    // Count supports and loads
    for (const auto& nodeView : nodeViews) {
        if (nodeView.support != truss::core::SupportType::Free) {
            ++supportCount;
        }
        if (nodeView.fx != 0.0 || nodeView.fy != 0.0) {
            ++loadCount;
        }
    }
    
    return QString("Nodes: %1\n"
                   "Members: %2\n"
                   "Supports: %3\n"
                   "Loads: %4")
        .arg(truss.getNodeCount())
        .arg(truss.getMemberCount())
        .arg(supportCount)
        .arg(loadCount);
}

QString TrussDataPresenter::formatNodeInfo(
    const truss::core::interfaces::NodeView& nodeView) const 
{
    truss::core::Point2D pos{nodeView.x, nodeView.y};
    QString info = QString("Node %1: %2")
        .arg(nodeView.id)
        .arg(formatCoordinate(pos));
    
    // Add support type if not free
    if (nodeView.support != truss::core::SupportType::Free) {
        info += QString("\n  Support: %1").arg(formatSupportType(nodeView.support));
    }
    
    // Add load if present
    if (nodeView.fx != 0.0 || nodeView.fy != 0.0) {
        info += QString("\n  Load: (%1, %2) N")
            .arg(nodeView.fx, 0, 'f', 1)
            .arg(nodeView.fy, 0, 'f', 1);
    }
    
    return info;
}

QString TrussDataPresenter::formatMemberInfo(
    const truss::core::interfaces::MemberView& memberView) const 
{
    return QString("Member %1: Nodes [%2 - %3], Length: %4")
        .arg(memberView.id)
        .arg(memberView.startNodeId)
        .arg(memberView.endNodeId)
        .arg(formatLength(memberView.length));
}

QString TrussDataPresenter::formatSupportType(truss::core::SupportType supportType) const {
    switch (supportType) {
        case truss::core::SupportType::Free:
            return "Free";
        case truss::core::SupportType::Pinned:
            return "Pinned";
        case truss::core::SupportType::RollerX:
            return "RollerX";
        case truss::core::SupportType::RollerY:
            return "RollerY";
        default:
            return "Unknown";
    }
}

QString TrussDataPresenter::formatLength(double lengthMeters) const {
    return QString("%1 m").arg(lengthMeters, 0, 'f', LENGTH_PRECISION);
}

QString TrussDataPresenter::formatSupportChangeMessage(
    truss::core::NodeId nodeId,
    truss::core::SupportType supportType) const 
{
    return QString("Node %1 support changed to %2")
        .arg(nodeId)
        .arg(formatSupportType(supportType));
}

QString TrussDataPresenter::formatNodeAddedMessage(
    truss::core::NodeId nodeId,
    const truss::core::Point2D& position) const 
{
    return QString("Node %1 added at (%2, %3)")
        .arg(nodeId)
        .arg(position.x, 0, 'f', COORDINATE_PRECISION)
        .arg(position.y, 0, 'f', COORDINATE_PRECISION);
}

QString TrussDataPresenter::formatMemberAddedMessage(
    truss::core::MemberId memberId,
    truss::core::NodeId startNodeId,
    truss::core::NodeId endNodeId) const 
{
    return QString("Member %1 added (Nodes %2 - %3)")
        .arg(memberId)
        .arg(startNodeId)
        .arg(endNodeId);
}

QString TrussDataPresenter::formatLoadAppliedMessage(
    truss::core::NodeId nodeId,
    const truss::core::Force2D& force) const 
{
    return QString("Load applied to node %1: (%2, %3) N")
        .arg(nodeId)
        .arg(force.fx, 0, 'f', FORCE_PRECISION)
        .arg(force.fy, 0, 'f', FORCE_PRECISION);
}

}  // namespace truss_presenters
