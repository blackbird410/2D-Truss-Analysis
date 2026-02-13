/**
 * @file json_truss_writer.hpp
 * @brief JSON format truss file writer
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#pragma once

#include "truss_writer.hpp"
#include <nlohmann/json.hpp>

namespace truss::infrastructure::io {

/**
 * @brief Concrete implementation of ITrussWriter for JSON format
 * 
 * Writes truss structures to JSON files with a structured format
 * including metadata, nodes, members, and loads.
 */
class JsonTrussWriter : public ITrussWriter {
public:
    JsonTrussWriter() = default;
    ~JsonTrussWriter() override = default;
    
    /**
     * @brief Write truss to JSON file
     */
    bool write(
        const core::Truss& truss,
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
     * @brief Create metadata JSON object
     */
    nlohmann::json createMetadata(const core::Truss& truss, const FileIOOptions& options);
    
    /**
     * @brief Create nodes JSON array
     */
    nlohmann::json createNodes(const core::Truss& truss);
    
    /**
     * @brief Create members JSON array
     */
    nlohmann::json createMembers(const core::Truss& truss);
    
    /**
     * @brief Create loads JSON array
     */
    nlohmann::json createLoads(const core::Truss& truss);
    
    /**
     * @brief Convert support type to string
     */
    std::string supportTypeToString(core::SupportType type);
};

} // namespace truss::infrastructure::io
