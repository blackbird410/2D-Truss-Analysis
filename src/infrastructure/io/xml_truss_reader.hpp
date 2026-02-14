/**
 * @file xml_truss_reader.hpp
 * @brief XML format truss file reader
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#pragma once

#include "truss_reader.hpp"
#include <tinyxml2.h>

namespace truss::infrastructure::io {

/**
 * @brief Concrete implementation of ITrussReader for XML format
 * 
 * Reads truss structures from XML files with the following format:
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
 */
class XmlTrussReader : public ITrussReader {
public:
    XmlTrussReader() = default;
    ~XmlTrussReader() override = default;
    
    /**
     * @brief Read truss from XML file
     */
    core::interfaces::TrussDTO read(
        const std::filesystem::path& filepath,
        const FileIOOptions& options = FileIOOptions{}
    ) override;
    
    bool supportsFormat(FileFormat format) const override {
        return format == FileFormat::XML;
    }
    
    FileFormat getFormat() const override {
        return FileFormat::XML;
    }

private:
    /**
     * @brief Parse metadata section
     */
    void parseMetadata(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto);
    
    /**
     * @brief Parse nodes section
     * @param element Nodes XML element
     * @param dto Target DTO
     */
    void parseNodes(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto);
    
    /**
     * @brief Parse members section
     * @param element Members XML element
     * @param dto Target DTO
     */
    void parseMembers(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto);
    
    /**
     * @brief Parse loads section
     * @param element Loads XML element
     * @param dto Target DTO
     */
    void parseLoads(tinyxml2::XMLElement* element, core::interfaces::TrussDTO& dto);
    
    /**
     * @brief Parse support type from string
     */
    core::SupportType parseSupportType(const std::string& str);
    
    /**
     * @brief Get required attribute as double
     */
    core::Real getDoubleAttribute(tinyxml2::XMLElement* element, const char* name);
    
    /**
     * @brief Get required attribute as int
     */
    int getIntAttribute(tinyxml2::XMLElement* element, const char* name);
    
    /**
     * @brief Get optional attribute as double with default
     */
    core::Real getDoubleAttribute(tinyxml2::XMLElement* element, const char* name, core::Real defaultValue);
};

} // namespace truss::infrastructure::io
