# 2D Truss Analysis - macOS Installation Guide

## Quick Installation

### Method 1: DMG Package (Recommended)

1. **Download/Locate the DMG file:**
   ```bash
   # If you built from source, the DMG is located at:
   # build_release/2D_Truss_Analysis-2.1.0-Darwin.dmg
   ```

2. **Mount the DMG:**
   ```bash
   # Double-click the DMG file or use command line:
   open 2D_Truss_Analysis-2.1.0-Darwin.dmg
   ```

3. **Install the Application:**
   - Drag `TrussAnalysisGUI.app` to your `/Applications` folder
   - Or drag to any desired location (Desktop, Documents, etc.)

4. **First Launch - Security Permission:**
   - Right-click on `TrussAnalysisGUI.app` → "Open"
   - macOS will show a security warning
   - Click "Open" to allow the application to run
   - This only needs to be done once

### Method 2: Command Line Installation

```bash
# Mount the DMG
hdiutil attach 2D_Truss_Analysis-2.1.0-Darwin.dmg

# Copy to Applications folder
cp -R "/Volumes/2D Truss Analysis/TrussAnalysisGUI.app" /Applications/

# Unmount the DMG
hdiutil detach "/Volumes/2D Truss Analysis"

# Launch the application
open /Applications/TrussAnalysisGUI.app
```

## Running the Application

### GUI Application
```bash
# From Applications folder
open /Applications/TrussAnalysisGUI.app

# Or from Finder: Double-click TrussAnalysisGUI.app
```

### CLI Application (if copied separately)
```bash
# If you copied the CLI tool
./TrussAnalysisCLI --help
./TrussAnalysisCLI --example
```

## Troubleshooting

### "App cannot be opened because it is from an unidentified developer"

**Solution 1 - Right-click method:**
1. Right-click the app → "Open"
2. Click "Open" in the security dialog

**Solution 2 - System Preferences:**
1. Go to System Preferences → Security & Privacy
2. Click "Open Anyway" for TrussAnalysisGUI

**Solution 3 - Command line override:**
```bash
# Remove quarantine attribute
xattr -d com.apple.quarantine /Applications/TrussAnalysisGUI.app
```

### "App is damaged and can't be opened" or App crashes immediately

**This indicates a code signature issue. Try these solutions:**

```bash
# Solution 1: Clear extended attributes and re-sign
xattr -cr /Applications/TrussAnalysisGUI.app
codesign --force --deep --sign - /Applications/TrussAnalysisGUI.app

# Solution 2: If the above doesn't work, the DMG may be corrupted
# Remove the app and reinstall from a fixed DMG
rm -rf /Applications/TrussAnalysisGUI.app
# Then reinstall from the DMG
```

**Note**: If you encounter persistent crashing with `EXC_BAD_ACCESS (Code Signature Invalid)`, the DMG package itself may have signature issues. Contact support for a fixed version.

### Permission Issues

```bash
# Fix permissions
chmod +x /Applications/TrussAnalysisGUI.app/Contents/MacOS/TrussAnalysisGUI
```

## Uninstallation

```bash
# Remove the application
rm -rf /Applications/TrussAnalysisGUI.app

# Clean any preferences (optional)
rm -rf ~/Library/Preferences/com.trussanalysis.*
rm -rf ~/Library/Application\ Support/TrussAnalysis
```

## System Requirements

- **macOS**: 10.15 (Catalina) or later
- **Architecture**: Intel x86_64 or Apple Silicon (Universal Binary)
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 100MB free space

## Features Available

### GUI Application
- Interactive drawing canvas for truss design
- Mouse-based node and member creation
- Real-time analysis and visualization
- Results display with forces and displacements
- Export capabilities

### CLI Application
- Command-line truss analysis
- Batch processing capabilities
- Example analysis included
- Scriptable interface

## Getting Started

1. **Launch the GUI application**
2. **Create a new truss:**
   - Use the drawing tools to add nodes
   - Connect nodes with members
   - Apply loads and supports
3. **Run analysis** and view results
4. **Save your work** for future use

## Support

For issues or questions:
- Check the troubleshooting section above
- Review the application documentation
- File issues on the project repository

---

**Version**: 2.1.0  
**Build Date**: June 30, 2025  
**Platform**: macOS Universal Binary
