# 2D Truss Analysis - Release Notes v2.1.3

**Release Date:** June 30, 2025  
**Status:** Stable Production Release  
**Package:** 2D_Truss_Analysis-2.1.1-STABLE.dmg (22MB)

## 🎯 Major Improvements and Critical Fixes

### 🔧 Critical Bug Fixes

#### Support Type Selection System - FIXED
- **Issue**: Major bug where support type selections in the property panel didn't map correctly to actual structural constraints
- **Impact**: Analysis failures due to incorrect boundary conditions, making the application unusable for structural analysis
- **Solution**: Implemented proper mapping between UI combo box indices and SupportType enum values
- **Result**: Support types are now correctly applied and structural analysis works properly

#### Support Icon Visibility - FIXED
- **Issue**: Support symbols (pinned/roller) not visible in the structural diagram
- **Solution**: Fixed drawing order to render supports on top of nodes, improved symbol positioning and styling
- **Result**: Clear green triangles for pinned supports, green circles for roller supports with hatching patterns

#### Load Arrow Direction - FIXED
- **Issue**: Y-axis load arrows pointing in wrong direction (downward instead of upward for positive forces)
- **Solution**: Corrected screen coordinate mapping and angle calculations
- **Result**: Load arrows now correctly point upward for positive Y forces and downward for negative Y forces

#### Reactions Display - FIXED
- **Issue**: Support reaction forces not displaying in the deformed structure view
- **Solution**: Fixed API consistency for reaction data access
- **Result**: Magenta reaction arrows properly display when "Reactions" checkbox is enabled

### 🎨 User Experience Improvements

#### Node Size Optimization
- Reduced node radius from 6px to 4px for better visual clarity
- Less visual clutter and improved interaction precision
- Better proportions relative to support symbols

#### Enhanced Deformed Truss Visualization
- Professional dark-themed control panel with improved contrast
- Better visibility and usability of visualization controls
- Optimized layout and sizing for dropdown controls
- Semi-transparent background for better contrast

#### Support Symbol Styling
- Added professional hatching patterns for ground symbols
- Increased pen thickness for better visibility
- Consistent green color scheme for all support types
- Proper offset calculations to prevent overlap

## 🚀 Technical Improvements

### Build System
- Clean rebuild workflow with all fixes integrated
- Qt dependency bundling completed successfully
- Ad-hoc code signing for local development
- Comprehensive testing and validation

### Code Quality
- Fixed unused parameter warnings in Member.cpp
- Improved API consistency across components
- Enhanced error handling and validation
- Better separation of concerns in drawing logic

## 📋 What's Fixed

### Before This Release
- ❌ Support type selection didn't work correctly
- ❌ Support icons were invisible or incorrectly displayed
- ❌ Load arrows pointed in wrong directions
- ❌ Reaction forces didn't show in results
- ❌ Analysis failed due to incorrect boundary conditions
- ❌ User interface had visibility and usability issues

### After This Release
- ✅ Support type selection works correctly via property panel
- ✅ Support icons clearly visible with proper symbols and styling
- ✅ Load arrows point in correct directions matching force vectors
- ✅ Reaction forces display properly in deformed structure view
- ✅ Structural analysis runs successfully without stability errors
- ✅ Professional UI with improved visibility and usability

## 🔍 Testing Status

- ✅ Build system: All builds complete successfully
- ✅ Application launch: Launches without errors
- ✅ Support selection: Property panel correctly applies support types
- ✅ Visual feedback: Support icons properly display in diagram
- ✅ Analysis engine: Structural analysis completes successfully
- ✅ Results display: All visualization features working correctly

## 💻 Installation

### macOS
1. Download `2D_Truss_Analysis-2.1.1-STABLE.dmg`
2. Mount the DMG file
3. Drag "2D Truss Analysis" to Applications folder
4. Launch from Applications or Launchpad

### System Requirements
- macOS 10.15 or later
- 100 MB available disk space
- OpenGL support for visualization

## 🎯 Known Issues
- None for core functionality
- Minor cosmetic improvements in progress for future releases

## 🔮 Next Steps
This release provides a stable, fully functional structural analysis tool. Future updates will focus on:
- Additional analysis features
- Performance optimizations
- Enhanced visualization options
- Extended material and section libraries

---

**Application Status:** ✅ READY FOR PRODUCTION USE

This release resolves all critical issues and provides a reliable tool for 2D truss structural analysis.
