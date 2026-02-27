/**
 * @file truss_builder.hpp
 * @brief Builder pattern for fluent truss construction
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 *
 * Provides a fluent API for programmatic truss construction, simplifying
 * the process of creating truss structures without direct manipulation of
 * Domain objects.
 *
 * Design Pattern: Builder (Fluent Interface)
 * Architecture: Interface Layer (simplifies Domain construction)
 *
 * Usage Example:
 * @code
 * TrussBuilder builder("Bridge");
 * auto truss = builder
 *     .addNode(0.0, 0.0, SupportType::Pinned)
 *     .addNode(4.0, 0.0, SupportType::RollerY)
 *     .addNode(2.0, 3.0, SupportType::Free)
 *     .addMember(1, 2, steel, section)
 *     .addMember(1, 3, steel, section)
 *     .addMember(2, 3, steel, section)
 *     .applyForce(3, 0.0, -10000.0)
 *     .build();
 * @endcode
 */

#pragma once

#include "../core/model/truss.hpp"
#include "../core/model/types.hpp"
#include "../application/truss_edit_dtos.hpp"

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

namespace truss::interface {

/**
 * @brief Fluent builder for constructing Truss objects
 *
 * TrussBuilder provides a convenient, type-safe API for building
 * truss structures. It validates construction order and ensures
 * referential integrity before creating the final Truss object.
 *
 * Features:
 * - Fluent interface (method chaining)
 * - Automatic node ID assignment
 * - Referential integrity validation
 * - Clear error messages for invalid construction
 *
 * Thread Safety: Not thread-safe (single-threaded construction)
 */
class TrussBuilder {
public:
    /**
     * @brief Construct a new Truss Builder
     * @param name Optional name for the truss (default: "Untitled Truss")
     */
    explicit TrussBuilder(const std::string& name = "Untitled Truss");

    // Disable copy (builder maintains internal state)
    TrussBuilder(const TrussBuilder&) = delete;
    TrussBuilder& operator=(const TrussBuilder&) = delete;

    // Allow move
    TrussBuilder(TrussBuilder&&) noexcept = default;
    TrussBuilder& operator=(TrussBuilder&&) noexcept = default;

    ~TrussBuilder() = default;

    /**
     * @brief Set the name of the truss
     * @param name Truss name
     * @return Reference to this builder (for chaining)
     */
    TrussBuilder& setName(const std::string& name);

    /**
     * @brief Add a node to the truss
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param support Support type (default: Free)
     * @return Reference to this builder (for chaining)
     *
     * Node IDs are automatically assigned sequentially starting from 1.
     */
    TrussBuilder& addNode(double x, double y, 
                          core::SupportType support = core::SupportType::Free);

    /**
     * @brief Add a member connecting two nodes
     * @param startNodeId Starting node ID (1-based)
     * @param endNodeId Ending node ID (1-based)
     * @param material Material properties
     * @param section Section properties
     * @return Reference to this builder (for chaining)
     * @throws std::invalid_argument if node IDs are invalid
     */
    TrussBuilder& addMember(core::NodeId startNodeId,
                            core::NodeId endNodeId,
                            const application::MaterialSpec& material,
                            const application::SectionSpec& section);

    /**
     * @brief Add a member with default material/section
     * @param startNodeId Starting node ID (1-based)
     * @param endNodeId Ending node ID (1-based)
     * @return Reference to this builder (for chaining)
     * @throws std::invalid_argument if node IDs are invalid
     */
    TrussBuilder& addMember(core::NodeId startNodeId,
                            core::NodeId endNodeId);

    /**
     * @brief Apply a force to a node
     * @param nodeId Node ID (1-based)
     * @param fx Force in X direction (N)
     * @param fy Force in Y direction (N)
     * @return Reference to this builder (for chaining)
     * @throws std::invalid_argument if node ID is invalid
     */
    TrussBuilder& applyForce(core::NodeId nodeId, double fx, double fy);

    /**
     * @brief Apply a force to a node
     * @param nodeId Node ID (1-based)
     * @param force Force vector
     * @return Reference to this builder (for chaining)
     * @throws std::invalid_argument if node ID is invalid
     */
    TrussBuilder& applyForce(core::NodeId nodeId, const core::Force2D& force);

    /**
     * @brief Set support type for a node
     * @param nodeId Node ID (1-based)
     * @param support New support type
     * @return Reference to this builder (for chaining)
     * @throws std::invalid_argument if node ID is invalid
     */
    TrussBuilder& setSupport(core::NodeId nodeId, core::SupportType support);

    /**
     * @brief Build and return the constructed truss
     * @return Shared pointer to the constructed Truss
     * @throws std::runtime_error if truss is invalid (no nodes, no members, etc.)
     */
    std::shared_ptr<core::Truss> build();

    /**
     * @brief Reset the builder to initial state
     * @return Reference to this builder (for chaining)
     *
     * Clears all nodes, members, forces, and resets to default name.
     */
    TrussBuilder& reset();

    /**
     * @brief Get the number of nodes added
     * @return Number of nodes
     */
    [[nodiscard]] size_t getNodeCount() const noexcept { return m_nodes.size(); }

    /**
     * @brief Get the number of members added
     * @return Number of members
     */
    [[nodiscard]] size_t getMemberCount() const noexcept { return m_members.size(); }

    /**
     * @brief Get the number of forces applied
     * @return Number of forces
     */
    [[nodiscard]] size_t getForceCount() const noexcept { return m_forces.size(); }

    /**
     * @brief Get the next node ID that would be assigned
     * @return Next node ID
     */
    [[nodiscard]] core::NodeId getNextNodeId() const noexcept { return m_nextNodeId; }

private:
    struct NodeData {
        double x, y;
        core::SupportType support;
    };

    struct MemberData {
        core::NodeId startNodeId;
        core::NodeId endNodeId;
        application::MaterialSpec material;
        application::SectionSpec section;
    };

    struct ForceData {
        core::NodeId nodeId;
        core::Force2D force;
    };

    std::string m_name;
    std::vector<NodeData> m_nodes;
    std::vector<MemberData> m_members;
    std::vector<ForceData> m_forces;
    core::NodeId m_nextNodeId{1};

    // Validation helpers
    bool isValidNodeId(core::NodeId nodeId) const noexcept;
    void validateNodeReference(core::NodeId nodeId, const std::string& context) const;
    application::MaterialSpec getDefaultMaterial() const;
    application::SectionSpec getDefaultSection() const;
};

}  // namespace truss::interface
