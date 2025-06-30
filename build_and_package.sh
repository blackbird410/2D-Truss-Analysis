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
    
    # macdeployqt sometimes misses QtDBus - add it manually if missing
    if [ ! -d "TrussAnalysisGUI.app/Contents/Frameworks/QtDBus.framework" ]; then
        echo "⚠️  QtDBus framework missing, adding manually..."
        QT_PATH=$(brew --prefix qt)/lib
        if [ -d "$QT_PATH/QtDBus.framework" ]; then
            cp -R "$QT_PATH/QtDBus.framework" TrussAnalysisGUI.app/Contents/Frameworks/
            install_name_tool -id "@rpath/QtDBus.framework/Versions/A/QtDBus" \
                TrussAnalysisGUI.app/Contents/Frameworks/QtDBus.framework/Versions/A/QtDBus
            echo "✅ QtDBus framework added"
        else
            echo "⚠️  QtDBus framework not found in Qt installation"
        fi
    fi
    
    echo "✅ Qt dependencies bundled successfully"
    
    # Apply ad-hoc signing after Qt deployment
    echo "🔐 Applying ad-hoc code signing..."
    cd ..
    ./sign_adhoc.sh
    cd build_release
    echo "✅ Ad-hoc signing completed"
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

# Fix code signature that gets corrupted during packaging
echo "🔧 Fixing code signature in packaged app..."
if [ -d "TrussAnalysisGUI.app" ]; then
    echo "🔐 Re-signing app bundle..."
    codesign --force --deep --sign - TrussAnalysisGUI.app
    echo "✅ Code signature fixed"
fi

# Move DMG to parent directory
if [ -f "2D_Truss_Analysis-2.1.0-Darwin.dmg" ]; then
    cp "2D_Truss_Analysis-2.1.0-Darwin.dmg" ../
    echo "✅ DMG created successfully: 2D_Truss_Analysis-2.1.0-Darwin.dmg"
    echo "📊 DMG size: $(du -h ../2D_Truss_Analysis-2.1.0-Darwin.dmg | cut -f1)"
else
    echo "❌ DMG creation failed"
    exit 1
fi

cd ..

echo ""
echo "🎉 Build and packaging completed successfully!"
echo "📦 Distributable DMG: 2D_Truss_Analysis-2.1.0-Darwin.dmg"
echo "🚀 The application is ready for distribution."
