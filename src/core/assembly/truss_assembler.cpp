/**
 * @file truss_assembler.cpp
 * @brief Constructs truss domain objects from data transfer objects.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "truss_assembler.hpp"

#include "../model/member.hpp"
#include "../model/node.hpp"
#include "../model/types.hpp"

#include <sstream>
#include <unordered_map>

namespace truss::core::assembly {

std::shared_ptr<Truss> TrussAssembler::assembleTruss(const interfaces::TrussDTO& dto) {
    auto truss = std::make_shared<Truss>(dto.name);

    // Map to track created nodes by their DTO IDs
    std::unordered_map<NodeId, std::shared_ptr<Node>> nodeMap;

    // Phase 1: Create all nodes
    for (const auto& nodeDTO : dto.nodes) {
        auto node = std::make_shared<Node>(
            nodeDTO.id, Point2D{nodeDTO.x, nodeDTO.y}, nodeDTO.support);

        // Apply forces if present
        if (nodeDTO.fx != 0.0 || nodeDTO.fy != 0.0) {
            node->setAppliedForce(Force2D(nodeDTO.fx, nodeDTO.fy));
        }

        truss->addNode(node);
        nodeMap[nodeDTO.id] = node;
    }

    // Phase 2: Create all members
    for (const auto& memberDTO : dto.members) {
        // Validate that referenced nodes exist
        auto startIt = nodeMap.find(memberDTO.startNodeId);
        auto endIt = nodeMap.find(memberDTO.endNodeId);

        if (startIt == nodeMap.end()) {
            std::ostringstream oss;
            oss << "Member " << memberDTO.id << " references non-existent start node "
                << memberDTO.startNodeId;
            throw std::invalid_argument(oss.str());
        }

        if (endIt == nodeMap.end()) {
            std::ostringstream oss;
            oss << "Member " << memberDTO.id << " references non-existent end node "
                << memberDTO.endNodeId;
            throw std::invalid_argument(oss.str());
        }

        // Create material and section properties
        MaterialProperties material;
        material.youngModulus = memberDTO.youngModulus;
        material.yieldStrength = memberDTO.yieldStrength;
        material.density = memberDTO.density;
        material.name = "Material_" + std::to_string(memberDTO.id);

        SectionProperties section;
        section.area = memberDTO.area;
        section.designation = "Section_" + std::to_string(memberDTO.id);

        // Add member to truss
        truss->addMember(startIt->second, endIt->second, material, section);
    }

    return truss;
}

interfaces::TrussDTO TrussAssembler::createDTO(const interfaces::ITrussView& truss) {
    interfaces::TrussDTO dto(truss.getName());

    // Convert NodeViews to NodeDTOs
    const auto& nodeViews = truss.getNodeViews();
    dto.nodes.reserve(nodeViews.size());

    for (const auto& nodeView : nodeViews) {
        dto.nodes.emplace_back(
            nodeView.id, nodeView.x, nodeView.y, nodeView.support, nodeView.fx, nodeView.fy);
    }

    // Convert MemberViews to MemberDTOs
    const auto& memberViews = truss.getMemberViews();
    dto.members.reserve(memberViews.size());

    for (const auto& memberView : memberViews) {
        dto.members.emplace_back(memberView.id,
                                 memberView.startNodeId,
                                 memberView.endNodeId,
                                 memberView.label,
                                 memberView.youngModulus,
                                 memberView.yieldStrength,
                                 memberView.density,
                                 memberView.area);
    }

    return dto;
}

}  // namespace truss::core::assembly
