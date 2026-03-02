/**
 * @file member_table_model.hpp
 * @brief Qt Item Model providing member data for QTableView with colour-coded
 *        utilisation ratio column.
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
 * @brief Read-only table model that exposes member data from ITrussView.
 *
 * Columns (12 total):
 *  0  ID                  1  Start Node       2  End Node
 *  3  Material            4  E [GPa]          5  A [cm²]
 *  6  Length [m]          7  Angle [°]        8  Axial Force [kN]  *
 *  9  Stress [MPa]  *    10  Util. Ratio  *  11  State  *
 *  (* displayed as "—" until setHasResults(true) is called)
 *
 * Roles implemented (Phase 3):
 *  - Qt::DisplayRole    — formatted numeric / text string
 *  - Qt::ForegroundRole — colour interpolation for column 10 (ratio):
 *                         0.0 → green (#34A853)
 *                         0.75 → amber (#FFC107)
 *                         1.0 → red (#EA4335)
 *  - Qt::ToolTipRole    — "Tension" / "Compression" / "Yielded" for column 11
 *
 * @todo Phase 3: Add Q_OBJECT macro, implement all QAbstractTableModel overrides,
 *       add refresh(ITrussView&) and setHasResults(bool) slots.
 */
class MemberTableModel : public QAbstractTableModel {
public:
    explicit MemberTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

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

    // TODO Phase 3: QVariant headerData(...) override
    // TODO Phase 3: public slots:
    //   void refresh(const core::interfaces::ITrussView& view)
    //   void setHasResults(bool hasResults)
};

}  // namespace truss::gui::model
