/**
 * @file Truss.cpp
 * @brief Implementation of the Truss class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "Truss.hpp"

#include <algorithm>
#include <stdexcept>

namespace truss::core {

Truss::Truss(const std::string& name) : m_name(name) {}

Truss::Truss(const Truss& other)
    : m_name(other.m_name), m_nodes(other.m_nodes), m_members(other.m_members),
      m_nodeIndexMap(other.m_nodeIndexMap), m_memberIndexMap(other.m_memberIndexMap),
      m_nextNodeId(other.m_nextNodeId), m_nextMemberId(other.m_nextMemberId) {}

Truss& Truss::operator=(const Truss& other) {
    if (this != &other) {
        m_name = other.m_name;
        m_nodes = other.m_nodes;
        m_members = other.m_members;
        m_nodeIndexMap = other.m_nodeIndexMap;
        m_memberIndexMap = other.m_memberIndexMap;
        m_nextNodeId = other.m_nextNodeId;
        m_nextMemberId = other.m_nextMemberId;
    }
    return *this;
}

NodePtr Truss::addNode(Real x, Real y, SupportType support) {
    return addNode(Point2D(x, y), support);
}

NodePtr Truss::addNode(const Point2D& position, SupportType support) {
    auto node = std::make_shared<Node>(m_nextNodeId++, position, support);
    m_nodes.push_back(node);
    updateNodeIndexMap();
    return node;
}

NodePtr Truss::addNode(NodePtr node) {
    if (node) {
        node->setId(m_nextNodeId++);
        m_nodes.push_back(node);
        updateNodeIndexMap();
    }
    return node;
}

MemberPtr Truss::addMember(NodeId startNodeId,
                           NodeId endNodeId,
                           const MaterialProperties& material,
                           const SectionProperties& section) {
    auto startNode = getNode(startNodeId);
    auto endNode = getNode(endNodeId);
    if (!startNode || !endNode) {
        throw std::invalid_argument("Invalid node IDs for member");
    }
    return addMember(startNode, endNode, material, section);
}

MemberPtr Truss::addMember(const NodePtr& startNode,
                           const NodePtr& endNode,
                           const MaterialProperties& material,
                           const SectionProperties& section) {
    if (!startNode || !endNode) {
        throw std::invalid_argument("Invalid nodes for member");
    }

    auto member = std::make_shared<Member>(m_nextMemberId++, startNode, endNode, material, section);
    m_members.push_back(member);
    updateMemberIndexMap();
    return member;
}

MemberPtr Truss::addMember(MemberPtr member) {
    if (member) {
        member->setId(m_nextMemberId++);
        m_members.push_back(member);
        updateMemberIndexMap();
    }
    return member;
}

NodePtr Truss::getNode(NodeId nodeId) const {
    for (const auto& node : m_nodes) {
        if (node->getId() == nodeId) {
            return node;
        }
    }
    return nullptr;
}

MemberPtr Truss::getMember(MemberId memberId) const {
    for (const auto& member : m_members) {
        if (member->getId() == memberId) {
            return member;
        }
    }
    return nullptr;
}

std::vector<MemberPtr> Truss::getMembersConnectedTo(NodeId nodeId) const {
    std::vector<MemberPtr> connectedMembers;
    for (const auto& member : m_members) {
        if (member->hasNode(nodeId)) {
            connectedMembers.push_back(member);
        }
    }
    return connectedMembers;
}

std::vector<MemberPtr> Truss::getMembersConnectedTo(const NodePtr& node) const {
    if (!node) {
        return std::vector<MemberPtr>();
    }
    return getMembersConnectedTo(node->getId());
}

std::vector<MemberPtr> Truss::getMembersAtNode(NodeId nodeId) const {
    return getMembersConnectedTo(nodeId);
}

std::vector<NodePtr> Truss::getConstrainedNodes() const {
    std::vector<NodePtr> result;
    for (const auto& node : m_nodes) {
        if (node->isConstrained()) {
            result.push_back(node);
        }
    }
    return result;
}

std::vector<NodePtr> Truss::getLoadedNodes() const {
    std::vector<NodePtr> result;
    for (const auto& node : m_nodes) {
        if (node->hasAppliedForce()) {
            result.push_back(node);
        }
    }
    return result;
}

std::vector<NodePtr> Truss::getFreeNodes() const {
    std::vector<NodePtr> result;
    for (const auto& node : m_nodes) {
        if (!node->isConstrained()) {
            result.push_back(node);
        }
    }
    return result;
}

std::vector<NodePtr> Truss::getNodesInRegion(const Point2D& bottomLeft,
                                             const Point2D& topRight) const {
    std::vector<NodePtr> result;
    for (const auto& node : m_nodes) {
        Real x = node->getX();
        Real y = node->getY();
        if (x >= bottomLeft.x && x <= topRight.x && y >= bottomLeft.y && y <= topRight.y) {
            result.push_back(node);
        }
    }
    return result;
}

size_t Truss::getTotalDofs() const {
    return m_nodes.size() * 2;  // 2 DOFs per node (X and Y)
}

size_t Truss::getFreeDofs() const {
    size_t freeDofs = 0;
    for (const auto& node : m_nodes) {
        SupportType support = node->getSupportType();

        // Count free X DOF
        if (support == SupportType::Free || support == SupportType::RollerX) {
            freeDofs++;
        }

        // Count free Y DOF
        if (support == SupportType::Free || support == SupportType::RollerY) {
            freeDofs++;
        }
    }
    return freeDofs;
}

size_t Truss::getConstrainedDofs() const {
    return getTotalDofs() - getFreeDofs();
}

void Truss::applyForce(NodeId nodeId, const Force2D& force) const {
    auto node = getNode(nodeId);
    if (node) {
        node->setAppliedForce(force);
    }
}

void Truss::applyForce(NodeId nodeId, Real fx, Real fy) const {
    applyForce(nodeId, Force2D(fx, fy));
}

void Truss::setSupportType(NodeId nodeId, SupportType support) const {
    auto node = getNode(nodeId);
    if (node) {
        node->setSupportType(support);
    }
}

// DEPRECATED: Primitive validation check. Use TrussValidator for comprehensive validation.
// Scheduled for removal in v4.0.0.
bool Truss::isValid() const {
    // Check minimum requirements
    if (m_nodes.size() < 2 || m_members.size() < 1) {
        return false;
    }

    // Check that all members have valid nodes
    for (const auto& member : m_members) {
        if (!member || !member->isValid()) {
            return false;
        }
    }

    return true;
}

void Truss::assignDofNumbers() {
    size_t dofIndex = 0;

    // Assign DOF numbers to all nodes
    for (auto& node : m_nodes) {
        node->setDofX(dofIndex++);
        node->setDofY(dofIndex++);
    }
}

void Truss::clear() {
    m_nodes.clear();
    m_members.clear();
    m_nodeIndexMap.clear();
    m_memberIndexMap.clear();
    m_nextNodeId = 1;
    m_nextMemberId = 1;
}

bool Truss::hasAppliedForces() const {
    for (const auto& node : m_nodes) {
        if (node->hasAppliedForce()) {
            return true;
        }
    }
    return false;
}

void Truss::updateNodeIndexMap() {
    m_nodeIndexMap.clear();
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        m_nodeIndexMap[m_nodes[i]->getId()] = i;
    }
}

void Truss::updateMemberIndexMap() {
    m_memberIndexMap.clear();
    for (size_t i = 0; i < m_members.size(); ++i) {
        m_memberIndexMap[m_members[i]->getId()] = i;
    }
}

// Additional missing methods that might be needed

bool Truss::removeNode(NodeId nodeId) {
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeId](const NodePtr& node) {
        return node->getId() == nodeId;
    });
    if (it != m_nodes.end()) {
        m_nodes.erase(it);
        updateNodeIndexMap();
        return true;
    }
    return false;
}

bool Truss::removeNode(const NodePtr& node) {
    if (!node)
        return false;
    return removeNode(node->getId());
}

bool Truss::removeMember(MemberId memberId) {
    auto it = std::find_if(m_members.begin(), m_members.end(), [memberId](const MemberPtr& member) {
        return member->getId() == memberId;
    });
    if (it != m_members.end()) {
        m_members.erase(it);
        updateMemberIndexMap();
        return true;
    }
    return false;
}

bool Truss::removeMember(const MemberPtr& member) {
    if (!member)
        return false;
    return removeMember(member->getId());
}

std::vector<std::string> Truss::getValidationErrors() const {
    std::vector<std::string> errors;

    if (m_nodes.size() < 2) {
        errors.push_back("Truss must have at least 2 nodes");
    }

    if (m_members.size() < 1) {
        errors.push_back("Truss must have at least 1 member");
    }

    // Static determinacy: 2n = m + r (where n = nodes, m = members, r = reactions)
    size_t n = m_nodes.size();
    size_t m = m_members.size();
    size_t r = getConstrainedDofs();
    if (2 * n != m + r) {
        errors.push_back("Truss is not statically determinate");
    }

    // Kinematic stability: minimum 3 constraints to prevent rigid body motion
    if (getConstrainedDofs() < 3) {
        errors.push_back("Truss is not kinematically stable");
    }

    return errors;
}

Point2D Truss::getBoundingBoxMin() const {
    if (m_nodes.empty())
        return Point2D(0, 0);

    Real minX = m_nodes[0]->getX();
    Real minY = m_nodes[0]->getY();

    for (const auto& node : m_nodes) {
        minX = std::min(minX, node->getX());
        minY = std::min(minY, node->getY());
    }

    return Point2D(minX, minY);
}

Point2D Truss::getBoundingBoxMax() const {
    if (m_nodes.empty())
        return Point2D(0, 0);

    Real maxX = m_nodes[0]->getX();
    Real maxY = m_nodes[0]->getY();

    for (const auto& node : m_nodes) {
        maxX = std::max(maxX, node->getX());
        maxY = std::max(maxY, node->getY());
    }

    return Point2D(maxX, maxY);
}

Point2D Truss::getCentroid() const {
    if (m_nodes.empty())
        return Point2D(0, 0);

    Real sumX = 0, sumY = 0;
    for (const auto& node : m_nodes) {
        sumX += node->getX();
        sumY += node->getY();
    }

    return Point2D(sumX / m_nodes.size(), sumY / m_nodes.size());
}

Real Truss::getTotalWeight() const {
    Real totalWeight = 0;
    for (const auto& member : m_members) {
        totalWeight += member->getWeight();
    }
    return totalWeight;
}

void Truss::clearForces() {
    for (auto& node : m_nodes) {
        node->setAppliedForce(Force2D(0, 0));
    }
}

Truss::TrussStatistics Truss::getStatistics() const {
    TrussStatistics stats;
    stats.totalNodes = getNodeCount();
    stats.totalMembers = getMemberCount();
    stats.freeDofs = getFreeDofs();
    stats.constrainedDofs = getConstrainedDofs();
    stats.totalWeight = getTotalWeight();
    stats.boundingBoxMin = getBoundingBoxMin();
    stats.boundingBoxMax = getBoundingBoxMax();

    // Count applied forces
    for (const auto& node : m_nodes) {
        if (node->hasAppliedForce()) {
            stats.appliedForces++;
        }
    }

    // Calculate total length
    for (const auto& member : m_members) {
        stats.totalLength += member->getLength();
    }

    return stats;
}

std::vector<interfaces::NodeView> Truss::getNodeViews() const {
    std::vector<interfaces::NodeView> views;
    views.reserve(m_nodes.size());

    for (const auto& node : m_nodes) {
        interfaces::NodeView view;
        view.id = node->getId();
        view.x = node->getPosition().x;
        view.y = node->getPosition().y;
        view.support = node->getSupportType();
        view.fx = node->getAppliedForce().fx;
        view.fy = node->getAppliedForce().fy;
        view.dx = node->getDisplacement().x;
        view.dy = node->getDisplacement().y;
        view.rx = node->getReaction().fx;
        view.ry = node->getReaction().fy;
        views.push_back(view);
    }

    return views;
}

std::vector<interfaces::MemberView> Truss::getMemberViews() const {
    std::vector<interfaces::MemberView> views;
    views.reserve(m_members.size());

    for (const auto& member : m_members) {
        interfaces::MemberView view;
        view.id = member->getId();
        view.startNodeId = member->getStartNode()->getId();
        view.endNodeId = member->getEndNode()->getId();
        view.label = member->getLabel();
        view.youngModulus = member->getMaterial().youngModulus;
        view.yieldStrength = member->getMaterial().yieldStrength;
        view.density = member->getMaterial().density;
        view.area = member->getSection().area;
        view.length = member->getLength();
        view.angle = member->getAngle();
        view.axialForce = member->getAxialForce();
        view.axialStress = member->getAxialStress();
        view.utilizationRatio = member->getUtilizationRatio();
        view.inTension = member->isInTension();
        view.yielded = member->hasYielded();
        views.push_back(view);
    }

    return views;
}

}  // namespace truss::core
