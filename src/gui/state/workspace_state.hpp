/**
 * @file workspace_state.hpp
 * @brief Workspace state value types for GUI state management.
 *
 * This header is intentionally Qt-free. It defines pure value types that
 * describe the complete state of the GUI workspace at any point in time.
 *
 * Owned by MainWindowController. All panels observe state transitions via
 * the stateChanged(WorkspaceState) signal; no panel holds a reference to
 * another panel's state.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace truss::gui::state {

/**
 * @brief Represents the current phase of the workspace workflow.
 *
 * Phases drive which controls are enabled/disabled. Advancing through the
 * workflow transitions the phase; panels subscribe to phase changes and
 * update their enabled state accordingly.
 *
 * State machine transitions:
 * @code
 * Empty ──► ModelBuilding ──► Validating ──► Analysing ──► ResultsReady
 *              ▲                              │               │
 *              └──────────────────────────────┘               │
 *              ▲                                              │
 *              └──────────────────────────────────────────────┘
 * @endcode
 */
enum class WorkspacePhase : std::uint8_t {
    Empty,          ///< No model loaded; canvas is empty
    ModelBuilding,  ///< Model is being constructed or edited interactively
    Validating,     ///< Validation is in progress (currently reserved for future async use)
    Analysing,      ///< Analysis is running on background thread; UI is partially locked
    ResultsReady    ///< Analysis completed successfully; all result panels are populated
};

/**
 * @brief Complete, immutable description of GUI workspace state at an instant.
 *
 * WorkspaceState is a plain value type: it is cheap to copy, equality-
 * comparable, and carries no pointers or reference semantics. MainWindowController
 * owns the canonical copy and emits stateChanged(WorkspaceState) whenever it
 * transitions. Panels replace their local copy on every emission.
 *
 * Handle values of 0 are treated as "invalid / not set".
 */
struct WorkspaceState {
    /// Current workflow phase
    WorkspacePhase phase{WorkspacePhase::Empty};

    /// Handle to the currently active truss (0 = no truss loaded)
    std::size_t trussHandle{0};

    /// Handle to the most recent analysis results (0 = no results)
    std::size_t resultsHandle{0};

    /// True when the model has unsaved modifications
    bool isDirty{false};

    /// Display name of the currently open project (empty when phase == Empty)
    std::string projectName;

    /// Most recent error description (empty when no error)
    std::string lastError;

    // -----------------------------------------------------------------------
    // Convenience query methods
    // -----------------------------------------------------------------------

    /// @brief Returns true when a valid truss has been loaded or created.
    [[nodiscard]] bool hasTruss() const noexcept { return trussHandle != 0; }

    /// @brief Returns true when analysis results are available.
    [[nodiscard]] bool hasResults() const noexcept { return resultsHandle != 0; }

    /// @brief Returns true when analysis is currently running on a background thread.
    [[nodiscard]] bool isAnalysing() const noexcept {
        return phase == WorkspacePhase::Analysing;
    }

    /// @brief Returns true when the phase is ResultsReady.
    [[nodiscard]] bool isResultsReady() const noexcept {
        return phase == WorkspacePhase::ResultsReady;
    }

    /// @brief Returns true when the workspace can accept structural edits.
    [[nodiscard]] bool isEditable() const noexcept {
        return phase == WorkspacePhase::ModelBuilding || phase == WorkspacePhase::ResultsReady;
    }

    // -----------------------------------------------------------------------
    // Value semantics
    // -----------------------------------------------------------------------

    [[nodiscard]] bool operator==(const WorkspaceState&) const noexcept = default;
    [[nodiscard]] bool operator!=(const WorkspaceState&) const noexcept = default;
};

}  // namespace truss::gui::state
