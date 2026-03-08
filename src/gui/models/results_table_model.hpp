/**
 * @file results_table_model.hpp
 * @brief Qt Item Model providing scalar analysis system summary data.
 *
 * Two-column (Property / Value) table model backed by IAnalysisResultsView.
 * Rows are populated via refresh(view) after a successful analysis.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#pragma once

#include "core/interfaces/ianalysis_results_view.hpp"

#include <QAbstractTableModel>
#include <QString>

#include <utility>
#include <vector>

namespace truss::gui::model {

using truss::core::interfaces::IAnalysisResultsView;

/**
 * @brief Two-column (Property / Value) table model for the system summary tab.
 *
 * Rows populated by refresh():
 *  Converged        | Yes / No
 *  Iterations       | n
 *  Residual Norm    | scientific
 *  Condition Number | g-format
 *  Max Displacement | value [mm]
 *  Max Stress       | value [MPa]
 *  Total Strain Energy | value [J]
 *  Total DOFs       | n
 *  Free DOFs        | n
 *  Constrained DOFs | n
 *
 * Thread safety: all methods must be called from the GUI thread.
 */
class ResultsTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ResultsTableModel(QObject* parent = nullptr);
    ~ResultsTableModel() override;

    // -----------------------------------------------------------------------
    // QAbstractTableModel interface
    // -----------------------------------------------------------------------
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /// Fixed column count: Key | Value
    static constexpr int kColumnCount = 2;
    static constexpr int kColKey = 0;
    static constexpr int kColValue = 1;

public Q_SLOTS:
    /**
     * @brief Populate rows from @p view.
     *
     * Calls beginResetModel/endResetModel; connected QTableView updates
     * automatically. Safe to call with the same view after re-analysis.
     */
    void refresh(const IAnalysisResultsView& view);

    /**
     * @brief Clear all rows.
     */
    void clear();

private:
    /// Each row is a (key, value) string pair.
    std::vector<std::pair<QString, QString>> m_rows;
};

}  // namespace truss::gui::model
