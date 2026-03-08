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

#include "application/material_library_service.hpp"
#include "application/truss_edit_dtos.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"
#include "gui/state/workspace_state.hpp"

#include <QStackedWidget>

#include <vector>

class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QHBoxLayout;
class QPushButton;

namespace truss::gui {

using truss::application::MaterialPreset;
using truss::application::MaterialSpec;
using truss::application::SectionPreset;
using truss::application::SectionSpec;
using truss::core::Force2D;
using truss::core::MemberId;
using truss::core::NodeId;
using truss::core::Point2D;
using truss::core::SupportType;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;

/**
 * @brief Context-sensitive property editor panel (right side of main window).
 *
 * Uses QStackedWidget internally with three pages:
 *  - Page 0 (kPageNoSelection):  Prompt to select a canvas element + default
 *                                 new-member material/section picker.
 *  - Page 1 (kPageNodeEditor):   Position/support/load editor for a selected node.
 *  - Page 2 (kPageMemberEditor): Editable material/section + read-only results
 *                                 for a selected member.
 *
 * Call populateMaterialLibrary() once after construction to fill the material
 * and section combo boxes on both pages from the application's MaterialLibraryService.
 *
 * The panel emits signals when the user modifies properties; the controlling
 * InspectorController connects those signals to the facade.
 */
class InspectorPanel : public QStackedWidget {
    Q_OBJECT

public:
    static constexpr int kPageNoSelection = 0;
    static constexpr int kPageNodeEditor = 1;
    static constexpr int kPageMemberEditor = 2;

    explicit InspectorPanel(QWidget* parent = nullptr);

    /**
     * @brief Populate material and section combo boxes from the library.
     *
     * Must be called once after construction (from MainWindow::connectSignals)
     * before any member can be selected.  Fills the combo boxes on both the
     * no-selection (default) page and the member-editor page.
     *
     * @param materials  All material presets from MaterialLibraryService.
     * @param sections   All section presets from MaterialLibraryService.
     */
    void populateMaterialLibrary(const std::vector<MaterialPreset>& materials,
                                 const std::vector<SectionPreset>& sections);

public slots:
    void showNoSelection();
    void showNodeEditor(const NodeView& node);
    void showMemberEditor(const MemberView& member);
    void onStateChanged(const truss::gui::state::WorkspaceState& state);

signals:
    void supportChangeRequested(NodeId nodeId, SupportType newType);
    void loadChangeRequested(NodeId nodeId, Force2D load);

    /**
     * @brief Emitted when the user edits the node coordinates and clicks
     *        "Apply Position".
     *
     * Connect to InspectorController::onNodePositionChangeRequested.
     */
    void nodePositionChangeRequested(NodeId nodeId, truss::core::Point2D newPosition);

    /**
     * @brief Emitted when the user clicks "Apply Changes" on the member editor.
     *
     * Carries the selected member ID plus the new material and section specs.
     * Connect to InspectorController::onMemberPropertiesChangeRequested.
     */
    void memberPropertiesChangeRequested(MemberId memberId, MaterialSpec mat, SectionSpec sec);

    /**
     * @brief Emitted when the default new-member material or area changes.
     *
     * Connect to CanvasController::onDefaultMaterialChanged so that newly
     * drawn members use the values picked on the no-selection page.
     */
    void defaultMaterialChanged(MaterialSpec mat, SectionSpec sec);

private slots:
    void onApplyLoadClicked();
    void onApplyPositionClicked();
    void onSupportComboChanged(int index);
    void onApplyMemberClicked();
    void onMaterialComboChanged(int index);

private:
    void buildNoSelectionPage();
    void buildNodeEditorPage();
    void buildMemberEditorPage();

    /**
     * @brief Lazily creates the member-editor's interactive widgets on first use.
     *
     * Called at the start of showMemberEditor().  Widgets are not created during
     * construction so that Qt's findChild<QComboBox*>() / findChild<QPushButton*>()
     * calls in unit tests always reach the node-editor page first.
     */
    void ensureMemberEditorInteractive();

    // Helper: build a MaterialSpec + SectionSpec from the given combo indices
    // and the current area spinbox value.  Used on both pages.
    MaterialSpec materialSpecFromIndex(int matIdx) const;
    SectionSpec sectionSpecFromArea(double areaCm2) const;

    // Material / section library data (populated via populateMaterialLibrary)
    std::vector<MaterialPreset> m_materialPresets;
    std::vector<SectionPreset> m_sectionPresets;

    // Pointers stored for use by ensureMemberEditorInteractive()
    QFormLayout* m_memberFormLayout{nullptr};
    QWidget* m_memberFormBox{nullptr};
    QHBoxLayout* m_memberBtnLayout{nullptr};

    // Node editor widgets
    QLabel* m_nodeIdLabel{nullptr};
    QDoubleSpinBox* m_nodeXSpin{nullptr};   ///< X coordinate (editable)
    QDoubleSpinBox* m_nodeYSpin{nullptr};   ///< Y coordinate (editable)
    QPushButton* m_applyPositionBtn{nullptr};  ///< Commit coordinate changes
    QComboBox* m_supportCombo{nullptr};
    QDoubleSpinBox* m_fxSpin{nullptr};
    QDoubleSpinBox* m_fySpin{nullptr};
    QPushButton* m_applyLoadBtn{nullptr};

    // Member editor widgets
    QLabel* m_memberIdLabel{nullptr};
    QComboBox* m_materialCombo{nullptr};      ///< Material selection (editable)
    QLabel* m_memberE_Label{nullptr};         ///< Young's modulus (auto-filled, read-only)
    QDoubleSpinBox* m_memberA_Spin{nullptr};  ///< Cross-section area in cm² (editable)
    QLabel* m_memberLenLabel{nullptr};
    QLabel* m_memberAngleLabel{nullptr};
    QLabel* m_memberForceLabel{nullptr};
    QLabel* m_memberStressLabel{nullptr};
    QLabel* m_memberRatioLabel{nullptr};
    QPushButton* m_applyMemberBtn{nullptr};  ///< Commit property changes

    // Retained state
    NodeId m_selectedNodeId{0};
    MemberId m_selectedMemberId{0};
};

}  // namespace truss::gui
