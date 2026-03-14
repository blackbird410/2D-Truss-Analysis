/**
 * @file validation_list_model.cpp
 * @brief Implementation of ValidationListModel.
 *
 * Phase 3: Qt Item Models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/validation_list_model.hpp"

#include <QPainter>
#include <QPixmap>
#include <QString>

namespace truss::gui::model {

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

ValidationListModel::ValidationListModel(QObject* parent) : QAbstractListModel(parent) {}

ValidationListModel::~ValidationListModel() = default;

// ---------------------------------------------------------------------------
// QAbstractListModel overrides
// ---------------------------------------------------------------------------

int ValidationListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_issues.size());
}

QVariant ValidationListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_issues.size()))
        return {};

    const ValidationIssue& issue = m_issues[static_cast<size_t>(row)];

    switch (role) {
        case Qt::DisplayRole: {
            return QStringLiteral("[%1] %2")
                .arg(QString::fromStdString(issue.category))
                .arg(QString::fromStdString(issue.message));
        }

        case Qt::DecorationRole:
            return severityIcon(issue.severity);

        case Qt::ToolTipRole:
            if (!issue.technicalDetail.empty())
                return QString::fromStdString(issue.technicalDetail);
            return {};

        case kNodeIdRole: {
            const quint32 id = issue.affectedNodes.empty()
                                   ? 0u
                                   : static_cast<quint32>(issue.affectedNodes.front());
            return QVariant(id);
        }

        case kMemberIdRole: {
            const quint32 id = issue.affectedMembers.empty()
                                   ? 0u
                                   : static_cast<quint32>(issue.affectedMembers.front());
            return QVariant(id);
        }

        default:
            return {};
    }
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void ValidationListModel::refresh(const ValidationResult& result) {
    beginResetModel();
    m_issues = result.getIssues();
    endResetModel();
}

void ValidationListModel::clear() {
    if (m_issues.empty())
        return;
    beginResetModel();
    m_issues.clear();
    endResetModel();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

QColor ValidationListModel::severityColor(ValidationSeverity severity) const {
    switch (severity) {
        case ValidationSeverity::Info:
            return QColor(0x42, 0x85, 0xF4);  // steel-blue  #4285F4
        case ValidationSeverity::Warning:
            return QColor(0xFF, 0xC1, 0x07);  // amber       #FFC107
        case ValidationSeverity::Error:
            return QColor(0xEA, 0x43, 0x35);  // red         #EA4335
        case ValidationSeverity::Fatal:
            return QColor(0x8B, 0x00, 0x00);  // dark-red    #8B0000
        default:
            return QColor(0x9E, 0x9E, 0x9E);  // grey
    }
}

QPixmap ValidationListModel::severityIcon(ValidationSeverity severity) const {
    constexpr int kSize = 12;
    QPixmap pix(kSize, kSize);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(severityColor(severity));
    p.setPen(Qt::NoPen);
    p.drawEllipse(1, 1, kSize - 2, kSize - 2);
    p.end();
    return pix;
}

}  // namespace truss::gui::model
