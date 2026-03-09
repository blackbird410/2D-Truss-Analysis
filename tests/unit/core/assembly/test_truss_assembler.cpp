/**
 * @file test_truss_assembler.cpp
 * @brief Branch-coverage tests for TrussAssembler.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Test Coverage Targets:
 * - assembleTruss() happy path with nodes that have forces (fx!=0||fy!=0 branch)
 * - assembleTruss() happy path where nodes have zero force (else branch)
 * - assembleTruss() throws on invalid start-node ID
 * - assembleTruss() throws on invalid end-node ID
 * - assembleTruss() assembles name and node support types
 * - assembleTruss() empty DTO
 * - createDTO() round-trips node and member data
 *
 * Note: TrussAssembler has a deleted constructor → all methods called
 *       as TrussAssembler::methodName(args).
 */

#include "../../../../src/core/assembly/truss_assembler.hpp"
#include "../../../../src/core/model/truss.hpp"

#include <gtest/gtest.h>

using namespace truss::core::assembly;
using namespace truss::core;
using namespace truss::core::interfaces;

// ─────────────────────────────────────────────────────────────────────────────
// Helper: construct a minimal valid TrussDTO
// ─────────────────────────────────────────────────────────────────────────────

static NodeDTO makeNodeDTO(NodeId id, double x, double y,
                           SupportType support = SupportType::Free,
                           double fx = 0.0, double fy = 0.0) {
    return NodeDTO(id, x, y, support, fx, fy);
}

static MemberDTO makeMemberDTO(MemberId id, NodeId start, NodeId end,
                                double E = 200e9, double yield = 250e6,
                                double density = 7850.0, double area = 1e-3) {
    return MemberDTO(id, start, end, "M" + std::to_string(id), E, yield, density, area);
}

// ─────────────────────────────────────────────────────────────────────────────
// assembleTruss – happy path
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussAssemblerTest, AssemblesNameCorrectly) {
    TrussDTO dto("MyTruss");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0, SupportType::Pinned));
    dto.nodes.push_back(makeNodeDTO(2, 4.0, 0.0, SupportType::RollerY));
    dto.members.push_back(makeMemberDTO(1, 1, 2));

    auto truss = TrussAssembler::assembleTruss(dto);

    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getName(), "MyTruss");
    EXPECT_EQ(truss->getNodeCount(), 2);
    EXPECT_EQ(truss->getMemberCount(), 1);
}

TEST(TrussAssemblerTest, AssemblesNodeSupportTypes) {
    TrussDTO dto("SupportTest");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0, SupportType::Pinned));
    dto.nodes.push_back(makeNodeDTO(2, 1.0, 0.0, SupportType::RollerX));
    dto.nodes.push_back(makeNodeDTO(3, 2.0, 0.0, SupportType::Free));

    auto truss = TrussAssembler::assembleTruss(dto);

    // Support types are preserved
    auto constrained = truss->getConstrainedNodes();
    EXPECT_EQ(constrained.size(), 2u);  // Pinned + RollerX are constrained

    auto freeNodes = truss->getFreeNodes();
    EXPECT_EQ(freeNodes.size(), 1u);
}

TEST(TrussAssemblerTest, AssemblesNodeWithAppliedForce) {
    // The fx != 0.0 || fy != 0.0 branch in assembleTruss
    TrussDTO dto("ForceTest");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0, SupportType::Pinned, 0.0, 0.0));
    dto.nodes.push_back(makeNodeDTO(2, 0.0, 3.0, SupportType::Free, 500.0, -1000.0));
    dto.members.push_back(makeMemberDTO(1, 1, 2));

    auto truss = TrussAssembler::assembleTruss(dto);

    ASSERT_NE(truss, nullptr);
    // The loaded node should be node with id matching nodeDTO.id=2
    auto loadedNodes = truss->getLoadedNodes();
    EXPECT_EQ(loadedNodes.size(), 1u);
    EXPECT_NEAR(loadedNodes[0]->getAppliedForce().fx, 500.0, 1e-10);
    EXPECT_NEAR(loadedNodes[0]->getAppliedForce().fy, -1000.0, 1e-10);
}

TEST(TrussAssemblerTest, AssemblesNodeWithZeroForceAppliesNothing) {
    // The fx == 0.0 && fy == 0.0 branch → setAppliedForce NOT called
    TrussDTO dto("NoForce");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0, SupportType::Pinned, 0.0, 0.0));
    dto.nodes.push_back(makeNodeDTO(2, 1.0, 0.0, SupportType::RollerY, 0.0, 0.0));
    dto.members.push_back(makeMemberDTO(1, 1, 2));

    auto truss = TrussAssembler::assembleTruss(dto);

    EXPECT_FALSE(truss->hasAppliedForces());
}

