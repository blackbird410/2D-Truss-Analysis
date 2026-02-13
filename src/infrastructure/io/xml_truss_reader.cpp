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

std::shared_ptr<core::Truss> XmlTrussReader::read(
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
    
    // Create truss
    auto truss = std::make_shared<core::Truss>("Untitled Truss");
    
    // Node ID mapping from file IDs to created IDs
    std::unordered_map<core::NodeId, core::NodeId> nodeIdMap;
    
    // Parse sections
    try {
        // Metadata (optional)
        tinyxml2::XMLElement* metadata = root->FirstChildElement("metadata");
        if (metadata) {
            parseMetadata(metadata, *truss);
        }
        
        // Nodes (required)
        tinyxml2::XMLElement* nodes = root->FirstChildElement("nodes");
        if (!nodes) {
            throw ParseException("Missing required <nodes> section");
        }
        parseNodes(nodes, *truss, nodeIdMap);
        
        // Members (optional)
        tinyxml2::XMLElement* members = root->FirstChildElement("members");
        if (members) {
            parseMembers(members, *truss, nodeIdMap);
        }
        
        // Loads (optional)
        tinyxml2::XMLElement* loads = root->FirstChildElement("loads");
        if (loads) {
            parseLoads(loads, *truss, nodeIdMap);
        }
    } catch (const ParseException& e) {
        // Preserve existing ParseException messages without adding another prefix
        throw;
    } catch (const std::exception& e) {
        throw ParseException(std::string("XML structure error: ") + e.what());
    }
    
    // Validate if requested
    if (options.validateOnRead) {
        core::validation::TrussValidator validator;
        auto result = validator.validate(*truss);
        
        if (!result.isValid()) {
            std::ostringstream oss;
            oss << "Truss validation failed:\n";
            for (const auto& issue : result.getIssues()) {
                if (issue.severity == core::validation::ValidationSeverity::Error ||
                    issue.severity == core::validation::ValidationSeverity::Fatal) {
                    oss << "  - " << issue.message << "\n";
                }
            }
            throw ValidationException(oss.str());
        }
    }
    
    return truss;
}

void XmlTrussReader::parseMetadata(tinyxml2::XMLElement* element, core::Truss& truss) {
    const char* name = element->Attribute("name");
    if (name) {
        truss.setName(name);
    }
}

void XmlTrussReader::parseNodes(tinyxml2::XMLElement* nodesElement, core::Truss& truss, std::unordered_map<core::NodeId, core::NodeId>& nodeIdMap) {
    for (tinyxml2::XMLElement* nodeElement = nodesElement->FirstChildElement("node");
         nodeElement != nullptr;
         nodeElement = nodeElement->NextSiblingElement("node")) {
        
        core::NodeId fileId = getIntAttribute(nodeElement, "id");
        core::Real x = getDoubleAttribute(nodeElement, "x");
        core::Real y = getDoubleAttribute(nodeElement, "y");
        
        // Check for duplicate node ID
        if (nodeIdMap.find(fileId) != nodeIdMap.end()) {
            throw ParseException("Duplicate node ID: " + std::to_string(fileId));
        }
        
        const char* supportStr = nodeElement->Attribute("support");
        core::SupportType support = core::SupportType::Free;
        if (supportStr) {
            support = parseSupportType(supportStr);
        }
        
        auto createdNode = truss.addNode(x, y, support);
        // Map file ID to created ID
        nodeIdMap[fileId] = createdNode->getId();
    }
}

void XmlTrussReader::parseMembers(tinyxml2::XMLElement* membersElement, core::Truss& truss, const std::unordered_map<core::NodeId, core::NodeId>& nodeIdMap) {
    for (tinyxml2::XMLElement* memberElement = membersElement->FirstChildElement("member");
         memberElement != nullptr;
         memberElement = memberElement->NextSiblingElement("member")) {
        
        core::NodeId fileStartNodeId = getIntAttribute(memberElement, "startNode");
        core::NodeId fileEndNodeId = getIntAttribute(memberElement, "endNode");
        
        // Translate file node IDs to created node IDs
        auto startIt = nodeIdMap.find(fileStartNodeId);
        auto endIt = nodeIdMap.find(fileEndNodeId);
        
        if (startIt == nodeIdMap.end()) {
            throw ParseException("Member references unknown start node ID: " + std::to_string(fileStartNodeId));
        }
        if (endIt == nodeIdMap.end()) {
            throw ParseException("Member references unknown end node ID: " + std::to_string(fileEndNodeId));
        }
        
        core::NodeId startNodeId = startIt->second;
        core::NodeId endNodeId = endIt->second;
        
        // Material properties
        core::MaterialProperties material;
        tinyxml2::XMLElement* materialElement = memberElement->FirstChildElement("material");
        if (materialElement) {
            material.youngModulus = getDoubleAttribute(materialElement, "youngsModulus", material.youngModulus);
            material.density = getDoubleAttribute(materialElement, "density", material.density);
            material.yieldStrength = getDoubleAttribute(materialElement, "yieldStrength", material.yieldStrength);
            
            const char* name = materialElement->Attribute("name");
            if (name) material.name = name;
        }
        
        // Section properties
        core::SectionProperties section;
        tinyxml2::XMLElement* sectionElement = memberElement->FirstChildElement("section");
        if (sectionElement) {
            section.area = getDoubleAttribute(sectionElement, "area", section.area);
        }
        
        try {
            truss.addMember(startNodeId, endNodeId, material, section);
        } catch (const std::invalid_argument& e) {
            // Convert domain exceptions to validation exceptions for I/O layer
            throw ValidationException(std::string("Member validation failed: ") + e.what());
        }
    }
}

void XmlTrussReader::parseLoads(tinyxml2::XMLElement* loadsElement, core::Truss& truss, const std::unordered_map<core::NodeId, core::NodeId>& nodeIdMap) {
    for (tinyxml2::XMLElement* loadElement = loadsElement->FirstChildElement("load");
         loadElement != nullptr;
         loadElement = loadElement->NextSiblingElement("load")) {
        
        core::NodeId fileNodeId = getIntAttribute(loadElement, "nodeId");
        core::Real fx = getDoubleAttribute(loadElement, "fx", 0.0);
        core::Real fy = getDoubleAttribute(loadElement, "fy", 0.0);
        
        // Translate file node ID to created node ID
        auto it = nodeIdMap.find(fileNodeId);
        if (it == nodeIdMap.end()) {
            throw ParseException("Load references unknown node ID: " + std::to_string(fileNodeId));
        }
        
        core::NodeId nodeId = it->second;
        truss.applyForce(nodeId, fx, fy);
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
