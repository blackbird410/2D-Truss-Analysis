/**
 * @file test_json_reader_branches.cpp
 * @brief Branch-coverage tests for JsonTrussReader — exercises inline material/section,
 *        unknown references, and missing optional fields not covered elsewhere.
 * @version 3.0.0
 * @date 2026-03-08
 */

#include "infrastructure/io/fileio_factory.hpp"
#include "infrastructure/io/json_truss_reader.hpp"
#include "infrastructure/io/io_types.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

using namespace truss::infrastructure::io;
using namespace truss::core;
namespace fs = std::filesystem;

// ============================================================
// Fixture: creates a temporary directory per test
// ============================================================
class JsonReaderBranchTest : public ::testing::Test {
protected:
    fs::path testDir;

    void SetUp() override {
        testDir = fs::temp_directory_path() / ("json_branch_test_" +
                  std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
        fs::create_directories(testDir);
    }

    void TearDown() override {
        if (fs::exists(testDir)) fs::remove_all(testDir);
    }

    fs::path writeJson(const std::string& filename, const std::string& content) {
        auto path = testDir / filename;
        std::ofstream f(path);
        f << content;
        return path;
    }
};

// ============================================================
// Member auto-assigned ID (no "id" field in member JSON)
// ============================================================

TEST_F(JsonReaderBranchTest, MemberWithoutIdField_GetsAutoAssignedId) {
    // This exercises the "else { memberDTO.id = dto.members.size() + 1; }" branch
    std::string json = R"({
        "metadata": {"name": "AutoId"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"startNode": 1, "endNode": 2}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("auto_id.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_GT(dto.members[0].id, 0u);  // Non-zero auto-assigned ID
}

// ============================================================
// Inline material object — memberJson["material"].is_object()
// ============================================================

TEST_F(JsonReaderBranchTest, InlineMaterialObject_ParsedCorrectly) {
    // This exercises the `else if (memberJson["material"].is_object())` branch
    std::string json = R"({
        "metadata": {"name": "InlineMat"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {
                "id": 1,
                "startNode": 1,
                "endNode": 2,
                "material": {
                    "youngsModulus": 70e9,
                    "density": 2700.0,
                    "yieldStrength": 270e6
                }
            }
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("inline_mat.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    // Inline material uses "youngsModulus" key
    EXPECT_NEAR(dto.members[0].youngModulus, 70e9, 1e6);
    EXPECT_NEAR(dto.members[0].density, 2700.0, 0.1);
}

// ============================================================
// Inline section object — memberJson["section"].is_object()
// ============================================================

TEST_F(JsonReaderBranchTest, InlineSectionObject_ParsedCorrectly) {
    // This exercises the `else if (memberJson["section"].is_object())` branch
    std::string json = R"({
        "metadata": {"name": "InlineSec"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {
                "id": 1,
                "startNode": 1,
                "endNode": 2,
                "section": {
                    "area": 0.005
                }
            }
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("inline_sec.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_NEAR(dto.members[0].area, 0.005, 1e-6);
}

// ============================================================
// Section ref uses "area" fallback (not "crossSectionalArea")
// ============================================================

TEST_F(JsonReaderBranchTest, SectionWithAreaField_FallbackParsed) {
    // secJson.contains("crossSectionalArea") is false → falls through to secJson.contains("area")
    std::string json = R"({
        "metadata": {"name": "AreaFallback"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "sections": [
            {"id": "s1", "area": 0.003}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "section": "s1"}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("area_fallback.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_NEAR(dto.members[0].area, 0.003, 1e-6);
}

// ============================================================
// Material ref with partial fields (missing youngModulus) → uses default
// ============================================================

TEST_F(JsonReaderBranchTest, MaterialMissingYoungModulus_UsesDefault) {
    // matJson.contains("youngModulus") is false → memberDTO.youngModulus stays at default 210e9
    std::string json = R"({
        "metadata": {"name": "PartialMat"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "materials": [
            {"id": "steel", "density": 7850.0, "yieldStrength": 250e6}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "material": "steel"}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("partial_mat.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    // youngModulus not in material JSON → stays at default 210e9
    EXPECT_NEAR(dto.members[0].youngModulus, 210e9, 1e6);
    // density IS provided → should be 7850
    EXPECT_NEAR(dto.members[0].density, 7850.0, 0.1);
}

// ============================================================
// Material ref with missing density field → uses default
// ============================================================

TEST_F(JsonReaderBranchTest, MaterialMissingDensity_UsesDefault) {
    std::string json = R"({
        "metadata": {"name": "NoDensity"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "materials": [
            {"id": "alum", "youngModulus": 70e9, "yieldStrength": 270e6}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "material": "alum"}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("no_density.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_NEAR(dto.members[0].youngModulus, 70e9, 1e6);
    // density missing → default 7850
    EXPECT_NEAR(dto.members[0].density, 7850.0, 0.1);
}

// ============================================================
// Material ref with missing yieldStrength field → uses default
// ============================================================

TEST_F(JsonReaderBranchTest, MaterialMissingYieldStrength_UsesDefault) {
    std::string json = R"({
        "metadata": {"name": "NoYield"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "materials": [
            {"id": "mat1", "youngModulus": 200e9, "density": 7850.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "material": "mat1"}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("no_yield.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    // yieldStrength missing → default 250e6
    EXPECT_NEAR(dto.members[0].yieldStrength, 250e6, 1e3);
}

// ============================================================
// Unknown material ID → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, UnknownMaterialId_ThrowsParseException) {
    // exercices: throw ParseException("Member references unknown material ID: " + materialId)
    std::string json = R"({
        "metadata": {"name": "UnknownMat"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "materials": [
            {"id": "realMat", "youngModulus": 200e9}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "material": "nonExistentMat"}
        ],
        "supports": [],
        "loads": []
    })";

    auto path = writeJson("unknown_mat.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Unknown section ID → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, UnknownSectionId_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "UnknownSec"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "sections": [
            {"id": "realSec", "crossSectionalArea": 0.001}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "section": "nonExistentSec"}
        ],
        "supports": [],
        "loads": []
    })";

    auto path = writeJson("unknown_sec.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Material missing "id" field → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, MaterialMissingIdField_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "NoMatId"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "materials": [
            {"youngModulus": 200e9}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "supports": [],
        "loads": []
    })";

    auto path = writeJson("no_mat_id.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Section missing "id" field → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, SectionMissingIdField_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "NoSecId"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "sections": [
            {"crossSectionalArea": 0.001}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "supports": [],
        "loads": []
    })";

    auto path = writeJson("no_sec_id.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Members is not a JSON array → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, MembersNotArray_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "BadMembers"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": "not an array"
    })";

    auto path = writeJson("bad_members.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Materials not a JSON array → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, MaterialsNotArray_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "BadMaterials"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "nodes_count": 2,
        "materials": {"id": "steel"},
        "members": []
    })";

    auto path = writeJson("bad_materials.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Member missing startNode/endNode → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, MemberMissingRequiredFields_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "BadMember"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1}
        ]
    })";

    auto path = writeJson("bad_member_fields.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// JSON without supports section — supports block absent
// ============================================================

TEST_F(JsonReaderBranchTest, JsonWithoutSupportsSection_Succeeds) {
    // j.contains("supports") → false → no support parsing called
    std::string json = R"({
        "metadata": {"name": "NoSupports"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "loads": []
    })";

    auto path = writeJson("no_supports.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    EXPECT_EQ(dto.nodes.size(), 2u);
    EXPECT_EQ(dto.members.size(), 1u);
}

// ============================================================
// JSON without loads section — loads block absent
// ============================================================

TEST_F(JsonReaderBranchTest, JsonWithoutLoadsSection_Succeeds) {
    // j.contains("loads") → false → no load parsing called
    std::string json = R"({
        "metadata": {"name": "NoLoads"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ]
    })";

    auto path = writeJson("no_loads.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    EXPECT_EQ(dto.nodes.size(), 2u);
    // All nodes should have zero forces (no loads parsed)
    for (const auto& n : dto.nodes) {
        EXPECT_DOUBLE_EQ(n.fx, 0.0);
        EXPECT_DOUBLE_EQ(n.fy, 0.0);
    }
}

// ============================================================
// JSON without metadata section — metadata absent
// ============================================================

TEST_F(JsonReaderBranchTest, JsonWithoutMetadata_Succeeds) {
    // j.contains("metadata") → false → name stays default
    std::string json = R"({
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ]
    })";

    auto path = writeJson("no_metadata.json", json);
    JsonTrussReader reader;
    // Should succeed — metadata is optional
    EXPECT_NO_THROW({
        auto dto = reader.read(path);
        EXPECT_EQ(dto.nodes.size(), 2u);
    });
}

// ============================================================
// FileIOFactory: createWriter(filepath) for JSON extension
// ============================================================

TEST_F(JsonReaderBranchTest, FactoryCreateWriterFromJsonPath_ReturnsWriter) {
    // Tests the createWriter(filepath) overload — exercises the factory writer-by-path branch
    auto path = testDir / "output.json";
    // Just check the factory creates a writer without throwing
    EXPECT_NO_THROW({
        auto writer = truss::infrastructure::io::FileIOFactory::createWriter(path);
        EXPECT_NE(writer, nullptr);
    });
}

// ============================================================
// FileIOFactory: createWriter(filepath) for XML extension
// ============================================================

TEST_F(JsonReaderBranchTest, FactoryCreateWriterFromXmlPath_ReturnsWriter) {
    auto path = testDir / "output.xml";
    EXPECT_NO_THROW({
        auto writer = truss::infrastructure::io::FileIOFactory::createWriter(path);
        EXPECT_NE(writer, nullptr);
    });
}

// ============================================================
// FileIOFactory: createWriter with unknown extension → throws
// ============================================================

TEST_F(JsonReaderBranchTest, FactoryCreateWriterFromUnknownExtension_Throws) {
    auto path = testDir / "output.xyz";
    EXPECT_THROW(truss::infrastructure::io::FileIOFactory::createWriter(path), std::invalid_argument);
}

// ============================================================
// Sections not a JSON array → throws ParseException
// ============================================================

TEST_F(JsonReaderBranchTest, SectionsNotArray_ThrowsParseException) {
    std::string json = R"({
        "metadata": {"name": "BadSections"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "sections": {"id": "s1", "crossSectionalArea": 0.001},
        "members": []
    })";

