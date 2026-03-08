/**
 * @file itruss_service.hpp
 * @brief Abstract interface for truss application services.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This interface defines the contract for truss lifecycle and editing operations,
 * enabling dependency injection and unit testing of GUI controllers.
 *
 * Design Pattern: Dependency Inversion Principle (depend on abstractions)
 * Architecture: Application Layer Interface
 */

#pragma once

#include "../../core/interfaces/itruss_view.hpp"
#include "../../core/model/types.hpp"
#include "../../core/validation/truss_validator.hpp"  // For ValidationResult
#include "../result.hpp"
#include "../truss_edit_dtos.hpp"

#include <filesystem>
#include <string>

// Forward declarations
namespace truss::core {
class Truss;
}

namespace truss::application {

/**
 * @brief Abstract interface for truss application services
 *
 * Enables polymorphic dependency injection for controllers,
 * allowing mock implementations for unit testing.
 *
 * All methods return Result<T> for consistent error handling.
 */
class ITrussService {
public:
    virtual ~ITrussService() = default;

    // ================================================================
    // Lifecycle operations
    // ================================================================

    /**
     * @brief Create a new empty truss model and register it with this service.
     * @param name Display name for the new truss.
     * @return Result containing the new TrussHandle on success, or a failure message.
     */
    virtual Result<TrussHandle> createTruss(const std::string& name) = 0;

    /**
     * @brief Load a truss model from a file (format auto-detected from extension).
     * @param filepath Path to the truss file (.json or .xml).
     * @return Result containing the loaded TrussHandle on success, or a failure message.
     */
    virtual Result<TrussHandle> loadTruss(const std::filesystem::path& filepath) = 0;

    /**
     * @brief Save a truss model to a file (format auto-detected from extension).
     * @param handle    Handle to the truss to save.
     * @param filepath  Destination path.
     * @param overwrite Whether to overwrite an existing file.
     * @return Result containing true on success, or a failure message.
     */
    virtual Result<bool> saveTruss(TrussHandle handle,
                                   const std::filesystem::path& filepath,
                                   bool overwrite = false) = 0;

    /**
     * @brief Remove all data associated with a specific truss handle.
     * @param handle Handle of the truss to clear.
     * @return true if the handle was valid and the truss was cleared, false otherwise.
     */
    virtual bool clearTruss(TrussHandle handle) = 0;

    /**
     * @brief Remove all trusses and analysis results managed by this service.
     */
    virtual void clearAll() = 0;

    /**
     * @brief Check whether a truss handle refers to a live truss instance.
     * @param handle Handle to validate.
     * @return true if the handle is valid, false if it has been cleared or never existed.
     */
    virtual bool isValidTrussHandle(TrussHandle handle) const = 0;

    // ================================================================
    // View access
    // ================================================================

    /**
     * @brief Obtain a read-only view of the truss for rendering / export.
     * @param handle Valid truss handle.
     * @return Const reference to the truss view (valid until the truss is mutated).
     * @note Behaviour is undefined if @p handle is invalid.
     */
    virtual const core::interfaces::ITrussView& getTrussView(TrussHandle handle) const = 0;

    /**
     * @brief Obtain a mutable reference to the underlying domain Truss object.
     *
     * Intended for Infrastructure and Application-layer components that must
     * write analysis results directly into the domain model.  GUI and CLI
     * consumers should use the editing methods (addNode, updateNode, etc.)
     * instead of mutating the domain object directly.
     *
     * @param handle Valid truss handle.
     * @return Mutable reference to the Truss (valid until the truss is cleared).
     * @note Behaviour is undefined if @p handle is invalid.
     */
    virtual core::Truss& getTrussMutable(TrussHandle handle) = 0;

    // ================================================================
    // Validation
    // ================================================================

    /**
     * @brief Run the full structural validation suite on a truss.
     * @param handle Handle of the truss to validate.
     * @return Result containing a ValidationResult (with issues list) on success,
     *         or a failure message if the handle is invalid.
     */
    virtual Result<core::validation::ValidationResult> validateTruss(TrussHandle handle) = 0;

