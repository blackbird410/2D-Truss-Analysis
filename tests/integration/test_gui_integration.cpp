/**
 * @file test_gui_integration.cpp
 * @brief Integration tests for the new GUI architecture signal/slot wiring.
 * @version 3.0.0
 * @date 2026-03-07
 * @author Neil Taison Rigaud
 *
 * @details
 * Replaces the legacy test_gui_integration.cpp (which tested the old
 * TrussEditController / ProjectController / Presenter architecture).
 * These tests verify that the new architecture internal connections,
 * established in MainWindowController's constructor, correctly propagate
 * signals through the controller graph.
 *
 * Tests cover:
 *  - CanvasController::trussModified wired to MainWindowController
 *  - NodeTableModel / MemberTableModel refreshed via trussViewChanged
 *  - AnalysisController signals wired to MainWindowController state transitions
 *  - resultsViewChanged fires after onAnalysisCompleted
 *  - trussViewChanged fires after onAnalysisCompleted (deformed view path)
 */

#include "application/result.hpp"
#include "core/interfaces/ianalysis_results_view.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "gui/controllers/analysis_controller.hpp"
#include "gui/controllers/canvas_controller.hpp"
#include "gui/controllers/main_window_controller.hpp"
#include "gui/models/member_table_model.hpp"
#include "gui/models/node_table_model.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::gui::ctrl;
using namespace truss::gui::state;
using namespace truss::core::interfaces;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using truss::test::MockTrussAnalysisFacade;

// ============================================================
//  Minimal ITrussView stub
// ============================================================

class StubTrussView final : public ITrussView {
public:
    explicit StubTrussView(int nodeCount = 0, int memberCount = 0) {
        for (int i = 0; i < nodeCount; ++i) {
            NodeView nv;
            nv.id = static_cast<truss::core::NodeId>(i + 1);
            nv.x = static_cast<double>(i);
            nv.y = 0.0;
            m_nodes.push_back(nv);
        }
        for (int i = 0; i < memberCount; ++i) {
            MemberView mv;
            mv.id = static_cast<truss::core::MemberId>(i + 1);
            mv.startNodeId = static_cast<truss::core::NodeId>(i + 1);
            mv.endNodeId = static_cast<truss::core::NodeId>(i + 2);
            m_members.push_back(mv);
        }
    }

    const std::string& getName() const override { return m_name; }
    std::vector<NodeView> getNodeViews() const override { return m_nodes; }
    std::size_t getNodeCount() const override { return m_nodes.size(); }
    std::vector<MemberView> getMemberViews() const override { return m_members; }
    std::size_t getMemberCount() const override { return m_members.size(); }
    std::size_t getTotalDofs() const override { return 0; }
    std::size_t getFreeDofs() const override { return 0; }
    std::size_t getConstrainedDofs() const override { return 0; }

private:
    std::string m_name{"StubTruss"};
    std::vector<NodeView> m_nodes;
    std::vector<MemberView> m_members;
};

// ============================================================
//  Minimal IAnalysisResultsView stub
// ============================================================

class StubResultsView final : public IAnalysisResultsView {
public:
    const std::vector<truss::core::Real>& getDisplacements() const override { return m_empty; }
    const std::vector<truss::core::Real>& getReactions() const override { return m_empty; }
    const std::vector<truss::core::Real>& getMemberForces() const override { return m_empty; }
    const std::vector<truss::core::Real>& getMemberStresses() const override { return m_empty; }
    const std::vector<truss::core::Real>& getUtilizationRatios() const override { return m_empty; }
    const std::vector<std::vector<truss::core::Real>>& getStiffnessMatrix() const override {
        return m_matrix;
    }
    bool hasConverged() const override { return true; }
    int getIterations() const override { return 1; }
    truss::core::Real getResidualNorm() const override { return 0.0; }
    truss::core::Real getConditionNumber() const override { return 1.0; }
    std::size_t getTotalDofs() const override { return 0; }
    std::size_t getFreeDofs() const override { return 0; }
    std::size_t getConstrainedDofs() const override { return 0; }
    truss::core::Real getTotalStrain() const override { return 0.0; }
    truss::core::Real getMaxDisplacement() const override { return 0.0; }
    truss::core::Real getMaxStress() const override { return 0.0; }

private:
    std::vector<truss::core::Real> m_empty;
    std::vector<std::vector<truss::core::Real>> m_matrix;
};

// ============================================================
//  Test Fixture
// ============================================================

class GUIArchIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_facade = std::make_unique<NiceMock<MockTrussAnalysisFacade>>();
        m_stubResults = std::make_unique<StubResultsView>();

        ON_CALL(*m_facade, isValidTrussHandle(_)).WillByDefault(Return(true));
        ON_CALL(*m_facade, isValidResultsHandle(_)).WillByDefault(Return(true));

        m_controller = std::make_unique<MainWindowController>(*m_facade);
    }

    void TearDown() override {
        m_controller.reset();
        m_stubResults.reset();
        m_facade.reset();
    }

    std::unique_ptr<NiceMock<MockTrussAnalysisFacade>> m_facade;
    std::unique_ptr<StubResultsView> m_stubResults;
    std::unique_ptr<MainWindowController> m_controller;
};

// ============================================================
//  Construction tests
// ============================================================

TEST_F(GUIArchIntegrationTest, AllSubControllersExistAfterConstruction) {
    EXPECT_NE(m_controller->canvasController(), nullptr);
    EXPECT_NE(m_controller->inspectorController(), nullptr);
    EXPECT_NE(m_controller->analysisController(), nullptr);
    EXPECT_NE(m_controller->projectController(), nullptr);
    EXPECT_NE(m_controller->exportController(), nullptr);
}

TEST_F(GUIArchIntegrationTest, AllModelsExistAfterConstruction) {
    EXPECT_NE(m_controller->nodeModel(), nullptr);
    EXPECT_NE(m_controller->memberModel(), nullptr);
    EXPECT_NE(m_controller->validationModel(), nullptr);
    EXPECT_NE(m_controller->resultsModel(), nullptr);
}

TEST_F(GUIArchIntegrationTest, InitialPhaseIsEmpty) {
    EXPECT_EQ(m_controller->state().phase, WorkspacePhase::Empty);
}

// ============================================================
//  CanvasController -> MainWindowController wiring
// ============================================================

TEST_F(GUIArchIntegrationTest, CanvasModified_TransitionsToModelBuilding) {
    QSignalSpy spy(m_controller.get(), &MainWindowController::stateChanged);
    StubTrussView view3(3, 0);
    EXPECT_CALL(*m_facade, getTrussView(1)).WillRepeatedly(ReturnRef(view3));

    emit m_controller->canvasController()->trussModified(1);
    QCoreApplication::processEvents();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(m_controller->state().phase, WorkspacePhase::ModelBuilding);
}

TEST_F(GUIArchIntegrationTest, CanvasModified_RefreshesNodeModel) {
    StubTrussView view5(5, 0);
    EXPECT_CALL(*m_facade, getTrussView(42)).WillRepeatedly(ReturnRef(view5));

    emit m_controller->canvasController()->trussModified(42);
    QCoreApplication::processEvents();

    EXPECT_EQ(m_controller->nodeModel()->rowCount({}), 5);
}

TEST_F(GUIArchIntegrationTest, CanvasModified_RefreshesMemberModel) {
    StubTrussView view(3, 2);
    EXPECT_CALL(*m_facade, getTrussView(7)).WillRepeatedly(ReturnRef(view));

    emit m_controller->canvasController()->trussModified(7);
    QCoreApplication::processEvents();

    EXPECT_EQ(m_controller->memberModel()->rowCount({}), 2);
}

TEST_F(GUIArchIntegrationTest, CanvasModified_EmitsTrussViewChangedWithNonNullPointer) {
    QSignalSpy spy(m_controller.get(), &MainWindowController::trussViewChanged);
    StubTrussView view(2, 1);
    EXPECT_CALL(*m_facade, getTrussView(3)).WillRepeatedly(ReturnRef(view));

    emit m_controller->canvasController()->trussModified(3);
    QCoreApplication::processEvents();

    ASSERT_EQ(spy.count(), 1);
    auto* ptr = spy.at(0).at(0).value<const ITrussView*>();
    EXPECT_NE(ptr, nullptr);
}

TEST_F(GUIArchIntegrationTest, SequentialEdits_NodeModelReflectsLatestView) {
    StubTrussView v1(1, 0), v2(2, 0), v3(3, 0);

    EXPECT_CALL(*m_facade, getTrussView(1)).WillRepeatedly(ReturnRef(v1));
    emit m_controller->canvasController()->trussModified(1);
    QCoreApplication::processEvents();
    EXPECT_EQ(m_controller->nodeModel()->rowCount({}), 1);

    EXPECT_CALL(*m_facade, getTrussView(1)).WillRepeatedly(ReturnRef(v2));
    emit m_controller->canvasController()->trussModified(1);
    QCoreApplication::processEvents();
    EXPECT_EQ(m_controller->nodeModel()->rowCount({}), 2);

    EXPECT_CALL(*m_facade, getTrussView(1)).WillRepeatedly(ReturnRef(v3));
    emit m_controller->canvasController()->trussModified(1);
    QCoreApplication::processEvents();
    EXPECT_EQ(m_controller->nodeModel()->rowCount({}), 3);
}

