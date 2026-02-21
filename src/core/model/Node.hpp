/**
 * @file Node.hpp
 * @brief Node class representing structural joints in the truss
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "Types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace truss::core {

/**
 * @brief Represents a structural node (joint) in the truss system
 *
 * A node is a connection point where members meet and where forces
 * and displacements are applied and calculated.
 */
class Node {
public:
    /**
     * @brief Construct a new Node at the origin
     */
    Node();

    /**
     * @brief Construct a new Node with given coordinates
     * @param id Unique identifier for the node
     * @param x X-coordinate
     * @param y Y-coordinate
     */
    Node(NodeId id, Real x, Real y);

    /**
     * @brief Construct a new Node with position and support
     * @param id Unique identifier for the node
     * @param position Node position
     * @param support Support constraint type
     */
    Node(NodeId id, const Point2D& position, SupportType support = SupportType::Free);

    // Copy and move constructors
    Node(const Node& other) = default;
    Node(Node&& other) noexcept = default;

    // Assignment operators
    Node& operator=(const Node& other) = default;
    Node& operator=(Node&& other) noexcept = default;

    // Destructor
    ~Node() = default;

    // Getters
    NodeId getId() const noexcept { return m_id; }
    const Point2D& getPosition() const noexcept { return m_position; }
    Real getX() const noexcept { return m_position.x; }
    Real getY() const noexcept { return m_position.y; }
    SupportType getSupportType() const noexcept { return m_supportType; }
    const Force2D& getAppliedForce() const noexcept { return m_appliedForce; }
    const std::string& getLabel() const noexcept { return m_label; }

    // DOF getters
    Index getDofX() const noexcept { return m_dofX; }
    Index getDofY() const noexcept { return m_dofY; }

    // Results getters
    const NodeResults& getResults() const noexcept { return m_results; }
    const Point2D& getDisplacement() const noexcept { return m_results.displacement; }
    const Force2D& getReaction() const noexcept { return m_results.reaction; }

    // Setters
    void setId(NodeId id) { m_id = id; }
    void setPosition(const Point2D& position) { m_position = position; }
    void setPosition(Real x, Real y) { m_position = Point2D(x, y); }
    void setSupportType(SupportType support) { m_supportType = support; }
    void setAppliedForce(const Force2D& force) { m_appliedForce = force; }
    void setAppliedForce(Real fx, Real fy) { m_appliedForce = Force2D(fx, fy); }
    [[maybe_unused]] void setLabel(const std::string& label) { m_label = label; }

    // DOF setters (used during analysis setup)
    void setDofX(Index dof) { m_dofX = dof; }
    void setDofY(Index dof) { m_dofY = dof; }

    // Results setters (used during analysis)
    void setResults(const NodeResults& results) { m_results = results; }
    [[maybe_unused]] void setDisplacement(const Point2D& displacement) {
        m_results.displacement = displacement;
    }
    [[maybe_unused]] void setDisplacement(Real dx, Real dy) {
        m_results.displacement = Point2D(dx, dy);
    }
    [[maybe_unused]] void setReaction(const Force2D& reaction) {
        m_results.reaction = reaction;
    }
    [[maybe_unused]] void setReaction(Real fx, Real fy) {
        m_results.reaction = Force2D(fx, fy);
    }

    // Constraint checking
    bool isConstrainedX() const noexcept;
    bool isConstrainedY() const noexcept;
    [[maybe_unused]] bool isFree() const noexcept;
    bool isPinned() const noexcept;
    bool isRoller() const noexcept;
    bool isConstrained() const noexcept;

    // Degrees of freedom queries
    int getDegreesOfFreedom() const noexcept;
    [[maybe_unused]] std::vector<Index> getGlobalDOFs() const;

    // Utility methods
    Real distanceTo(const Node& other) const;
    Real distanceTo(const Point2D& point) const;

    /**
     * @brief Check if this node is at the same position as another (within tolerance)
     */
    [[maybe_unused]] bool isCoincidentWith(
        const Node& other, Real tolerance = Constants::GEOMETRY_TOLERANCE) const;

    /**
     * @brief Check if the node has any applied forces
     */
    bool hasAppliedForce() const;

    /**
     * @brief Get total number of constraints at this node
     */
    int getConstraintCount() const;

    /**
     * @brief Get number of degrees of freedom at this node
     */
    int getDofCount() const;

    // Comparison operators
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

private:
    NodeId m_id{0};                                ///< Unique node identifier
    Point2D m_position{0.0, 0.0};                  ///< Node coordinates
    SupportType m_supportType{SupportType::Free};  ///< Support constraint type
    Force2D m_appliedForce{0.0, 0.0};              ///< Applied external force
    std::string m_label;                           ///< Optional node label

    // Degrees of freedom indices (set during analysis)
    Index m_dofX{0};  ///< X-direction DOF index
    Index m_dofY{0};  ///< Y-direction DOF index

    // Analysis results
    NodeResults m_results;  ///< Node analysis results
};

// Type aliases for convenience
using NodePtr = std::shared_ptr<Node>;
using NodeConstPtr = std::shared_ptr<const Node>;
using NodeVector = std::vector<NodePtr>;

}  // namespace truss::core
