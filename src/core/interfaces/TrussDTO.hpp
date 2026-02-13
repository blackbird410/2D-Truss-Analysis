/**
 * @file TrussDTO.hpp
 * @brief Data Transfer Objects for Truss serialization/deserialization
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * DTOs provide Infrastructure layer with simple data structures for
 * file I/O operations, avoiding dependencies on concrete Domain types.
 */

#pragma once

#include "../model/Types.hpp"
#include <string>
#include <vector>

namespace truss::core::interfaces {

/**
 * @brief Data Transfer Object for Node
 * 
 * Simple POD struct for transferring node data between
 * Infrastructure (file I/O) and Domain layers.
 */
struct NodeDTO {
    NodeId id{0};
    Real x{0.0};
    Real y{0.0};
    SupportType support{SupportType::Free};
    Real fx{0.0};  ///< Applied force X-component
    Real fy{0.0};  ///< Applied force Y-component
    
    NodeDTO() = default;
    
    NodeDTO(NodeId nodeId, Real xPos, Real yPos, 
            SupportType supportType = SupportType::Free,
            Real forceX = 0.0, Real forceY = 0.0)
        : id(nodeId), x(xPos), y(yPos), support(supportType), fx(forceX), fy(forceY) {}
};

/**
 * @brief Data Transfer Object for Member
 * 
 * Simple POD struct for transferring member data between
 * Infrastructure (file I/O) and Domain layers.
 */
struct MemberDTO {
    MemberId id{0};
    NodeId startNodeId{0};
    NodeId endNodeId{0};
    std::string label;
    
    // Material properties
    Real youngModulus{210e9};    ///< Default: 210 GPa (steel)
    Real yieldStrength{250e6};   ///< Default: 250 MPa (steel)
    Real density{7850.0};        ///< Default: 7850 kg/m³ (steel)
    
    // Section properties
    Real area{0.01};             ///< Default: 0.01 m² (100 cm²)
    
    MemberDTO() = default;
    
    MemberDTO(MemberId memberId, NodeId start, NodeId end,
              const std::string& memberLabel = "",
              Real E = 210e9, Real fy = 250e6, Real rho = 7850.0, Real A = 0.01)
        : id(memberId), startNodeId(start), endNodeId(end), label(memberLabel),
          youngModulus(E), yieldStrength(fy), density(rho), area(A) {}
};

/**
 * @brief Data Transfer Object for complete Truss
 * 
 * Contains collections of NodeDTO and MemberDTO for full truss representation.
 * Used by readers/writers to serialize/deserialize truss data.
 */
struct TrussDTO {
    std::string name{"Untitled Truss"};
    std::vector<NodeDTO> nodes;
    std::vector<MemberDTO> members;
    
    TrussDTO() = default;
    
    explicit TrussDTO(const std::string& trussName)
        : name(trussName) {}
    
    TrussDTO(const std::string& trussName, 
             std::vector<NodeDTO> nodeList,
             std::vector<MemberDTO> memberList)
        : name(trussName), nodes(std::move(nodeList)), members(std::move(memberList)) {}
};

} // namespace truss::core::interfaces
