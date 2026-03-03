/**
 * @file member_table_model.hpp
 * @brief Qt Item Model providing member data for QTableView.
 *
 * Phase 3: Qt Item Models — full implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#pragma once

#include "core/interfaces/itruss_view.hpp"

#include <QAbstractTableModel>
#include <QColor>

#include <vector>

namespace truss::gui::model {

using truss::core::interfaces::ITrussView;
using truss::core::interfaces::MemberView;

/**
 * @brief Read-only table model that exposes member data from ITrussView.
 *
 * Columns (12 total):
 *  0  ID           1  Start Node   2  End Node
 *  3  Material     4  E [GPa]      5  A [cm²]
 *  6  Length [m]   7  Angle [°]
 *  8  Force [kN]*  9  Stress [MPa]* 10  Util. Ratio*  11  State*
 *
 *  Columns 8–11 display "—" until setHasResults(true) is called.
 *
 * Roles:
 *  Qt::DisplayRole     — formatted string
 *  Qt::ForegroundRole  — colour-coded text on column 10 (utilisation ratio)
 *                         0.0 → green (#34A853), 0.75 → amber (#FFC107),
 *                         ≥1.0 → red (#EA4335)
 *  Qt::ToolTipRole     — state detail on column 11
 *  Qt::TextAlignmentRole — right-align numeric columns
 *
 * Thread safety: all methods must be called from the GUI thread.
 */
class MemberTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit MemberTableModel(QObject* parent = nullptr);
    ~MemberTableModel() override;

    // -----------------------------------------------------------------------
    // QAbstractTableModel interface
    // -----------------------------------------------------------------------
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    /// Total number of columns in this model.
    static constexpr int kColumnCount = 12;

    // Column index constants
    static constexpr int kColId       = 0;
    static constexpr int kColStart    = 1;
    static constexpr int kColEnd      = 2;
    static constexpr int kColMaterial = 3;
    static constexpr int kColE        = 4;
    static constexpr int kColA        = 5;
    static constexpr int kColLength   = 6;
    static constexpr int kColAngle    = 7;
    static constexpr int kColForce    = 8;   ///< Results-only
    static constexpr int kColStress   = 9;   ///< Results-only
    static constexpr int kColRatio    = 10;  ///< Results-only
    static constexpr int kColState    = 11;  ///< Results-only

public Q_SLOTS:
    /**
     * @brief Replace the cached member rows with data from @p view.
     * Calls beginResetModel / endResetModel.
     */
    void refresh(const ITrussView& view);

    /**
     * @brief Toggle results-column visibility.
     */
    void setHasResults(bool hasResults);

private:
    [[nodiscard]] QColor  ratioColor(double ratio) const;
    [[nodiscard]] QString stateString(const MemberView& m) const;

    std::vector<MemberView> m_rows;
    bool                    m_hasResults{false};

    // Colour anchors for ratio interpolation
    static const QColor kColorGreen;   ///< #34A853 — safe    (ratio = 0.0)
    static const QColor kColorAmber;   ///< #FFC107 — caution (ratio = 0.75)
    static const QColor kColorRed;     ///< #EA4335 — overload (ratio >= 1.0)
};

} // namespace truss::gui::model
