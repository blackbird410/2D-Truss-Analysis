/**
 * @file node_table_model.cpp
 * @brief Implementation of NodeTableModel.
 *
 * Phase 3: Qt Item Models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/node_table_model.hpp"

#include <QColor>
#include <QLocale>
#include <QString>

#include <cmath>

namespace truss::gui::model {

// ---------------------------------------------------------------------------
// Static member initialisation
// ---------------------------------------------------------------------------

// Amber (Material Design A400) at ~35 % opacity — subtle row tint
const QColor NodeTableModel::kLoadedBg = QColor(255, 193, 7, 90);

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

NodeTableModel::NodeTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

NodeTableModel::~NodeTableModel() = default;

// ---------------------------------------------------------------------------
// QAbstractTableModel overrides
// ---------------------------------------------------------------------------

int NodeTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_rows.size());
}

int NodeTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return kColumnCount;
}

QVariant NodeTableModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Vertical)
        return section + 1;

    static const char* kHeaders[kColumnCount] = {
        "ID",      "X [m]",   "Y [m]",   "Support",
        "Fx [kN]", "Fy [kN]", "dx [mm]", "dy [mm]",
        "Rx [kN]", "Ry [kN]",
    };

    if (section < 0 || section >= kColumnCount)
        return {};
    return kHeaders[section];
}

QVariant NodeTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= static_cast<int>(m_rows.size()))
        return {};
    if (col < 0 || col >= kColumnCount)
        return {};

    const NodeView& n = m_rows[static_cast<size_t>(row)];

    // ------------------------------------------------------------------
    // TextAlignmentRole
    // ------------------------------------------------------------------
    if (role == Qt::TextAlignmentRole) {
        if (col == kColId || col == kColSupport)
            return static_cast<int>(Qt::AlignCenter);
        return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }

    // ------------------------------------------------------------------
    // BackgroundRole — amber tint for loaded nodes
    // ------------------------------------------------------------------
    if (role == Qt::BackgroundRole) {
        if (isLoaded(n))
            return kLoadedBg;
        return {};
    }

    // ------------------------------------------------------------------
    // ToolTipRole — support description (col 3) + numeric precision elsewhere
    // ------------------------------------------------------------------
    if (role == Qt::ToolTipRole) {
        if (col == kColSupport)
            return formatSupport(n.support);
        return {};
    }

    // ------------------------------------------------------------------
    // DisplayRole
    // ------------------------------------------------------------------
    if (role != Qt::DisplayRole)
        return {};

    // Results columns: show "—" when no analysis has been run
    const bool isResultsCol = (col >= kColDx && col <= kColRy);
    if (isResultsCol && !m_hasResults)
        return QStringLiteral("—");

    switch (col) {
        case kColId:
            return QString::number(n.id);

        case kColX:
            return QString::number(n.x, 'f', 4);

        case kColY:
            return QString::number(n.y, 'f', 4);

        case kColSupport:
            return formatSupport(n.support);

        case kColFx:
            // Convert N → kN
            return QString::number(n.fx / 1000.0, 'f', 3);

        case kColFy:
            return QString::number(n.fy / 1000.0, 'f', 3);

        case kColDx:
            // Convert m → mm for readability
            return QString::number(n.dx * 1000.0, 'f', 4);

        case kColDy:
            return QString::number(n.dy * 1000.0, 'f', 4);

        case kColRx:
            return QString::number(n.rx / 1000.0, 'f', 3);

        case kColRy:
            return QString::number(n.ry / 1000.0, 'f', 3);

        default:
            return {};
    }
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void NodeTableModel::refresh(const ITrussView& view)
{
    beginResetModel();
    m_rows = view.getNodeViews();
    endResetModel();
}

void NodeTableModel::setHasResults(bool hasResults)
{
    if (m_hasResults == hasResults)
        return;
    beginResetModel();
    m_hasResults = hasResults;
    endResetModel();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

QString NodeTableModel::formatSupport(truss::core::SupportType s) const
{
    switch (s) {
        case truss::core::SupportType::Free:    return QStringLiteral("Free");
        case truss::core::SupportType::Pinned:  return QStringLiteral("Pinned (X+Y)");
        case truss::core::SupportType::RollerX: return QStringLiteral("Roller X");
        case truss::core::SupportType::RollerY: return QStringLiteral("Roller Y");
        default:                                return QStringLiteral("Unknown");
    }
}

bool NodeTableModel::isLoaded(const NodeView& n) const
{
    constexpr double kEpsilon = 1e-10;
    return (std::abs(n.fx) > kEpsilon) || (std::abs(n.fy) > kEpsilon);
}

} // namespace truss::gui::model
