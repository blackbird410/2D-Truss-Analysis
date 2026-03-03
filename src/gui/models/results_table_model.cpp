/**
 * @file results_table_model.cpp
 * @brief Implementation of ResultsTableModel skeleton.
 *
 * Phase 3: Q_OBJECT skeleton — columnCount and headerData are functional.
 * refresh() is a no-op placeholder until Phase 6 (State Management).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/results_table_model.hpp"

namespace truss::gui::model {

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

ResultsTableModel::ResultsTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

ResultsTableModel::~ResultsTableModel() = default;

// ---------------------------------------------------------------------------
// QAbstractTableModel overrides
// ---------------------------------------------------------------------------

int ResultsTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_rows.size());
}

int ResultsTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return kColumnCount;
}

QVariant ResultsTableModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (section) {
        case kColKey:   return QStringLiteral("Property");
        case kColValue: return QStringLiteral("Value");
        default:        return {};
    }
}

QVariant ResultsTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= static_cast<int>(m_rows.size()))
        return {};
    if (col < 0 || col >= kColumnCount)
        return {};

    if (role == Qt::DisplayRole) {
        const auto& pair = m_rows[static_cast<size_t>(row)];
        return (col == kColKey) ? pair.first : pair.second;
    }

    if (role == Qt::TextAlignmentRole) {
        if (col == kColKey)
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {};
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void ResultsTableModel::refresh(const IAnalysisResultsView& /*view*/)
{
    // TODO Phase 6: populate m_rows from view data:
    //   { "Converged",        view.hasConverged() ? "Yes" : "No"   }
    //   { "Iterations",       QString::number(view.getIterations()) }
    //   { "Condition Number", QString::number(view.getConditionNumber(), 'g', 6) }
    //   { "Max Displacement", QString::number(view.getMaxDisplacement()*1000, 'f', 4) + " mm" }
    //   { "Max Stress",       QString::number(view.getMaxStress()/1e6, 'f', 3) + " MPa" }
    //   { "Free DOFs",        QString::number(view.getFreeDofs()) }
    //   { "Constrained DOFs", QString::number(view.getConstrainedDofs()) }
    //   { "Total Strain",     QString::number(view.getTotalStrain(), 'g', 6) + " J" }
}

void ResultsTableModel::clear()
{
    if (m_rows.empty())
        return;
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

} // namespace truss::gui::model
