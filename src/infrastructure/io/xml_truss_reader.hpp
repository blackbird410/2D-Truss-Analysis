/**
 * @file xml_truss_reader.hpp
 * @brief XML format truss file reader.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "truss_reader.hpp"

#include <tinyxml2.h>

#include <unordered_map>
#include <unordered_set>

namespace truss::infrastructure::io {

/**
 * @brief Reads truss structures from XML format files.
 *
 * Reads truss structures from XML files with the following format:
 * @code{.xml}
 * <truss>
 *   <metadata name="..." />
 *   <nodes>
 *     <node id="1" x="0.0" y="0.0" support="pinned" />
 *   </nodes>
 *   <members>
 *     <member id="1" startNode="1" endNode="2">
 *       <material youngsModulus="..." density="..." />
 *       <section area="..." />
 *     </member>
 *   </members>
 *   <loads>
 *     <load nodeId="1" fx="0.0" fy="-1000.0" />
 *   </loads>
 * </truss>
 * @endcode
 */
class XmlTrussReader : public ITrussReader {
public:
    XmlTrussReader() = default;
    ~XmlTrussReader() override = default;

    /**
     * @brief Read truss from XML file
     */
    core::interfaces::TrussDTO read(const std::filesystem::path& filepath,
                                    const FileIOOptions& options = FileIOOptions{}) override;

    bool supportsFormat(FileFormat format) const override { return format == FileFormat::XML; }

    FileFormat getFormat() const override { return FileFormat::XML; }

private:
    /**
     * @brief Parse metadata section
     * @param element Metadata XML element
     * @param dto Target DTO to populate with metadata
     */
    static void parseMetadata(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto);

    /**
     * @brief Parse nodes section
     * @param element Nodes XML element
     * @param dto Target DTO
     * @return Set of parsed node IDs for validation
     */
    static std::unordered_set<core::NodeId> parseNodes(tinyxml2::XMLElement* nodesElement,
                                                       core::interfaces::TrussDTO& dto);

    /**
     * @brief Parse members section
     * @param membersElement Members XML element
     * @param dto Target DTO
     * @param validNodeIds Set of valid node IDs for referential integrity checking
     * @param materials Map of material IDs to their properties
     * @param sections Map of section IDs to their properties
     */
    static void
    parseMembers(tinyxml2::XMLElement* membersElement,
                 core::interfaces::TrussDTO& dto,
                 const std::unordered_set<core::NodeId>& validNodeIds,
                 const std::unordered_map<std::string, tinyxml2::XMLElement*>& materials,
                 const std::unordered_map<std::string, tinyxml2::XMLElement*>& sections);

    /**
     * @brief Parse materials section
     * @return Map of material IDs to their XML elements
     */
    static std::unordered_map<std::string, tinyxml2::XMLElement*>
    parseMaterials(tinyxml2::XMLElement* materialsElement);

    /**
     * @brief Parse sections section
     * @return Map of section IDs to their XML elements
     */
    static std::unordered_map<std::string, tinyxml2::XMLElement*>
    parseSections(tinyxml2::XMLElement* sectionsElement);

    /**
     * @brief Parse supports section
     * @param supportsElement Supports XML element
     * @param dto Target DTO
     * @param validNodeIds Set of valid node IDs for referential integrity checking
     */
    static void parseSupports(tinyxml2::XMLElement* supportsElement,
                              core::interfaces::TrussDTO& dto,
                              const std::unordered_set<core::NodeId>& validNodeIds);

    /**
     * @brief Parse loads section
     * @param loadsElement Loads XML element
     * @param dto Target DTO
     * @param validNodeIds Set of valid node IDs for referential integrity checking
     */
    static void parseLoads(tinyxml2::XMLElement* loadsElement,
                           core::interfaces::TrussDTO& dto,
                           const std::unordered_set<core::NodeId>& validNodeIds);

    /**
     * @brief Get required attribute as double
     * @param element XML element to read from
     * @param name Attribute name
     * @return Attribute value as double
     */
    static core::Real getDoubleAttribute(tinyxml2::XMLElement* element, const char* name);

    /**
     * @brief Get required attribute as int
     * @param element XML element to read from
     * @param name Attribute name
     * @return Attribute value as integer
     */
    static int getIntAttribute(tinyxml2::XMLElement* element, const char* name);

    /**
     * @brief Get optional attribute as double with default
     * @param element XML element to read from
     * @param name Attribute name
     * @param defaultValue Default value if attribute is missing
     * @return Attribute value or default
     */
    static core::Real
    getDoubleAttribute(tinyxml2::XMLElement* element, const char* name, core::Real defaultValue);
};

}  // namespace truss::infrastructure::io
