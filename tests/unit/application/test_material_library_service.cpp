/**
 * @file test_material_library_service.cpp
 * @brief Unit tests for MaterialLibraryService
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Test Coverage:
 * - Default materials (Steel, Aluminum, Concrete, Timber)
 * - Default sections (Circular, Square, Rectangular - various sizes)
 * - Material properties correctness (E, ρ, fy, fu values)
 * - Section properties correctness (A, I, As values)
 * - Custom material add/remove operations
 * - Custom section add/remove operations
 * - getMaterial() with valid/invalid names
 * - getSection() with valid/invalid names
 * - hasMaterial() existence checks
 * - hasSection() existence checks
 * - getAvailableMaterials() completeness
 * - getAvailableSections() completeness
 *
 * Target Coverage: >95%
 */

#include "../../../src/application/material_library_service.hpp"
#include "../../../src/core/model/types.hpp"

#include <algorithm>
#include <gtest/gtest.h>

using namespace truss::application;
using namespace truss::core;

/**
 * @brief Test fixture for MaterialLibraryService
 */
class MaterialLibraryServiceTest : public ::testing::Test {
protected:
    MaterialLibraryService library;

    void SetUp() override {
        // Library is initialized with defaults in constructor
    }
};

// ============================================================================
// DEFAULT MATERIALS TESTS
// ============================================================================

TEST_F(MaterialLibraryServiceTest, DefaultMaterials_SteelExists) {
    EXPECT_TRUE(library.hasMaterial("Steel"));

    auto material = library.getMaterial("Steel");
    ASSERT_TRUE(material.has_value());

    EXPECT_DOUBLE_EQ(material->youngModulus, 200.0e9);      // 200 GPa
    EXPECT_DOUBLE_EQ(material->density, 7850.0);            // 7850 kg/m³
    EXPECT_DOUBLE_EQ(material->yieldStrength, 250.0e6);     // 250 MPa
    EXPECT_DOUBLE_EQ(material->ultimateStrength, 400.0e6);  // 400 MPa
    EXPECT_EQ(material->name, "Steel");
}

TEST_F(MaterialLibraryServiceTest, DefaultMaterials_AluminumExists) {
    EXPECT_TRUE(library.hasMaterial("Aluminum"));

    auto material = library.getMaterial("Aluminum");
    ASSERT_TRUE(material.has_value());

    EXPECT_DOUBLE_EQ(material->youngModulus, 70.0e9);       // 70 GPa
    EXPECT_DOUBLE_EQ(material->density, 2700.0);            // 2700 kg/m³
    EXPECT_DOUBLE_EQ(material->yieldStrength, 200.0e6);     // 200 MPa
    EXPECT_DOUBLE_EQ(material->ultimateStrength, 310.0e6);  // 310 MPa
    EXPECT_EQ(material->name, "Aluminum");
}

TEST_F(MaterialLibraryServiceTest, DefaultMaterials_ConcreteExists) {
    EXPECT_TRUE(library.hasMaterial("Concrete"));

    auto material = library.getMaterial("Concrete");
    ASSERT_TRUE(material.has_value());

    EXPECT_DOUBLE_EQ(material->youngModulus, 25.0e9);      // 25 GPa
    EXPECT_DOUBLE_EQ(material->density, 2400.0);           // 2400 kg/m³
    EXPECT_DOUBLE_EQ(material->yieldStrength, 30.0e6);     // 30 MPa
    EXPECT_DOUBLE_EQ(material->ultimateStrength, 35.0e6);  // 35 MPa
    EXPECT_EQ(material->name, "Concrete");
}

TEST_F(MaterialLibraryServiceTest, DefaultMaterials_TimberExists) {
    EXPECT_TRUE(library.hasMaterial("Timber"));

    auto material = library.getMaterial("Timber");
    ASSERT_TRUE(material.has_value());

    EXPECT_DOUBLE_EQ(material->youngModulus, 12.0e9);      // 12 GPa
    EXPECT_DOUBLE_EQ(material->density, 500.0);            // 500 kg/m³
    EXPECT_DOUBLE_EQ(material->yieldStrength, 30.0e6);     // 30 MPa
    EXPECT_DOUBLE_EQ(material->ultimateStrength, 40.0e6);  // 40 MPa
    EXPECT_EQ(material->name, "Timber");
}

