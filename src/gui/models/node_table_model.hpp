/**
 * @file node_table_model.hpp
 * @brief Qt Item Model providing node data for QTableView.
 *
 * Phase 1 stub — minimal compilable declaration only.
 * Full implementation in Phase 3 (Qt Item Models).
 *
 * @note Q_OBJECT is added in Phase 3.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QAbstractTableModel>

namespace truss::core::interfaces { class ITrussView; }

namespace truss::gui::model {

/**
 * @brief Read-only table model that exposes node data from ITrussView.
 *
 * Columns (10 total):
 *  0  ID            1  X [m]         2  Y [m]
 *  3  Support       4  Fx [N]        5  Fy [N]
 *  6  dx [m]  *     7  dy [m]  *     8  Rx [N]  *    9  Ry [N]  *
 *  (* displayed as "—" until setHasResults(true) is called)
 *
 * Roles implemented (Phase 3):
 *  - Qt::DisplayRole   — formatted numeric string
 *  - Qt::BackgroundRole — amber highlight for loaded nodes (fx or fy != 0)
 *  - Qt::ToolTipRole   — support type description
 *
 * Data is cached in m_rows after each refresh() call. The data() method
 * does NOT call getTrussView() — it reads only from the cached vector.
 *
 * @todo Phase 3: Add Q_OBJECT macro, implement all QAbstractTableModel overrides,
 *       add refresh(ITrussView&) slot, add setHasResults(bool) slot.
 */
class NodeTableModel : public QAbstractTableModel {
public:
    explicit NodeTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

    // Required overrides (stub returns zero/null — Phase 3 provides full implementation)
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override {
        Q_UNUSED(parent)
        return 0;
    }

    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override {
        Q_UNUSED(parent)
        return 0;
    }

    [[nodiscard]] QVariant data(const QModelIndex& /*index*/,
                                int /*role*/ = Qt::DisplayRole) const override {
        return {};
    }

    // TODO Phase 3: QVariant headerData(int section, Qt::Orientation, int role) const override
    // TODO Phase 3: public slots:
    //   void refresh(const core::interfaces::ITrussView& view)
    //   void setHasResults(bool hasResults)
};

}  // namespace truss::gui::model
