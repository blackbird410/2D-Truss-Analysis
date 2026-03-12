/**
 * @file test_inspector_panel.cpp
 * @brief Unit tests for InspectorPanel (Phase 5).
 *
 * Verifies:
 *  - Default page is kPageNoSelection (0).
 *  - showNodeEditor() switches to kPageNodeEditor (1).
 *  - showMemberEditor() switches to kPageMemberEditor (2).
 *  - showNoSelection() switches back to kPageNoSelection (0).
 *  - supportChangeRequested is emitted when support combo index changes.
 *  - loadChangeRequested is emitted when Apply Load is clicked.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"
#include "gui/panels/inspector_panel.hpp"
#include "gui/state/workspace_state.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

using truss::core::Force2D;
using truss::core::NodeId;
using truss::core::SupportType;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;
using truss::gui::InspectorPanel;
using truss::gui::state::WorkspacePhase;
using truss::gui::state::WorkspaceState;

// ============================================================
// QApplication bootstrap
// ============================================================

namespace {
QApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* e = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return e;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

NodeView makeNode(NodeId id = NodeId{1},
                  double x = 0.0,
                  double y = 0.0,
                  SupportType sup = SupportType::Free) {
    NodeView n;
    n.id = id;
    n.x = x;
    n.y = y;
    n.support = sup;
    n.fx = 0.0;
    n.fy = 0.0;
    return n;
}

MemberView makeMember(int id = 1) {
    MemberView m;
    m.id = id;
    m.startNodeId = 1;
    m.endNodeId = 2;
    m.youngModulus = 200e9;
    m.area = 0.001;
    m.length = 3.0;
    m.angle = 0.0;
    m.axialForce = 1000.0;
    m.axialStress = 1e6;
    m.utilizationRatio = 0.1;
    m.inTension = true;
    m.yielded = false;
    return m;
}
}  // namespace

// ============================================================
// Fixture
// ============================================================

class InspectorPanelTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        panel = std::make_unique<InspectorPanel>();
    }
    void TearDown() override { panel.reset(); }

    std::unique_ptr<InspectorPanel> panel;
};

// ============================================================
// Page switching
// ============================================================

TEST_F(InspectorPanelTest, DefaultPage_IsNoSelection) {
    EXPECT_EQ(panel->currentIndex(), InspectorPanel::kPageNoSelection);
}

TEST_F(InspectorPanelTest, ShowNodeEditor_SwitchesToPage1) {
    panel->showNodeEditor(makeNode());
    EXPECT_EQ(panel->currentIndex(), InspectorPanel::kPageNodeEditor);
}

TEST_F(InspectorPanelTest, ShowMemberEditor_SwitchesToPage2) {
    panel->showMemberEditor(makeMember());
    EXPECT_EQ(panel->currentIndex(), InspectorPanel::kPageMemberEditor);
}

TEST_F(InspectorPanelTest, ShowNoSelection_SwitchesBackToPage0) {
    panel->showNodeEditor(makeNode());
    panel->showNoSelection();
    EXPECT_EQ(panel->currentIndex(), InspectorPanel::kPageNoSelection);
}

// ============================================================
// Signal: supportChangeRequested
// ============================================================

TEST_F(InspectorPanelTest, SupportComboChange_EmitsSupportChangeRequested) {
    // Populate node editor with a node and switch to that page.
    NodeView n = makeNode(NodeId{5}, 1.0, 2.0, SupportType::Free);
    panel->showNodeEditor(n);

    // Find the support combo box (objectName approach or findChild).
    auto* combo = panel->findChild<QComboBox*>();
    ASSERT_NE(combo, nullptr) << "Expected a QComboBox on the node editor page";

    QSignalSpy spy{panel.get(), &InspectorPanel::supportChangeRequested};

    // Change to index 1 (Pinned).
    combo->setCurrentIndex(1);
    // onSupportComboChanged slot is triggered by setCurrentIndex.
    QApplication::processEvents();

    // The signal should have been emitted with the node's id and SupportType::Pinned.
    EXPECT_GE(spy.count(), 1);
}

// ============================================================
// Signal: loadChangeRequested
// ============================================================

TEST_F(InspectorPanelTest, ApplyLoadButton_EmitsLoadChangeRequested) {
    NodeView n = makeNode(NodeId{3}, 0.0, 0.0, SupportType::Free);
    panel->showNodeEditor(n);

    // Set spin box values (node editor page).
    for (auto* spin : panel->findChildren<QDoubleSpinBox*>()) {
        spin->setValue(0.0);
    }

    QSignalSpy spy{panel.get(), &InspectorPanel::loadChangeRequested};

    auto* btn = panel->findChild<QPushButton*>("inspector_applyLoadBtn");
    ASSERT_NE(btn, nullptr) << "Expected inspector_applyLoadBtn QPushButton on node editor page";
    QTest::mouseClick(btn, Qt::LeftButton);
    QApplication::processEvents();

    // Zero load → loadChangeRequested with {0, 0}
    EXPECT_GE(spy.count(), 1);
}

// ============================================================
// populateMaterialLibrary
// ============================================================

#include "application/material_library_service.hpp"
#include "core/model/types.hpp"

TEST_F(InspectorPanelTest, PopulateMaterialLibrary_EmptyVectors_DoesNotCrash) {
    ASSERT_NO_FATAL_FAILURE(panel->populateMaterialLibrary({}, {}));
}

TEST_F(InspectorPanelTest, PopulateMaterialLibrary_BeforeMemberEditor_StoresPresets) {
    // Populate before the member-editor page is opened (lazy-init path).
    truss::application::MaterialPreset steel;
    steel.name = "Steel";
    steel.description = "Structural steel";
    steel.properties.youngModulus = 200e9;
    steel.properties.density = 7850.0;
    steel.properties.yieldStrength = 250e6;

    truss::application::SectionPreset circ;
    circ.name = "50mm Circle";
    circ.description = "Solid circular cross-section";
    circ.properties.area = 1.963e-3;

    ASSERT_NO_FATAL_FAILURE(panel->populateMaterialLibrary({steel}, {circ}));

    // Open the member editor — ensureMemberEditorInteractive() should pick up the stored presets.
    panel->showMemberEditor(makeMember());
    QApplication::processEvents();

    EXPECT_EQ(panel->currentIndex(), InspectorPanel::kPageMemberEditor);

    // The material combo should now exist and have one entry.
    auto* matCombo = panel->findChild<QComboBox*>("inspector_materialCombo");
    ASSERT_NE(matCombo, nullptr);
    EXPECT_EQ(matCombo->count(), 1);
    EXPECT_EQ(matCombo->itemText(0), QStringLiteral("Steel"));
}

TEST_F(InspectorPanelTest, PopulateMaterialLibrary_AfterMemberEditor_RepopulatesCombo) {
    // Open member editor first (initialises lazy widgets with empty presets).
    panel->showMemberEditor(makeMember());
    QApplication::processEvents();

    // The member-editor interactive widgets are now live.
    // Calling populateMaterialLibrary() should repopulate the combo.
    truss::application::MaterialPreset aluminium;
    aluminium.name = "Aluminium";
    aluminium.description = "Aluminium alloy";
    aluminium.properties.youngModulus = 70e9;
    aluminium.properties.density = 2700.0;
    aluminium.properties.yieldStrength = 270e6;

    ASSERT_NO_FATAL_FAILURE(panel->populateMaterialLibrary({aluminium}, {}));
    QApplication::processEvents();

    auto* matCombo = panel->findChild<QComboBox*>("inspector_materialCombo");
    ASSERT_NE(matCombo, nullptr);
    EXPECT_EQ(matCombo->count(), 1);
    EXPECT_EQ(matCombo->itemText(0), QStringLiteral("Aluminium"));
}