TEST_F(MaterialLibraryServiceTest, GetAvailableMaterials_IncludesAllDefaults) {
    auto materials = library.getAvailableMaterials();

    EXPECT_GE(materials.size(), 4);  // At least 4 default materials

    std::vector<std::string> materialNames;
    for (const auto& preset : materials) {
        materialNames.push_back(preset.name);
    }

    EXPECT_TRUE(std::find(materialNames.begin(), materialNames.end(), "Steel") !=
                materialNames.end());
    EXPECT_TRUE(std::find(materialNames.begin(), materialNames.end(), "Aluminum") !=
                materialNames.end());
    EXPECT_TRUE(std::find(materialNames.begin(), materialNames.end(), "Concrete") !=
                materialNames.end());
    EXPECT_TRUE(std::find(materialNames.begin(), materialNames.end(), "Timber") !=
                materialNames.end());
}

TEST_F(MaterialLibraryServiceTest, GetMaterial_NonExistent_ReturnsNullopt) {
    auto material = library.getMaterial("NonExistentMaterial");

    EXPECT_FALSE(material.has_value());
}

TEST_F(MaterialLibraryServiceTest, HasMaterial_ExistingMaterial_ReturnsTrue) {
    EXPECT_TRUE(library.hasMaterial("Steel"));
    EXPECT_TRUE(library.hasMaterial("Aluminum"));
    EXPECT_TRUE(library.hasMaterial("Concrete"));
    EXPECT_TRUE(library.hasMaterial("Timber"));
}

TEST_F(MaterialLibraryServiceTest, HasMaterial_NonExistentMaterial_ReturnsFalse) {
    EXPECT_FALSE(library.hasMaterial("Unobtainium"));
    EXPECT_FALSE(library.hasMaterial(""));
}

// ============================================================================
// DEFAULT SECTIONS TESTS
// ============================================================================

TEST_F(MaterialLibraryServiceTest, DefaultSections_Circular50mmExists) {
    EXPECT_TRUE(library.hasSection("Circular 50mm"));

    auto section = library.getSection("Circular 50mm");
    ASSERT_TRUE(section.has_value());

    EXPECT_NEAR(section->area, 0.001963, 1e-6);             // π*(0.025)²
    EXPECT_NEAR(section->momentOfInertia, 3.068e-7, 1e-9);  // π/64*d⁴
    EXPECT_NEAR(section->shearArea, 0.001473, 1e-6);        // ~75% of area
    EXPECT_EQ(section->designation, "Circular 50mm");
}

TEST_F(MaterialLibraryServiceTest, DefaultSections_Circular100mmExists) {
    EXPECT_TRUE(library.hasSection("Circular 100mm"));

    auto section = library.getSection("Circular 100mm");
    ASSERT_TRUE(section.has_value());

    EXPECT_NEAR(section->area, 0.007854, 1e-6);  // π*(0.05)²
    EXPECT_NEAR(section->momentOfInertia, 4.909e-6, 1e-9);
    EXPECT_NEAR(section->shearArea, 0.005891, 1e-6);
    EXPECT_EQ(section->designation, "Circular 100mm");
}

TEST_F(MaterialLibraryServiceTest, DefaultSections_Square50mmExists) {
    EXPECT_TRUE(library.hasSection("Square 50mm"));

    auto section = library.getSection("Square 50mm");
    ASSERT_TRUE(section.has_value());

    EXPECT_DOUBLE_EQ(section->area, 0.0025);                // 50*50 mm²
    EXPECT_NEAR(section->momentOfInertia, 5.208e-7, 1e-9);  // bh³/12
    EXPECT_NEAR(section->shearArea, 0.002083, 1e-6);
    EXPECT_EQ(section->designation, "Square 50mm");
}

