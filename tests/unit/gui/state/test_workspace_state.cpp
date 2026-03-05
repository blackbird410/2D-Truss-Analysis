/**
 * @file test_workspace_state.cpp
 * @brief Unit tests for WorkspaceState (Phase 6).
 *
 * WorkspaceState is a pure C++ value type (Qt-free). These tests verify:
 *  - Default construction yields WorkspacePhase::Empty.
 *  - hasTruss(), hasResults(), isAnalysing(), isResultsReady(), isEditable()
 *    return correct values for representative states.
 *  - operator== and operator!= satisfy value-equality semantics.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-05
 */

#include "gui/state/workspace_state.hpp"

#include <gtest/gtest.h>

using namespace truss::gui::state;

// ============================================================
// Tests — default construction
// ============================================================

TEST(WorkspaceStateTest, DefaultPhaseIsEmpty)
{
    WorkspaceState s;
    EXPECT_EQ(s.phase, WorkspacePhase::Empty);
}

TEST(WorkspaceStateTest, DefaultTrussHandleIsZero)
{
    WorkspaceState s;
    EXPECT_EQ(s.trussHandle, 0u);
}

TEST(WorkspaceStateTest, DefaultResultsHandleIsZero)
{
    WorkspaceState s;
    EXPECT_EQ(s.resultsHandle, 0u);
}

TEST(WorkspaceStateTest, DefaultIsDirtyIsFalse)
{
    WorkspaceState s;
    EXPECT_FALSE(s.isDirty);
}

TEST(WorkspaceStateTest, DefaultProjectNameIsEmpty)
{
    WorkspaceState s;
    EXPECT_TRUE(s.projectName.empty());
}

TEST(WorkspaceStateTest, DefaultLastErrorIsEmpty)
{
    WorkspaceState s;
    EXPECT_TRUE(s.lastError.empty());
}

// ============================================================
// Tests — hasTruss()
// ============================================================

TEST(WorkspaceStateTest, HasTrussReturnsFalseWhenHandleIsZero)
{
    WorkspaceState s;
    EXPECT_FALSE(s.hasTruss());
}

TEST(WorkspaceStateTest, HasTrussReturnsTrueWhenHandleNonZero)
{
    WorkspaceState s;
    s.trussHandle = 1u;
    EXPECT_TRUE(s.hasTruss());
}

// ============================================================
// Tests — hasResults()
// ============================================================

TEST(WorkspaceStateTest, HasResultsReturnsFalseWhenHandleIsZero)
{
    WorkspaceState s;
    EXPECT_FALSE(s.hasResults());
}

TEST(WorkspaceStateTest, HasResultsReturnsTrueWhenHandleNonZero)
{
    WorkspaceState s;
    s.resultsHandle = 7u;
    EXPECT_TRUE(s.hasResults());
}

// ============================================================
// Tests — isAnalysing()
// ============================================================

TEST(WorkspaceStateTest, IsAnalysingReturnsTrueOnlyForAnalysingPhase)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::Analysing;
    EXPECT_TRUE(s.isAnalysing());
}

TEST(WorkspaceStateTest, IsAnalysingReturnsFalseForOtherPhases)
{
    for (auto phase : {WorkspacePhase::Empty,
                       WorkspacePhase::ModelBuilding,
                       WorkspacePhase::Validating,
                       WorkspacePhase::ResultsReady}) {
        WorkspaceState s;
        s.phase = phase;
        EXPECT_FALSE(s.isAnalysing()) << "Phase should not be Analysing";
    }
}

// ============================================================
// Tests — isResultsReady()
// ============================================================

TEST(WorkspaceStateTest, IsResultsReadyReturnsTrueOnlyForResultsReadyPhase)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::ResultsReady;
    EXPECT_TRUE(s.isResultsReady());
}

TEST(WorkspaceStateTest, IsResultsReadyReturnsFalseForOtherPhases)
{
    for (auto phase : {WorkspacePhase::Empty,
                       WorkspacePhase::ModelBuilding,
                       WorkspacePhase::Validating,
                       WorkspacePhase::Analysing}) {
        WorkspaceState s;
        s.phase = phase;
        EXPECT_FALSE(s.isResultsReady()) << "Phase should not be ResultsReady";
    }
}

// ============================================================
// Tests — isEditable()
// ============================================================

TEST(WorkspaceStateTest, IsEditableReturnsTrueForModelBuilding)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::ModelBuilding;
    EXPECT_TRUE(s.isEditable());
}

TEST(WorkspaceStateTest, IsEditableReturnsTrueForResultsReady)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::ResultsReady;
    EXPECT_TRUE(s.isEditable());
}

TEST(WorkspaceStateTest, IsEditableReturnsFalseForEmpty)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::Empty;
    EXPECT_FALSE(s.isEditable());
}

TEST(WorkspaceStateTest, IsEditableReturnsFalseWhileAnalysing)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::Analysing;
    EXPECT_FALSE(s.isEditable());
}

// ============================================================
// Tests — value equality (operator== / operator!=)
// ============================================================

TEST(WorkspaceStateTest, DefaultConstructedStatesAreEqual)
{
    WorkspaceState a;
    WorkspaceState b;
    EXPECT_EQ(a, b);
}

TEST(WorkspaceStateTest, StatesWithDifferentPhasesAreNotEqual)
{
    WorkspaceState a;
    WorkspaceState b;
    b.phase = WorkspacePhase::ModelBuilding;
    EXPECT_NE(a, b);
}

TEST(WorkspaceStateTest, StatesWithDifferentHandlesAreNotEqual)
{
    WorkspaceState a;
    WorkspaceState b;
    b.trussHandle = 99u;
    EXPECT_NE(a, b);
}

TEST(WorkspaceStateTest, StatesWithDifferentIsDirtyAreNotEqual)
{
    WorkspaceState a;
    WorkspaceState b;
    b.isDirty = true;
    EXPECT_NE(a, b);
}

TEST(WorkspaceStateTest, StatesWithDifferentLastErrorAreNotEqual)
{
    WorkspaceState a;
    WorkspaceState b;
    b.lastError = "singular matrix";
    EXPECT_NE(a, b);
}

TEST(WorkspaceStateTest, CopiedStateEqualsOriginal)
{
    WorkspaceState a;
    a.phase         = WorkspacePhase::ResultsReady;
    a.trussHandle   = 5u;
    a.resultsHandle = 2u;
    a.isDirty       = true;
    a.projectName   = "bridge.truss";
    a.lastError     = "";

    WorkspaceState b = a;
    EXPECT_EQ(a, b);
}
