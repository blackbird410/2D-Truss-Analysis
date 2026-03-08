/**
 * @file member_table_model.cpp
 * @brief Implementation of MemberTableModel.
 *
 * Phase 3: Qt Item Models.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-03
 */

#include "gui/models/member_table_model.hpp"

#include <QBrush>
#include <QColor>
#include <QString>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace truss::gui::model {

// ---------------------------------------------------------------------------
// Static member initialisation
// ---------------------------------------------------------------------------

const QColor MemberTableModel::kColorGreen = QColor(0x34, 0xA8, 0x53);  // #34A853
const QColor MemberTableModel::kColorAmber = QColor(0xFF, 0xC1, 0x07);  // #FFC107
const QColor MemberTableModel::kColorRed = QColor(0xEA, 0x43, 0x35);    // #EA4335

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

MemberTableModel::MemberTableModel(QObject* parent) : QAbstractTableModel(parent) {}

MemberTableModel::~MemberTableModel() = default;

// ---------------------------------------------------------------------------
// QAbstractTableModel overrides
// ---------------------------------------------------------------------------

int MemberTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_rows.size());
}

int MemberTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return kColumnCount;
}

QVariant MemberTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Vertical)
        return section + 1;

    static const char* kHeaders[kColumnCount] = {
        "ID",
        "Start",
        "End",
        "Material",
        "E [GPa]",
        "A [cm²]",
        "Length [m]",
        "Angle [°]",
        "Force [kN]",
        "Stress [MPa]",
        "Util. Ratio",
        "State",
    };

    if (section < 0 || section >= kColumnCount)
        return {};
    return kHeaders[section];
}

QVariant MemberTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= static_cast<int>(m_rows.size()))
        return {};
    if (col < 0 || col >= kColumnCount)
        return {};

    const MemberView& m = m_rows[static_cast<size_t>(row)];

    // ------------------------------------------------------------------
    // TextAlignmentRole
    // ------------------------------------------------------------------
    if (role == Qt::TextAlignmentRole) {
        if (col == kColId || col == kColMaterial || col == kColState)
            return static_cast<int>(Qt::AlignCenter);
        return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }

    // ------------------------------------------------------------------
    // ForegroundRole — tension/compression/yield colour coding
    // Colors mirror the canvas member colours so the table and canvas tell
    // the same visual story.  Utilisation ratio column keeps the green→amber→red
    // interpolation; force and stress columns use the state-based palette.
    // ------------------------------------------------------------------
    if (role == Qt::ForegroundRole) {
        if (!m_hasResults)
            return {};
        if (col == kColRatio)
            return ratioColor(m.utilizationRatio);
        // Force and stress: colour by mechanical state
        if (col == kColForce || col == kColStress) {
            if (m.yielded)
                return QColor(0xFF, 0x17, 0x44);  // red — yielded
            if (m.inTension)
                return QColor(0x4F, 0xC3, 0xF7);  // cyan — tension
            if (m.axialForce < -1e-10)
                return QColor(0xFF, 0x70, 0x43);  // orange — compression
        }
        return {};
    }

    // ------------------------------------------------------------------
    // BackgroundRole — semi-transparent state tint on the State column
    // A light tint provides a quick at-a-glance row colour cue that reads
    // correctly in both dark and light themes at alpha ≈40.
    // ------------------------------------------------------------------
    if (role == Qt::BackgroundRole && m_hasResults) {
        if (col == kColState) {
            if (m.yielded)
                return QBrush(QColor(0xFF, 0x17, 0x44, 45));
            if (m.inTension)
                return QBrush(QColor(0x4F, 0xC3, 0xF7, 45));
            if (m.axialForce < -1e-10)
                return QBrush(QColor(0xFF, 0x70, 0x43, 45));
        }
        return {};
    }

    // ------------------------------------------------------------------
    // ToolTipRole — state description (col 11 only)
    // ------------------------------------------------------------------
    if (role == Qt::ToolTipRole) {
        if (col == kColState && m_hasResults)
            return stateString(m);
        if (col == kColE)
            return QStringLiteral("Young's Modulus");
        if (col == kColA)
            return QStringLiteral("Cross-sectional area");
        return {};
    }

    // ------------------------------------------------------------------
    // DisplayRole
    // ------------------------------------------------------------------
    if (role != Qt::DisplayRole)
        return {};

    // Results columns: show "—" when no analysis has been run
    const bool isResultsCol = (col >= kColForce && col <= kColState);
    if (isResultsCol && !m_hasResults)
        return QStringLiteral("—");

    switch (col) {
        case kColId:
            return QString::number(m.id);

        case kColStart:
            return QString::number(m.startNodeId);

        case kColEnd:
            return QString::number(m.endNodeId);

        case kColMaterial:
            return QString::fromStdString(m.label);

        case kColE:
            // Pa → GPa
            return QString::number(m.youngModulus / 1.0e9, 'f', 1);

        case kColA:
            // m² → cm²  (1 m² = 1×10⁴ cm²)
            return QString::number(m.area * 1.0e4, 'f', 4);

        case kColLength:
            return QString::number(m.length, 'f', 4);

        case kColAngle: {
            const double deg = m.angle * (180.0 / std::numbers::pi);
            return QString::number(deg, 'f', 2);
        }

        case kColForce:
            // N → kN
            return QString::number(m.axialForce / 1000.0, 'f', 3);

        case kColStress:
            // Pa → MPa
            return QString::number(m.axialStress / 1.0e6, 'f', 3);

        case kColRatio:
            return QString::number(m.utilizationRatio, 'f', 3);

        case kColState:
            return stateString(m);

        default:
            return {};
    }
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void MemberTableModel::refresh(const ITrussView& view) {
    beginResetModel();
    m_rows = view.getMemberViews();
    endResetModel();
}

void MemberTableModel::setHasResults(bool hasResults) {
    if (m_hasResults == hasResults)
        return;
    beginResetModel();
    m_hasResults = hasResults;
    endResetModel();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/**
 * @brief Interpolate between three colour anchors based on utilisation ratio.
 *
 * Segment 1: [0.0, 0.75]  — green → amber
 * Segment 2: [0.75, 1.0+] — amber → red  (clamped at 1.0)
 */
QColor MemberTableModel::ratioColor(double ratio) const {
    ratio = std::clamp(ratio, 0.0, 1.0);

    auto lerp = [](int a, int b, double t) -> int {
        return static_cast<int>(std::round(a + (b - a) * t));
    };

    QColor c1, c2;
    double t = 0.0;

    if (ratio <= 0.75) {
        c1 = kColorGreen;
        c2 = kColorAmber;
        t = ratio / 0.75;
    } else {
        c1 = kColorAmber;
        c2 = kColorRed;
        t = (ratio - 0.75) / 0.25;
    }

    return QColor(lerp(c1.red(), c2.red(), t),
                  lerp(c1.green(), c2.green(), t),
                  lerp(c1.blue(), c2.blue(), t));
}

QString MemberTableModel::stateString(const MemberView& m) const {
    if (m.yielded)
        return QStringLiteral("Yielded");
    if (m.inTension)
        return QStringLiteral("Tension");
    return QStringLiteral("Compression");
}

}  // namespace truss::gui::model
