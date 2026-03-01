/**
 * @file xml_truss_writer.hpp
 * @brief XML format truss file writer.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "truss_writer.hpp"

#include <tinyxml2.h>

namespace truss::infrastructure::io {

/**
 * @brief Writes truss structures to XML format files.
 *
 * Writes truss structures to XML files with a structured format.
 */
class XmlTrussWriter : public ITrussWriter {
public:
    XmlTrussWriter() = default;
    ~XmlTrussWriter() override = default;

    /**
     * @brief Write truss to XML file
     */
    bool write(const core::interfaces::TrussDTO& trussData,
               const std::filesystem::path& filepath,
               const FileIOOptions& options = FileIOOptions{}) override;

    bool supportsFormat(FileFormat format) const override { return format == FileFormat::XML; }

    FileFormat getFormat() const override { return FileFormat::XML; }

private:
    /**
     * @brief Create metadata element
     */
    static void createMetadata(tinyxml2::XMLDocument& doc,
                               tinyxml2::XMLElement* root,
                               const core::interfaces::TrussDTO& trussData,
                               const FileIOOptions& options);

    /**
     * @brief Create nodes element
     */
    static void createNodes(tinyxml2::XMLDocument& doc,
                            tinyxml2::XMLElement* root,
                            const core::interfaces::TrussDTO& trussData);

    /**
     * @brief Create members element
     */
    static void createMembers(tinyxml2::XMLDocument& doc,
                              tinyxml2::XMLElement* root,
                              const core::interfaces::TrussDTO& trussData);

    /**
     * @brief Create loads element
     */
    static void createLoads(tinyxml2::XMLDocument& doc,
                            tinyxml2::XMLElement* root,
                            const core::interfaces::TrussDTO& trussData);
};

}  // namespace truss::infrastructure::io