TEST_F(MaterialLibraryServiceTest, DefaultSections_Square100mmExists) {
    EXPECT_TRUE(library.hasSection("Square 100mm"));

    auto section = library.getSection("Square 100mm");
    ASSERT_TRUE(section.has_value());

    EXPECT_DOUBLE_EQ(section->area, 0.01);  // 100*100 mm²
    EXPECT_NEAR(section->momentOfInertia, 8.333e-6, 1e-9);
    EXPECT_NEAR(section->shearArea, 0.008333, 1e-6);
    EXPECT_EQ(section->designation, "Square 100mm");
}

TEST_F(MaterialLibraryServiceTest, DefaultSections_Rectangular30x50mmExists) {
    EXPECT_TRUE(library.hasSection("Rectangular 30x50mm"));

    auto section = library.getSection("Rectangular 30x50mm");
    ASSERT_TRUE(section.has_value());

    EXPECT_DOUBLE_EQ(section->area, 0.0015);  // 30*50 mm²
    EXPECT_NEAR(section->momentOfInertia, 3.125e-7, 1e-9);
    EXPECT_NEAR(section->shearArea, 0.00125, 1e-6);
    EXPECT_EQ(section->designation, "Rectangular 30x50mm");
}

TEST_F(MaterialLibraryServiceTest, DefaultSections_Rectangular60x100mmExists) {
    EXPECT_TRUE(library.hasSection("Rectangular 60x100mm"));

    auto section = library.getSection("Rectangular 60x100mm");
    ASSERT_TRUE(section.has_value());

    EXPECT_DOUBLE_EQ(section->area, 0.006);  // 60*100 mm²
    EXPECT_NEAR(section->momentOfInertia, 5.0e-6, 1e-9);
    EXPECT_NEAR(section->shearArea, 0.005, 1e-6);
    EXPECT_EQ(section->designation, "Rectangular 60x100mm");
}

TEST_F(MaterialLibraryServiceTest, GetAvailableSections_IncludesAllDefaults) {
    auto sections = library.getAvailableSections();

    EXPECT_GE(sections.size(), 6);  // At least 6 default sections

    std::vector<std::string> sectionNames;
    for (const auto& preset : sections) {
        sectionNames.push_back(preset.name);
    }

    EXPECT_TRUE(std::find(sectionNames.begin(), sectionNames.end(), "Circular 50mm") !=
                sectionNames.end());
    EXPECT_TRUE(std::find(sectionNames.begin(), sectionNames.end(), "Circular 100mm") !=
                sectionNames.end());
    EXPECT_TRUE(std::find(sectionNames.begin(), sectionNames.end(), "Square 50mm") !=
                sectionNames.end());
    EXPECT_TRUE(std::find(sectionNames.begin(), sectionNames.end(), "Square 100mm") !=
                sectionNames.end());
    EXPECT_TRUE(std::find(sectionNames.begin(), sectionNames.end(), "Rectangular 30x50mm") !=
                sectionNames.end());
    EXPECT_TRUE(std::find(sectionNames.begin(), sectionNames.end(), "Rectangular 60x100mm") !=
                sectionNames.end());
}

TEST_F(MaterialLibraryServiceTest, GetSection_NonExistent_ReturnsNullopt) {
    auto section = library.getSection("NonExistentSection");

    EXPECT_FALSE(section.has_value());
}

TEST_F(MaterialLibraryServiceTest, HasSection_ExistingSection_ReturnsTrue) {
    EXPECT_TRUE(library.hasSection("Circular 50mm"));
    EXPECT_TRUE(library.hasSection("Square 100mm"));
    EXPECT_TRUE(library.hasSection("Rectangular 30x50mm"));
}

TEST_F(MaterialLibraryServiceTest, HasSection_NonExistentSection_ReturnsFalse) {
    EXPECT_FALSE(library.hasSection("I-Beam W12x26"));
    EXPECT_FALSE(library.hasSection(""));
}

