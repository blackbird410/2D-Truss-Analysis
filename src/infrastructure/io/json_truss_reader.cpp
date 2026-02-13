/**
 * @file json_truss_reader.cpp
 * @brief JSON format truss file reader implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include "json_truss_reader.hpp"
#include "../../core/validation/TrussValidator.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

using json = nlohmann::json;

namespace truss::infrastructure::io {

core::interfaces::TrussDTO JsonTrussReader::read(
    const std::filesystem::path& filepath,
    const FileIOOptions& options
) {
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
        
        if (j.contains("nodes")) {
            parseNodes(j["nodes"], dto);
        } else {
            throw ParseException("Missing required 'nodes' section");
        }
        
        if (j.contains("members")) {
            parseMembers(j["members"], dto);
        }
        
        if (j.contains("loads")) {
            parseLoads(j["loads"], dto);
        }
    } catch (const json::exception& e) {
        throw ParseException(std::string("JSON structure error: ") + e.what());
    }
    
    // Note: Validation removed - DTOs are pure data, validation happens at Domain level
    
    return dto;
}

void JsonTrussReader::parseMetadata(const json& j, core::interfaces::TrussDTO& dto) {
    if (j.contains("name")) {
        dto.name = j["name"].get<std::string>();
    }
    // Additional metadata fields can be added here
}

void JsonTrussReader::parseNodes(const json& j, core::interfaces::TrussDTO& dto) {
    if (!j.is_array()) {
        throw ParseException("'nodes' must be an array");
    }
    
    for (const auto& nodeJson : j) {
        if (!nodeJson.contains("id") || !nodeJson.contains("x") || !nodeJson.contains("y")) {
            throw ParseException("Node missing required fields (id, x, y)");
        }
        
        core::interfaces::NodeDTO nodeDTO;
        nodeDTO.id = nodeJson["id"].get<core::NodeId>();
        nodeDTO.x = nodeJson["x"].get<core::Real>();
        nodeDTO.y = nodeJson["y"].get<core::Real>();
        
        // Check for duplicate node ID
        for (const auto& existing : dto.nodes) {
            if (existing.id == nodeDTO.id) {
                throw ParseException("Duplicate node ID: " + std::to_string(nodeDTO.id));
            }
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
}

void JsonTrussReader::parseMembers(const json& j, core::interfaces::TrussDTO& dto) {
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
        
        // Material properties (defaults will be used if not specified)
        if (memberJson.contains("material")) {
            const auto& matJson = memberJson["material"];
            memberDTO.youngModulus = matJson.value("youngsModulus", 210e9);
            memberDTO.density = matJson.value("density", 7850.0);
            memberDTO.yieldStrength = matJson.value("yieldStrength", 250e6);
        }
        
        // Section properties
        if (memberJson.contains("section")) {
            const auto& secJson = memberJson["section"];
            memberDTO.area = secJson.value("area", 0.01);
        }
        
        dto.members.push_back(memberDTO);
    }
}

void JsonTrussReader::parseLoads(const json& j, core::interfaces::TrussDTO& dto) {
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

core::SupportType JsonTrussReader::parseSupportType(const std::string& str) {
    if (str == "free" || str == "Free") return core::SupportType::Free;
    if (str == "pinned" || str == "Pinned") return core::SupportType::Pinned;
    if (str == "fixed" || str == "Fixed") return core::SupportType::Pinned;  // Fixed maps to Pinned
    if (str == "roller_x" || str == "RollerX") return core::SupportType::RollerX;
    if (str == "roller_y" || str == "RollerY") return core::SupportType::RollerY;
    
    throw ParseException("Unknown support type: " + str);
}

} // namespace truss::infrastructure::io