    auto path = writeJson("bad_sections.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Section referenced by string ID with "crossSectionalArea" key
// Covers: if (secJson.contains("crossSectionalArea")) → true branch
// ============================================================

TEST_F(JsonReaderBranchTest, SectionWithCrossSectionalAreaKey_ParsedCorrectly) {
    std::string json = R"({
        "metadata": {"name": "CSA"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "sections": [
            {"id": "s1", "crossSectionalArea": 0.007}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2, "section": "s1"}
        ],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("csa_section.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_NEAR(dto.members[0].area, 0.007, 1e-6);
}

// ============================================================
// Restrained array with only "x" → RollerY
// Covers: else if (xRestrained && !yRestrained) branch
// ============================================================

TEST_F(JsonReaderBranchTest, Supports_RestrainedXOnly_SetsRollerY) {
    std::string json = R"({
        "metadata": {"name": "RollerY"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "supports": [
            {"nodeId": 1, "restrained": ["x"]},
            {"nodeId": 2, "type": "pinned"}
        ],
        "loads": []
    })";

    auto path = writeJson("roller_y.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.nodes.size(), 2u);
    // Node 1 restrained only in x → RollerY
    EXPECT_EQ(dto.nodes[0].support, SupportType::RollerY);
}

// ============================================================
// Restrained array with unknown direction → Free
// Covers: else branch (neither x nor y restrained)
// ============================================================

TEST_F(JsonReaderBranchTest, Supports_RestrainedUnknownDirection_SetsFree) {
    std::string json = R"({
        "metadata": {"name": "FreeFallback"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "supports": [
            {"nodeId": 1, "restrained": ["z"]},
            {"nodeId": 2, "type": "pinned"}
        ],
        "loads": []
    })";

    auto path = writeJson("free_fallback.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.nodes.size(), 2u);
    // "z" is not x or y, so none restrained → Free
    EXPECT_EQ(dto.nodes[0].support, SupportType::Free);
}

// ============================================================
// Restrained array with uppercase "X" and "Y"
// Covers: else if (restraint == "X") and else if (restraint == "Y")
// ============================================================

TEST_F(JsonReaderBranchTest, Supports_RestrainedUppercase_SetsPinned) {
    std::string json = R"({
        "metadata": {"name": "UppercaseXY"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [
            {"id": 1, "startNode": 1, "endNode": 2}
        ],
        "supports": [
            {"nodeId": 1, "restrained": ["X", "Y"]},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": []
    })";

    auto path = writeJson("uppercase_xy.json", json);
    JsonTrussReader reader;
    auto dto = reader.read(path);

    ASSERT_EQ(dto.nodes.size(), 2u);
    // X+Y → Pinned
    EXPECT_EQ(dto.nodes[0].support, SupportType::Pinned);
}
// ============================================================
// File-not-found → throws FileReadException (line 33)
// ============================================================

TEST_F(JsonReaderBranchTest, MissingFile_ThrowsFileNotFoundException) {
    // A path that does not exist on disk triggers the !exists() branch (line 26)
    fs::path nonExistent = testDir / "does_not_exist.json";
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(nonExistent), FileNotFoundException);
}

// ============================================================
// 'supports' must be an array → line 280
// ============================================================

TEST_F(JsonReaderBranchTest, SupportsNotArray_ThrowsParseException) {
    // 'supports' is an object instead of an array → ParseException
    std::string json = R"({
        "metadata": {"name": "BadSupports"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [],
        "supports": {"nodeId": 1, "type": "pinned"},
        "loads": []
    })";

    auto path = writeJson("supports_not_array.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Support entry missing 'nodeId' field → line 285
// ============================================================

TEST_F(JsonReaderBranchTest, SupportMissingNodeId_ThrowsParseException) {
    // A support entry without the required 'nodeId' field → ParseException
    std::string json = R"({
        "metadata": {"name": "BadSupportEntry"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [],
        "supports": [
            {"type": "pinned"}
        ],
        "loads": []
    })";

    auto path = writeJson("support_missing_nodeid.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Support references unknown node ID → line 292
// ============================================================

TEST_F(JsonReaderBranchTest, SupportUnknownNodeId_ThrowsParseException) {
    // A support entry referencing a nodeId that does not exist → ParseException
    std::string json = R"({
        "metadata": {"name": "BadSupportRef"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [],
        "supports": [
            {"nodeId": 999, "type": "pinned"}
        ],
        "loads": []
    })";

    auto path = writeJson("support_unknown_node.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}

// ============================================================
// Load entry missing 'nodeId' field → line 349
// ============================================================

TEST_F(JsonReaderBranchTest, LoadMissingNodeId_ThrowsParseException) {
    // A load entry without the required 'nodeId' field → ParseException
    std::string json = R"({
        "metadata": {"name": "BadLoadEntry"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 4.0, "y": 0.0}
        ],
        "members": [],
        "supports": [
            {"nodeId": 1, "type": "pinned"},
            {"nodeId": 2, "type": "roller_x"}
        ],
        "loads": [
            {"fx": 1000.0, "fy": 0.0}
        ]
    })";

    auto path = writeJson("load_missing_nodeid.json", json);
    JsonTrussReader reader;
    EXPECT_THROW(reader.read(path), ParseException);
}