// ============================================================================
// CUSTOM MATERIAL TESTS
// ============================================================================

TEST_F(MaterialLibraryServiceTest, AddCustomMaterial_NewMaterial_Success) {
    MaterialProperties titanium;
    titanium.youngModulus = 110e9;
    titanium.density = 4500.0;
    titanium.yieldStrength = 880e6;
    titanium.ultimateStrength = 950e6;
    titanium.name = "Titanium";

    library.addCustomMaterial("Titanium", "Titanium alloy Ti-6Al-4V", titanium);

    EXPECT_TRUE(library.hasMaterial("Titanium"));

    auto retrieved = library.getMaterial("Titanium");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_DOUBLE_EQ(retrieved->youngModulus, 110e9);
    EXPECT_DOUBLE_EQ(retrieved->density, 4500.0);
    EXPECT_DOUBLE_EQ(retrieved->yieldStrength, 880e6);
}

TEST_F(MaterialLibraryServiceTest, AddCustomMaterial_OverwriteExisting_Success) {
    // Get original steel
    auto originalSteel = library.getMaterial("Steel");
    ASSERT_TRUE(originalSteel.has_value());

    // Overwrite with different properties
    MaterialProperties customSteel;
    customSteel.youngModulus = 210e9;  // Different from default 200e9
    customSteel.density = 7900.0;
    customSteel.yieldStrength = 350e6;
    customSteel.ultimateStrength = 500e6;
    customSteel.name = "Steel";

    library.addCustomMaterial("Steel", "High-strength steel", customSteel);

    auto updatedSteel = library.getMaterial("Steel");
    ASSERT_TRUE(updatedSteel.has_value());
    EXPECT_DOUBLE_EQ(updatedSteel->youngModulus, 210e9);  // Verify updated
    EXPECT_NE(updatedSteel->youngModulus, originalSteel->youngModulus);
}

TEST_F(MaterialLibraryServiceTest, RemoveCustomMaterial_ExistingMaterial_ReturnsTrue) {
    // Add custom material
    MaterialProperties custom;
    library.addCustomMaterial("Custom", "Test material", custom);
    EXPECT_TRUE(library.hasMaterial("Custom"));

    // Remove it
    bool removed = library.removeCustomMaterial("Custom");

    EXPECT_TRUE(removed);
    EXPECT_FALSE(library.hasMaterial("Custom"));
}

TEST_F(MaterialLibraryServiceTest, RemoveCustomMaterial_NonExistent_ReturnsFalse) {
    bool removed = library.removeCustomMaterial("NonExistent");

    EXPECT_FALSE(removed);
}

TEST_F(MaterialLibraryServiceTest, RemoveCustomMaterial_CanRemoveDefault_Success) {
    // Can remove even default materials
    EXPECT_TRUE(library.hasMaterial("Steel"));

    bool removed = library.removeCustomMaterial("Steel");

    EXPECT_TRUE(removed);
    EXPECT_FALSE(library.hasMaterial("Steel"));
}

// ============================================================================
// CUSTOM SECTION TESTS
// ============================================================================

TEST_F(MaterialLibraryServiceTest, AddCustomSection_NewSection_Success) {
    SectionProperties iBeam;
    iBeam.area = 0.0084;
    iBeam.momentOfInertia = 167e-6;
    iBeam.shearArea = 0.0042;
    iBeam.designation = "W12x26";

    library.addCustomSection("W12x26", "Wide flange I-beam", iBeam);

    EXPECT_TRUE(library.hasSection("W12x26"));

    auto retrieved = library.getSection("W12x26");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_DOUBLE_EQ(retrieved->area, 0.0084);
    EXPECT_DOUBLE_EQ(retrieved->momentOfInertia, 167e-6);
    EXPECT_EQ(retrieved->designation, "W12x26");
}

