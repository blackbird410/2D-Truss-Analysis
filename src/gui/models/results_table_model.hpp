/**
 * @file results_table_model.hpp
 * @brief Qt Item Model providing scalar analysis system summary data.
 *
 * Phase 3: Qt Item Models — Q_OBJECT skeleton.
 * Full row population deferred to Phase 6 (State Management & Interaction Logic).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#pragma once

#include "core/interfaces/ianalysis_results_view.hpp"

#include <QAbstractTableModel>
#include <QString>

#include <vector>
#include <utility>

namespace truss::gui::model {

using truss::core::interfaces::IAnalysisResultsView;

/**
 * @brief Two-column (Key / Value) table model for the system summary tab.
 *
 * Phase 3 skeleton — columnCount is 2 and headerData is implemented.
 * rowCount returns 0 until refresh() is called (Phase 6).
 *
 * Planned rows (Phase 6):
 *  Converged        | Yes / No
 *  Iterations       | n
 *  Condition Number | formatted real
 *  Max Displacement | value [mm]
 *  Max Stress       | value [MPa]
 *  Free DOFs        | n
 *  Constrained DOFs | n
 *  Total Strain     | value [J]
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
    [[nodiscard]] int     rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int     columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    /// Fixed column count: Key | Value
    static constexpr int kColumnCount = 2;
    static constexpr int kColKey      = 0;
    static constexpr int kColValue    = 1;

public Q_SLOTS:
    /**
     * @brief Populate rows from @p view (Phase 6 full implementation).
     * Phase 3: clears rows and resets model — body left as TODO for Phase 6.
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

} // namespace truss::gui::model
