/**
 * @file xml_truss_writer.cpp
 * @brief XML format truss file writer implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include "xml_truss_writer.hpp"
#include "../../core/validation/TrussValidator.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace truss::infrastructure::io {

bool XmlTrussWriter::write(
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
        createMetadata(doc, root, truss, options);
    }
    
    createNodes(doc, root, truss);
    createMembers(doc, root, truss);
    createLoads(doc, root, truss);
    
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
    const core::Truss& truss,
    const FileIOOptions& options
) {
    tinyxml2::XMLElement* metadata = doc.NewElement("metadata");
    metadata->SetAttribute("name", truss.getName().c_str());
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
    const core::Truss& truss
) {
    tinyxml2::XMLElement* nodesElement = doc.NewElement("nodes");
    
    for (const auto& node : truss.getNodes()) {
        tinyxml2::XMLElement* nodeElement = doc.NewElement("node");
        nodeElement->SetAttribute("id", node->getId());
        nodeElement->SetAttribute("x", node->getX());
        nodeElement->SetAttribute("y", node->getY());
        nodeElement->SetAttribute("support", supportTypeToString(node->getSupportType()).c_str());
        
        nodesElement->InsertEndChild(nodeElement);
    }
    
    root->InsertEndChild(nodesElement);
}

void XmlTrussWriter::createMembers(
    tinyxml2::XMLDocument& doc,
    tinyxml2::XMLElement* root,
    const core::Truss& truss
) {
    tinyxml2::XMLElement* membersElement = doc.NewElement("members");
    
    for (const auto& member : truss.getMembers()) {
        tinyxml2::XMLElement* memberElement = doc.NewElement("member");
        memberElement->SetAttribute("id", member->getId());
        memberElement->SetAttribute("startNode", member->getStartNode()->getId());
        memberElement->SetAttribute("endNode", member->getEndNode()->getId());
        
        // Material properties
        const auto& material = member->getMaterial();
        tinyxml2::XMLElement* materialElement = doc.NewElement("material");
        materialElement->SetAttribute("youngsModulus", material.youngModulus);
        materialElement->SetAttribute("density", material.density);
        materialElement->SetAttribute("yieldStrength", material.yieldStrength);
        materialElement->SetAttribute("name", material.name.c_str());
        memberElement->InsertEndChild(materialElement);
        
        // Section properties
        const auto& section = member->getSection();
        tinyxml2::XMLElement* sectionElement = doc.NewElement("section");
        sectionElement->SetAttribute("area", section.area);
        memberElement->InsertEndChild(sectionElement);
        
        membersElement->InsertEndChild(memberElement);
    }
    
    root->InsertEndChild(membersElement);
}

void XmlTrussWriter::createLoads(
    tinyxml2::XMLDocument& doc,
    tinyxml2::XMLElement* root,
    const core::Truss& truss
) {
    tinyxml2::XMLElement* loadsElement = doc.NewElement("loads");
    
    for (const auto& node : truss.getLoadedNodes()) {
        const auto& force = node->getAppliedForce();
        
        // Only write non-zero forces
        if (force.magnitude() > 1e-10) {
            tinyxml2::XMLElement* loadElement = doc.NewElement("load");
            loadElement->SetAttribute("nodeId", node->getId());
            loadElement->SetAttribute("fx", force.fx);
            loadElement->SetAttribute("fy", force.fy);
            
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
