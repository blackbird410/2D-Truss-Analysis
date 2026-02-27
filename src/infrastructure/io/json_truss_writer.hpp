/**
 * @file json_truss_writer.hpp
 * @brief JSON format truss file writer.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "truss_writer.hpp"

#include <nlohmann/json.hpp>

namespace truss::infrastructure::io {

/**
 * @brief Writes truss structures to JSON format files.
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
    bool write(const core::interfaces::TrussDTO& trussData,
               const std::filesystem::path& filepath,
               const FileIOOptions& options = FileIOOptions{}) override;

    bool supportsFormat(FileFormat format) const override { return format == FileFormat::JSON; }

    FileFormat getFormat() const override { return FileFormat::JSON; }

private:
    /**
     * @brief Create metadata JSON object
     */
    static nlohmann::json createMetadata(const core::interfaces::TrussDTO& trussData,
                                         const FileIOOptions& options);

    /**
     * @brief Create nodes JSON array
     */
    static nlohmann::json createNodes(const core::interfaces::TrussDTO& trussData);

    /**
     * @brief Create members JSON array
     */
    static nlohmann::json createMembers(const core::interfaces::TrussDTO& trussData);

    /**
     * @brief Create loads JSON array
     */
    static nlohmann::json createLoads(const core::interfaces::TrussDTO& trussData);

    /**
     * @brief Convert support type to string
     */
    static std::string supportTypeToString(core::SupportType type);
};

}  // namespace truss::infrastructure::io
