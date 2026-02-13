/**
 * @file xml_truss_reader.cpp
 * @brief XML format truss file reader implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include "xml_truss_reader.hpp"
#include "../../core/validation/TrussValidator.hpp"
#include <sstream>
#include <unordered_map>

namespace truss::infrastructure::io {

core::interfaces::TrussDTO XmlTrussReader::read(
    const std::filesystem::path& filepath,
    const FileIOOptions& options
) {
    // Check file exists
    if (!std::filesystem::exists(filepath)) {
        throw FileNotFoundException(filepath.string());
    }
    
    // Load XML document
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError result = doc.LoadFile(filepath.string().c_str());
    
    if (result != tinyxml2::XML_SUCCESS) {
        throw ParseException(std::string("XML parse error: ") + doc.ErrorStr());
    }
    
    // Get root element
    tinyxml2::XMLElement* root = doc.FirstChildElement("truss");
    if (!root) {
        throw ParseException("Missing root <truss> element");
    }
    
    // Create DTO
    core::interfaces::TrussDTO dto;
    
    // Parse sections
    try {
        // Metadata (optional)
        tinyxml2::XMLElement* metadata = root->FirstChildElement("metadata");
        if (metadata) {
            parseMetadata(metadata, dto);
        }
        
        // Nodes (required)
        tinyxml2::XMLElement* nodes = root->FirstChildElement("nodes");
        if (!nodes) {
            throw ParseException("Missing required <nodes> section");
        }
        parseNodes(nodes, dto);
        
        // Members (optional)
        tinyxml2::XMLElement* members = root->FirstChildElement("members");
        if (members) {
            parseMembers(members, dto);
        }
        
        // Loads (optional)
        tinyxml2::XMLElement* loads = root->FirstChildElement("loads");
        if (loads) {
            parseLoads(loads, dto);
        }
    } catch (const ParseException& e) {
        // Preserve existing ParseException messages without adding another prefix
        throw;
    } catch (const std::exception& e) {
        throw ParseException(std::string("XML structure error: ") + e.what());
    }
    
    // Note: Validation removed - DTOs are pure data, validation happens at Domain level
    
    return dto;
}

void XmlTrussReader::parseMetadata(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto) {
    const char* name = element->Attribute("name");
    if (name) {
        dto.name = name;
    }
}

void XmlTrussReader::parseNodes(tinyxml2::XMLElement* nodesElement, core::interfaces::TrussDTO& dto) {
    for (tinyxml2::XMLElement* nodeElement = nodesElement->FirstChildElement("node");
         nodeElement != nullptr;
         nodeElement = nodeElement->NextSiblingElement("node")) {
        
        core::interfaces::NodeDTO nodeDTO;
        nodeDTO.id = getIntAttribute(nodeElement, "id");
        nodeDTO.x = getDoubleAttribute(nodeElement, "x");
        nodeDTO.y = getDoubleAttribute(nodeElement, "y");
        
        // Check for duplicate node ID
        for (const auto& existing : dto.nodes) {
            if (existing.id == nodeDTO.id) {
                throw ParseException("Duplicate node ID: " + std::to_string(nodeDTO.id));
            }
        }
        
        const char* supportStr = nodeElement->Attribute("support");
        nodeDTO.support = core::SupportType::Free;
        if (supportStr) {
            nodeDTO.support = parseSupportType(supportStr);
        }
        
        // Forces will be set by parseLoads
        nodeDTO.fx = 0.0;
        nodeDTO.fy = 0.0;
        
        dto.nodes.push_back(nodeDTO);
    }
}

void XmlTrussReader::parseMembers(tinyxml2::XMLElement* membersElement, core::interfaces::TrussDTO& dto) {
    for (tinyxml2::XMLElement* memberElement = membersElement->FirstChildElement("member");
         memberElement != nullptr;
         memberElement = memberElement->NextSiblingElement("member")) {
        
        core::interfaces::MemberDTO memberDTO;
        
        // Get member ID - STRICT REQUIREMENT for validation
        memberDTO.id = getIntAttribute(memberElement, "id");
        memberDTO.startNodeId = getIntAttribute(memberElement, "startNode");
        memberDTO.endNodeId = getIntAttribute(memberElement, "endNode");
        
        // Validate that referenced nodes exist
        bool startExists = false, endExists = false;
        for (const auto& node : dto.nodes) {
            if (node.id == memberDTO.startNodeId) startExists = true;
            if (node.id == memberDTO.endNodeId) endExists = true;
        }
        
        if (!startExists) {
            throw ParseException("Member references unknown start node ID: " + std::to_string(memberDTO.startNodeId));
        }
        if (!endExists) {
            throw ParseException("Member references unknown end node ID: " + std::to_string(memberDTO.endNodeId));
        }
        
        // Material properties (defaults if not specified)
        tinyxml2::XMLElement* materialElement = memberElement->FirstChildElement("material");
        if (materialElement) {
            memberDTO.youngModulus = getDoubleAttribute(materialElement, "youngsModulus", 210e9);
            memberDTO.density = getDoubleAttribute(materialElement, "density", 7850.0);
            memberDTO.yieldStrength = getDoubleAttribute(materialElement, "yieldStrength", 250e6);
        } else {
            // Use defaults
            memberDTO.youngModulus = 210e9;
            memberDTO.density = 7850.0;
            memberDTO.yieldStrength = 250e6;
        }
        
        // Section properties
        tinyxml2::XMLElement* sectionElement = memberElement->FirstChildElement("section");
        if (sectionElement) {
            memberDTO.area = getDoubleAttribute(sectionElement, "area", 0.01);
        } else {
            memberDTO.area = 0.01;
        }
        
        dto.members.push_back(memberDTO);
    }
}

void XmlTrussReader::parseLoads(tinyxml2::XMLElement* loadsElement, core::interfaces::TrussDTO& dto) {
    for (tinyxml2::XMLElement* loadElement = loadsElement->FirstChildElement("load");
         loadElement != nullptr;
         loadElement = loadElement->NextSiblingElement("load")) {
        
        core::NodeId nodeId = getIntAttribute(loadElement, "nodeId");
        core::Real fx = getDoubleAttribute(loadElement, "fx", 0.0);
        core::Real fy = getDoubleAttribute(loadElement, "fy", 0.0);
        
        // Find node and apply force
        bool found = false;
        for (auto& node : dto.nodes) {
            if (node.id == nodeId) {
                node.fx = fx;
                node.fy = fy;
                found = true;
                break;
            }
        }
        
        if (!found) {
            throw ParseException("Load references unknown node ID: " + std::to_string(nodeId));
        }
    }
}

core::SupportType XmlTrussReader::parseSupportType(const std::string& str) {
    if (str == "free" || str == "Free") return core::SupportType::Free;
    if (str == "pinned" || str == "Pinned") return core::SupportType::Pinned;
    if (str == "fixed" || str == "Fixed") return core::SupportType::Pinned;  // Fixed maps to Pinned
    if (str == "roller_x" || str == "RollerX") return core::SupportType::RollerX;
    if (str == "roller_y" || str == "RollerY") return core::SupportType::RollerY;
    
    throw ParseException("Unknown support type: " + str);
}

core::Real XmlTrussReader::getDoubleAttribute(tinyxml2::XMLElement* element, const char* name) {
    double value;
    tinyxml2::XMLError result = element->QueryDoubleAttribute(name, &value);
    if (result != tinyxml2::XML_SUCCESS) {
        throw ParseException(std::string("Missing or invalid attribute: ") + name);
    }
    return static_cast<core::Real>(value);
}

int XmlTrussReader::getIntAttribute(tinyxml2::XMLElement* element, const char* name) {
    int value;
    tinyxml2::XMLError result = element->QueryIntAttribute(name, &value);
    if (result != tinyxml2::XML_SUCCESS) {
        throw ParseException(std::string("Missing or invalid attribute: ") + name);
    }
    return value;
}

core::Real XmlTrussReader::getDoubleAttribute(
    tinyxml2::XMLElement* element, 
    const char* name, 
    core::Real defaultValue
) {
    double value;
    tinyxml2::XMLError result = element->QueryDoubleAttribute(name, &value);
    if (result != tinyxml2::XML_SUCCESS) {
        return defaultValue;
    }
    return static_cast<core::Real>(value);
}

} // namespace truss::infrastructure::io
