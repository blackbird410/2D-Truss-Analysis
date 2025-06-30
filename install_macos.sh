#!/bin/bash

# 2D Truss Analysis - macOS Installation Script
# Version 2.1.0

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script configuration
DMG_FILE="2D_Truss_Analysis-2.1.3-Darwin.dmg"
APP_NAME="TrussAnalysisGUI.app"
INSTALL_DIR="/Applications"

echo -e "${BLUE}==================================================${NC}"
echo -e "${BLUE}     2D Truss Analysis - macOS Installer         ${NC}"
echo -e "${BLUE}                Version 2.1.0                    ${NC}"
echo -e "${BLUE}==================================================${NC}"
echo

# Check if DMG exists
if [ ! -f "$DMG_FILE" ]; then
    echo -e "${RED}❌ Error: DMG file not found at $DMG_FILE${NC}"
    echo -e "${YELLOW}💡 Please build the project first or locate the DMG file${NC}"
    exit 1
fi

echo -e "${BLUE}📦 Found DMG file: $DMG_FILE${NC}"
DMG_SIZE=$(du -sh "$DMG_FILE" | cut -f1)
echo -e "${BLUE}📏 Size: $DMG_SIZE${NC}"
echo

# Check if running as user (not root)
if [ "$EUID" -eq 0 ]; then
    echo -e "${YELLOW}⚠️  Warning: Running as root. This may cause permission issues.${NC}"
    echo -e "${YELLOW}   Consider running as a regular user.${NC}"
    echo
fi

# Mount the DMG
echo -e "${BLUE}🔧 Mounting DMG file...${NC}"
MOUNT_POINT=$(hdiutil attach "$DMG_FILE" -readonly -nobrowse | grep "/Volumes" | cut -f3)

if [ -z "$MOUNT_POINT" ]; then
    echo -e "${RED}❌ Failed to mount DMG file${NC}"
    exit 1
fi

echo -e "${GREEN}✅ DMG mounted at: $MOUNT_POINT${NC}"

# Function to cleanup on exit
cleanup() {
    echo -e "${BLUE}🧹 Cleaning up...${NC}"
    if [ -n "$MOUNT_POINT" ] && [ -d "$MOUNT_POINT" ]; then
        hdiutil detach "$MOUNT_POINT" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

# Check if app exists in DMG
APP_SOURCE="$MOUNT_POINT/bin/$APP_NAME"
if [ ! -d "$APP_SOURCE" ]; then
    echo -e "${RED}❌ Application not found in DMG: $APP_SOURCE${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Found application in DMG${NC}"

# Check destination
APP_DEST="$INSTALL_DIR/$APP_NAME"
if [ -d "$APP_DEST" ]; then
    echo -e "${YELLOW}⚠️  Application already exists at $APP_DEST${NC}"
    read -p "Do you want to replace it? (y/N): " -r
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${BLUE}ℹ️  Installation cancelled by user${NC}"
        exit 0
    fi
    echo -e "${BLUE}🗑️  Removing existing application...${NC}"
    rm -rf "$APP_DEST"
fi

# Copy the application
echo -e "${BLUE}📋 Installing application to $INSTALL_DIR...${NC}"
cp -R "$APP_SOURCE" "$APP_DEST"

# Set proper permissions
echo -e "${BLUE}🔧 Setting permissions...${NC}"
chmod +x "$APP_DEST/Contents/MacOS/TrussAnalysisGUI"

# Verify installation
if [ -d "$APP_DEST" ]; then
    echo -e "${GREEN}✅ Installation completed successfully!${NC}"
    APP_SIZE=$(du -sh "$APP_DEST" | cut -f1)
    echo -e "${GREEN}📏 Installed size: $APP_SIZE${NC}"
    echo
    
    # Check code signature
    echo -e "${BLUE}🔍 Verifying code signature...${NC}"
    if codesign -v "$APP_DEST" 2>/dev/null; then
        echo -e "${GREEN}✅ Code signature valid${NC}"
    else
        echo -e "${YELLOW}⚠️  Code signature verification failed (expected for ad-hoc signed apps)${NC}"
    fi
    echo
    
    # Security notice
    echo -e "${YELLOW}🛡️  SECURITY NOTICE:${NC}"
    echo -e "${YELLOW}   When you first launch the app, macOS may show a security warning.${NC}"
    echo -e "${YELLOW}   To run the app:${NC}"
    echo -e "${YELLOW}   1. Right-click on the app → 'Open'${NC}"
    echo -e "${YELLOW}   2. Click 'Open' in the security dialog${NC}"
    echo -e "${YELLOW}   3. This only needs to be done once${NC}"
    echo
    
    # Launch options
    echo -e "${GREEN}🚀 Launch the application:${NC}"
    echo -e "${GREEN}   • From Finder: Navigate to Applications and double-click $APP_NAME${NC}"
    echo -e "${GREEN}   • From Terminal: open '$APP_DEST'${NC}"
    echo -e "${GREEN}   • From Spotlight: Search for 'TrussAnalysis'${NC}"
    echo
    
    # Optional: Ask to launch now
    read -p "Would you like to launch the application now? (y/N): " -r
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${BLUE}🚀 Launching application...${NC}"
        open "$APP_DEST" &
        echo -e "${GREEN}✅ Application launched!${NC}"
    fi
    
else
    echo -e "${RED}❌ Installation failed${NC}"
    exit 1
fi

echo
echo -e "${GREEN}🎉 Installation completed successfully!${NC}"
echo -e "${BLUE}📖 For troubleshooting, see: INSTALL_MACOS.md${NC}"
