/**
 * @file node.cpp
 * @brief Truss node with position and support constraints.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "node.hpp"

#include <cmath>

namespace truss::core {

Node::Node() : m_id(0), m_position(0.0, 0.0), m_supportType(SupportType::Free) {
    m_label = "Node_" + std::to_string(m_id);
}

Node::Node(NodeId id, Real x, Real y)
    : m_id(id), m_position(x, y), m_supportType(SupportType::Free) {
    m_label = "Node_" + std::to_string(m_id);
}

Node::Node(NodeId id, const Point2D& position, SupportType support)
    : m_id(id), m_position(position), m_supportType(support) {
    m_label = "Node_" + std::to_string(m_id);
}

bool Node::isConstrainedX() const noexcept {
    return m_supportType == SupportType::Pinned || m_supportType == SupportType::RollerY;
}

bool Node::isConstrainedY() const noexcept {
    return m_supportType == SupportType::Pinned || m_supportType == SupportType::RollerX;
}

bool Node::isFree() const noexcept {
    return m_supportType == SupportType::Free;
}

bool Node::isPinned() const noexcept {
    return m_supportType == SupportType::Pinned;
}

bool Node::isRoller() const noexcept {
    return m_supportType == SupportType::RollerX || m_supportType == SupportType::RollerY;
}

bool Node::isConstrained() const noexcept {
    return m_supportType != SupportType::Free;
}

int Node::getDegreesOfFreedom() const noexcept {
    return getDofCount();
}

std::vector<Index> Node::getGlobalDOFs() const {
    std::vector<Index> dofs;
    dofs.reserve(2);

    // Add X DOF if not constrained in X
    if (!isConstrainedX()) {
        dofs.push_back(m_dofX);
    }

    // Add Y DOF if not constrained in Y
    if (!isConstrainedY()) {
        dofs.push_back(m_dofY);
    }

    return dofs;
}

Real Node::distanceTo(const Node& other) const {
    return m_position.distance(other.m_position);
}

Real Node::distanceTo(const Point2D& point) const {
    return m_position.distance(point);
}

bool Node::isCoincidentWith(const Node& other, Real tolerance) const {
    return distanceTo(other) < tolerance;
}

bool Node::hasAppliedForce() const {
    return !Utils::isZero(m_appliedForce.fx) || !Utils::isZero(m_appliedForce.fy);
}

int Node::getConstraintCount() const {
    switch (m_supportType) {
        case SupportType::Free:
            return 0;
        case SupportType::RollerX:
        case SupportType::RollerY:
            return 1;
        case SupportType::Pinned:
            return 2;
        default:
            return 0;
    }
}

int Node::getDofCount() const {
    return 2 - getConstraintCount();
}

bool Node::operator==(const Node& other) const {
    return m_id == other.m_id;
}

bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

}  // namespace truss::core
