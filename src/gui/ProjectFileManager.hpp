/**
 * @file ProjectFileManager.hpp
 * @brief Project file management for saving and loading truss projects
 * @version 2.1.0
 */

#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <memory>
#include "../core/model/Truss.hpp"

namespace truss::gui {

/**
 * @brief Manages project file operations for saving and loading truss projects
 */
class ProjectFileManager {
public:
    /**
     * @brief Save a truss project to file
     * @param truss The truss to save
     * @param filename Path to save the project file
     * @return true if successful, false otherwise
     */
    static bool saveProject(const truss::core::Truss& truss, const QString& filename);
    
    /**
     * @brief Load a truss project from file
     * @param filename Path to the project file
     * @return Unique pointer to loaded truss, nullptr if failed
     */
    static std::unique_ptr<truss::core::Truss> loadProject(const QString& filename);
    
    /**
     * @brief Get the last error message
     * @return Error message string
     */
    static QString getLastError() { return s_lastError; }

private:
    /**
     * @brief Convert truss to JSON object
     * @param truss The truss to convert
     * @return JSON object representation
     */
    static QJsonObject trussToJson(const truss::core::Truss& truss);
    
    /**
     * @brief Convert JSON object to truss
     * @param json The JSON object to convert
     * @return Unique pointer to truss, nullptr if failed
     */
    static std::unique_ptr<truss::core::Truss> jsonToTruss(const QJsonObject& json);
    
    /**
     * @brief Convert node to JSON object
     * @param node The node to convert
     * @return JSON object representation
     */
    static QJsonObject nodeToJson(const truss::core::Node& node);
    
    /**
     * @brief Convert member to JSON object
     * @param member The member to convert
     * @return JSON object representation
     */
    static QJsonObject memberToJson(const truss::core::Member& member);
    
    /**
     * @brief Convert material properties to JSON object
     * @param material The material to convert
     * @return JSON object representation
     */
    static QJsonObject materialToJson(const truss::core::MaterialProperties& material);
    
    /**
     * @brief Convert section properties to JSON object
     * @param section The section to convert
     * @return JSON object representation
     */
    static QJsonObject sectionToJson(const truss::core::SectionProperties& section);
    
    static QString s_lastError;
};

} // namespace truss::gui
