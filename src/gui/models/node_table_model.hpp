/**
 * @file node_table_model.hpp
 * @brief Qt Item Model providing node data for QTableView.
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
using truss::core::interfaces::NodeView;

/**
 * @brief Read-only table model that exposes node data from ITrussView.
 *
 * Columns (10 total):
 *  0  ID            1  X [m]         2  Y [m]
 *  3  Support       4  Fx [kN]       5  Fy [kN]
 *  6  dx [mm]  *   7  dy [mm]  *    8  Rx [kN]  *   9  Ry [kN]  *
 *
 *  Columns 6–9 display "—" until setHasResults(true) is called.
 *
 * Roles:
 *  Qt::DisplayRole    — formatted string
 *  Qt::BackgroundRole — amber tint for nodes with non-zero applied load
 *  Qt::ToolTipRole    — support type description for column 3
 *  Qt::TextAlignmentRole — right-align numeric columns
 *
 * Thread safety: all methods must be called from the GUI thread.
 */
class NodeTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit NodeTableModel(QObject* parent = nullptr);
    ~NodeTableModel() override;

    // -----------------------------------------------------------------------
    // QAbstractTableModel interface
    // -----------------------------------------------------------------------
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    /// Total number of columns in this model.
    static constexpr int kColumnCount = 10;

    // Column index constants
    static constexpr int kColId      = 0;
    static constexpr int kColX       = 1;
    static constexpr int kColY       = 2;
    static constexpr int kColSupport = 3;
    static constexpr int kColFx      = 4;
    static constexpr int kColFy      = 5;
    static constexpr int kColDx      = 6;  ///< Results-only
    static constexpr int kColDy      = 7;  ///< Results-only
    static constexpr int kColRx      = 8;  ///< Results-only
    static constexpr int kColRy      = 9;  ///< Results-only

public Q_SLOTS:
    /**
     * @brief Replace the cached node rows with data from @p view.
     * Calls beginResetModel / endResetModel — existing selections are lost.
     */
    void refresh(const ITrussView& view);

    /**
     * @brief Toggle results-column visibility.
     * Causes a full model reset so views re-query all cells.
     */
    void setHasResults(bool hasResults);

private:
    [[nodiscard]] QString formatSupport(truss::core::SupportType s) const;
    [[nodiscard]] bool    isLoaded(const NodeView& n) const;

    std::vector<NodeView> m_rows;
    bool                  m_hasResults{false};

    // Colours
    static const QColor kLoadedBg;   ///< Amber tint for loaded-node rows
};

} // namespace truss::gui::model
