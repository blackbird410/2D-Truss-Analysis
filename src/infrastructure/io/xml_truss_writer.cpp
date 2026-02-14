/**
 * @file xml_truss_writer.cpp
 * @brief XML format truss file writer implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include "xml_truss_writer.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace truss::infrastructure::io {

bool XmlTrussWriter::write(
    const core::interfaces::TrussDTO& trussData,
    const std::filesystem::path& filepath,
    const FileIOOptions& options
) {
    // Check if file exists and overwrite not allowed
    if (std::filesystem::exists(filepath) && !options.overwriteExisting) {
        throw FileWriteException(filepath.string() + " (file exists, overwrite not allowed)");
    }
    
    // Note: Validation moved to Domain layer - Infrastructure only serializes
    
    // Create XML document
    tinyxml2::XMLDocument doc;
    
    // Add XML declaration
    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration();
    doc.InsertFirstChild(declaration);
    
    // Create root element
    tinyxml2::XMLElement* root = doc.NewElement("truss");
    doc.InsertEndChild(root);
    
    // Add sections
    if (options.includeMetadata) {
        createMetadata(doc, root, trussData, options);
    }
    
    createNodes(doc, root, trussData);
    createMembers(doc, root, trussData);
    createLoads(doc, root, trussData);
    
    // Write to file
    tinyxml2::XMLError result = doc.SaveFile(filepath.string().c_str());
    if (result != tinyxml2::XML_SUCCESS) {
        throw FileWriteException(filepath.string() + " (" + doc.ErrorStr() + ")");
    }
    
    return true;
}

void XmlTrussWriter::createMetadata(
    tinyxml2::XMLDocument& doc,
    tinyxml2::XMLElement* root,
    const core::interfaces::TrussDTO& trussData,
    [[maybe_unused]] const FileIOOptions& options
) {
    tinyxml2::XMLElement* metadata = doc.NewElement("metadata");
    metadata->SetAttribute("name", trussData.name.c_str());
    metadata->SetAttribute("version", "3.0.0");
    metadata->SetAttribute("format", "truss-xml-v1");
    
    // Add timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    metadata->SetAttribute("timestamp", oss.str().c_str());
    
    root->InsertEndChild(metadata);
}

void XmlTrussWriter::createNodes(
    tinyxml2::XMLDocument& doc,
    tinyxml2::XMLElement* root,
    const core::interfaces::TrussDTO& trussData
) {
    tinyxml2::XMLElement* nodesElement = doc.NewElement("nodes");
    
    for (const auto& node : trussData.nodes) {
        tinyxml2::XMLElement* nodeElement = doc.NewElement("node");
        nodeElement->SetAttribute("id", node.id);
        nodeElement->SetAttribute("x", node.x);
        nodeElement->SetAttribute("y", node.y);
        nodeElement->SetAttribute("support", supportTypeToString(node.support).c_str());
        
        nodesElement->InsertEndChild(nodeElement);
    }
    
    root->InsertEndChild(nodesElement);
}

void XmlTrussWriter::createMembers(
    tinyxml2::XMLDocument& doc,
    tinyxml2::XMLElement* root,
    const core::interfaces::TrussDTO& trussData
) {
    tinyxml2::XMLElement* membersElement = doc.NewElement("members");
    
    for (const auto& member : trussData.members) {
        tinyxml2::XMLElement* memberElement = doc.NewElement("member");
        memberElement->SetAttribute("id", member.id);
        memberElement->SetAttribute("startNode", member.startNodeId);
        memberElement->SetAttribute("endNode", member.endNodeId);
        
        // Material properties
        tinyxml2::XMLElement* materialElement = doc.NewElement("material");
        materialElement->SetAttribute("youngsModulus", member.youngModulus);
        materialElement->SetAttribute("density", member.density);
        materialElement->SetAttribute("yieldStrength", member.yieldStrength);
        memberElement->InsertEndChild(materialElement);
        
        // Section properties
        tinyxml2::XMLElement* sectionElement = doc.NewElement("section");
        sectionElement->SetAttribute("area", member.area);
        memberElement->InsertEndChild(sectionElement);
        
        membersElement->InsertEndChild(memberElement);
    }
    
    root->InsertEndChild(membersElement);
}

void XmlTrussWriter::createLoads(
    tinyxml2::XMLDocument& doc,
    tinyxml2::XMLElement* root,
    const core::interfaces::TrussDTO& trussData
) {
    tinyxml2::XMLElement* loadsElement = doc.NewElement("loads");
    
    for (const auto& node : trussData.nodes) {
        // Only write non-zero forces
        double magnitude = std::sqrt(node.fx * node.fx + node.fy * node.fy);
        if (magnitude > 1e-10) {
            tinyxml2::XMLElement* loadElement = doc.NewElement("load");
            loadElement->SetAttribute("nodeId", node.id);
            loadElement->SetAttribute("fx", node.fx);
            loadElement->SetAttribute("fy", node.fy);
            
            loadsElement->InsertEndChild(loadElement);
        }
    }
    
    root->InsertEndChild(loadsElement);
}

std::string XmlTrussWriter::supportTypeToString(core::SupportType type) {
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
