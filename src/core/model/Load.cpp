/**
 * @file Load.cpp
 * @brief Implementation of the Load class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "Load.hpp"

namespace truss::core {

Load::Load(LoadId id, NodeId nodeId, const Force2D& force, const std::string& label)
    : m_id(id), m_nodeId(nodeId), m_type(LoadType::NodalForce), m_force(force), m_label(label) {
    if (m_label.empty()) {
        m_label = "Load_" + std::to_string(m_id);
    }
}

Load::Load(LoadId id, NodeId nodeId, Real fx, Real fy, const std::string& label)
    : Load(id, nodeId, Force2D(fx, fy), label) {
}

bool Load::isZero(Real tolerance) const {
    return Utils::isZero(m_force.fx, tolerance) && Utils::isZero(m_force.fy, tolerance);
}

bool Load::isHorizontal(Real tolerance) const {
    return Utils::isZero(m_force.fy, tolerance) && !Utils::isZero(m_force.fx, tolerance);
}

bool Load::isVertical(Real tolerance) const {
    return Utils::isZero(m_force.fx, tolerance) && !Utils::isZero(m_force.fy, tolerance);
}

bool Load::operator==(const Load& other) const {
    return m_id == other.m_id && 
           m_nodeId == other.m_nodeId &&
           Utils::isEqual(m_force.fx, other.m_force.fx) &&
           Utils::isEqual(m_force.fy, other.m_force.fy);
}

bool Load::operator!=(const Load& other) const {
    return !(*this == other);
}

} // namespace truss::core
