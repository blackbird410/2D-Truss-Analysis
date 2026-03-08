/**
 * @file member.cpp
 * @brief Truss member connecting two nodes with material properties.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "member.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace truss::core {

Member::Member(MemberId id,
               std::shared_ptr<Node> startNode,
               std::shared_ptr<Node> endNode,
               MaterialProperties material,
               SectionProperties section)
    : m_id(id), m_startNode(std::move(startNode)), m_endNode(std::move(endNode)),
      m_material(std::move(material)), m_section(std::move(section)),
      m_label("Member_" + std::to_string(id)) {
    validateNodes();
    updateResults();
}

Member& Member::operator=(const Member& other) {
    if (this != &other) {
        m_id = other.m_id;
        m_startNode = other.m_startNode;
        m_endNode = other.m_endNode;
        m_material = other.m_material;
        m_section = other.m_section;
        m_label = other.m_label;
        m_results = other.m_results;
    }
    return *this;
}

Real Member::getLength() const {
    return m_startNode->getPosition().distance(m_endNode->getPosition());
}

Real Member::getAngle() const {
    Vector2d dir = getDirection();
    return std::atan2(dir.y(), dir.x());
}

Real Member::getAngleDegrees() const {
    return truss::utils::math::radiansToDegrees(getAngle());
}

Vector2d Member::getUnitVector() const {
    Vector2d dir = getDirection();
    return dir.normalized();
}

Vector2d Member::getDirection() const {
    return m_endNode->getPosition().toEigen() - m_startNode->getPosition().toEigen();
}

Real Member::getStiffness() const {
    return (m_material.youngModulus * m_section.area) / getLength();
}

Real Member::getAxialStiffness() const {
    return m_material.youngModulus * m_section.area;
}

Real Member::getWeight() const {
    return m_section.area * getLength() * m_material.density;
}

void Member::setAxialForce(Real force) {
    m_results.axialForce = force;
    updateResults();
}

bool Member::isConnectedTo(const Node& node) const {
    return (m_startNode.get() == &node || m_endNode.get() == &node);
}

bool Member::isConnectedTo(NodeId nodeId) const {
    return (m_startNode->getId() == nodeId || m_endNode->getId() == nodeId);
}

bool Member::hasNode(NodeId nodeId) const {
    return isConnectedTo(nodeId);
}

bool Member::connectsNodes(NodeId id1, NodeId id2) const {
    return (m_startNode->getId() == id1 && m_endNode->getId() == id2) ||
           (m_startNode->getId() == id2 && m_endNode->getId() == id1);
}

std::shared_ptr<Node> Member::getOtherNode(const Node& node) const {
    if (m_startNode.get() == &node)
        return m_endNode;
    if (m_endNode.get() == &node)
        return m_startNode;
    throw std::invalid_argument("Node is not connected to member");
}

std::shared_ptr<Node> Member::getOtherNode(NodeId nodeId) const {
    if (m_startNode->getId() == nodeId)
        return m_endNode;
    if (m_endNode->getId() == nodeId)
        return m_startNode;
    throw std::invalid_argument("Node ID is not connected to member");
}

bool Member::isVertical(Real tolerance) const {
    return truss::utils::math::isZero(getDirection().x(), tolerance);
}

bool Member::isHorizontal(Real tolerance) const {
    return truss::utils::math::isZero(getDirection().y(), tolerance);
}

bool Member::isParallelTo(const Member& other, Real tolerance) const {
    // For 2D vectors, cross product is the z-component of 3D cross product
    Vector2d dir1 = getDirection();
    Vector2d dir2 = other.getDirection();
    Real crossProduct = (dir1.x() * dir2.y()) - (dir1.y() * dir2.x());
    return truss::utils::math::isZero(crossProduct, tolerance);
}

bool Member::isPerpendicularTo(const Member& other, Real tolerance) const {
    return truss::utils::math::isZero(getDirection().dot(other.getDirection()), tolerance);
}

bool Member::isValid() const {
    return !hasZeroLength() && m_startNode && m_endNode;
}

bool Member::hasZeroLength(Real tolerance) const {
    return truss::utils::math::isZero(getLength(), tolerance);
}

Matrix2d Member::getTransformationMatrix() const {
    Vector2d unitVec = getUnitVector();
    Matrix2d transformation = Matrix2d::Zero();
    transformation << unitVec.x(), unitVec.y(), -unitVec.y(), unitVec.x();
    return transformation;
}

std::vector<Index> Member::getGlobalDofIndices() const {
    return {
        m_startNode->getDofX(), m_startNode->getDofY(), m_endNode->getDofX(), m_endNode->getDofY()};
}

Point2D Member::getMidpoint() const {
    return {(m_startNode->getX() + m_endNode->getX()) / 2.0,
            (m_startNode->getY() + m_endNode->getY()) / 2.0};
}

Real Member::getSlope() const {
    Vector2d dir = getDirection();
    if (dir.x() == 0)
        return std::numeric_limits<Real>::infinity();
    return dir.y() / dir.x();
}

bool Member::intersectsWith(const Member& other, Real tolerance) const {
    // Get line segments
    Point2D p1 = m_startNode->getPosition();
    Point2D q1 = m_endNode->getPosition();
    Point2D p2 = other.m_startNode->getPosition();
    Point2D q2 = other.m_endNode->getPosition();

    // Check if lines are collinear or parallel
    Vector2d dir1 = getDirection();
    Vector2d dir2 = other.getDirection();
    Real crossProduct = (dir1.x() * dir2.y()) - (dir1.y() * dir2.x());

    if (truss::utils::math::isZero(crossProduct, tolerance)) {
        // Lines are parallel or collinear
        return false;
    }

    // Calculate intersection parameters
    Real dx1 = q1.x - p1.x;
    Real dy1 = q1.y - p1.y;
    Real dx2 = q2.x - p2.x;
    Real dy2 = q2.y - p2.y;
    Real dx12 = p1.x - p2.x;
    Real dy12 = p1.y - p2.y;

    Real denom = (dx1 * dy2) - (dy1 * dx2);
    if (truss::utils::math::isZero(denom, tolerance)) {
        return false;
    }

    Real t1 = ((dx2 * dy12) - (dy2 * dx12)) / denom;
    Real t2 = ((dx1 * dy12) - (dy1 * dx12)) / denom;

    // Check if intersection point lies within both line segments
    return (t1 >= -tolerance && t1 <= 1.0 + tolerance && t2 >= -tolerance && t2 <= 1.0 + tolerance);
}

Point2D Member::getIntersectionPoint(const Member& other) const {
    // Get line segments
    Point2D p1 = m_startNode->getPosition();
    Point2D q1 = m_endNode->getPosition();
    Point2D p2 = other.m_startNode->getPosition();
    Point2D q2 = other.m_endNode->getPosition();

    // Calculate intersection using parametric line equations
    Real dx1 = q1.x - p1.x;
    Real dy1 = q1.y - p1.y;
    Real dx2 = q2.x - p2.x;
    Real dy2 = q2.y - p2.y;
    Real dx12 = p1.x - p2.x;
    Real dy12 = p1.y - p2.y;

    Real denom = (dx1 * dy2) - (dy1 * dx2);
    if (truss::utils::math::isZero(denom)) {
        // Lines are parallel or collinear - no unique intersection
        throw std::runtime_error(
            "Cannot calculate intersection point: lines are parallel or collinear");
    }

    Real t1 = ((dx2 * dy12) - (dy2 * dx12)) / denom;

    // Calculate intersection point using parameter t1
    Point2D intersection;
    intersection.x = p1.x + (t1 * dx1);
    intersection.y = p1.y + (t1 * dy1);

    return intersection;
}

bool Member::operator==(const Member& other) const {
    return m_id == other.m_id && *m_startNode == *other.m_startNode &&
           *m_endNode == *other.m_endNode;
}

bool Member::operator!=(const Member& other) const {
    return !(*this == other);
}

void Member::updateResults() {
    m_results.axialStress = m_results.axialForce / m_section.area;
    m_results.utilizationRatio = m_results.axialStress / m_material.yieldStrength;
    m_results.inTension = m_results.axialForce > 0;
    m_results.yielded = m_results.utilizationRatio > 1.0;
}

MatrixXd Member::getLocalStiffnessMatrix() const {
    Real k = getStiffness();
    MatrixXd local(4, 4);
    local << k, 0, -k, 0, 0, 0, 0, 0, -k, 0, k, 0, 0, 0, 0, 0;
    return local;
}

MatrixXd Member::getGlobalStiffnessMatrix() const {
    MatrixXd local = getLocalStiffnessMatrix();

    // Create full transformation matrix
    Vector2d unitVec = getUnitVector();
    Real c = unitVec.x();
    Real s = unitVec.y();

    MatrixXd T(4, 4);
    T << c, s, 0, 0, -s, c, 0, 0, 0, 0, c, s, 0, 0, -s, c;

    return T.transpose() * local * T;
}

void Member::validateNodes() const {
    if (!m_startNode || !m_endNode) {
        throw std::invalid_argument("Member must have valid start and end nodes");
    }
}

}  // namespace truss::core
