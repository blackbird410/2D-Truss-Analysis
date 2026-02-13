/**
 * @file json_truss_writer.cpp
 * @brief JSON format truss file writer implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include "json_truss_writer.hpp"
#include "../../core/validation/TrussValidator.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

namespace truss::infrastructure::io {

bool JsonTrussWriter::write(
    const core::Truss& truss,
    const std::filesystem::path& filepath,
    const FileIOOptions& options
) {
    // Check if file exists and overwrite not allowed
    if (std::filesystem::exists(filepath) && !options.overwriteExisting) {
        throw FileWriteException(filepath.string() + " (file exists, overwrite not allowed)");
    }
    
    // Validate if requested
    if (options.validateOnWrite) {
        core::validation::TrussValidator validator;
        auto result = validator.validate(truss);
        
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
    
    // Create JSON structure
    json j;
    
    if (options.includeMetadata) {
        j["metadata"] = createMetadata(truss, options);
    }
    
    j["nodes"] = createNodes(truss);
    j["members"] = createMembers(truss);
    j["loads"] = createLoads(truss);
    
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

json JsonTrussWriter::createMetadata(const core::Truss& truss, const FileIOOptions& options) {
    json metadata;
    metadata["name"] = truss.getName();
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

json JsonTrussWriter::createNodes(const core::Truss& truss) {
    json nodesArray = json::array();
    
    for (const auto& node : truss.getNodes()) {
        json nodeObj;
        nodeObj["id"] = node->getId();
        nodeObj["x"] = node->getX();
        nodeObj["y"] = node->getY();
        nodeObj["support"] = supportTypeToString(node->getSupportType());
        
        nodesArray.push_back(nodeObj);
    }
    
    return nodesArray;
}

json JsonTrussWriter::createMembers(const core::Truss& truss) {
    json membersArray = json::array();
    
    for (const auto& member : truss.getMembers()) {
        json memberObj;
        memberObj["id"] = member->getId();
        memberObj["startNode"] = member->getStartNode()->getId();
        memberObj["endNode"] = member->getEndNode()->getId();
        
        // Material properties
        const auto& material = member->getMaterial();
        json materialObj;
        materialObj["youngsModulus"] = material.youngModulus;
        materialObj["density"] = material.density;
        materialObj["yieldStrength"] = material.yieldStrength;
        materialObj["name"] = material.name;
        memberObj["material"] = materialObj;
        
        // Section properties
        const auto& section = member->getSection();
        json sectionObj;
        sectionObj["area"] = section.area;
        memberObj["section"] = sectionObj;
        
        membersArray.push_back(memberObj);
    }
    
    return membersArray;
}

json JsonTrussWriter::createLoads(const core::Truss& truss) {
    json loadsArray = json::array();
    
    for (const auto& node : truss.getLoadedNodes()) {
        const auto& force = node->getAppliedForce();
        
        // Only write non-zero forces
        if (force.magnitude() > 1e-10) {
            json loadObj;
            loadObj["nodeId"] = node->getId();
            loadObj["fx"] = force.fx;
            loadObj["fy"] = force.fy;
            
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
