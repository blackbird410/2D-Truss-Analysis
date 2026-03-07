/**
 * @file inspector_panel.cpp
 * @brief InspectorPanel implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/panels/inspector_panel.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace truss::gui {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

InspectorPanel::InspectorPanel(QWidget* parent) : QStackedWidget{parent} {
    setObjectName(QStringLiteral("inspectorPanel"));
    buildNoSelectionPage();   // index 0
    buildNodeEditorPage();    // index 1
    buildMemberEditorPage();  // index 2
    setCurrentIndex(kPageNoSelection);
}

// ---------------------------------------------------------------------------
// Page builders
// ---------------------------------------------------------------------------

void InspectorPanel::buildNoSelectionPage() {
    auto* page = new QWidget{this};
    auto* vbox = new QVBoxLayout{page};
    vbox->setContentsMargins(12, 20, 12, 12);
    vbox->setSpacing(8);

    auto* hint = new QLabel{
        QStringLiteral("Select a node or member\non the canvas to inspect\nits properties."), page};
    hint->setObjectName(QStringLiteral("inspector_hint"));
    hint->setAlignment(Qt::AlignCenter);
    hint->setWordWrap(true);

    vbox->addWidget(hint);
    vbox->addStretch();
    addWidget(page);  // index 0
}

void InspectorPanel::buildNodeEditorPage() {
    auto* page = new QWidget{this};
    auto* vbox = new QVBoxLayout{page};
    vbox->setContentsMargins(12, 12, 12, 12);
    vbox->setSpacing(8);

    // ---- Section title ----
    auto* title = new QLabel{QStringLiteral("Node Inspector"), page};
    title->setObjectName(QStringLiteral("inspector_sectionTitle"));
    vbox->addWidget(title);

    // ---- Read-only identity fields ----
    auto* identityBox = new QWidget{page};
    auto* identityLayout = new QFormLayout{identityBox};
    identityLayout->setContentsMargins(0, 0, 0, 0);
    identityLayout->setSpacing(4);

    m_nodeIdLabel = new QLabel{QStringLiteral("—"), identityBox};
    m_nodeIdLabel->setObjectName(QStringLiteral("inspector_nodeId"));
    identityLayout->addRow(QStringLiteral("ID:"), m_nodeIdLabel);

    m_nodeXLabel = new QLabel{QStringLiteral("—"), identityBox};
    m_nodeXLabel->setObjectName(QStringLiteral("inspector_nodeX"));
    identityLayout->addRow(QStringLiteral("X [m]:"), m_nodeXLabel);

    m_nodeYLabel = new QLabel{QStringLiteral("—"), identityBox};
    m_nodeYLabel->setObjectName(QStringLiteral("inspector_nodeY"));
    identityLayout->addRow(QStringLiteral("Y [m]:"), m_nodeYLabel);

    vbox->addWidget(identityBox);

    // ---- Support type combo ----
    auto* supportBox = new QWidget{page};
    auto* supportLayout = new QFormLayout{supportBox};
    supportLayout->setContentsMargins(0, 0, 0, 0);

    m_supportCombo = new QComboBox{supportBox};
    m_supportCombo->setObjectName(QStringLiteral("inspector_supportCombo"));
    m_supportCombo->addItem(QStringLiteral("Free"));
    m_supportCombo->addItem(QStringLiteral("Pinned"));
    m_supportCombo->addItem(QStringLiteral("Roller X (slides in X)"));
    m_supportCombo->addItem(QStringLiteral("Roller Y (slides in Y)"));
    supportLayout->addRow(QStringLiteral("Support:"), m_supportCombo);
    vbox->addWidget(supportBox);

    // ---- Load editor ----
    auto* loadBox = new QWidget{page};
    auto* loadLayout = new QFormLayout{loadBox};
    loadLayout->setContentsMargins(0, 4, 0, 0);

    m_fxSpin = new QDoubleSpinBox{loadBox};
    m_fxSpin->setObjectName(QStringLiteral("inspector_fxSpin"));
    m_fxSpin->setRange(-1e6, 1e6);
    m_fxSpin->setSingleStep(1.0);
    m_fxSpin->setDecimals(3);
    m_fxSpin->setSuffix(QStringLiteral(" kN"));
    loadLayout->addRow(QStringLiteral("Fx:"), m_fxSpin);

    m_fySpin = new QDoubleSpinBox{loadBox};
    m_fySpin->setObjectName(QStringLiteral("inspector_fySpin"));
    m_fySpin->setRange(-1e6, 1e6);
    m_fySpin->setSingleStep(1.0);
    m_fySpin->setDecimals(3);
    m_fySpin->setSuffix(QStringLiteral(" kN"));
    loadLayout->addRow(QStringLiteral("Fy:"), m_fySpin);

    m_applyLoadBtn = new QPushButton{QStringLiteral("Apply Load"), loadBox};
    m_applyLoadBtn->setObjectName(QStringLiteral("inspector_applyLoadBtn"));
    loadLayout->addRow(QString{}, m_applyLoadBtn);

    vbox->addWidget(loadBox);
    vbox->addStretch();

    // ---- Tab order ----
    QWidget::setTabOrder(m_supportCombo, m_fxSpin);
    QWidget::setTabOrder(m_fxSpin, m_fySpin);
    QWidget::setTabOrder(m_fySpin, m_applyLoadBtn);

    // ---- Connections ----
    connect(m_supportCombo,
            &QComboBox::currentIndexChanged,
            this,
            &InspectorPanel::onSupportComboChanged);
    connect(m_applyLoadBtn, &QPushButton::clicked, this, &InspectorPanel::onApplyLoadClicked);

    addWidget(page);  // index 1
}

void InspectorPanel::buildMemberEditorPage() {
    auto* page = new QWidget{this};
    auto* vbox = new QVBoxLayout{page};
    vbox->setContentsMargins(12, 12, 12, 12);
    vbox->setSpacing(8);

    auto* title = new QLabel{QStringLiteral("Member Inspector"), page};
    title->setObjectName(QStringLiteral("inspector_sectionTitle"));
    vbox->addWidget(title);

    auto* form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(4);

    auto makeLabel = [&](QLabel*& ptr, const QString& objName) {
        ptr = new QLabel{QStringLiteral("—"), page};
        ptr->setObjectName(objName);
    };

    makeLabel(m_memberIdLabel, QStringLiteral("inspector_memberId"));
    makeLabel(m_memberE_Label, QStringLiteral("inspector_memberE"));
    makeLabel(m_memberA_Label, QStringLiteral("inspector_memberA"));
    makeLabel(m_memberLenLabel, QStringLiteral("inspector_memberLen"));
    makeLabel(m_memberAngleLabel, QStringLiteral("inspector_memberAngle"));
    makeLabel(m_memberForceLabel, QStringLiteral("inspector_memberForce"));
    makeLabel(m_memberStressLabel, QStringLiteral("inspector_memberStress"));
    makeLabel(m_memberRatioLabel, QStringLiteral("inspector_memberRatio"));

    form->addRow(QStringLiteral("ID:"), m_memberIdLabel);
    form->addRow(QStringLiteral("E [GPa]:"), m_memberE_Label);
    form->addRow(QStringLiteral("A [cm\u00b2]:"), m_memberA_Label);
    form->addRow(QStringLiteral("Length [m]:"), m_memberLenLabel);
    form->addRow(QStringLiteral("Angle [\u00b0]:"), m_memberAngleLabel);
    form->addRow(QStringLiteral("Axial force [kN]:"), m_memberForceLabel);
    form->addRow(QStringLiteral("Axial stress [MPa]:"), m_memberStressLabel);
    form->addRow(QStringLiteral("Utilisation:"), m_memberRatioLabel);

    vbox->addLayout(form);
    vbox->addStretch();

    addWidget(page);  // index 2
}

void InspectorPanel::ensureMemberEditorInteractive() {
    if (m_materialCombo != nullptr)
        return;  // already initialised

    // ---- Row 1: Material combo ----
    m_materialCombo = new QComboBox{m_memberFormBox};
    m_materialCombo->setObjectName(QStringLiteral("inspector_materialCombo"));
    m_materialCombo->setToolTip(QStringLiteral("Select material from library"));
    m_memberFormLayout->insertRow(1, QStringLiteral("Material:"), m_materialCombo);

    // ---- Row 2: Young's modulus (auto-fill label) ----
    m_memberE_Label = new QLabel{QStringLiteral("\u2014"), m_memberFormBox};
    m_memberE_Label->setObjectName(QStringLiteral("inspector_memberE"));
    m_memberFormLayout->insertRow(2, QStringLiteral("E [GPa]:"), m_memberE_Label);

    // ---- Row 3: Cross-section area spin ----
    m_memberA_Spin = new QDoubleSpinBox{m_memberFormBox};
    m_memberA_Spin->setObjectName(QStringLiteral("inspector_memberASpin"));
    m_memberA_Spin->setRange(0.001, 10000.0);
    m_memberA_Spin->setSingleStep(1.0);
    m_memberA_Spin->setDecimals(3);
    m_memberA_Spin->setSuffix(QStringLiteral(" cm\u00b2"));
    m_memberA_Spin->setToolTip(QStringLiteral("Cross-section area"));
    m_memberFormLayout->insertRow(3, QStringLiteral("A [cm\u00b2]:"), m_memberA_Spin);

    // ---- Apply button ----
    m_applyMemberBtn = new QPushButton{QStringLiteral("Apply Changes"), m_memberFormBox->parentWidget()};
    m_applyMemberBtn->setObjectName(QStringLiteral("inspector_applyMemberBtn"));
    m_applyMemberBtn->setToolTip(
        QStringLiteral("Update member with selected material and area.  "
                       "The member will be re-created with the new properties; "
                       "re-run analysis to see updated results."));
    m_memberBtnLayout->addWidget(m_applyMemberBtn);

    // ---- Tab order ----
    QWidget::setTabOrder(m_materialCombo, m_memberA_Spin);
    QWidget::setTabOrder(m_memberA_Spin, m_applyMemberBtn);

    // ---- Connections ----
    connect(m_materialCombo,
            &QComboBox::currentIndexChanged,
            this,
            &InspectorPanel::onMaterialComboChanged);
    connect(m_applyMemberBtn, &QPushButton::clicked, this, &InspectorPanel::onApplyMemberClicked);

    // ---- Populate combo from stored presets ----
    if (!m_materialPresets.empty()) {
        QSignalBlocker bMat{m_materialCombo};
        for (const auto& preset : m_materialPresets) {
            const QString name = QString::fromStdString(preset.name);
            const QString tip  = QStringLiteral("E = %1 GPa \u2014 %2")
                                     .arg(preset.properties.youngModulus / 1e9, 0, 'f', 1)
                                     .arg(QString::fromStdString(preset.description));
            m_materialCombo->addItem(name);
            m_materialCombo->setItemData(m_materialCombo->count() - 1, tip, Qt::ToolTipRole);
        }
        m_memberE_Label->setText(
            QString::number(m_materialPresets[0].properties.youngModulus / 1e9, 'f', 1));
    }
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void InspectorPanel::showNoSelection() {
    setCurrentIndex(kPageNoSelection);
}

void InspectorPanel::showNodeEditor(const NodeView& node) {
    m_selectedNodeId = node.id;

    m_nodeIdLabel->setText(QString::number(node.id));
    m_nodeXLabel->setText(QString::number(node.x, 'f', 4));
    m_nodeYLabel->setText(QString::number(node.y, 'f', 4));

    // Block signals while populating to avoid re-emitting supportChangeRequested
    QSignalBlocker blockCombo{m_supportCombo};
    m_supportCombo->setCurrentIndex(static_cast<int>(node.support));

    QSignalBlocker blockFx{m_fxSpin};
    QSignalBlocker blockFy{m_fySpin};
    // Convert Pa -> kN : facade stores force in N; divide by 1000 for kN display
    m_fxSpin->setValue(node.fx / 1000.0);
    m_fySpin->setValue(node.fy / 1000.0);

    setCurrentIndex(kPageNodeEditor);
}

void InspectorPanel::showMemberEditor(const MemberView& member) {
    m_selectedMemberId = member.id;

    m_memberIdLabel->setText(QString::number(member.id));
    m_memberE_Label->setText(QString::number(member.youngModulus / 1e9, 'f', 1));
    m_memberA_Label->setText(QString::number(member.area * 1e4, 'f', 2));
    m_memberLenLabel->setText(QString::number(member.length, 'f', 4));
    m_memberAngleLabel->setText(QString::number(qRadiansToDegrees(member.angle), 'f', 2));
    m_memberForceLabel->setText(member.axialForce != 0.0
                                    ? QString::number(member.axialForce / 1000.0, 'f', 3)
                                    : QStringLiteral("\u2014"));
    m_memberStressLabel->setText(member.axialStress != 0.0
                                     ? QString::number(member.axialStress / 1e6, 'f', 3)
                                     : QStringLiteral("\u2014"));
    m_memberRatioLabel->setText(member.utilizationRatio != 0.0
                                    ? QString::number(member.utilizationRatio * 100.0, 'f', 1) +
                                          QStringLiteral(" %")
                                    : QStringLiteral("\u2014"));

    setCurrentIndex(kPageMemberEditor);
}

void InspectorPanel::onStateChanged(const truss::gui::state::WorkspaceState& state) {
    using truss::gui::state::WorkspacePhase;
    const bool editable = (state.phase == WorkspacePhase::ModelBuilding ||
                           state.phase == WorkspacePhase::ResultsReady);

    m_supportCombo->setEnabled(editable);
    m_fxSpin->setEnabled(editable);
    m_fySpin->setEnabled(editable);
    m_applyLoadBtn->setEnabled(editable);
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void InspectorPanel::onApplyLoadClicked() {
    // Convert kN back to N for the facade (which uses SI units)
    const Force2D load{m_fxSpin->value() * 1000.0, m_fySpin->value() * 1000.0};
    emit loadChangeRequested(m_selectedNodeId, load);
}

void InspectorPanel::onSupportComboChanged(int index) {
    emit supportChangeRequested(m_selectedNodeId, static_cast<SupportType>(index));
}

void InspectorPanel::onApplyMemberClicked() {
    const int idx = m_materialCombo->currentIndex();
    emit memberPropertiesChangeRequested(m_selectedMemberId,
                                         materialSpecFromIndex(idx),
                                         sectionSpecFromArea(m_memberA_Spin->value()));
}

void InspectorPanel::onMaterialComboChanged(int index) {
    if (index >= 0 && index < static_cast<int>(m_materialPresets.size())) {
        m_memberE_Label->setText(
            QString::number(m_materialPresets[index].properties.youngModulus / 1e9, 'f', 1));
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

MaterialSpec InspectorPanel::materialSpecFromIndex(int matIdx) const {
    MaterialSpec spec;
    if (matIdx >= 0 && matIdx < static_cast<int>(m_materialPresets.size())) {
        const auto& p = m_materialPresets[matIdx];
        spec.youngsModulusPa = p.properties.youngModulus;
        spec.name = p.name;
    } else {
        // Fallback to structural steel if library is empty
        spec.youngsModulusPa = 200e9;
        spec.name = "Steel";
    }
    return spec;
}

SectionSpec InspectorPanel::sectionSpecFromArea(double areaCm2) const {
    SectionSpec sec;
    sec.areaM2 = areaCm2 * 1e-4;  // cm² → m²
    sec.profile = "Custom";
    return sec;
}

}  // namespace truss::gui
