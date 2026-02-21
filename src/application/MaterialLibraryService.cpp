/**
 * @file MaterialLibraryService.cpp
 * @brief Implementation of material and section library service
 */

#include "MaterialLibraryService.hpp"

#include <stdexcept>

namespace truss::application {

using namespace truss::core;

MaterialLibraryService::MaterialLibraryService() {
    initializeDefaultMaterials();
    initializeDefaultSections();
}

std::vector<MaterialPreset> MaterialLibraryService::getAvailableMaterials() const {
    std::vector<MaterialPreset> result;
    result.reserve(m_materials.size());

    for (const auto& [name, preset] : m_materials) {
        result.push_back(preset);
    }

    return result;
}

std::optional<MaterialProperties>
MaterialLibraryService::getMaterial(const std::string& name) const {
    auto it = m_materials.find(name);
    if (it != m_materials.end()) {
        return it->second.properties;
    }
    return std::nullopt;
}

std::vector<SectionPreset> MaterialLibraryService::getAvailableSections() const {
    std::vector<SectionPreset> result;
    result.reserve(m_sections.size());

    for (const auto& [name, preset] : m_sections) {
        result.push_back(preset);
    }

    return result;
}

std::optional<SectionProperties> MaterialLibraryService::getSection(const std::string& name) const {
    auto it = m_sections.find(name);
    if (it != m_sections.end()) {
        return it->second.properties;
    }
    return std::nullopt;
}

void MaterialLibraryService::addCustomMaterial(const std::string& name,
                                               const std::string& description,
                                               const MaterialProperties& properties) {
    MaterialPreset preset{name, description, properties};
    m_materials[name] = preset;
}

void MaterialLibraryService::addCustomSection(const std::string& name,
                                              const std::string& description,
                                              const SectionProperties& properties) {
    SectionPreset preset{name, description, properties};
    m_sections[name] = preset;
}

[[maybe_unused]] bool MaterialLibraryService::removeCustomMaterial(const std::string& name) {
    return m_materials.erase(name) > 0;
}

[[maybe_unused]] bool MaterialLibraryService::removeCustomSection(const std::string& name) {
    return m_sections.erase(name) > 0;
}

[[maybe_unused]] bool MaterialLibraryService::hasMaterial(const std::string& name) const {
    return m_materials.find(name) != m_materials.end();
}

[[maybe_unused]] bool MaterialLibraryService::hasSection(const std::string& name) const {
    return m_sections.find(name) != m_sections.end();
}

void MaterialLibraryService::initializeDefaultMaterials() {
    // Steel Grade 50 (Structural steel, common in construction)
    {
        MaterialProperties props;
        props.youngModulus = 200.0e9;      // 200 GPa
        props.density = 7850.0;            // 7850 kg/m³
        props.yieldStrength = 250.0e6;     // 250 MPa
        props.ultimateStrength = 400.0e6;  // 400 MPa
        props.name = "Steel";

        addMaterialInternal("Steel", "Structural steel (Grade 50)", props);
    }

    // Aluminum Alloy 6061-T6 (Common structural aluminum)
    {
        MaterialProperties props;
        props.youngModulus = 70.0e9;       // 70 GPa
        props.density = 2700.0;            // 2700 kg/m³
        props.yieldStrength = 200.0e6;     // 200 MPa
        props.ultimateStrength = 310.0e6;  // 310 MPa
        props.name = "Aluminum";

        addMaterialInternal("Aluminum", "Aluminum alloy 6061-T6", props);
    }

    // Concrete (C25/30 - Common structural concrete)
    {
        MaterialProperties props;
        props.youngModulus = 25.0e9;      // 25 GPa
        props.density = 2400.0;           // 2400 kg/m³
        props.yieldStrength = 30.0e6;     // 30 MPa (compressive)
        props.ultimateStrength = 35.0e6;  // 35 MPa
        props.name = "Concrete";

        addMaterialInternal("Concrete", "Structural concrete C25/30", props);
    }

    // Timber (Softwood - Spruce/Pine/Fir)
    {
        MaterialProperties props;
        props.youngModulus = 12.0e9;      // 12 GPa
        props.density = 500.0;            // 500 kg/m³
        props.yieldStrength = 30.0e6;     // 30 MPa (bending)
        props.ultimateStrength = 40.0e6;  // 40 MPa
        props.name = "Timber";

        addMaterialInternal("Timber", "Softwood (Spruce/Pine/Fir)", props);
    }
}

void MaterialLibraryService::initializeDefaultSections() {
    // Circular sections (solid)
    {
        SectionProperties props;
        props.area = 0.001963;             // π * (0.025)² = 1963 mm² (50mm diameter)
        props.momentOfInertia = 3.068e-7;  // π/64 * d⁴
        props.shearArea = 0.001473;        // ~75% of gross area (typical for circular)
        props.designation = "Circular 50mm";

        addSectionInternal("Circular 50mm", "Solid circular section (diameter 50mm)", props);
    }

    {
        SectionProperties props;
        props.area = 0.007854;             // π * (0.05)² = 7854 mm² (100mm diameter)
        props.momentOfInertia = 4.909e-6;  // π/64 * d⁴
        props.shearArea = 0.005891;        // ~75% of gross area
        props.designation = "Circular 100mm";

        addSectionInternal("Circular 100mm", "Solid circular section (diameter 100mm)", props);
    }

    // Square sections (solid)
    {
        SectionProperties props;
        props.area = 0.0025;               // 50mm × 50mm = 2500 mm²
        props.momentOfInertia = 5.208e-7;  // bh³/12
        props.shearArea = 0.002083;        // ~83% of gross area (typical for square)
        props.designation = "Square 50mm";

        addSectionInternal("Square 50mm", "Solid square section (50mm × 50mm)", props);
    }

    {
        SectionProperties props;
        props.area = 0.01;                 // 100mm × 100mm = 10000 mm²
        props.momentOfInertia = 8.333e-6;  // bh³/12
        props.shearArea = 0.008333;        // ~83% of gross area
        props.designation = "Square 100mm";

        addSectionInternal("Square 100mm", "Solid square section (100mm × 100mm)", props);
    }

    // Rectangular sections (solid)
    {
        SectionProperties props;
        props.area = 0.0015;               // 30mm × 50mm = 1500 mm²
        props.momentOfInertia = 3.125e-7;  // bh³/12 (major axis)
        props.shearArea = 0.00125;         // ~83% of gross area
        props.designation = "Rectangular 30x50mm";

        addSectionInternal("Rectangular 30x50mm", "Solid rectangular section (30mm × 50mm)", props);
    }

    {
        SectionProperties props;
        props.area = 0.006;              // 60mm × 100mm = 6000 mm²
        props.momentOfInertia = 5.0e-6;  // bh³/12 (major axis)
        props.shearArea = 0.005;         // ~83% of gross area
        props.designation = "Rectangular 60x100mm";

        addSectionInternal("Rectangular 60x100mm", "Solid rectangular section (60mm × 100mm)", props);
    }
}

// Non-virtual helper methods to avoid virtual calls in constructor
void MaterialLibraryService::addMaterialInternal(const std::string& name,
                                                  const std::string& description,
                                                  const MaterialProperties& properties) {
    m_materials[name] = MaterialPreset{name, description, properties};
}

void MaterialLibraryService::addSectionInternal(const std::string& name,
                                                 const std::string& description,
                                                 const SectionProperties& properties) {
    m_sections[name] = SectionPreset{name, description, properties};
}

}  // namespace truss::application
