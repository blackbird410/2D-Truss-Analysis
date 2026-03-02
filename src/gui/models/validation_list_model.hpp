/**
 * @file validation_list_model.hpp
 * @brief Qt Item Model providing validation issues for QListView.
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

#include <QAbstractListModel>

namespace truss::gui::model {

/**
 * @brief List model that exposes validation issues from a ValidationResult.
 *
 * Each row represents one ValidationIssue.
 *
 * Roles implemented (Phase 3):
 *  - Qt::DisplayRole       — "[CATEGORY] message text"
 *  - Qt::DecorationRole    — severity icon from resource: :/icons/severity_*.svg
 *  - Qt::UserRole + 1      — QVariant(NodeId) of first affected node (0 if none)
 *  - Qt::UserRole + 2      — QVariant(MemberId) of first affected member (0 if none)
 *
 * The UserRole data enables click-to-select: InspectorController subscribes to
 * QListView::activated and uses these roles to focus the canvas selection.
 *
 * @todo Phase 3: Add Q_OBJECT macro, implement all QAbstractListModel overrides,
 *       add refresh(const ValidationResult&) slot.
 */
class ValidationListModel : public QAbstractListModel {
public:
    explicit ValidationListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override {
        Q_UNUSED(parent)
        return 0;
    }

    [[nodiscard]] QVariant data(const QModelIndex& /*index*/,
                                int /*role*/ = Qt::DisplayRole) const override {
        return {};
    }

    // TODO Phase 3: void refresh(const core::validation::ValidationResult& result)
};

}  // namespace truss::gui::model
