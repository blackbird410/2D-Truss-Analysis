/**
 * @file test_workspace_state.cpp
 * @brief Unit tests for WorkspaceState (Phase 6).
 *
 * WorkspaceState is a pure C++ value type (Qt-free). These tests verify:
 *  - Default construction yields WorkspacePhase::Empty.
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

