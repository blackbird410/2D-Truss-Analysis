/**
 * @file Member.hpp
 * @brief Member class representing structural elements in the truss
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "node.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace truss::core {

/**
 * @brief Represents a structural member (beam/bar) in the truss system
 *
 * A member connects two nodes and carries axial forces. It has material
 * and geometric properties that define its structural behavior.
 */
class Member {
public:
    /**
     * @brief Construct a Member connecting two nodes
     * @param id Unique identifier for the member
     * @param startNode Pointer to the start node
     * @param endNode Pointer to the end node
     * @param material Material properties
     * @param section Section properties
     */
    Member(MemberId id,
           std::shared_ptr<Node> startNode,
           std::shared_ptr<Node> endNode,
           const MaterialProperties& material = MaterialProperties{},
           const SectionProperties& section = SectionProperties{});

    // Copy constructor (deep copy of properties, shared nodes)
    Member(const Member& other);

    // Move constructor
    Member(Member&& other) noexcept = default;

    // Assignment operators
    Member& operator=(const Member& other);
    Member& operator=(Member&& other) noexcept = default;

    // Destructor
    ~Member() = default;

    // Basic getters
    MemberId getId() const noexcept { return m_id; }
    std::shared_ptr<Node> getStartNode() const { return m_startNode; }
    std::shared_ptr<Node> getEndNode() const { return m_endNode; }
    const MaterialProperties& getMaterial() const noexcept { return m_material; }
    const SectionProperties& getSection() const noexcept { return m_section; }
    const std::string& getLabel() const noexcept { return m_label; }

    // Geometric properties
    Real getLength() const;
    Real getAngle() const;                          ///< Angle with respect to X-axis (radians)
    [[maybe_unused]] Real getAngleDegrees() const;  ///< Angle with respect to X-axis (degrees)
    Vector2d getUnitVector() const;                 ///< Unit vector from start to end node
    Vector2d getDirection() const;                  ///< Direction vector (not normalized)

    // Structural properties
    Real getStiffness() const;                        ///< Axial stiffness (EA/L)
    [[maybe_unused]] Real getAxialStiffness() const;  ///< Axial rigidity (EA)
    Real getWeight() const;                           ///< Self-weight of the member

    // Analysis results
    const MemberResults& getResults() const noexcept { return m_results; }
    Real getAxialForce() const noexcept { return m_results.axialForce; }
    Real getAxialStress() const noexcept { return m_results.axialStress; }
    Real getUtilizationRatio() const noexcept { return m_results.utilizationRatio; }
    bool isInTension() const noexcept { return m_results.inTension; }
    bool hasYielded() const noexcept { return m_results.yielded; }

    // Setters
    void setId(MemberId id) { m_id = id; }
    [[maybe_unused]] void setMaterial(const MaterialProperties& material) { m_material = material; }
    [[maybe_unused]] void setSection(const SectionProperties& section) { m_section = section; }
    void setLabel(const std::string& label) { m_label = label; }

    // Results setters (used during analysis)
    void setResults(const MemberResults& results) { m_results = results; }
    [[maybe_unused]] void setAxialForce(Real force);
    [[maybe_unused]] void setAxialStress(Real stress) { m_results.axialStress = stress; }
    [[maybe_unused]] void setUtilizationRatio(Real ratio) { m_results.utilizationRatio = ratio; }

    // Node connectivity
    bool isConnectedTo(const Node& node) const;
    bool isConnectedTo(NodeId nodeId) const;
    bool hasNode(NodeId nodeId) const;  ///< Check if member has specific node (by ID)
    [[maybe_unused]] bool
    connectsNodes(NodeId id1,
                  NodeId id2) const;  ///< Check if member connects two specific nodes
    [[maybe_unused]] std::shared_ptr<Node> getOtherNode(const Node& node) const;
    std::shared_ptr<Node> getOtherNode(NodeId nodeId) const;

    // Geometric checks
    bool isVertical(Real tolerance = Constants::GEOMETRY_TOLERANCE) const;
    bool isHorizontal(Real tolerance = Constants::GEOMETRY_TOLERANCE) const;
    [[maybe_unused]] bool isParallelTo(const Member& other,
                                       Real tolerance = Constants::GEOMETRY_TOLERANCE) const;
    [[maybe_unused]] bool isPerpendicularTo(const Member& other,
                                            Real tolerance = Constants::GEOMETRY_TOLERANCE) const;

    // Validation
    bool isValid() const;
    bool hasZeroLength(Real tolerance = Constants::GEOMETRY_TOLERANCE) const;

    // Local to global transformation
    [[maybe_unused]] Matrix2d getTransformationMatrix() const;
    MatrixXd getLocalStiffnessMatrix() const;
    MatrixXd getGlobalStiffnessMatrix() const;

    // DOF mapping for finite element assembly
    std::vector<Index> getGlobalDofIndices() const;

    // Utility methods
    [[maybe_unused]] Point2D getMidpoint() const;
    [[maybe_unused]] Real
    getSlope() const;  ///< Slope (dy/dx), returns infinity for vertical members

    /**
     * @brief Check if this member intersects with another member
     * @param other The other member to check intersection with
     * @param tolerance Geometric tolerance for intersection check
     * @return true if members intersect (excluding endpoints)
     */
    [[maybe_unused]] bool intersectsWith(const Member& other,
                                         Real tolerance = Constants::GEOMETRY_TOLERANCE) const;

    /**
     * @brief Get the intersection point with another member
     * @param other The other member
     * @return Intersection point (only valid if intersectsWith returns true)
     */
    [[maybe_unused]] Point2D getIntersectionPoint(const Member& other) const;

    // Comparison operators
    bool operator==(const Member& other) const;
    bool operator!=(const Member& other) const;

private:
    MemberId m_id{0};                   ///< Unique member identifier
    std::shared_ptr<Node> m_startNode;  ///< Start node
    std::shared_ptr<Node> m_endNode;    ///< End node
    MaterialProperties m_material;      ///< Material properties
    SectionProperties m_section;        ///< Section properties
    std::string m_label;                ///< Member label

    // Analysis results
    MemberResults m_results;  ///< Member analysis results

    // Helper methods
    void updateResults();        ///< Update derived results after force calculation
    void validateNodes() const;  ///< Validate node pointers
};

// Type aliases for convenience
using MemberPtr = std::shared_ptr<Member>;
using MemberConstPtr = std::shared_ptr<const Member>;
using MemberVector = std::vector<MemberPtr>;

}  // namespace truss::core
