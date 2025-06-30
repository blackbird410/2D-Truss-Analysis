#!/bin/bash

# 2D Truss Analysis - Build and Package Script
# This script builds the application and creates a distributable DMG

set -e  # Exit on any error

echo "🏗️  Building 2D Truss Analysis..."

# Create clean build directory
echo "📁 Setting up build directory..."
rm -rf build_release
mkdir build_release
cd build_release

# Configure with CMake
echo "🔧 Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build the project
echo "🔨 Building project..."
make -j$(sysctl -n hw.ncpu)

# Deploy Qt dependencies
echo "📦 Bundling Qt dependencies..."
if command -v macdeployqt &> /dev/null; then
    macdeployqt TrussAnalysisGUI.app
    echo "✅ Qt dependencies bundled successfully"
else
    echo "⚠️  macdeployqt not found - the app may not work on other systems"
fi

# Test the app
echo "🧪 Testing the application..."
if ./TrussAnalysisGUI.app/Contents/MacOS/TrussAnalysisGUI --version &> /dev/null; then
    echo "✅ Application test passed"
else
    echo "⚠️  Application test failed or version check not supported"
fi

# Create DMG package
echo "📦 Creating DMG package..."
make package

# Move DMG to parent directory
if [ -f "2D_Truss_Analysis-2.0.0-Darwin.dmg" ]; then
    cp "2D_Truss_Analysis-2.0.0-Darwin.dmg" ../
    echo "✅ DMG created successfully: 2D_Truss_Analysis-2.0.0-Darwin.dmg"
    echo "📊 DMG size: $(du -h ../2D_Truss_Analysis-2.0.0-Darwin.dmg | cut -f1)"
else
    echo "❌ DMG creation failed"
    exit 1
fi

cd ..

echo ""
echo "🎉 Build and packaging completed successfully!"
echo "📦 Distributable DMG: 2D_Truss_Analysis-2.0.0-Darwin.dmg"
echo "🚀 The application is ready for distribution."
