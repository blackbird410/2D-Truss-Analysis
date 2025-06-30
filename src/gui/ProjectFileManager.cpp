/**
 * @file ProjectFileManager.cpp
 * @brief Implementation of project file management
 * @version 2.1.0
 */

#include "ProjectFileManager.hpp"
#include <QFile>
#include <QJsonParseError>
#include <QFileInfo>
#include <QDir>

namespace truss::gui {

QString ProjectFileManager::s_lastError;

bool ProjectFileManager::saveProject(const truss::core::Truss& truss, const QString& filename) {
    s_lastError.clear();
    
    try {
        // Create JSON document
        QJsonObject projectJson;
        projectJson["version"] = "2.1.0";
        projectJson["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        projectJson["truss"] = trussToJson(truss);
        
        QJsonDocument doc(projectJson);
        
        // Ensure directory exists
        QFileInfo fileInfo(filename);
        QDir dir = fileInfo.absoluteDir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        // Save to file
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            s_lastError = QString("Cannot write to file: %1").arg(file.errorString());
            return false;
        }
        
        qint64 written = file.write(doc.toJson());
        if (written == -1) {
            s_lastError = "Failed to write data to file";
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        s_lastError = QString("Exception while saving: %1").arg(e.what());
        return false;
    }
}

std::unique_ptr<truss::core::Truss> ProjectFileManager::loadProject(const QString& filename) {
    s_lastError.clear();
    
    try {
        // Read file
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            s_lastError = QString("Cannot read file: %1").arg(file.errorString());
            return nullptr;
        }
        
        QByteArray data = file.readAll();
        
        // Parse JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            s_lastError = QString("JSON parse error: %1").arg(parseError.errorString());
            return nullptr;
        }
        
        QJsonObject projectJson = doc.object();
        
        // Check version compatibility
        QString version = projectJson["version"].toString();
        if (version.isEmpty()) {
            s_lastError = "Invalid project file: missing version information";
            return nullptr;
        }
        
        // Load truss
        QJsonObject trussJson = projectJson["truss"].toObject();
        auto truss = jsonToTruss(trussJson);
        
        if (!truss) {
            // Error message already set in jsonToTruss
            return nullptr;
        }
        
        return truss;
        
    } catch (const std::exception& e) {
        s_lastError = QString("Exception while loading: %1").arg(e.what());
        return nullptr;
    }
}

QJsonObject ProjectFileManager::trussToJson(const truss::core::Truss& truss) {
    QJsonObject trussJson;
    
    // Basic properties
    trussJson["name"] = QString::fromStdString(truss.getName());
    
    // Nodes
    QJsonArray nodesArray;
    for (const auto& node : truss.getNodes()) {
        nodesArray.append(nodeToJson(*node));
    }
    trussJson["nodes"] = nodesArray;
    
    // Members
    QJsonArray membersArray;
    for (const auto& member : truss.getMembers()) {
        membersArray.append(memberToJson(*member));
    }
    trussJson["members"] = membersArray;
    
    return trussJson;
}

