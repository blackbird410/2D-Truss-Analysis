/**
 * @file MaterialLibraryService.hpp
 * @brief Material and section property library service
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Provides centralized management of material and section properties
 * for use in GUI and other interface layers.
 * 
 * Features:
 * - Standard material presets (Steel, Aluminum, Concrete)
 * - Standard section presets (Circular, Square, Rectangular)
 * - Custom material/section management
 * - Type-safe property retrieval
 */

#pragma once

#include "../core/model/Types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace truss::application {

/**
 * @brief Material preset descriptor
 */
struct MaterialPreset {
    std::string name;          ///< Display name (e.g., "Steel Grade 50")
    std::string description;   ///< Description (e.g., "Structural steel")
    core::MaterialProperties properties;
};

/**
 * @brief Section preset descriptor
 */
struct SectionPreset {
    std::string name;          ///< Display name (e.g., "50mm Circular")
    std::string description;   ///< Description (e.g., "Solid circular section")
    core::SectionProperties properties;
};

/**
 * @brief Interface for material and section library
 */
class IMaterialLibrary {
public:
    virtual ~IMaterialLibrary() = default;
    
    /**
     * @brief Get all available material presets
     * @return Vector of material presets
     */
    virtual std::vector<MaterialPreset> getAvailableMaterials() const = 0;
    
    /**
     * @brief Get material properties by name
     * @param name Material name
     * @return Material properties if found
     */
    virtual std::optional<core::MaterialProperties> getMaterial(const std::string& name) const = 0;
    
    /**
     * @brief Get all available section presets
     * @return Vector of section presets
     */
    virtual std::vector<SectionPreset> getAvailableSections() const = 0;
    
    /**
     * @brief Get section properties by name
     * @param name Section name
     * @return Section properties if found
     */
    virtual std::optional<core::SectionProperties> getSection(const std::string& name) const = 0;
    
    /**
     * @brief Add custom material preset
     * @param name Material name
     * @param description Material description
     * @param properties Material properties
     */
    virtual void addCustomMaterial(const std::string& name,
                                   const std::string& description,
                                   const core::MaterialProperties& properties) = 0;
    
    /**
     * @brief Add custom section preset
     * @param name Section name
     * @param description Section description
     * @param properties Section properties
     */
    virtual void addCustomSection(const std::string& name,
                                  const std::string& description,
                                  const core::SectionProperties& properties) = 0;
};

/**
 * @brief Material and section library service implementation
 * 
 * Provides standard engineering materials and section profiles
 * with support for custom user-defined properties.
 * 
 * Thread Safety: Not thread-safe (intended for single-threaded GUI applications)
 * 
 * Usage Example:
 * @code
 * MaterialLibraryService library;
 * auto materials = library.getAvailableMaterials();
 * for (const auto& preset : materials) {
 *     // Present or log preset information as needed
 * }
 * 
 * auto steel = library.getMaterial("Steel");
 * if (steel) {
 *     // Use steel properties
 * }
 * @endcode
 */
class MaterialLibraryService : public IMaterialLibrary {
public:
    /**
     * @brief Construct library with default materials and sections
     */
    MaterialLibraryService();
    
    // IMaterialLibrary interface implementation
    std::vector<MaterialPreset> getAvailableMaterials() const override;
    std::optional<core::MaterialProperties> getMaterial(const std::string& name) const override;
    
    std::vector<SectionPreset> getAvailableSections() const override;
    std::optional<core::SectionProperties> getSection(const std::string& name) const override;
    
    void addCustomMaterial(const std::string& name,
                          const std::string& description,
                          const core::MaterialProperties& properties) override;
    
    void addCustomSection(const std::string& name,
                         const std::string& description,
                         const core::SectionProperties& properties) override;
    
    /**
     * @brief Remove custom material
     * @param name Material name
     * @return true if material was removed
     */
    bool removeCustomMaterial(const std::string& name);
    
    /**
     * @brief Remove custom section
     * @param name Section name
     * @return true if section was removed
     */
    bool removeCustomSection(const std::string& name);
    
    /**
     * @brief Check if material exists
     * @param name Material name
     * @return true if material exists in library
     */
    bool hasMaterial(const std::string& name) const;
    
    /**
     * @brief Check if section exists
     * @param name Section name
     * @return true if section exists in library
     */
    bool hasSection(const std::string& name) const;

private:
    std::unordered_map<std::string, MaterialPreset> m_materials;
    std::unordered_map<std::string, SectionPreset> m_sections;
    
    void initializeDefaultMaterials();
    void initializeDefaultSections();
};

} // namespace truss::application