TEST_F(MaterialLibraryServiceTest, AddCustomSection_OverwriteExisting_Success) {
    auto originalCircular = library.getSection("Circular 50mm");
    ASSERT_TRUE(originalCircular.has_value());

    SectionProperties customCircular;
    customCircular.area = 0.002;  // Different from default
    customCircular.momentOfInertia = 4e-7;
    customCircular.shearArea = 0.0015;
    customCircular.designation = "Circular 50mm";

    library.addCustomSection("Circular 50mm", "Modified circular", customCircular);

    auto updated = library.getSection("Circular 50mm");
    ASSERT_TRUE(updated.has_value());
    EXPECT_DOUBLE_EQ(updated->area, 0.002);
    EXPECT_NE(updated->area, originalCircular->area);
}

TEST_F(MaterialLibraryServiceTest, RemoveCustomSection_ExistingSection_ReturnsTrue) {
    SectionProperties custom;
    library.addCustomSection("CustomSection", "Test section", custom);
    EXPECT_TRUE(library.hasSection("CustomSection"));

    bool removed = library.removeCustomSection("CustomSection");

    EXPECT_TRUE(removed);
    EXPECT_FALSE(library.hasSection("CustomSection"));
}

TEST_F(MaterialLibraryServiceTest, RemoveCustomSection_NonExistent_ReturnsFalse) {
    bool removed = library.removeCustomSection("NonExistent");

    EXPECT_FALSE(removed);
}

TEST_F(MaterialLibraryServiceTest, RemoveCustomSection_CanRemoveDefault_Success) {
    EXPECT_TRUE(library.hasSection("Square 50mm"));

    bool removed = library.removeCustomSection("Square 50mm");

    EXPECT_TRUE(removed);
    EXPECT_FALSE(library.hasSection("Square 50mm"));
}

// ============================================================================
// MATERIAL PRESET TESTS
// ============================================================================

TEST_F(MaterialLibraryServiceTest, MaterialPreset_ContainsDescription) {
    auto materials = library.getAvailableMaterials();

    for (const auto& preset : materials) {
        if (preset.name == "Steel") {
            EXPECT_FALSE(preset.description.empty());
            EXPECT_EQ(preset.description, "Structural steel (Grade 50)");
        } else if (preset.name == "Aluminum") {
            EXPECT_FALSE(preset.description.empty());
            EXPECT_EQ(preset.description, "Aluminum alloy 6061-T6");
        }
    }
}

TEST_F(MaterialLibraryServiceTest, SectionPreset_ContainsDescription) {
    auto sections = library.getAvailableSections();

    for (const auto& preset : sections) {
        if (preset.name == "Circular 50mm") {
            EXPECT_FALSE(preset.description.empty());
            EXPECT_EQ(preset.description, "Solid circular section (diameter 50mm)");
        } else if (preset.name == "Square 100mm") {
            EXPECT_FALSE(preset.description.empty());
            EXPECT_EQ(preset.description, "Solid square section (100mm × 100mm)");
        }
    }
}

// ============================================================================
// EDGE CASES AND ROBUSTNESS TESTS
// ============================================================================

TEST_F(MaterialLibraryServiceTest, EmptyStringName_MaterialOperations) {
    MaterialProperties material;
    library.addCustomMaterial("", "Empty name", material);

    EXPECT_TRUE(library.hasMaterial(""));

    auto retrieved = library.getMaterial("");
    EXPECT_TRUE(retrieved.has_value());

    bool removed = library.removeCustomMaterial("");
    EXPECT_TRUE(removed);
    EXPECT_FALSE(library.hasMaterial(""));
}

TEST_F(MaterialLibraryServiceTest, EmptyStringName_SectionOperations) {
    SectionProperties section;
    library.addCustomSection("", "Empty name", section);

    EXPECT_TRUE(library.hasSection(""));

    auto retrieved = library.getSection("");
    EXPECT_TRUE(retrieved.has_value());

    bool removed = library.removeCustomSection("");
    EXPECT_TRUE(removed);
    EXPECT_FALSE(library.hasSection(""));
}

