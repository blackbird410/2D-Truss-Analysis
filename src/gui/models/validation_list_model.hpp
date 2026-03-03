/**
 * @file validation_list_model.hpp
 * @brief Qt Item Model providing validation issues for QListView.
 *
 * Phase 3: Qt Item Models — full implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#pragma once

#include "core/validation/truss_validator.hpp"

#include <QAbstractListModel>
#include <QColor>
#include <QPixmap>

#include <vector>

namespace truss::gui::model {

using truss::core::validation::ValidationIssue;
using truss::core::validation::ValidationResult;
using truss::core::validation::ValidationSeverity;

/**
 * @brief List model that exposes validation issues from a ValidationResult.
 *
 * Each row represents one ValidationIssue.
 *
 * Roles:
 *  Qt::DisplayRole    — "[CATEGORY] message"
 *  Qt::DecorationRole — 12×12 colour square per severity:
 *                       Info=steel-blue, Warning=amber, Error=red, Fatal=dark-red
 *  Qt::ToolTipRole    — technicalDetail string (if non-empty)
 *  Qt::UserRole + 1   — QVariant(quint32) first affected NodeId, or 0
 *  Qt::UserRole + 2   — QVariant(quint32) first affected MemberId, or 0
 *
 * These UserRole values let controllers navigate the canvas to the issue location.
 *
 * Thread safety: all methods must be called from the GUI thread.
 */
class ValidationListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ValidationListModel(QObject* parent = nullptr);
    ~ValidationListModel() override;

    // -----------------------------------------------------------------------
    // QAbstractListModel interface
    // -----------------------------------------------------------------------
    [[nodiscard]] int     rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Custom role constants
    static constexpr int kNodeIdRole   = Qt::UserRole + 1;
    static constexpr int kMemberIdRole = Qt::UserRole + 2;

public Q_SLOTS:
    /**
     * @brief Replace the cached issues with the contents of @p result.
     * Calls beginResetModel / endResetModel.
     */
    void refresh(const ValidationResult& result);

    /**
     * @brief Clear all rows (convenience overload).
     */
    void clear();

private:
    [[nodiscard]] QColor   severityColor(ValidationSeverity severity) const;
    [[nodiscard]] QPixmap  severityIcon(ValidationSeverity severity) const;

    std::vector<ValidationIssue> m_issues;
};

} // namespace truss::gui::model
