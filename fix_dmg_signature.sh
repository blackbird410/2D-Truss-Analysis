#!/bin/bash

# Fix DMG Code Signature Script
# This script mounts the existing DMG, fixes the code signature, and creates a new DMG

set -e

DMG_FILE="build_release/2D_Truss_Analysis-2.1.0-Darwin.dmg"
TEMP_DIR="/tmp/truss_dmg_fix"
NEW_DMG="2D_Truss_Analysis-2.1.0-Darwin-Fixed.dmg"

echo "🔧 Fixing DMG code signature issue..."

# Check if DMG exists
if [ ! -f "$DMG_FILE" ]; then
    echo "❌ Error: DMG file not found at $DMG_FILE"
    exit 1
fi

# Clean temp directory
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR"

# Mount the existing DMG
echo "📂 Mounting existing DMG..."
MOUNT_POINT=$(hdiutil attach "$DMG_FILE" -readonly -nobrowse | grep "/Volumes" | cut -f3)

if [ -z "$MOUNT_POINT" ]; then
    echo "❌ Failed to mount DMG"
    exit 1
fi

echo "✅ DMG mounted at: $MOUNT_POINT"

# Copy contents to temp directory
echo "📋 Copying DMG contents..."
cp -R "$MOUNT_POINT"/* "$TEMP_DIR/"

# Unmount the original DMG
hdiutil detach "$MOUNT_POINT"

# Fix the code signature of the app in temp directory
APP_PATH="$TEMP_DIR/bin/TrussAnalysisGUI.app"
if [ -d "$APP_PATH" ]; then
    echo "🔐 Fixing code signature..."
    codesign --force --deep --sign - "$APP_PATH"
    echo "✅ Code signature fixed"
    
    # Verify the signature
    if codesign -v "$APP_PATH" 2>/dev/null; then
        echo "✅ Signature verification passed"
    else
        echo "⚠️  Signature verification failed"
    fi
else
    echo "❌ App not found at $APP_PATH"
    exit 1
fi

# Create new DMG
echo "📦 Creating fixed DMG..."
hdiutil create -srcfolder "$TEMP_DIR" -volname "2D Truss Analysis" -fs HFS+ -format UDZO "$NEW_DMG"

# Clean up
rm -rf "$TEMP_DIR"

echo "✅ Fixed DMG created: $NEW_DMG"
echo "📊 Size: $(du -h "$NEW_DMG" | cut -f1)"

# Replace the original DMG
if [ -f "$NEW_DMG" ]; then
    mv "$NEW_DMG" "$DMG_FILE"
    echo "✅ Original DMG replaced with fixed version"
else
    echo "❌ Failed to create fixed DMG"
    exit 1
fi

echo "🎉 DMG signature fix completed successfully!"
