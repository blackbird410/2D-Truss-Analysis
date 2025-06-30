#!/bin/bash

DMG_FILE="$1"

if [ -z "$DMG_FILE" ]; then
    echo "Usage: $0 <dmg_file>"
    echo "Example: $0 build/2D_Truss_Analysis-2.1.3-Darwin.dmg"
    exit 1
fi

if [ ! -f "$DMG_FILE" ]; then
    echo "❌ DMG file not found: $DMG_FILE"
    exit 1
fi

echo "🔧 Fixing code signatures in DMG: $DMG_FILE"

# Mount the DMG
echo "📦 Mounting DMG..."
hdiutil attach "$DMG_FILE" -nobrowse -quiet

# Find the mount point
MOUNT_POINT=$(hdiutil info | grep "2D Truss Analysis" | awk '{print $1}')
VOLUME_PATH="/Volumes/2D Truss Analysis"

if [ ! -d "$VOLUME_PATH" ]; then
    echo "❌ Failed to mount DMG or find volume"
    exit 1
fi

echo "✅ DMG mounted at: $VOLUME_PATH"

# Re-sign applications in the mounted volume
echo "🔐 Re-signing applications..."

# Sign GUI app
if [ -d "$VOLUME_PATH/TrussAnalysisGUI.app" ]; then
    echo "  📱 Signing TrussAnalysisGUI.app..."
    codesign --force --deep --sign - "$VOLUME_PATH/TrussAnalysisGUI.app"
    echo "  ✅ GUI app re-signed"
else
    echo "  ⚠️  GUI app not found in DMG"
fi

# Sign CLI executable
if [ -f "$VOLUME_PATH/bin/TrussAnalysisCLI" ]; then
    echo "  💻 Signing TrussAnalysisCLI..."
    codesign --force --sign - "$VOLUME_PATH/bin/TrussAnalysisCLI"
    echo "  ✅ CLI executable re-signed"
else
    echo "  ℹ️  CLI executable not found in DMG (this is normal)"
fi

# Verify signatures
echo "🔍 Verifying signatures..."
if [ -d "$VOLUME_PATH/TrussAnalysisGUI.app" ]; then
    if codesign --verify --deep --strict "$VOLUME_PATH/TrussAnalysisGUI.app" 2>/dev/null; then
        echo "  ✅ GUI app signature verified"
    else
        echo "  ❌ GUI app signature verification failed"
    fi
fi

# Unmount the DMG
echo "📤 Unmounting DMG..."
hdiutil detach "$VOLUME_PATH" -quiet

echo "✅ DMG signature fix completed!"
echo "ℹ️  Note: Changes are temporary and only affect mounted copies."
echo "    For permanent fixes, consider using a proper Developer ID certificate."