TEST_F(MaterialLibraryServiceTest, CaseSensitivity_MaterialNames) {
    EXPECT_TRUE(library.hasMaterial("Steel"));
    EXPECT_FALSE(library.hasMaterial("steel"));  // Different case
    EXPECT_FALSE(library.hasMaterial("STEEL"));
}

TEST_F(MaterialLibraryServiceTest, CaseSensitivity_SectionNames) {
    EXPECT_TRUE(library.hasSection("Circular 50mm"));
    EXPECT_FALSE(library.hasSection("circular 50mm"));  // Different case
}

TEST_F(MaterialLibraryServiceTest, MultipleAddRemoveCycles_Stability) {
    MaterialProperties material;

    for (int i = 0; i < 100; ++i) {
        library.addCustomMaterial("Test", "Test material", material);
        EXPECT_TRUE(library.hasMaterial("Test"));

        library.removeCustomMaterial("Test");
        EXPECT_FALSE(library.hasMaterial("Test"));
    }
}

TEST_F(MaterialLibraryServiceTest, LargeNumberOfCustomMaterials_Performance) {
    // Add 100 custom materials
    for (int i = 0; i < 100; ++i) {
        MaterialProperties material;
        material.youngModulus = 200e9 + i * 1e9;

        std::string name = "Material_" + std::to_string(i);
        library.addCustomMaterial(name, "Custom material", material);
    }

    auto materials = library.getAvailableMaterials();
    EXPECT_GE(materials.size(), 100);  // At least 100 (plus defaults)

    // Verify we can retrieve them
    for (int i = 0; i < 100; ++i) {
        std::string name = "Material_" + std::to_string(i);
        EXPECT_TRUE(library.hasMaterial(name));

        auto material = library.getMaterial(name);
        ASSERT_TRUE(material.has_value());
        EXPECT_DOUBLE_EQ(material->youngModulus, 200e9 + i * 1e9);
    }
}

// ============================================================================
// INTEGRATION TEST - Real-World Usage
// ============================================================================

TEST_F(MaterialLibraryServiceTest, Integration_RealWorldUsage) {
    // Scenario: User selects steel material and circular section for truss member

    // 1. Get available materials
    auto materials = library.getAvailableMaterials();
    EXPECT_GE(materials.size(), 4);

    // 2. User selects "Steel"
    ASSERT_TRUE(library.hasMaterial("Steel"));
    auto steel = library.getMaterial("Steel");
    ASSERT_TRUE(steel.has_value());

    // 3. Get available sections
    auto sections = library.getAvailableSections();
    EXPECT_GE(sections.size(), 6);

    // 4. User selects "Circular 50mm"
    ASSERT_TRUE(library.hasSection("Circular 50mm"));
    auto circular = library.getSection("Circular 50mm");
    ASSERT_TRUE(circular.has_value());

    // 5. Verify properties are reasonable for truss analysis
    EXPECT_GT(steel->youngModulus, 0.0);
    EXPECT_GT(steel->density, 0.0);
    EXPECT_GT(circular->area, 0.0);
    EXPECT_GT(circular->momentOfInertia, 0.0);

    // 6. User adds custom high-strength steel
    MaterialProperties hsSteel;
    hsSteel.youngModulus = 200e9;
    hsSteel.density = 7850.0;
    hsSteel.yieldStrength = 350e6;  // Higher yield strength
    hsSteel.ultimateStrength = 500e6;
    hsSteel.name = "High-Strength Steel";

    library.addCustomMaterial("High-Strength Steel", "Grade 50 structural steel", hsSteel);

    // 7. Verify custom material available
    auto customMaterials = library.getAvailableMaterials();
    EXPECT_GT(customMaterials.size(), materials.size());

    ASSERT_TRUE(library.hasMaterial("High-Strength Steel"));
    auto hsRetrieved = library.getMaterial("High-Strength Steel");
    ASSERT_TRUE(hsRetrieved.has_value());
    EXPECT_DOUBLE_EQ(hsRetrieved->yieldStrength, 350e6);
}
