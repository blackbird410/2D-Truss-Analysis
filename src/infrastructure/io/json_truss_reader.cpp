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

using json = nlohmann::json;

namespace truss::infrastructure::io {

std::shared_ptr<core::Truss> JsonTrussReader::read(
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
    
    // Create truss
    auto truss = std::make_shared<core::Truss>("Untitled Truss");
    
    // Parse sections
    try {
        if (j.contains("metadata")) {
            parseMetadata(j["metadata"], *truss);
        }
        
        if (j.contains("nodes")) {
            parseNodes(j["nodes"], *truss);
        } else {
            throw ParseException("Missing required 'nodes' section");
        }
        
        if (j.contains("members")) {
            parseMembers(j["members"], *truss);
        }
        
        if (j.contains("loads")) {
            parseLoads(j["loads"], *truss);
        }
    } catch (const json::exception& e) {
        throw ParseException(std::string("JSON structure error: ") + e.what());
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

void JsonTrussReader::parseMetadata(const json& j, core::Truss& truss) {
    if (j.contains("name")) {
        truss.setName(j["name"].get<std::string>());
    }
    // Additional metadata fields can be added here
}

void JsonTrussReader::parseNodes(const json& j, core::Truss& truss) {
    if (!j.is_array()) {
        throw ParseException("'nodes' must be an array");
    }
    
    for (const auto& nodeJson : j) {
        if (!nodeJson.contains("id") || !nodeJson.contains("x") || !nodeJson.contains("y")) {
            throw ParseException("Node missing required fields (id, x, y)");
        }
        
        core::NodeId id = nodeJson["id"].get<core::NodeId>();
        core::Real x = nodeJson["x"].get<core::Real>();
        core::Real y = nodeJson["y"].get<core::Real>();
        
        core::SupportType support = core::SupportType::Free;
        if (nodeJson.contains("support")) {
            support = parseSupportType(nodeJson["support"].get<std::string>());
        }
        
        auto node = truss.addNode(x, y, support);
        // Note: Truss auto-assigns IDs, but we respect the file's ID if needed
        // For now, we rely on insertion order matching file order
    }
}

void JsonTrussReader::parseMembers(const json& j, core::Truss& truss) {
    if (!j.is_array()) {
        throw ParseException("'members' must be an array");
    }
    
    for (const auto& memberJson : j) {
        if (!memberJson.contains("startNode") || !memberJson.contains("endNode")) {
            throw ParseException("Member missing required fields (startNode, endNode)");
        }
        
        core::NodeId startNodeId = memberJson["startNode"].get<core::NodeId>();
        core::NodeId endNodeId = memberJson["endNode"].get<core::NodeId>();
        
        // Material properties
        core::MaterialProperties material;
        if (memberJson.contains("material")) {
            const auto& matJson = memberJson["material"];
            if (matJson.contains("youngsModulus")) {
                material.youngModulus = matJson["youngsModulus"].get<core::Real>();
            }
            if (matJson.contains("density")) {
                material.density = matJson["density"].get<core::Real>();
            }
            if (matJson.contains("yieldStrength")) {
                material.yieldStrength = matJson["yieldStrength"].get<core::Real>();
            }
            if (matJson.contains("name")) {
                material.name = matJson["name"].get<std::string>();
            }
        }
        
        // Section properties
        core::SectionProperties section;
        if (memberJson.contains("section")) {
            const auto& secJson = memberJson["section"];
            if (secJson.contains("area")) {
                section.area = secJson["area"].get<core::Real>();
            }
        }
        
        truss.addMember(startNodeId, endNodeId, material, section);
    }
}

void JsonTrussReader::parseLoads(const json& j, core::Truss& truss) {
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
        
        truss.applyForce(nodeId, fx, fy);
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
