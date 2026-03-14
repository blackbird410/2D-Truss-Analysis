/**
 * @file test_xml_reader_branches.cpp
 * @brief Branch coverage tests for XmlTrussReader.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Covers the many parsing paths in xml_truss_reader.cpp that are not exercised
 * by the existing test_fileio.cpp tests:
 *  - metadata name attribute present / absent
 *  - materials/sections library (by-ID lookup)
 *  - inline material / section elements on members
 *  - unknown material/section ID → ParseException
 *  - material/section missing 'id' attribute → ParseException
 *  - parseSupports with type="" attribute
 *  - parseSupports with restrained="xy|y|x|" attribute (all four branches)
 *  - parseLoads referencing unknown node → ParseException
 *  - getDoubleAttribute default-value overload (crossSectionalArea vs area)
 */

#include "infrastructure/io/xml_truss_reader.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

using namespace truss::infrastructure::io;
using namespace truss::core;

namespace {

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
class XmlReaderBranchTest : public ::testing::Test {
protected:
    std::filesystem::path tempDir;
    XmlTrussReader reader;

    void SetUp() override {
        std::ostringstream oss;
        oss << "xml_branch_test-" << std::this_thread::get_id();
        tempDir = std::filesystem::temp_directory_path() / oss.str();
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        if (std::filesystem::exists(tempDir)) {
            std::filesystem::remove_all(tempDir);
        }
    }

    std::filesystem::path writeXml(const std::string& filename, const std::string& content) {
        auto path = tempDir / filename;
        std::ofstream f(path);
        f << content;
        return path;
    }

    // Minimal valid truss XML – no metadata, two nodes, one member
    static std::string minimalTruss() {
        return R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" area="0.01" youngModulus="200e9"/>
  </members>
</truss>)xml";
    }
};

}  // anonymous namespace

// ============================================================================
// Metadata
// ============================================================================

