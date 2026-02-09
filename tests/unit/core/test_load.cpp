/**
 * @file test_load.cpp
 * @brief Google Test unit tests for Load class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include "../../src/core/model/Load.hpp"
#include "../../src/core/model/Types.hpp"

using namespace truss::core;

// Test Suite: LoadTest
// Tests for Load class creation and behavior

TEST(LoadTest, BasicCreation) {
    Load load(1, 5, 100.0, -200.0, "Test Load");
    
    EXPECT_EQ(load.getId(), 1);
    EXPECT_EQ(load.getNodeId(), 5);
    EXPECT_NEAR(load.getFx(), 100.0, 1e-10);
    EXPECT_NEAR(load.getFy(), -200.0, 1e-10);
    EXPECT_EQ(load.getLabel(), "Test Load");
    EXPECT_EQ(load.getType(), LoadType::NodalForce);
}

TEST(LoadTest, CreationWithForce2D) {
    Force2D force(50.0, -75.0);
    Load load(2, 10, force);
    
    EXPECT_EQ(load.getId(), 2);
    EXPECT_EQ(load.getNodeId(), 10);
    EXPECT_NEAR(load.getFx(), 50.0, 1e-10);
    EXPECT_NEAR(load.getFy(), -75.0, 1e-10);
    EXPECT_EQ(load.getLabel(), "Load_2"); // Auto-generated label
}

TEST(LoadTest, MagnitudeCalculation) {
    Load load(1, 1, 3.0, 4.0);
    
    EXPECT_NEAR(load.getMagnitude(), 5.0, 1e-10);
    
    Load horizontalLoad(2, 1, 100.0, 0.0);
    EXPECT_NEAR(horizontalLoad.getMagnitude(), 100.0, 1e-10);
    
    Load verticalLoad(3, 1, 0.0, -50.0);
    EXPECT_NEAR(verticalLoad.getMagnitude(), 50.0, 1e-10);
}

TEST(LoadTest, ZeroForceDetection) {
    Load zeroLoad(1, 1, 0.0, 0.0);
    EXPECT_TRUE(zeroLoad.isZero());
    
    // Use a force larger than default tolerance (1e-6)
    Load nonZeroLoad(2, 1, 1e-5, 0.0);
    EXPECT_FALSE(nonZeroLoad.isZero());
    
    Load nearZeroLoad(3, 1, 1e-15, 1e-15);
    EXPECT_TRUE(nearZeroLoad.isZero(1e-12));
}

TEST(LoadTest, DirectionChecks) {
    Load horizontal(1, 1, 100.0, 0.0);
    EXPECT_TRUE(horizontal.isHorizontal());
    EXPECT_FALSE(horizontal.isVertical());
    
    Load vertical(2, 1, 0.0, -50.0);
    EXPECT_FALSE(vertical.isHorizontal());
    EXPECT_TRUE(vertical.isVertical());
    
    Load diagonal(3, 1, 100.0, -50.0);
    EXPECT_FALSE(diagonal.isHorizontal());
    EXPECT_FALSE(diagonal.isVertical());
}

TEST(LoadTest, NodeApplicationCheck) {
    Load load(1, 5, 100.0, -200.0);
    
    EXPECT_TRUE(load.isAppliedAt(5));
    EXPECT_FALSE(load.isAppliedAt(1));
    EXPECT_FALSE(load.isAppliedAt(10));
}

TEST(LoadTest, ForceModification) {
    Load load(1, 1, 100.0, 200.0);
    
    load.setForce(50.0, -75.0);
    EXPECT_NEAR(load.getFx(), 50.0, 1e-10);
    EXPECT_NEAR(load.getFy(), -75.0, 1e-10);
    
    load.setForce(Force2D(25.0, 30.0));
    EXPECT_NEAR(load.getFx(), 25.0, 1e-10);
    EXPECT_NEAR(load.getFy(), 30.0, 1e-10);
}

TEST(LoadTest, LabelManagement) {
    Load load(1, 1, 0.0, 0.0);
    EXPECT_EQ(load.getLabel(), "Load_1");
    
    load.setLabel("Vertical Load");
    EXPECT_EQ(load.getLabel(), "Vertical Load");
}

TEST(LoadTest, EqualityOperators) {
    Load load1(1, 5, 100.0, -200.0);
    Load load2(1, 5, 100.0, -200.0);
    Load load3(2, 5, 100.0, -200.0);
    Load load4(1, 6, 100.0, -200.0);
    Load load5(1, 5, 100.0, -201.0);
    
    EXPECT_EQ(load1, load2);
    EXPECT_NE(load1, load3); // Different ID
    EXPECT_NE(load1, load4); // Different node
    EXPECT_NE(load1, load5); // Different force
}

TEST(LoadTest, CopyAndMove) {
    Load original(1, 5, 100.0, -200.0, "Original");
    
    // Copy constructor
    Load copy(original);
    EXPECT_EQ(copy.getId(), original.getId());
    EXPECT_EQ(copy.getNodeId(), original.getNodeId());
    EXPECT_NEAR(copy.getFx(), original.getFx(), 1e-10);
    EXPECT_NEAR(copy.getFy(), original.getFy(), 1e-10);
    EXPECT_EQ(copy.getLabel(), original.getLabel());
    
    // Copy assignment
    Load assigned(2, 1, 0.0, 0.0);
    assigned = original;
    EXPECT_EQ(assigned, original);
    
    // Move constructor
    Load moved(std::move(copy));
    EXPECT_EQ(moved, original);
    
    // Move assignment
    Load moveAssigned(3, 1, 0.0, 0.0);
    moveAssigned = std::move(moved);
    EXPECT_EQ(moveAssigned, original);
}
