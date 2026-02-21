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

#include <unordered_set>

namespace truss::infrastructure::io {

/**
 * @brief Concrete implementation of ITrussReader for JSON format
 *
 * Reads truss structures from JSON files with the following format:
 * {
 *   "metadata": { "name": "...", "version": "..." },
 *   "nodes": [ { "x": 0.0, "y": 0.0, "support": "pinned" }, ... ],
 *   "members": [ { "startNode": 1, "endNode": 2, "material": { ... }, "section": { ... } }, ... ],
 *   "loads": [ { "nodeId": 1, "fx": 0.0, "fy": -1000.0 }, ... ]
 * }
 */
class JsonTrussReader : public ITrussReader {
public:
    JsonTrussReader() = default;
    ~JsonTrussReader() override = default;

    /**
     * @brief Read truss from JSON file
     */
    core::interfaces::TrussDTO read(const std::filesystem::path& filepath,
                                    const FileIOOptions& options = FileIOOptions{}) override;

    bool supportsFormat(FileFormat format) const override { return format == FileFormat::JSON; }

    FileFormat getFormat() const override { return FileFormat::JSON; }

private:
    /**
     * @brief Parse metadata section
     */
    void parseMetadata(const nlohmann::json& j, core::interfaces::TrussDTO& dto);

    /**
     * @brief Parse nodes section - creates NodeDTO objects
     * @return Set of parsed node IDs for validation
     */
    std::unordered_set<core::NodeId> parseNodes(const nlohmann::json& j,
                                                core::interfaces::TrussDTO& dto);

    /**
     * @brief Parse members section - creates MemberDTO objects
     * @param validNodeIds Set of valid node IDs for referential integrity checking
     */
    void parseMembers(const nlohmann::json& j,
                      core::interfaces::TrussDTO& dto,
                      const std::unordered_set<core::NodeId>& validNodeIds);

    /**
     * @brief Parse loads section - updates NodeDTO force fields
     * @param validNodeIds Set of valid node IDs for referential integrity checking
     */
    void parseLoads(const nlohmann::json& j,
                    core::interfaces::TrussDTO& dto,
                    const std::unordered_set<core::NodeId>& validNodeIds);

    /**
     * @brief Parse support type from string
     */
    core::SupportType parseSupportType(const std::string& str);
};

}  // namespace truss::infrastructure::io
