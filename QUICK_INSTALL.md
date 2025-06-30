# 🚀 Quick Install Guide - 2D Truss Analysis

## For End Users (Simple Installation)

### Option 1: Double-Click Installation (Easiest)
1. **Double-click** `2D_Truss_Analysis-2.1.0-Darwin.dmg`
2. **Drag** `TrussAnalysisGUI.app` to the `Applications` folder shortcut
3. **Right-click** the app in Applications → **"Open"**
4. Click **"Open"** in the security dialog
5. **Done!** 🎉

### Option 2: Automated Script Installation
```bash
# Run the installation script
./install_macos.sh
```

## For Developers/Advanced Users

### Command Line Installation
```bash
# Mount DMG
hdiutil attach 2D_Truss_Analysis-2.1.0-Darwin.dmg

# Install to Applications
cp -R "/Volumes/2D Truss Analysis/bin/TrussAnalysisGUI.app" /Applications/

# Unmount DMG
hdiutil detach "/Volumes/2D Truss Analysis"

# Launch
open /Applications/TrussAnalysisGUI.app
```

### Bypass Security Warnings
```bash
# Remove quarantine (if needed)
xattr -d com.apple.quarantine /Applications/TrussAnalysisGUI.app

# Re-sign if necessary
codesign --force --deep --sign - /Applications/TrussAnalysisGUI.app
```

## Troubleshooting

### "Cannot open app from unidentified developer"
- **Right-click** the app → **"Open"**
- Or: System Preferences → Security & Privacy → **"Open Anyway"**

### "App is damaged"
```bash
xattr -cr /Applications/TrussAnalysisGUI.app
```

## System Requirements
- macOS 10.15+ (Catalina or later)
- 4GB RAM (8GB recommended)
- 100MB free space

---
**Need help?** See the full guide: `INSTALL_MACOS.md`