// ============================================================
//  AnalysisController -> MainWindowController wiring
// ============================================================

TEST_F(GUIArchIntegrationTest, AnalysisStarted_TransitionsToAnalysing) {
    StubTrussView view(2, 1);
    EXPECT_CALL(*m_facade, getTrussView(5)).WillRepeatedly(ReturnRef(view));
    emit m_controller->canvasController()->trussModified(5);
    QCoreApplication::processEvents();

    emit m_controller->analysisController()->analysisStarted();
    QCoreApplication::processEvents();

    EXPECT_EQ(m_controller->state().phase, WorkspacePhase::Analysing);
}

TEST_F(GUIArchIntegrationTest, AnalysisCompleted_EmitsResultsViewChangedWithNonNullPointer) {
    StubTrussView trussView(2, 1);
    EXPECT_CALL(*m_facade, getTrussView(10)).WillRepeatedly(ReturnRef(trussView));
    emit m_controller->canvasController()->trussModified(10);
    QCoreApplication::processEvents();

    QSignalSpy spy(m_controller.get(), &MainWindowController::resultsViewChanged);
    EXPECT_CALL(*m_facade, getResultsView(99)).WillRepeatedly(ReturnRef(*m_stubResults));

    emit m_controller->analysisController()->analysisCompleted(99);
    QCoreApplication::processEvents();

    ASSERT_EQ(spy.count(), 1);
    auto* ptr = spy.at(0).at(0).value<const IAnalysisResultsView*>();
    EXPECT_NE(ptr, nullptr);
}

TEST_F(GUIArchIntegrationTest, AnalysisCompleted_EmitsTrussViewChangedForDeformedCanvas) {
    StubTrussView trussView(3, 2);
    EXPECT_CALL(*m_facade, getTrussView(5)).WillRepeatedly(ReturnRef(trussView));
    emit m_controller->canvasController()->trussModified(5);
    QCoreApplication::processEvents();

    QSignalSpy spy(m_controller.get(), &MainWindowController::trussViewChanged);
    EXPECT_CALL(*m_facade, getResultsView(20)).WillRepeatedly(ReturnRef(*m_stubResults));

    emit m_controller->analysisController()->analysisCompleted(20);
    QCoreApplication::processEvents();

    EXPECT_GE(spy.count(), 1)
        << "trussViewChanged must fire after analysis so canvas can show the deformed shape";
}

TEST_F(GUIArchIntegrationTest, AnalysisCompleted_TransitionsToResultsReady) {
    StubTrussView trussView(2, 1);
    EXPECT_CALL(*m_facade, getTrussView(3)).WillRepeatedly(ReturnRef(trussView));
    emit m_controller->canvasController()->trussModified(3);
    QCoreApplication::processEvents();

    EXPECT_CALL(*m_facade, getResultsView(77)).WillRepeatedly(ReturnRef(*m_stubResults));

    emit m_controller->analysisController()->analysisCompleted(77);
    QCoreApplication::processEvents();

    EXPECT_EQ(m_controller->state().phase, WorkspacePhase::ResultsReady);
}

TEST_F(GUIArchIntegrationTest, AnalysisFailed_TransitionsToModelBuildingWithError) {
    StubTrussView trussView(2, 1);
    EXPECT_CALL(*m_facade, getTrussView(3)).WillRepeatedly(ReturnRef(trussView));
    emit m_controller->canvasController()->trussModified(3);
    QCoreApplication::processEvents();

    emit m_controller->analysisController()->analysisStarted();
    QCoreApplication::processEvents();
    ASSERT_EQ(m_controller->state().phase, WorkspacePhase::Analysing);

    emit m_controller->analysisController()->analysisFailed(
        QStringLiteral("Singular stiffness matrix"));
    QCoreApplication::processEvents();

    EXPECT_EQ(m_controller->state().phase, WorkspacePhase::ModelBuilding);
    EXPECT_FALSE(m_controller->state().lastError.empty());
}

// ============================================================
//  GTest main -- creates QApplication before running tests
// ============================================================

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
