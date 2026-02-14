/**
 * @file json_truss_writer.cpp
 * @brief JSON format truss file writer implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include "json_truss_writer.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

namespace truss::infrastructure::io {

bool JsonTrussWriter::write(
    const core::interfaces::TrussDTO& trussData,
    const std::filesystem::path& filepath,
    const FileIOOptions& options
) {
    // Check if file exists and overwrite not allowed
    if (std::filesystem::exists(filepath) && !options.overwriteExisting) {
        throw FileWriteException(filepath.string() + " (file exists, overwrite not allowed)");
    }
    
    // Note: validateOnWrite flag exists but is not enforced here.
    // Infrastructure layer works with DTOs (data), not Domain objects.
    // Validation should occur at Application layer using TrussValidator
    // BEFORE calling write(). Tests set validateOnWrite=false to skip
    // application-level validation when testing pure I/O serialization.
    
    // Create JSON structure
    json j;
    
    if (options.includeMetadata) {
        j["metadata"] = createMetadata(trussData, options);
    }
    
    j["nodes"] = createNodes(trussData);
    j["members"] = createMembers(trussData);
    j["loads"] = createLoads(trussData);
    
    // Write to file
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            throw FileWriteException(filepath.string());
        }
        
        if (options.prettyPrint) {
            file << j.dump(options.indentSize);
        } else {
            file << j.dump();
        }
        
        file.close();
        return true;
    } catch (const std::exception& e) {
        throw FileWriteException(filepath.string() + " (" + e.what() + ")");
    }
}

json JsonTrussWriter::createMetadata(const core::interfaces::TrussDTO& trussData, 
                                     [[maybe_unused]] const FileIOOptions& options) {
    json metadata;
    metadata["name"] = trussData.name;
    metadata["version"] = "3.0.0";
    metadata["format"] = "truss-json-v1";
    
    // Add timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    metadata["timestamp"] = oss.str();
    
    return metadata;
}

json JsonTrussWriter::createNodes(const core::interfaces::TrussDTO& trussData) {
    json nodesArray = json::array();
    
    for (const auto& node : trussData.nodes) {
        json nodeObj;
        nodeObj["id"] = node.id;
        nodeObj["x"] = node.x;
        nodeObj["y"] = node.y;
        nodeObj["support"] = supportTypeToString(node.support);
        
        nodesArray.push_back(nodeObj);
    }
    
    return nodesArray;
}

json JsonTrussWriter::createMembers(const core::interfaces::TrussDTO& trussData) {
    json membersArray = json::array();
    
    for (const auto& member : trussData.members) {
        json memberObj;
        memberObj["id"] = member.id;
        memberObj["startNode"] = member.startNodeId;
        memberObj["endNode"] = member.endNodeId;
        
        // Material properties (flattened from DTO)
        json materialObj;
        materialObj["youngsModulus"] = member.youngModulus;
        materialObj["density"] = member.density;
        materialObj["yieldStrength"] = member.yieldStrength;
        memberObj["material"] = materialObj;
        
        // Section properties (flattened from DTO)
        json sectionObj;
        sectionObj["area"] = member.area;
        memberObj["section"] = sectionObj;
        
        membersArray.push_back(memberObj);
    }
    
    return membersArray;
}

json JsonTrussWriter::createLoads(const core::interfaces::TrussDTO& trussData) {
    json loadsArray = json::array();
    
    for (const auto& node : trussData.nodes) {
        // Only write non-zero forces
        if (std::abs(node.fx) > 1e-10 || std::abs(node.fy) > 1e-10) {
            json loadObj;
            loadObj["nodeId"] = node.id;
            loadObj["fx"] = node.fx;
            loadObj["fy"] = node.fy;
            
            loadsArray.push_back(loadObj);
        }
    }
    
    return loadsArray;
}

std::string JsonTrussWriter::supportTypeToString(core::SupportType type) {
    switch (type) {
        case core::SupportType::Free:    return "free";
        case core::SupportType::Pinned:  return "pinned";
        case core::SupportType::PinnedX: return "pinned_x";
        case core::SupportType::PinnedY: return "pinned_y";
        case core::SupportType::RollerX: return "roller_x";
        case core::SupportType::RollerY: return "roller_y";
        default: return "free";
    }
}

} // namespace truss::infrastructure::io
