/**
 * @file truss_builder.cpp
 * @brief Implementation of TrussBuilder fluent API
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 */

#include "truss_builder.hpp"

#include <sstream>

namespace truss::interface {

TrussBuilder::TrussBuilder(const std::string& name) : m_name(name) {}

TrussBuilder& TrussBuilder::setName(const std::string& name) {
    m_name = name;
    return *this;
}

TrussBuilder& TrussBuilder::addNode(double x, double y, core::SupportType support) {
    m_nodes.push_back({x, y, support});
    m_nextNodeId++;
    return *this;
}

TrussBuilder& TrussBuilder::addMember(core::NodeId startNodeId,
                                      core::NodeId endNodeId,
                                      const application::MaterialSpec& material,
                                      const application::SectionSpec& section) {
    // Validate node references
    validateNodeReference(startNodeId, "addMember start node");
    validateNodeReference(endNodeId, "addMember end node");

    // Additional validation: prevent self-loops
    if (startNodeId == endNodeId) {
        throw std::invalid_argument(
            "Cannot create member: start and end nodes must be different (node " +
            std::to_string(startNodeId) + ")");
    }

    m_members.push_back({startNodeId, endNodeId, material, section});
    return *this;
}

TrussBuilder& TrussBuilder::addMember(core::NodeId startNodeId, core::NodeId endNodeId) {
    return addMember(startNodeId, endNodeId, getDefaultMaterial(), getDefaultSection());
}

TrussBuilder& TrussBuilder::applyForce(core::NodeId nodeId, double fx, double fy) {
    return applyForce(nodeId, core::Force2D(fx, fy));
}

TrussBuilder& TrussBuilder::applyForce(core::NodeId nodeId, const core::Force2D& force) {
    validateNodeReference(nodeId, "applyForce");
    m_forces.push_back({nodeId, force});
    return *this;
}

TrussBuilder& TrussBuilder::setSupport(core::NodeId nodeId, core::SupportType support) {
    validateNodeReference(nodeId, "setSupport");

    // Update the support type of the existing node
    size_t index = static_cast<size_t>(nodeId - 1);
    m_nodes[index].support = support;
    return *this;
}

std::shared_ptr<core::Truss> TrussBuilder::build() {
    // Validate minimum requirements
    if (m_nodes.empty()) {
        throw std::runtime_error("Cannot build truss: no nodes defined. Add at least 2 nodes.");
    }
    if (m_nodes.size() < 2) {
        throw std::runtime_error("Cannot build truss: need at least 2 nodes, got " +
                                 std::to_string(m_nodes.size()));
    }
    if (m_members.empty()) {
        throw std::runtime_error("Cannot build truss: no members defined. Add at least 1 member.");
    }

    // Create the truss object
    auto truss = std::make_shared<core::Truss>(m_name);

    // Add all nodes (NodeId assignment happens automatically in Truss::addNode)
    for (const auto& nodeData : m_nodes) {
        truss->addNode(nodeData.x, nodeData.y, nodeData.support);
    }

    // Add all members
    for (const auto& memberData : m_members) {
        // Convert application DTOs to domain types (following TrussApplicationService pattern)
        core::MaterialProperties material{
            memberData.material.youngsModulusPa,            // E
            7850.0,                                        // density (default for steel)
            memberData.material.youngsModulusPa * 0.00125, // yield strength (estimate)
            memberData.material.youngsModulusPa * 0.002,   // ultimate strength (estimate)
            memberData.material.name                       // name
        };

        core::SectionProperties section{
            memberData.section.areaM2,                              // area
            memberData.section.areaM2 * memberData.section.areaM2 / 12.0,  // moment of inertia (estimate)
            memberData.section.areaM2,                              // shear area
            memberData.section.profile                              // designation
        };

        truss->addMember(memberData.startNodeId, memberData.endNodeId, material, section);
    }

    // Apply all forces
    for (const auto& forceData : m_forces) {
        truss->applyForce(forceData.nodeId, forceData.force);
    }

    // Note: We don't call reset() here to allow inspection of what was built
    // Users can explicitly call reset() if they want to reuse the builder
    return truss;
}

TrussBuilder& TrussBuilder::reset() {
    m_name = "Untitled Truss";
    m_nodes.clear();
    m_members.clear();
    m_forces.clear();
    m_nextNodeId = 1;
    return *this;
}

// Private helper methods

bool TrussBuilder::isValidNodeId(core::NodeId nodeId) const noexcept {
    return nodeId >= 1 && nodeId < m_nextNodeId;
}

void TrussBuilder::validateNodeReference(core::NodeId nodeId,
                                          const std::string& context) const {
    if (!isValidNodeId(nodeId)) {
        std::ostringstream oss;
        oss << "Invalid node reference in " << context << ": node ID " << nodeId;
        if (m_nodes.empty()) {
            oss << " (no nodes defined yet)";
        } else {
            oss << " (valid range: 1-" << (m_nextNodeId - 1) << ")";
        }
        throw std::invalid_argument(oss.str());
    }
}

application::MaterialSpec TrussBuilder::getDefaultMaterial() const {
    // Default steel material (using static factory method)
    return application::MaterialSpec::Steel();
}

application::SectionSpec TrussBuilder::getDefaultSection() const {
    // Default circular section (36mm diameter)
    return application::SectionSpec::Circular(0.036);  // 0.036 m = 36 mm
}

}  // namespace truss::interface
