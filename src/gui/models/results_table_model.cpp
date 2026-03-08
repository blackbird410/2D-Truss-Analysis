/**
 * @file results_table_model.cpp
 * @brief ResultsTableModel — Qt Item Model for analysis system summary data.
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

ResultsTableModel::ResultsTableModel(QObject* parent) : QAbstractTableModel(parent) {}

ResultsTableModel::~ResultsTableModel() = default;

// ---------------------------------------------------------------------------
// QAbstractTableModel overrides
// ---------------------------------------------------------------------------

int ResultsTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_rows.size());
}

int ResultsTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return kColumnCount;
}

QVariant ResultsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (section) {
        case kColKey:
            return QStringLiteral("Property");
        case kColValue:
            return QStringLiteral("Value");
        default:
            return {};
    }
}

QVariant ResultsTableModel::data(const QModelIndex& index, int role) const {
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

void ResultsTableModel::refresh(const IAnalysisResultsView& view) {
    beginResetModel();
    m_rows.clear();

    auto add = [this](const char* key, const QString& val) {
        m_rows.push_back({QString::fromUtf8(key), val});
    };

    // ---- Solver status ----
    add("Converged",
        view.hasConverged() ? QStringLiteral("Yes \u2713") : QStringLiteral("No \u2717"));
    add("Iterations", QString::number(view.getIterations()));
    add("Residual Norm", QString::number(view.getResidualNorm(), 'e', 4));
    add("Condition Number", QString::number(view.getConditionNumber(), 'g', 6));

    // ---- Peak response ----
    add("Max Displacement",
        QStringLiteral("%1 mm").arg(view.getMaxDisplacement() * 1000.0, 0, 'f', 4));
    add("Max Stress", QStringLiteral("%1 MPa").arg(view.getMaxStress() / 1.0e6, 0, 'f', 3));
    add("Total Strain Energy", QStringLiteral("%1 J").arg(view.getTotalStrain(), 0, 'g', 6));

    // ---- Degrees of freedom ----
    add("Total DOFs", QString::number(static_cast<long long>(view.getTotalDofs())));
    add("Free DOFs", QString::number(static_cast<long long>(view.getFreeDofs())));
    add("Constrained DOFs", QString::number(static_cast<long long>(view.getConstrainedDofs())));

    endResetModel();
}

void ResultsTableModel::clear() {
    if (m_rows.empty())
        return;
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

}  // namespace truss::gui::model
