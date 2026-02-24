/**
 * @file truss_data_presenter.hpp
 * @brief Presenter that formats truss geometry and status data for GUI display.
 *
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"

#include <QString>

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
    static StatusData formatStatus(const truss::core::interfaces::ITrussView& truss);

    /**
     * @brief Format coordinate point
     *
     * @param point 2D point to format
     * @param unit Unit string (default: "m")
     * @return QString Formatted coordinate (e.g., "(1.500, 2.000) m")
     */
    static QString formatCoordinate(const truss::core::Point2D& point, const QString& unit = "m");

    /**
     * @brief Format node count
     *
     * @param count Number of nodes
     * @return QString Formatted string (e.g., "12 nodes")
     */
    static QString formatNodeCount(size_t count);

    /**
     * @brief Format member count
     *
     * @param count Number of members
     * @return QString Formatted string (e.g., "18 members")
     */
    static QString formatMemberCount(size_t count);

    /**
     * @brief Generate statistics text
     *
     * @param truss Truss view
     * @return QString Detailed statistics text
     */
    static QString generateStatistics(const truss::core::interfaces::ITrussView& truss);

    /**
     * @brief Format node information
     *
     * @param nodeView Node view struct
     * @return QString Formatted node info (ID, position, support, loads)
     */
    static QString formatNodeInfo(const truss::core::interfaces::NodeView& nodeView);

    /**
     * @brief Format member information
     *
     * @param memberView Member view struct
     * @return QString Formatted member info (ID, nodes, length, properties)
     */
    static QString formatMemberInfo(const truss::core::interfaces::MemberView& memberView);

    /**
     * @brief Format support type
     *
     * @param supportType Support type enum
     * @return QString Human-readable support type
     */
    static QString formatSupportType(truss::core::SupportType supportType);

    /**
     * @brief Format length value
     *
     * @param lengthMeters Length in meters
     * @return QString Formatted length (e.g., "2.500 m")
     */
    static QString formatLength(double lengthMeters);

    /**
     * @brief Format support change message for status bar
     *
     * @param nodeId Node identifier
     * @param supportType New support type
     * @return QString User-friendly status message
     */
    static QString formatSupportChangeMessage(truss::core::NodeId nodeId,
                                              truss::core::SupportType supportType);

    /**
     * @brief Format node added message for status bar
     *
     * @param nodeId Node identifier
     * @param position Node position
     * @return QString User-friendly status message
     */
    static QString formatNodeAddedMessage(truss::core::NodeId nodeId,
                                          const truss::core::Point2D& position);

    /**
     * @brief Format member added message for status bar
     *
     * @param memberId Member identifier
     * @param startNodeId Start node
     * @param endNodeId End node
     * @return QString User-friendly status message
     */
    static QString formatMemberAddedMessage(truss::core::MemberId memberId,
                                            truss::core::NodeId startNodeId,
                                            truss::core::NodeId endNodeId);

    /**
     * @brief Format load applied message for status bar
     *
     * @param nodeId Node identifier
     * @param force Force vector
     * @return QString User-friendly status message
     */
    static QString formatLoadAppliedMessage(truss::core::NodeId nodeId,
                                            const truss::core::Force2D& force);

private:
    static constexpr int COORDINATE_PRECISION = 3;
    static constexpr int LENGTH_PRECISION = 3;
    static constexpr int FORCE_PRECISION = 1;
};

}  // namespace truss_presenters