TEST(TrussAssemblerTest, AssemblesMemberMaterialAndSection) {
    TrussDTO dto("MaterialTest");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0));
    dto.nodes.push_back(makeNodeDTO(2, 3.0, 4.0));
    dto.members.push_back(makeMemberDTO(1, 1, 2, 150e9, 200e6, 8000.0, 5e-4));

    auto truss = TrussAssembler::assembleTruss(dto);

    auto member = truss->getMember(1);
    ASSERT_NE(member, nullptr);
    EXPECT_NEAR(member->getMaterial().youngModulus, 150e9, 1.0);
    EXPECT_NEAR(member->getSection().area, 5e-4, 1e-10);
}

// ─────────────────────────────────────────────────────────────────────────────
// assembleTruss – throwing branches
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussAssemblerTest, AssemblyThrowsOnInvalidStartNodeId) {
    TrussDTO dto("BadStart");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0));
    dto.nodes.push_back(makeNodeDTO(2, 1.0, 0.0));
    // Member references non-existent start node 999
    dto.members.push_back(makeMemberDTO(1, 999, 2));

    EXPECT_THROW(TrussAssembler::assembleTruss(dto), std::invalid_argument);
}

TEST(TrussAssemblerTest, AssemblyThrowsOnInvalidEndNodeId) {
    TrussDTO dto("BadEnd");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0));
    dto.nodes.push_back(makeNodeDTO(2, 1.0, 0.0));
    // Member references non-existent end node 999
    dto.members.push_back(makeMemberDTO(1, 1, 999));

    EXPECT_THROW(TrussAssembler::assembleTruss(dto), std::invalid_argument);
}

TEST(TrussAssemblerTest, AssemblyThrowsErrorMessageMentionsMemberId) {
    TrussDTO dto("BadMember");
    dto.nodes.push_back(makeNodeDTO(1, 0.0, 0.0));
    dto.members.push_back(makeMemberDTO(42, 1, 999));  // member id=42

    try {
        TrussAssembler::assembleTruss(dto);
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        std::string msg(e.what());
        EXPECT_TRUE(msg.find("42") != std::string::npos ||
                    msg.find("999") != std::string::npos)
            << "Error message: " << msg;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// assembleTruss – empty DTO
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussAssemblerTest, AssemblesEmptyDtoCreatesEmptyTruss) {
    TrussDTO dto("Empty");
    auto truss = TrussAssembler::assembleTruss(dto);

    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getNodeCount(), 0);
    EXPECT_EQ(truss->getMemberCount(), 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// createDTO – round-trip
// ─────────────────────────────────────────────────────────────────────────────

TEST(TrussAssemblerTest, CreateDTORoundTripsNodeData) {
    // Build a truss object then convert it to DTO
    Truss truss("RoundTrip");
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto n3 = truss.addNode(2.0, 3.0, SupportType::Free);
    truss.applyForce(3, Force2D(0.0, -1000.0));

    MaterialProperties mat;
    mat.youngModulus  = 200e9;
    mat.yieldStrength = 250e6;
    mat.density       = 7850.0;
    mat.name          = "Steel";

    SectionProperties sec;
    sec.area        = 1e-3;
    sec.designation = "S1";

    truss.addMember(n1, n2, mat, sec);
    truss.addMember(n1, n3, mat, sec);

    auto dto = TrussAssembler::createDTO(truss);

    EXPECT_EQ(dto.name, "RoundTrip");
    EXPECT_EQ(dto.nodes.size(), 3u);
    EXPECT_EQ(dto.members.size(), 2u);

    // Verify node data preserved
    EXPECT_EQ(dto.nodes[0].support, SupportType::Pinned);
    EXPECT_NEAR(dto.nodes[2].fy, -1000.0, 1e-10);
}

TEST(TrussAssemblerTest, CreateDTORoundTripsMemberData) {
    Truss truss("MemberDTO");
    auto n1 = truss.addNode(0.0, 0.0);
    auto n2 = truss.addNode(3.0, 4.0);  // length=5

    MaterialProperties mat;
    mat.youngModulus  = 70e9;
    mat.yieldStrength = 110e6;
    mat.density       = 2700.0;
    mat.name          = "Aluminium";

    SectionProperties sec;
    sec.area        = 2e-3;
    sec.designation = "A2";

    truss.addMember(n1, n2, mat, sec);

    auto dto = TrussAssembler::createDTO(truss);

    ASSERT_EQ(dto.members.size(), 1u);
    EXPECT_NEAR(dto.members[0].youngModulus, 70e9, 1.0);
    EXPECT_NEAR(dto.members[0].area, 2e-3, 1e-10);
    EXPECT_EQ(dto.members[0].startNodeId, n1->getId());
    EXPECT_EQ(dto.members[0].endNodeId, n2->getId());
}