std::unique_ptr<truss::core::Truss> ProjectFileManager::jsonToTruss(const QJsonObject& json) {
    try {
        auto truss = std::make_unique<truss::core::Truss>();
        
        // Basic properties
        if (json.contains("name")) {
            truss->setName(json["name"].toString().toStdString());
        }
        
        // Load nodes first
        std::map<truss::core::NodeId, std::shared_ptr<truss::core::Node>> nodeMap;
        
        QJsonArray nodesArray = json["nodes"].toArray();
        for (const auto& nodeValue : nodesArray) {
            QJsonObject nodeJson = nodeValue.toObject();
            
            truss::core::NodeId id = nodeJson["id"].toInt();
            double x = nodeJson["x"].toDouble();
            double y = nodeJson["y"].toDouble();
            
            auto node = std::make_shared<truss::core::Node>(id, x, y);
            
            // Set support conditions
            if (nodeJson.contains("supports")) {
                QJsonObject supportsJson = nodeJson["supports"].toObject();
                if (supportsJson["fixedX"].toBool() && supportsJson["fixedY"].toBool()) {
                    node->setSupportType(truss::core::SupportType::Pinned);
                } else if (supportsJson["fixedX"].toBool()) {
                    node->setSupportType(truss::core::SupportType::PinnedX);
                } else if (supportsJson["fixedY"].toBool()) {
                    node->setSupportType(truss::core::SupportType::PinnedY);
                }
            }
            
            // Set applied forces
            if (nodeJson.contains("force")) {
                QJsonObject forceJson = nodeJson["force"].toObject();
                double fx = forceJson["fx"].toDouble();
                double fy = forceJson["fy"].toDouble();
                if (fx != 0.0 || fy != 0.0) {
                    node->setAppliedForce(fx, fy);
                }
            }
            
            nodeMap[id] = node;
            truss->addNode(node);
        }
        
        // Load members
        QJsonArray membersArray = json["members"].toArray();
        for (const auto& memberValue : membersArray) {
            QJsonObject memberJson = memberValue.toObject();
            
            truss::core::MemberId id = memberJson["id"].toInt();
            truss::core::NodeId startNodeId = memberJson["startNodeId"].toInt();
            truss::core::NodeId endNodeId = memberJson["endNodeId"].toInt();
            
            auto startNode = nodeMap[startNodeId];
            auto endNode = nodeMap[endNodeId];
            
            if (!startNode || !endNode) {
                s_lastError = QString("Invalid member: node not found (member %1)").arg(id);
                return nullptr;
            }
            
            // Material properties
            truss::core::MaterialProperties material;
            if (memberJson.contains("material")) {
                QJsonObject matJson = memberJson["material"].toObject();
                material.youngModulus = matJson["youngModulus"].toDouble();
                material.yieldStrength = matJson["yieldStrength"].toDouble();
                material.density = matJson["density"].toDouble();
            }
            
            // Section properties
            truss::core::SectionProperties section;
            if (memberJson.contains("section")) {
                QJsonObject secJson = memberJson["section"].toObject();
                section.area = secJson["area"].toDouble();
            }
            
            auto member = std::make_shared<truss::core::Member>(id, startNode, endNode, material, section);
            truss->addMember(member);
        }
        
        return truss;
        
    } catch (const std::exception& e) {
        s_lastError = QString("Error parsing truss data: %1").arg(e.what());
        return nullptr;
    }
}

QJsonObject ProjectFileManager::nodeToJson(const truss::core::Node& node) {
    QJsonObject nodeJson;
    
    nodeJson["id"] = static_cast<int>(node.getId());
    nodeJson["x"] = node.getX();
    nodeJson["y"] = node.getY();
    nodeJson["label"] = QString::fromStdString(node.getLabel());
    
    // Support conditions
    QJsonObject supportsJson;
    supportsJson["fixedX"] = node.isConstrainedX();
    supportsJson["fixedY"] = node.isConstrainedY();
    nodeJson["supports"] = supportsJson;
    
    // Applied forces
    if (node.hasAppliedForce()) {
        QJsonObject forceJson;
        auto force = node.getAppliedForce();
        forceJson["fx"] = force.fx;
        forceJson["fy"] = force.fy;
        nodeJson["force"] = forceJson;
    }
    
    return nodeJson;
}

QJsonObject ProjectFileManager::memberToJson(const truss::core::Member& member) {
    QJsonObject memberJson;
    
    memberJson["id"] = static_cast<int>(member.getId());
    memberJson["startNodeId"] = static_cast<int>(member.getStartNode()->getId());
    memberJson["endNodeId"] = static_cast<int>(member.getEndNode()->getId());
    memberJson["label"] = QString::fromStdString(member.getLabel());
    
    // Material properties
    memberJson["material"] = materialToJson(member.getMaterial());
    
    // Section properties
    memberJson["section"] = sectionToJson(member.getSection());
    
    return memberJson;
}

QJsonObject ProjectFileManager::materialToJson(const truss::core::MaterialProperties& material) {
    QJsonObject materialJson;
    
    materialJson["youngModulus"] = material.youngModulus;
    materialJson["yieldStrength"] = material.yieldStrength;
    materialJson["density"] = material.density;
    
    return materialJson;
}

QJsonObject ProjectFileManager::sectionToJson(const truss::core::SectionProperties& section) {
    QJsonObject sectionJson;
    
    sectionJson["area"] = section.area;
    
    return sectionJson;
}

} // namespace truss::gui
