/**
 * @file inspector_panel.hpp
 * @brief Context-sensitive inspector panel for node and member property editing.
 *
 * Phase 5: Full Q_OBJECT implementation with QStackedWidget.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "application/truss_edit_dtos.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"
#include "gui/state/workspace_state.hpp"

#include <QStackedWidget>

class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QFormLayout;

namespace truss::gui {

using truss::core::Force2D;
using truss::core::MemberId;
using truss::core::NodeId;
using truss::core::SupportType;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;
using truss::application::MaterialPreset;
using truss::application::SectionPreset;
using truss::application::MaterialSpec;
using truss::application::SectionSpec;

/**
 * @brief Context-sensitive property editor panel (right side of main window).
 *
 * Uses QStackedWidget internally with three pages:
 *  - Page 0 (kPageNoSelection):  Prompt to select a canvas element.
 *  - Page 1 (kPageNodeEditor):   Position/support/load editor for a selected node.
 *  - Page 2 (kPageMemberEditor): Read-only material/geometry/result view for a member.
 *
 * The panel emits signals when the user modifies a support type or applies a
 * load; the controlling InspectorController connects those signals to the facade.
 */
class InspectorPanel : public QStackedWidget {
    Q_OBJECT

public:
    static constexpr int kPageNoSelection = 0;
    static constexpr int kPageNodeEditor = 1;
    static constexpr int kPageMemberEditor = 2;

    explicit InspectorPanel(QWidget* parent = nullptr);

public slots:
    void showNoSelection();
    void showNodeEditor(const NodeView& node);
    void showMemberEditor(const MemberView& member);
    void onStateChanged(const truss::gui::state::WorkspaceState& state);

signals:
    void supportChangeRequested(NodeId nodeId, SupportType newType);
    void loadChangeRequested(NodeId nodeId, Force2D load);

    /**
     * @brief Emitted when the user clicks "Apply Changes" on the member editor.
     *
     * Carries the selected member ID plus the new material and section specs.
     * Connect to InspectorController::onMemberPropertiesChangeRequested.
     */
    void memberPropertiesChangeRequested(MemberId memberId,
                                         MaterialSpec mat,
                                         SectionSpec sec);

    /**
     * @brief Emitted when the default new-member material or area changes.
     *
     * Connect to CanvasController::onDefaultMaterialChanged so that newly
     * drawn members use the values picked on the no-selection page.
     */
    void defaultMaterialChanged(MaterialSpec mat, SectionSpec sec);

private slots:
    void onApplyLoadClicked();
    void onSupportComboChanged(int index);

private:
    void buildNoSelectionPage();
    void buildNodeEditorPage();
    void buildMemberEditorPage();

    // Helper: build a MaterialSpec + SectionSpec from the given combo indices
    // and the current area spinbox value.  Used on both pages.
    MaterialSpec materialSpecFromIndex(int matIdx) const;
    SectionSpec sectionSpecFromArea(double areaCm2) const;

    // Material / section library data (populated via populateMaterialLibrary)
    std::vector<MaterialPreset> m_materialPresets;
    std::vector<SectionPreset> m_sectionPresets;

    // Node editor widgets
    QLabel* m_nodeIdLabel{nullptr};
    QLabel* m_nodeXLabel{nullptr};
    QLabel* m_nodeYLabel{nullptr};
    QComboBox* m_supportCombo{nullptr};
    QDoubleSpinBox* m_fxSpin{nullptr};
    QDoubleSpinBox* m_fySpin{nullptr};
    QPushButton* m_applyLoadBtn{nullptr};

    // Member editor widgets
    QLabel* m_memberIdLabel{nullptr};
    QLabel* m_memberE_Label{nullptr};
    QLabel* m_memberA_Label{nullptr};
    QLabel* m_memberLenLabel{nullptr};
    QLabel* m_memberAngleLabel{nullptr};
    QLabel* m_memberForceLabel{nullptr};
    QLabel* m_memberStressLabel{nullptr};
    QLabel* m_memberRatioLabel{nullptr};

    // Retained state
    NodeId m_selectedNodeId{0};
    MemberId m_selectedMemberId{0};
};

}  // namespace truss::gui