    // ================================================================
    // Editing operations (Application DTOs)
    // ================================================================

    /**
     * @brief Add a new node at the given position.
     * @param handle      Truss handle.
     * @param position    World-space coordinate (metres, Y+ upward).
     * @param supportType Constraint type for the new node (default: Free).
     * @return Result containing the new NodeId on success, or a failure message.
     */
    virtual Result<core::NodeId>
    addNode(TrussHandle handle,
            const core::Point2D& position,
            core::SupportType supportType = core::SupportType::Free) = 0;

    /**
     * @brief Add a new member connecting two existing nodes.
     * @param handle      Truss handle.
     * @param startNodeId ID of the start node.
     * @param endNodeId   ID of the end node.
     * @param material    Material specification for the new member.
     * @param section     Cross-section specification for the new member.
     * @return Result containing the new MemberId on success, or a failure message.
     */
    virtual Result<core::MemberId> addMember(TrussHandle handle,
                                             core::NodeId startNodeId,
                                             core::NodeId endNodeId,
                                             const MaterialSpec& material,
                                             const SectionSpec& section) = 0;

    /**
     * @brief Remove a node and all members connected to it.
     * @param handle Truss handle.
     * @param nodeId ID of the node to remove.
     * @return Result containing true if removed, or a failure message if not found.
     */
    virtual Result<bool> removeNode(TrussHandle handle, core::NodeId nodeId) = 0;

    /**
     * @brief Remove a single member (nodes are preserved).
     * @param handle   Truss handle.
     * @param memberId ID of the member to remove.
     * @return Result containing true if removed, or a failure message if not found.
     */
    virtual Result<bool> removeMember(TrussHandle handle, core::MemberId memberId) = 0;

    /**
     * @brief Change the support constraint type of an existing node.
     * @param handle      Truss handle.
     * @param nodeId      ID of the node to modify.
     * @param supportType New support type.
     * @return Result containing true on success, or a failure message.
     */
    virtual Result<bool>
    setNodeSupport(TrussHandle handle, core::NodeId nodeId, core::SupportType supportType) = 0;

    /**
     * @brief Apply (or replace) an external force on a node.
     * @param handle Truss handle.
     * @param nodeId ID of the node to load.
     * @param force  Force vector (N, world-space X and Y components).
     * @return Result containing true on success, or a failure message.
     */
    virtual Result<bool>
    applyNodeLoad(TrussHandle handle, core::NodeId nodeId, const core::Force2D& force) = 0;

    /**
     * @brief Remove the applied load from a node (set to zero).
     * @param handle Truss handle.
     * @param nodeId ID of the node whose load is to be cleared.
     * @return Result containing true on success, or a failure message.
     */
    virtual Result<bool> clearNodeLoad(TrussHandle handle, core::NodeId nodeId) = 0;

    // ================================================================
    // Update operations  (preserve ID and structural identity)
    // ================================================================

    /**
     * @brief Update an existing node's position in-place.
     *
     * The node's ID, support condition, and applied loads are NOT affected.
     * Connected members retain their connectivity; their lengths and angles
     * are recomputed from the updated node coordinates on demand.
     *
     * @param handle  Truss handle.
     * @param nodeId  ID of the node to update.
     * @param update  New position (x, y in metres).
     * @return        Result<bool> — true on success, failure with message otherwise.
     */
    virtual Result<bool>
    updateNode(TrussHandle handle, core::NodeId nodeId, const NodeUpdateSpec& update) = 0;

    /**
     * @brief Update an existing member's material and section properties in-place.
     *
     * The member's ID and node connectivity are preserved.  Only the
     * material stiffness and cross-section area are replaced.  The caller
     * must re-run analysis to obtain updated results.
     *
     * @param handle    Truss handle.
     * @param memberId  ID of the member to update.
     * @param update    New material and section specifications.
     * @return          Result<bool> — true on success, failure with message otherwise.
     */
    virtual Result<bool>
    updateMember(TrussHandle handle, core::MemberId memberId, const MemberUpdateSpec& update) = 0;
};

}  // namespace truss::application
