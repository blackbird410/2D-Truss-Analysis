/**
 * @file Truss.hpp
 * @brief Truss class representing the complete structural system
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "Types.hpp"
#include "Node.hpp"
#include "Member.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace truss::core {

/**
 * @brief Represents a complete 2D truss structural system
 * 
 * This class manages the collection of nodes and members that make up
 * a truss structure, providing methods for construction, validation,
 * and analysis preparation.
 */
class Truss {
public:
    /**
     * @brief Construct an empty truss
     * @param name Optional name for the truss
     */
    explicit Truss(const std::string& name = "Untitled Truss");
    
    // Copy and move constructors
    Truss(const Truss& other);
    Truss(Truss&& other) noexcept = default;
    
    // Assignment operators
    Truss& operator=(const Truss& other);
    Truss& operator=(Truss&& other) noexcept = default;
    
    // Destructor
    ~Truss() = default;
    
    // Basic properties
    const std::string& getName() const noexcept { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    
    // Node management
    NodePtr addNode(Real x, Real y, SupportType support = SupportType::Free);
    NodePtr addNode(const Point2D& position, SupportType support = SupportType::Free);
    NodePtr addNode(NodePtr node);
    bool removeNode(NodeId nodeId);
    bool removeNode(const NodePtr& node);
    
    // Member management
    MemberPtr addMember(NodeId startNodeId, NodeId endNodeId,
                       const MaterialProperties& material = MaterialProperties{},
                       const SectionProperties& section = SectionProperties{});
    MemberPtr addMember(const NodePtr& startNode, const NodePtr& endNode,
                       const MaterialProperties& material = MaterialProperties{},
                       const SectionProperties& section = SectionProperties{});
    MemberPtr addMember(MemberPtr member);
    bool removeMember(MemberId memberId);
    bool removeMember(const MemberPtr& member);
    
    // Access methods
    NodePtr getNode(NodeId nodeId) const;
    MemberPtr getMember(MemberId memberId) const;
    const NodeVector& getNodes() const noexcept { return m_nodes; }
    const MemberVector& getMembers() const noexcept { return m_members; }
    
    // Counts
    size_t getNodeCount() const noexcept { return m_nodes.size(); }
    size_t getMemberCount() const noexcept { return m_members.size(); }
    size_t getTotalDofs() const;
    size_t getFreeDofs() const;
    size_t getConstrainedDofs() const;
    
    // Geometric queries
    std::vector<NodePtr> getNodesInRegion(const Point2D& bottomLeft, const Point2D& topRight) const;
    std::vector<MemberPtr> getMembersConnectedTo(NodeId nodeId) const;
    std::vector<MemberPtr> getMembersConnectedTo(const NodePtr& node) const;
    std::vector<MemberPtr> getMembersAtNode(NodeId nodeId) const;  ///< Alias for getMembersConnectedTo
    
    // Node queries by state
    std::vector<NodePtr> getConstrainedNodes() const;              ///< Get all nodes with constraints
    std::vector<NodePtr> getLoadedNodes() const;                   ///< Get all nodes with applied forces
    std::vector<NodePtr> getFreeNodes() const;                     ///< Get all unconstrained nodes
    
    // Boundary conditions
    void applyForce(NodeId nodeId, const Force2D& force);
    void applyForce(NodeId nodeId, Real fx, Real fy);
    void setSupportType(NodeId nodeId, SupportType support);
    
    // Validation
    bool isValid() const;
    bool isStaticallyDeterminate() const;
    bool isKinematicallyStable() const;
    std::vector<std::string> getValidationErrors() const;
    
    // Analysis preparation
    void assignDofNumbers();
    size_t getNextNodeId() const { return m_nextNodeId; }
    size_t getNextMemberId() const { return m_nextMemberId; }
    
    // Utility methods
    void clear();
    Point2D getBoundingBoxMin() const;
    Point2D getBoundingBoxMax() const;
    Point2D getCentroid() const;
    Real getTotalWeight() const;
    
    // Load case management
    void clearForces();
    bool hasAppliedForces() const;
    
    // Member connectivity
    std::vector<std::vector<MemberId>> getConnectivityMatrix() const;
    
    // Find methods
    NodePtr findNearestNode(const Point2D& point, Real maxDistance = std::numeric_limits<Real>::max()) const;
    MemberPtr findNearestMember(const Point2D& point, Real maxDistance = std::numeric_limits<Real>::max()) const;
    std::vector<NodePtr> findCoincidentNodes(Real tolerance = Constants::GEOMETRY_TOLERANCE) const;
    
    // Statistics
    struct TrussStatistics {
        size_t totalNodes{0};
        size_t totalMembers{0};
        size_t freeDofs{0};
        size_t constrainedDofs{0};
        size_t appliedForces{0};
        Real totalLength{0.0};
        Real totalWeight{0.0};
        Point2D boundingBoxMin{0.0, 0.0};
        Point2D boundingBoxMax{0.0, 0.0};
    };
    
    TrussStatistics getStatistics() const;
    
private:
    std::string m_name;                                    ///< Truss name
    NodeVector m_nodes;                                    ///< Collection of nodes
    MemberVector m_members;                                ///< Collection of members
    std::unordered_map<NodeId, size_t> m_nodeIndexMap;    ///< Node ID to index mapping
    std::unordered_map<MemberId, size_t> m_memberIndexMap; ///< Member ID to index mapping
    NodeId m_nextNodeId{1};                               ///< Next available node ID
    MemberId m_nextMemberId{1};                           ///< Next available member ID
    
    // Helper methods
    void updateNodeIndexMap();
    void updateMemberIndexMap();
    bool isNodeReferenced(NodeId nodeId) const;
    void validateConnectivity() const;
    
    // DOF numbering
    void assignFreeDofNumbers();
    void assignConstrainedDofNumbers();
};

// Type aliases
using TrussPtr = std::shared_ptr<Truss>;
using TrussConstPtr = std::shared_ptr<const Truss>;

} // namespace truss::core
