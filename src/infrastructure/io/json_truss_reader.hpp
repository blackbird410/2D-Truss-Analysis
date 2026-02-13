/**
 * @file json_truss_reader.hpp
 * @brief JSON format truss file reader
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#pragma once

#include "truss_reader.hpp"
#include <nlohmann/json.hpp>

namespace truss::infrastructure::io {

/**
 * @brief Concrete implementation of ITrussReader for JSON format
 * 
 * Reads truss structures from JSON files with the following format:
 * {
 *   "metadata": { "name": "...", "description": "...", "version": "..." },
 *   "nodes": [ {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"}, ... ],
 *   "members": [ {"id": 1, "startNode": 1, "endNode": 2, "material": {...}, "section": {...}}, ... ],
 *   "loads": [ {"id": 1, "nodeId": 1, "fx": 0.0, "fy": -1000.0, "label": "..."}, ... ]
 * }
 */
class JsonTrussReader : public ITrussReader {
public:
    JsonTrussReader() = default;
    ~JsonTrussReader() override = default;
    
    /**
     * @brief Read truss from JSON file
     */
    std::shared_ptr<core::Truss> read(
        const std::filesystem::path& filepath,
        const FileIOOptions& options = FileIOOptions{}
    ) override;
    
    bool supportsFormat(FileFormat format) const override {
        return format == FileFormat::JSON;
    }
    
    FileFormat getFormat() const override {
        return FileFormat::JSON;
    }

private:
    /**
     * @brief Parse metadata section
     */
    void parseMetadata(const nlohmann::json& j, core::Truss& truss);
    
    /**
     * @brief Parse nodes section
     */
    void parseNodes(const nlohmann::json& j, core::Truss& truss);
    
    /**
     * @brief Parse members section
     */
    void parseMembers(const nlohmann::json& j, core::Truss& truss);
    
    /**
     * @brief Parse loads section
     */
    void parseLoads(const nlohmann::json& j, core::Truss& truss);
    
    /**
     * @brief Parse support type from string
     */
    core::SupportType parseSupportType(const std::string& str);
};

} // namespace truss::infrastructure::io
