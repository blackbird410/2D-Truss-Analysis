/**
 * @file results_table_model.hpp
 * @brief Qt Item Model providing scalar analysis system summary data.
 *
 * Phase 1 stub — minimal compilable declaration only.
 * Full implementation in Phase 6 (State Management & Interaction Logic).
 *
 * @note Q_OBJECT is added in Phase 6.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QAbstractTableModel>

namespace truss::core::interfaces { class IAnalysisResultsView; }

namespace truss::gui::model {

/**
 * @brief Two-column (key / value) table model for the system summary tab.
 *
 * Rows (Phase 6):
 *  Converged          Yes / No
 *  Iterations         n
 *  Condition Number   formatted real
 *  Max Displacement   value [m]
 *  Max Stress         value [MPa]
 *  Free DOFs          n
 *  Constrained DOFs   n
 *  Total Strain       value [J]
 *
 * Data is lazily populated — rowCount returns 0 until
 * refresh(IAnalysisResultsView&) is called.
 *
 * @todo Phase 6: Add Q_OBJECT macro, implement all model overrides,
 *       add refresh(const core::interfaces::IAnalysisResultsView& view) slot.
 */
class ResultsTableModel : public QAbstractTableModel {
public:
    explicit ResultsTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override {
        Q_UNUSED(parent)
        return 0;
    }

    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override {
        Q_UNUSED(parent)
        return 2;  // Key | Value
    }

    [[nodiscard]] QVariant data(const QModelIndex& /*index*/,
                                int /*role*/ = Qt::DisplayRole) const override {
        return {};
    }

    // TODO Phase 6: void refresh(const core::interfaces::IAnalysisResultsView& view)
};

}  // namespace truss::gui::model
