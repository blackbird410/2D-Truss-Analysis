/**
 * @file xml_truss_reader.cpp
 * @brief Reads truss models from XML files.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "xml_truss_reader.hpp"

#include "infrastructure/io/support_type_serializer.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace truss::infrastructure::io {

core::interfaces::TrussDTO XmlTrussReader::read(const std::filesystem::path& filepath,
                                                [[maybe_unused]] const FileIOOptions& options) {
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

        // Parse materials and sections first (these are referenced by members)
        std::unordered_map<std::string, tinyxml2::XMLElement*> materials;
        tinyxml2::XMLElement* materialsElement = root->FirstChildElement("materials");
        if (materialsElement) {
            materials = parseMaterials(materialsElement);
        }

        std::unordered_map<std::string, tinyxml2::XMLElement*> sections;
        tinyxml2::XMLElement* sectionsElement = root->FirstChildElement("sections");
        if (sectionsElement) {
            sections = parseSections(sectionsElement);
        }

        // Nodes (required)
        tinyxml2::XMLElement* nodes = root->FirstChildElement("nodes");
        if (!nodes) {
            throw ParseException("Missing required <nodes> section");
        }
        std::unordered_set<core::NodeId> validNodeIds = parseNodes(nodes, dto);

        // Members (optional)
        tinyxml2::XMLElement* members = root->FirstChildElement("members");
        if (members) {
            parseMembers(members, dto, validNodeIds, materials, sections);
        }

        // Supports (optional)
        tinyxml2::XMLElement* supports = root->FirstChildElement("supports");
        if (supports) {
            parseSupports(supports, dto, validNodeIds);
        }

        // Loads (optional)
        tinyxml2::XMLElement* loads = root->FirstChildElement("loads");
        if (loads) {
            parseLoads(loads, dto, validNodeIds);
        }
    } catch (const ParseException& e) {
        // Preserve existing ParseException messages without adding another prefix
        throw;
    } catch (const std::exception& e) {
        throw ParseException(std::string("XML structure error: ") + e.what());
    }

    // Note: validateOnRead flag exists but is not enforced here.
    // Infrastructure layer works with DTOs (data), not Domain objects.
    // Validation should occur at Application layer using TrussValidator
    // AFTER assembling DTO → Truss via TrussAssembler. Tests set
    // validateOnRead=false to skip validation when testing pure I/O parsing.

    return dto;
}

void XmlTrussReader::parseMetadata(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto) {
    const char* name = element->Attribute("name");
    if (name) {
        dto.name = name;
    }
}

std::unordered_set<core::NodeId> XmlTrussReader::parseNodes(tinyxml2::XMLElement* nodesElement,
                                                            core::interfaces::TrussDTO& dto) {
    std::unordered_set<core::NodeId> seenNodeIds;

    for (tinyxml2::XMLElement* nodeElement = nodesElement->FirstChildElement("node");
         nodeElement != nullptr;
         nodeElement = nodeElement->NextSiblingElement("node")) {
        core::interfaces::NodeDTO nodeDTO;
        nodeDTO.id = getIntAttribute(nodeElement, "id");
        nodeDTO.x = getDoubleAttribute(nodeElement, "x");
        nodeDTO.y = getDoubleAttribute(nodeElement, "y");

        // Check for duplicate node ID (O(1) with unordered_set)
        if (!seenNodeIds.insert(nodeDTO.id).second) {
            throw ParseException("Duplicate node ID: " + std::to_string(nodeDTO.id));
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

    return seenNodeIds;
}

void XmlTrussReader::parseMembers(
    tinyxml2::XMLElement* membersElement,
    core::interfaces::TrussDTO& dto,
    const std::unordered_set<core::NodeId>& validNodeIds,
    const std::unordered_map<std::string, tinyxml2::XMLElement*>& materials,
    const std::unordered_map<std::string, tinyxml2::XMLElement*>& sections) {
    for (tinyxml2::XMLElement* memberElement = membersElement->FirstChildElement("member");
         memberElement != nullptr;
         memberElement = memberElement->NextSiblingElement("member")) {
        core::interfaces::MemberDTO memberDTO;

        // Get member ID - STRICT REQUIREMENT for validation
        memberDTO.id = getIntAttribute(memberElement, "id");
        memberDTO.startNodeId = getIntAttribute(memberElement, "startNode");
        memberDTO.endNodeId = getIntAttribute(memberElement, "endNode");

        // Validate that referenced nodes exist (O(1) lookup with unordered_set)
        if (validNodeIds.find(memberDTO.startNodeId) == validNodeIds.end()) {
            throw ParseException("Member references unknown start node ID: " +
                                 std::to_string(memberDTO.startNodeId));
        }
        if (validNodeIds.find(memberDTO.endNodeId) == validNodeIds.end()) {
            throw ParseException("Member references unknown end node ID: " +
                                 std::to_string(memberDTO.endNodeId));
        }

        // Material properties - default values
        memberDTO.youngModulus = 210e9;   // Default steel
        memberDTO.density = 7850.0;       // Default steel
        memberDTO.yieldStrength = 250e6;  // Default steel

        // Check for material specification
        const char* materialStr = memberElement->Attribute("material");
        if (materialStr) {
            // Material specified by ID - look it up
            auto matIt = materials.find(materialStr);
            if (matIt != materials.end()) {
                memberDTO.youngModulus = getDoubleAttribute(matIt->second, "youngModulus", 210e9);
                memberDTO.density = getDoubleAttribute(matIt->second, "density", 7850.0);
                memberDTO.yieldStrength = getDoubleAttribute(matIt->second, "yieldStrength", 250e6);
            } else {
                throw ParseException("Member references unknown material ID: " +
                                     std::string(materialStr));
            }
        } else {
            // Check for inline material element
            tinyxml2::XMLElement* materialElement = memberElement->FirstChildElement("material");
            if (materialElement) {
                memberDTO.youngModulus = getDoubleAttribute(
                    materialElement, "youngsModulus", 210e9);
                memberDTO.density = getDoubleAttribute(materialElement, "density", 7850.0);
                memberDTO.yieldStrength = getDoubleAttribute(
                    materialElement, "yieldStrength", 250e6);
            }
        }

        // Section properties - default value
        memberDTO.area = 0.01;  // Default area

        // Check for section specification
        const char* sectionStr = memberElement->Attribute("section");
        if (sectionStr) {
            // Section specified by ID - look it up
            auto secIt = sections.find(sectionStr);
            if (secIt != sections.end()) {
                // Try both "crossSectionalArea" and "area" attribute names for compatibility
                double area = 0.01;
                tinyxml2::XMLError result = secIt->second->QueryDoubleAttribute(
                    "crossSectionalArea", &area);
                if (result != tinyxml2::XML_SUCCESS) {
                    result = secIt->second->QueryDoubleAttribute("area", &area);
                }
                memberDTO.area = area;
            } else {
                throw ParseException("Member references unknown section ID: " +
                                     std::string(sectionStr));
            }
        } else {
            // Check for inline section element
            tinyxml2::XMLElement* sectionElement = memberElement->FirstChildElement("section");
            if (sectionElement) {
                memberDTO.area = getDoubleAttribute(sectionElement, "area", 0.01);
            }
        }

        dto.members.push_back(memberDTO);
    }
}

void XmlTrussReader::parseLoads(tinyxml2::XMLElement* loadsElement,
                                core::interfaces::TrussDTO& dto,
                                const std::unordered_set<core::NodeId>& validNodeIds) {
    for (tinyxml2::XMLElement* loadElement = loadsElement->FirstChildElement("load");
         loadElement != nullptr;
         loadElement = loadElement->NextSiblingElement("load")) {
        core::NodeId nodeId = getIntAttribute(loadElement, "nodeId");
        core::Real fx = getDoubleAttribute(loadElement, "fx", 0.0);
        core::Real fy = getDoubleAttribute(loadElement, "fy", 0.0);

        // Validate node exists (O(1) lookup)
        if (validNodeIds.find(nodeId) == validNodeIds.end()) {
            throw ParseException("Load references unknown node ID: " + std::to_string(nodeId));
        }

        // Find node and apply force
        auto nodeIt = std::find_if(dto.nodes.begin(), dto.nodes.end(), [nodeId](const auto& node) {
            return node.id == nodeId;
        });
        if (nodeIt != dto.nodes.end()) {
            nodeIt->fx = fx;
            nodeIt->fy = fy;
        }
    }
}

std::unordered_map<std::string, tinyxml2::XMLElement*>
XmlTrussReader::parseMaterials(tinyxml2::XMLElement* materialsElement) {
    std::unordered_map<std::string, tinyxml2::XMLElement*> materials;

    for (tinyxml2::XMLElement* materialElement = materialsElement->FirstChildElement("material");
         materialElement != nullptr;
         materialElement = materialElement->NextSiblingElement("material")) {
        const char* id = materialElement->Attribute("id");
        if (!id) {
            throw ParseException("Material missing required attribute 'id'");
        }
        materials[id] = materialElement;
    }

    return materials;
}

std::unordered_map<std::string, tinyxml2::XMLElement*>
XmlTrussReader::parseSections(tinyxml2::XMLElement* sectionsElement) {
    std::unordered_map<std::string, tinyxml2::XMLElement*> sections;

    for (tinyxml2::XMLElement* sectionElement = sectionsElement->FirstChildElement("section");
         sectionElement != nullptr;
         sectionElement = sectionElement->NextSiblingElement("section")) {
        const char* id = sectionElement->Attribute("id");
        if (!id) {
            throw ParseException("Section missing required attribute 'id'");
        }
        sections[id] = sectionElement;
    }

    return sections;
}

void XmlTrussReader::parseSupports(tinyxml2::XMLElement* supportsElement,
                                   core::interfaces::TrussDTO& dto,
                                   const std::unordered_set<core::NodeId>& validNodeIds) {
    for (tinyxml2::XMLElement* supportElement = supportsElement->FirstChildElement("support");
         supportElement != nullptr;
         supportElement = supportElement->NextSiblingElement("support")) {
        core::NodeId nodeId = getIntAttribute(supportElement, "nodeId");

        // Validate node exists
        if (validNodeIds.find(nodeId) == validNodeIds.end()) {
            throw ParseException("Support references unknown node ID: " + std::to_string(nodeId));
        }

        // Find node and apply support
        auto nodeIt = std::find_if(dto.nodes.begin(), dto.nodes.end(), [nodeId](const auto& node) {
            return node.id == nodeId;
        });

        if (nodeIt != dto.nodes.end()) {
            // Parse support type
            const char* typeStr = supportElement->Attribute("type");
            if (typeStr) {
                nodeIt->support = parseSupportType(typeStr);
            }

            // Alternative: parse from restrained attribute (space-separated)
            const char* restrainedStr = supportElement->Attribute("restrained");
            if (restrainedStr) {
                std::string restrainedString(restrainedStr);
                bool xRestrained{restrainedString.find('x') != std::string::npos ||
                                 restrainedString.find('X') != std::string::npos};
                bool yRestrained{restrainedString.find('y') != std::string::npos ||
                                 restrainedString.find('Y') != std::string::npos};

                // Determine support type based on restraints
                if (xRestrained && yRestrained) {
                    nodeIt->support = core::SupportType::Pinned;
                } else if (yRestrained && !xRestrained) {
                    nodeIt->support = core::SupportType::RollerX;  // Y constrained, can move in X
                } else if (xRestrained && !yRestrained) {
                    nodeIt->support = core::SupportType::RollerY;  // X constrained, can move in Y
                } else {
                    nodeIt->support = core::SupportType::Free;
                }
            }
        }
    }
}

core::Real XmlTrussReader::getDoubleAttribute(tinyxml2::XMLElement* element, const char* name) {
    double value{0.0};
    const tinyxml2::XMLError result = element->QueryDoubleAttribute(name, &value);
    if (result != tinyxml2::XML_SUCCESS) {
        throw ParseException(std::string("Missing or invalid attribute: ") + name);
    }
    return static_cast<core::Real>(value);
}

int XmlTrussReader::getIntAttribute(tinyxml2::XMLElement* element, const char* name) {
    int value{0};
    const tinyxml2::XMLError result = element->QueryIntAttribute(name, &value);
    if (result != tinyxml2::XML_SUCCESS) {
        throw ParseException(std::string("Missing or invalid attribute: ") + name);
    }
    return value;
}

core::Real XmlTrussReader::getDoubleAttribute(tinyxml2::XMLElement* element,
                                              const char* name,
                                              core::Real defaultValue) {
    double value{0.0};
    const tinyxml2::XMLError result = element->QueryDoubleAttribute(name, &value);
    if (result != tinyxml2::XML_SUCCESS) {
        return defaultValue;
    }
    return static_cast<core::Real>(value);
}

}  // namespace truss::infrastructure::io
