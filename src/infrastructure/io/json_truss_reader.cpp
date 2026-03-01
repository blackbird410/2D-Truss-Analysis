/**
 * @file json_truss_reader.cpp
 * @brief Reads truss models from JSON files.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "json_truss_reader.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "infrastructure/io/support_type_serializer.hpp"

using json = nlohmann::json;

namespace truss::infrastructure::io {

core::interfaces::TrussDTO JsonTrussReader::read(const std::filesystem::path& filepath,
                                                 [[maybe_unused]] const FileIOOptions& options) {
    // Check file exists
    if (!std::filesystem::exists(filepath)) {
        throw FileNotFoundException(filepath.string());
    }

    // Read file content
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw FileReadException(filepath.string());
    }

    // Parse JSON
    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        throw ParseException(std::string("JSON parse error: ") + e.what());
    }

    // Create DTO
    core::interfaces::TrussDTO dto;

    // Parse sections
    try {
        if (j.contains("metadata")) {
            parseMetadata(j["metadata"], dto);
        }

        // Parse materials and sections first (these are referenced by members)
        std::unordered_map<std::string, json> materials;
        if (j.contains("materials")) {
            materials = parseMaterials(j["materials"]);
        }

        std::unordered_map<std::string, json> sections;
        if (j.contains("sections")) {
            sections = parseSections(j["sections"]);
        }

        std::unordered_set<core::NodeId> validNodeIds;
        if (j.contains("nodes")) {
            validNodeIds = parseNodes(j["nodes"], dto);
        } else {
            throw ParseException("Missing required 'nodes' section");
        }

        if (j.contains("members")) {
            parseMembers(j["members"], dto, validNodeIds, materials, sections);
        }

        if (j.contains("supports")) {
            parseSupports(j["supports"], dto, validNodeIds);
        }

        if (j.contains("loads")) {
            parseLoads(j["loads"], dto, validNodeIds);
        }
    } catch (const json::exception& e) {
        throw ParseException(std::string("JSON structure error: ") + e.what());
    }

    // Note: validateOnRead flag exists but is not enforced here.
    // Infrastructure layer works with DTOs (data), not Domain objects.
    // Validation should occur at Application layer using TrussValidator
    // AFTER assembling DTO → Truss via TrussAssembler. Tests set
    // validateOnRead=false to skip validation when testing pure I/O parsing.

    return dto;
}

void JsonTrussReader::parseMetadata(const json& j, core::interfaces::TrussDTO& dto) {
    if (j.contains("name")) {
        dto.name = j["name"].get<std::string>();
    }
    // Additional metadata fields can be added here
}

std::unordered_set<core::NodeId> JsonTrussReader::parseNodes(const json& j,
                                                             core::interfaces::TrussDTO& dto) {
    if (!j.is_array()) {
        throw ParseException("'nodes' must be an array");
    }

    std::unordered_set<core::NodeId> seenNodeIds;

    for (const auto& nodeJson : j) {
        if (!nodeJson.contains("id") || !nodeJson.contains("x") || !nodeJson.contains("y")) {
            throw ParseException("Node missing required fields (id, x, y)");
        }

        core::interfaces::NodeDTO nodeDTO;
        nodeDTO.id = nodeJson["id"].get<core::NodeId>();
        nodeDTO.x = nodeJson["x"].get<core::Real>();
        nodeDTO.y = nodeJson["y"].get<core::Real>();

        // Check for duplicate node ID (O(1) with unordered_set)
        if (!seenNodeIds.insert(nodeDTO.id).second) {
            throw ParseException("Duplicate node ID: " + std::to_string(nodeDTO.id));
        }

        nodeDTO.support = core::SupportType::Free;
        if (nodeJson.contains("support")) {
            nodeDTO.support = parseSupportType(nodeJson["support"].get<std::string>());
        }

        // Forces will be set by parseLoads
        nodeDTO.fx = 0.0;
        nodeDTO.fy = 0.0;

        dto.nodes.push_back(nodeDTO);
    }

    return seenNodeIds;
}

void JsonTrussReader::parseMembers(const json& j,
                                   core::interfaces::TrussDTO& dto,
                                   const std::unordered_set<core::NodeId>& validNodeIds,
                                   const std::unordered_map<std::string, json>& materials,
                                   const std::unordered_map<std::string, json>& sections) {
    if (!j.is_array()) {
        throw ParseException("'members' must be an array");
    }

    for (const auto& memberJson : j) {
        if (!memberJson.contains("startNode") || !memberJson.contains("endNode")) {
            throw ParseException("Member missing required fields (startNode, endNode)");
        }

        core::interfaces::MemberDTO memberDTO;

        if (memberJson.contains("id")) {
            memberDTO.id = memberJson["id"].get<core::MemberId>();
        } else {
            // Auto-assign ID if not specified
            memberDTO.id = dto.members.size() + 1;
        }

        memberDTO.startNodeId = memberJson["startNode"].get<core::NodeId>();
        memberDTO.endNodeId = memberJson["endNode"].get<core::NodeId>();

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

        // Check if material is specified by ID reference
        if (memberJson.contains("material")) {
            if (memberJson["material"].is_string()) {
                // Material specified by ID - look it up
                std::string materialId = memberJson["material"].get<std::string>();
                auto matIt = materials.find(materialId);
                if (matIt != materials.end()) {
                    const auto& matJson = matIt->second;
                    if (matJson.contains("youngModulus")) {
                        memberDTO.youngModulus = matJson["youngModulus"].get<core::Real>();
                    }
                    if (matJson.contains("density")) {
                        memberDTO.density = matJson["density"].get<core::Real>();
                    }
                    if (matJson.contains("yieldStrength")) {
                        memberDTO.yieldStrength = matJson["yieldStrength"].get<core::Real>();
                    }
                } else {
                    throw ParseException("Member references unknown material ID: " + materialId);
                }
            } else if (memberJson["material"].is_object()) {
                // Material specified inline
                const auto& matJson = memberJson["material"];
                memberDTO.youngModulus = matJson.value("youngsModulus", 210e9);
                memberDTO.density = matJson.value("density", 7850.0);
                memberDTO.yieldStrength = matJson.value("yieldStrength", 250e6);
            }
        }

        // Section properties - default value
        memberDTO.area = 0.01;  // Default area

        // Check if section is specified by ID reference
        if (memberJson.contains("section")) {
            if (memberJson["section"].is_string()) {
                // Section specified by ID - look it up
                std::string sectionId = memberJson["section"].get<std::string>();
                auto secIt = sections.find(sectionId);
                if (secIt != sections.end()) {
                    const auto& secJson = secIt->second;
                    if (secJson.contains("crossSectionalArea")) {
                        memberDTO.area = secJson["crossSectionalArea"].get<core::Real>();
                    } else if (secJson.contains("area")) {
                        memberDTO.area = secJson["area"].get<core::Real>();
                    }
                } else {
                    throw ParseException("Member references unknown section ID: " + sectionId);
                }
            } else if (memberJson["section"].is_object()) {
                // Section specified inline
                const auto& secJson = memberJson["section"];
                memberDTO.area = secJson.value("area", 0.01);
            }
        }

        dto.members.push_back(memberDTO);
    }
}

std::unordered_map<std::string, json> JsonTrussReader::parseMaterials(const json& j) {
    std::unordered_map<std::string, json> materials;

    if (!j.is_array()) {
        throw ParseException("'materials' must be an array");
    }

    for (const auto& matJson : j) {
        if (!matJson.contains("id")) {
            throw ParseException("Material missing required field 'id'");
        }
        std::string id = matJson["id"].get<std::string>();
        materials[id] = matJson;
    }

    return materials;
}

std::unordered_map<std::string, json> JsonTrussReader::parseSections(const json& j) {
    std::unordered_map<std::string, json> sections;

    if (!j.is_array()) {
        throw ParseException("'sections' must be an array");
    }

    for (const auto& secJson : j) {
        if (!secJson.contains("id")) {
            throw ParseException("Section missing required field 'id'");
        }
        std::string id = secJson["id"].get<std::string>();
        sections[id] = secJson;
    }

    return sections;
}

void JsonTrussReader::parseSupports(const json& j,
                                    core::interfaces::TrussDTO& dto,
                                    const std::unordered_set<core::NodeId>& validNodeIds) {
    if (!j.is_array()) {
        throw ParseException("'supports' must be an array");
    }

    for (const auto& supportJson : j) {
        if (!supportJson.contains("nodeId")) {
            throw ParseException("Support missing required field 'nodeId'");
        }

        core::NodeId nodeId = supportJson["nodeId"].get<core::NodeId>();

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
            if (supportJson.contains("type")) {
                nodeIt->support = parseSupportType(supportJson["type"].get<std::string>());
            }

            // Alternative: parse from "restrained" array
            if (supportJson.contains("restrained")) {
                const auto& restrainedArray = supportJson["restrained"];
                if (restrainedArray.is_array()) {
                    bool xRestrained = false;
                    bool yRestrained = false;

                    for (const auto& item : restrainedArray) {
                        std::string restraint = item.get<std::string>();
                        if (restraint == "x" || restraint == "X") {
                            xRestrained = true;
                        } else if (restraint == "y" || restraint == "Y") {
                            yRestrained = true;
                        }
                    }

                    // Determine support type based on restraints
                    if (xRestrained && yRestrained) {
                        nodeIt->support = core::SupportType::Pinned;
                    } else if (yRestrained && !xRestrained) {
                        nodeIt->support =
                            core::SupportType::RollerX;  // Y constrained, can move in X
                    } else if (xRestrained && !yRestrained) {
                        nodeIt->support =
                            core::SupportType::RollerY;  // X constrained, can move in Y
                    } else {
                        nodeIt->support = core::SupportType::Free;
                    }
                }
            }
        }
    }
}

void JsonTrussReader::parseLoads(const json& j,
                                 core::interfaces::TrussDTO& dto,
                                 const std::unordered_set<core::NodeId>& validNodeIds) {
    if (!j.is_array()) {
        throw ParseException("'loads' must be an array");
    }

    for (const auto& loadJson : j) {
        if (!loadJson.contains("nodeId")) {
            throw ParseException("Load missing required field 'nodeId'");
        }

        core::NodeId nodeId = loadJson["nodeId"].get<core::NodeId>();
        core::Real fx = loadJson.value("fx", 0.0);
        core::Real fy = loadJson.value("fy", 0.0);

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

}  // namespace truss::infrastructure::io
