#!/bin/bash

echo "🔍 Verifying code signing status..."
echo "=================================="

BUILD_DIR="build"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Build directory not found. Please run 'make build' first."
    exit 1
fi

# Check GUI app signing
GUI_APP="$BUILD_DIR/TrussAnalysisGUI.app"
if [ -d "$GUI_APP" ]; then
    echo "📱 TrussAnalysisGUI.app:"
    codesign -dv --verbose=4 "$GUI_APP" 2>&1 | head -10
    echo ""
    
    echo "🔐 Signature verification:"
    if codesign --verify --deep --strict --verbose=2 "$GUI_APP" 2>&1; then
        echo "✅ GUI app signature is valid"
    else
        echo "❌ GUI app signature verification failed"
    fi
    echo ""
else
    echo "❌ GUI app not found at $GUI_APP"
fi

# Check CLI executable signing
CLI_EXE="$BUILD_DIR/TrussAnalysisCLI"
if [ -f "$CLI_EXE" ]; then
    echo "💻 TrussAnalysisCLI:"
    codesign -dv --verbose=4 "$CLI_EXE" 2>&1 | head -10
    echo ""
    
    echo "🔐 Signature verification:"
    if codesign --verify --deep --strict --verbose=2 "$CLI_EXE" 2>&1; then
        echo "✅ CLI executable signature is valid"
    else
        echo "❌ CLI executable signature verification failed"
    fi
    echo ""
else
    echo "❌ CLI executable not found at $CLI_EXE"
fi

# Check Gatekeeper status
echo "🛡️  Gatekeeper Assessment:"
if [ -d "$GUI_APP" ]; then
    spctl --assess --type exec --verbose "$GUI_APP" 2>&1 || echo "ℹ️  Note: Ad-hoc signed apps will be rejected by Gatekeeper but can run locally"
fi

echo ""
echo "✅ Code signing verification complete"