TEST_F(XmlReaderBranchTest, Metadata_NameAttributePresent_SetsName) {
    auto path = writeXml("meta_name.xml", R"xml(<?xml version="1.0"?>
<truss>
  <metadata name="My Bridge"/>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    EXPECT_EQ(dto.name, "My Bridge");
}

TEST_F(XmlReaderBranchTest, Metadata_NoNameAttribute_NameRemainsDefault) {
    // <metadata> element present but has no "name" attribute
    auto path = writeXml("meta_noname.xml", R"xml(<?xml version="1.0"?>
<truss>
  <metadata created="2025-01-01"/>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    // No "name" attribute → parseMetadata does not overwrite the default
    EXPECT_EQ(dto.name, "Untitled Truss");
}

// ============================================================================
// Materials / Sections library (by–ID lookup on members)
// ============================================================================

TEST_F(XmlReaderBranchTest, Materials_MemberReferencesByID_AppliesMaterial) {
    auto path = writeXml("mat_byid.xml", R"xml(<?xml version="1.0"?>
<truss>
  <materials>
    <material id="steel" youngModulus="200e9" density="7850" yieldStrength="250e6"/>
  </materials>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" material="steel"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_NO_THROW(reader.read(path, opts));
}

TEST_F(XmlReaderBranchTest, Materials_UnknownMaterialID_ThrowsParseException) {
    auto path = writeXml("mat_unknown.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" material="nonexistent"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

TEST_F(XmlReaderBranchTest, Materials_MissingIdAttribute_ThrowsParseException) {
    auto path = writeXml("mat_noid.xml", R"xml(<?xml version="1.0"?>
<truss>
  <materials>
    <material youngModulus="200e9"/>
  </materials>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

TEST_F(XmlReaderBranchTest, Sections_MemberReferencesByID_CrossSectionalAreaAttr) {
    // Uses "crossSectionalArea" attribute name path
    auto path = writeXml("sec_byid_csa.xml", R"xml(<?xml version="1.0"?>
<truss>
  <sections>
    <section id="s1" crossSectionalArea="0.05"/>
  </sections>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" section="s1"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    ASSERT_FALSE(dto.members.empty());
    EXPECT_DOUBLE_EQ(dto.members[0].area, 0.05);
}

TEST_F(XmlReaderBranchTest, Sections_MemberReferencesByID_AreaAttr) {
    // Uses "area" attribute name (fallback path when crossSectionalArea missing)
    auto path = writeXml("sec_byid_area.xml", R"xml(<?xml version="1.0"?>
<truss>
  <sections>
    <section id="s2" area="0.03"/>
  </sections>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" section="s2"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    ASSERT_FALSE(dto.members.empty());
    EXPECT_DOUBLE_EQ(dto.members[0].area, 0.03);
}

TEST_F(XmlReaderBranchTest, Sections_UnknownSectionID_ThrowsParseException) {
    auto path = writeXml("sec_unknown.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" section="nonexistent"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

TEST_F(XmlReaderBranchTest, Sections_MissingIdAttribute_ThrowsParseException) {
    auto path = writeXml("sec_noid.xml", R"xml(<?xml version="1.0"?>
<truss>
  <sections>
    <section area="0.01"/>
  </sections>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================================
// Inline material / section elements on <member>
// ============================================================================

TEST_F(XmlReaderBranchTest, InlineMaterial_OnMember_AppliesToMember) {
    auto path = writeXml("inline_mat.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2">
      <material youngsModulus="70e9" density="2700" yieldStrength="100e6"/>
    </member>
  </members>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    ASSERT_FALSE(dto.members.empty());
    EXPECT_DOUBLE_EQ(dto.members[0].youngModulus, 70e9);
}

TEST_F(XmlReaderBranchTest, InlineSection_OnMember_AppliesToMember) {
    auto path = writeXml("inline_sec.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2">
      <section area="0.0025"/>
    </member>
  </members>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    ASSERT_FALSE(dto.members.empty());
    EXPECT_DOUBLE_EQ(dto.members[0].area, 0.0025);
}

// ============================================================================
// parseSupports — type attribute paths
// ============================================================================

TEST_F(XmlReaderBranchTest, Supports_TypePinned_SetsSupport) {
    auto path = writeXml("sup_pinned.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <supports>
    <support nodeId="1" type="pinned"/>
  </supports>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    EXPECT_EQ(dto.nodes[0].support, SupportType::Pinned);
}

TEST_F(XmlReaderBranchTest, Supports_RestrainedXY_SetsPinned) {
    auto path = writeXml("sup_xy.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <supports>
    <support nodeId="1" restrained="xy"/>
  </supports>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    EXPECT_EQ(dto.nodes[0].support, SupportType::Pinned);
}

TEST_F(XmlReaderBranchTest, Supports_RestrainedY_SetsRollerX) {
    auto path = writeXml("sup_y.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <supports>
    <support nodeId="1" restrained="y"/>
  </supports>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    EXPECT_EQ(dto.nodes[0].support, SupportType::RollerX);
}

TEST_F(XmlReaderBranchTest, Supports_RestrainedX_SetsRollerY) {
    auto path = writeXml("sup_x.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <supports>
    <support nodeId="1" restrained="x"/>
  </supports>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    EXPECT_EQ(dto.nodes[0].support, SupportType::RollerY);
}

TEST_F(XmlReaderBranchTest, Supports_RestrainedNone_SetsFree) {
    auto path = writeXml("sup_none.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <supports>
    <support nodeId="1" restrained="z"/>
  </supports>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    EXPECT_EQ(dto.nodes[0].support, SupportType::Free);
}

TEST_F(XmlReaderBranchTest, Supports_UnknownNodeId_ThrowsParseException) {
    auto path = writeXml("sup_bad_node.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <supports>
    <support nodeId="99" type="pinned"/>
  </supports>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================================
// parseLoads — unknown node reference
// ============================================================================

TEST_F(XmlReaderBranchTest, Loads_UnknownNodeId_ThrowsParseException) {
    auto path = writeXml("load_badnode.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <loads>
    <load nodeId="99" fx="0.0" fy="-1000.0"/>
  </loads>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

TEST_F(XmlReaderBranchTest, Loads_ValidNode_AppliesForce) {
    auto path = writeXml("load_valid.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
  </nodes>
  <loads>
    <load nodeId="1" fx="500.0" fy="-2000.0"/>
  </loads>
</truss>)xml");

    FileIOOptions opts;
    auto dto = reader.read(path, opts);
    ASSERT_FALSE(dto.nodes.empty());
    EXPECT_DOUBLE_EQ(dto.nodes[0].fx, 500.0);
    EXPECT_DOUBLE_EQ(dto.nodes[0].fy, -2000.0);
}

// ============================================================================
// Root element missing
// ============================================================================

TEST_F(XmlReaderBranchTest, MissingRootTrussElement_ThrowsParseException) {
    auto path = writeXml("no_root.xml", R"xml(<?xml version="1.0"?>
<structure>
  <nodes/>
</structure>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================================
// Duplicate node IDs
// ============================================================================

TEST_F(XmlReaderBranchTest, DuplicateNodeIds_ThrowsParseException) {
    auto path = writeXml("dup_nodes.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="1" x="1.0" y="0.0"/>
  </nodes>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================================
// Member referencing invalid start/end node
// ============================================================================

TEST_F(XmlReaderBranchTest, Member_UnknownStartNodeId_ThrowsParseException) {
    auto path = writeXml("mem_bad_start.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="99" endNode="2"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

TEST_F(XmlReaderBranchTest, Member_UnknownEndNodeId_ThrowsParseException) {
    auto path = writeXml("mem_bad_end.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="99"/>
  </members>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================
// getDoubleAttribute (required) — missing attribute → line 347
// ============================================================

TEST_F(XmlReaderBranchTest, Node_MissingXAttribute_ThrowsParseException) {
    // A node element without the required 'x' attribute → getDoubleAttribute
    // (no default) throws ParseException (line 347)
    auto path = writeXml("node_no_x.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" y="0.0"/>
  </nodes>
  <members/>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================
// getIntAttribute — missing attribute → line 356
// ============================================================

TEST_F(XmlReaderBranchTest, Node_MissingIdAttribute_ThrowsParseException) {
    // A node element without the required 'id' attribute → getIntAttribute
    // throws ParseException (line 356)
    auto path = writeXml("node_no_id.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node x="0.0" y="0.0"/>
  </nodes>
  <members/>
</truss>)xml");

    FileIOOptions opts;
    EXPECT_THROW(reader.read(path, opts), ParseException);
}

// ============================================================
// getDoubleAttribute (with default) — missing attribute → line 367
// A member with an inline <material> element that omits the
// 'youngsModulus' attribute exercises the return-defaultValue path.
// ============================================================

TEST_F(XmlReaderBranchTest, InlineMaterial_MissingYoungsModulus_UsesDefault) {
    // Inline <material> without 'youngsModulus' → getDoubleAttribute returns
    // the default 210e9 (line 367); parsing should succeed
    auto path = writeXml("inline_mat_no_E.xml", R"xml(<?xml version="1.0"?>
<truss>
  <nodes>
    <node id="1" x="0.0" y="0.0"/>
    <node id="2" x="1.0" y="0.0"/>
  </nodes>
  <members>
    <member id="1" startNode="1" endNode="2" area="0.01">
      <material density="7800.0" yieldStrength="250000000.0"/>
    </member>
  </members>
</truss>)xml");

    FileIOOptions opts;
    // Should parse without error; youngModulus will be the default value
    auto dto = reader.read(path, opts);
    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_DOUBLE_EQ(dto.members[0].youngModulus, 210e9);
}
