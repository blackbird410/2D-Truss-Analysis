/**
 * @file TrussDataPresenter.hpp
 * @brief Presenter for formatting truss geometry and statistics for GUI display
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This presenter formats truss structural data into user-friendly display strings:
 * - Node coordinates with precision formatting
 * - Member properties and connections
 * - Support type descriptions
 * - Truss statistics (node count, member count, etc.)
 * - Status messages for user feedback
 * 
 * Architecture: GUI Presentation Layer (MVP Pattern)
 * Dependencies: Domain interfaces (ITrussView), Domain types (Point2D, SupportType)
 */

#pragma once

#include <QString>
#include "core/interfaces/ITrussView.hpp"
#include "core/model/Types.hpp"

namespace truss_presenters {

/**
 * @brief Formats truss data for display in GUI
 * 
 * This presenter handles formatting of truss geometry, statistics,
 * and status information for display in the GUI.
 */
class TrussDataPresenter {
public:
    /**
     * @brief Formatted status data for truss
     */
    struct StatusData {
        QString statusMessage;      // Overall status (e.g., "12 nodes, 18 members")
        QString coordinateText;     // Current coordinate display
        QString statisticsText;     // Detailed statistics
        QString validationSummary;  // Validation status summary
    };
    
    /**
     * @brief Format truss status for display
     * 
     * @param truss Truss view (read-only)
     * @return StatusData Formatted status information
     */
    StatusData formatStatus(const truss::core::interfaces::ITrussView& truss) const;
    
    /**
     * @brief Format coordinate point
     * 
     * @param point 2D point to format
     * @param unit Unit string (default: "m")
     * @return QString Formatted coordinate (e.g., "(1.500, 2.000) m")
     */
    QString formatCoordinate(const truss::core::Point2D& point,
                              const QString& unit = "m") const;
    
    /**
     * @brief Format node count
     * 
     * @param count Number of nodes
     * @return QString Formatted string (e.g., "12 nodes")
     */
    QString formatNodeCount(size_t count) const;
    
    /**
     * @brief Format member count
     * 
     * @param count Number of members
     * @return QString Formatted string (e.g., "18 members")
     */
    QString formatMemberCount(size_t count) const;
    
    /**
     * @brief Generate statistics text
     * 
     * @param truss Truss view
     * @return QString Detailed statistics text
     */
    QString generateStatistics(const truss::core::interfaces::ITrussView& truss) const;
    
    /**
     * @brief Format node information
     * 
     * @param nodeView Node view struct
     * @return QString Formatted node info (ID, position, support, loads)
     */
    QString formatNodeInfo(const truss::core::interfaces::NodeView& nodeView) const;
    
    /**
     * @brief Format member information
     * 
     * @param memberView Member view struct
     * @return QString Formatted member info (ID, nodes, length, properties)
     */
    QString formatMemberInfo(const truss::core::interfaces::MemberView& memberView) const;
    
    /**
     * @brief Format support type
     * 
     * @param supportType Support type enum
     * @return QString Human-readable support type
     */
    QString formatSupportType(truss::core::SupportType supportType) const;
    
    /**
     * @brief Format length value
     * 
     * @param lengthMeters Length in meters
     * @return QString Formatted length (e.g., "2.500 m")
     */
    QString formatLength(double lengthMeters) const;
    
    /**
     * @brief Format support change message for status bar
     * 
     * @param nodeId Node identifier
     * @param supportType New support type
     * @return QString User-friendly status message
     */
    QString formatSupportChangeMessage(truss::core::NodeId nodeId,
                                         truss::core::SupportType supportType) const;
    
    /**
     * @brief Format node added message for status bar
     * 
     * @param nodeId Node identifier
     * @param position Node position
     * @return QString User-friendly status message
     */
    QString formatNodeAddedMessage(truss::core::NodeId nodeId,
                                     const truss::core::Point2D& position) const;
    
    /**
     * @brief Format member added message for status bar
     * 
     * @param memberId Member identifier
     * @param startNodeId Start node
     * @param endNodeId End node
     * @return QString User-friendly status message
     */
    QString formatMemberAddedMessage(truss::core::MemberId memberId,
                                       truss::core::NodeId startNodeId,
                                       truss::core::NodeId endNodeId) const;
    
    /**
     * @brief Format load applied message for status bar
     * 
     * @param nodeId Node identifier
     * @param force Force vector
     * @return QString User-friendly status message
     */
    QString formatLoadAppliedMessage(truss::core::NodeId nodeId,
                                       const truss::core::Force2D& force) const;

private:
    static constexpr int COORDINATE_PRECISION = 3;
    static constexpr int LENGTH_PRECISION = 3;
    static constexpr int FORCE_PRECISION = 1;
};

}  // namespace truss_presenters
