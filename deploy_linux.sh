#!/bin/bash

# 2D Truss Analysis - Linux Deployment Script
# This script packages the application for distribution

echo " Deploying 2D Truss Analysis for Linux..."
echo

# Set up directories
DEPLOY_DIR="./deploy_linux"
APP_NAME="TrussAnalysis"
VERSION="2.2.1"

# Clean and create deployment directory
echo " Setting up deployment directory..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR/bin"
mkdir -p "$DEPLOY_DIR/lib"
mkdir -p "$DEPLOY_DIR/examples"
mkdir -p "$DEPLOY_DIR/docs"

# Copy executables
echo " Copying application binaries..."
cp build_release/TrussAnalysisGUI "$DEPLOY_DIR/bin/"
cp build_release/TrussAnalysisCLI "$DEPLOY_DIR/bin/"

# Make executable
chmod +x "$DEPLOY_DIR/bin/"*

# Copy Qt libraries if needed (for portable deployment)
echo " Checking Qt dependencies..."
ldd build_release/TrussAnalysisGUI | grep -q "libQt"
if [ $? -eq 0 ]; then
    echo "   Qt libraries detected - consider using linuxdeployqt for portable deployment"
fi

# Create application launcher script
echo " Creating launcher script..."
cat > "$DEPLOY_DIR/truss-analysis-gui.sh" << 'EOF'
#!/bin/bash
# 2D Truss Analysis GUI Launcher

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library path if needed
export LD_LIBRARY_PATH="$SCRIPT_DIR/lib:$LD_LIBRARY_PATH"

# Launch the application
exec "$SCRIPT_DIR/bin/TrussAnalysisGUI" "$@"
EOF

chmod +x "$DEPLOY_DIR/truss-analysis-gui.sh"

# Create README for deployment
echo " Creating deployment documentation..."
cat > "$DEPLOY_DIR/README.md" << EOF
# 2D Truss Analysis v${VERSION} - Linux Release

## What's Fixed in This Release

### 🖥️ Linux Display Issues - SOLVED!

This release addresses all major Linux display compatibility issues:

####  **Fixed Window Sizing Problems**
- **Adaptive window sizing**: Automatically adjusts to your screen resolution
- **High-DPI support**: Proper scaling on 4K and high-resolution displays  
- **Minimum size constraints**: Ensures the application remains usable at all sizes
- **Smart initial positioning**: Centers window on screen with optimal size

####  **Fixed Resizing and Fullscreen Issues**
- **Proper window resizing**: All widgets now scale correctly when window is resized
- **Fullscreen support**: Press F11 to toggle fullscreen mode
- **Responsive layouts**: Drawing canvas and property panels adapt to window changes
- **Window state persistence**: Remembers your window size and position between sessions

####  **Enhanced Linux Compatibility**
- **Qt6 compatibility**: Updated for modern Qt framework
- **Wayland/X11 support**: Works on both display servers
- **Font rendering**: Improved text clarity on Linux systems
- **Native dialogs**: Uses system file dialogs when appropriate

####  **Improved User Experience**
- **Better viewport management**: Drawing area updates correctly during resize
- **Smooth scaling**: Zoom and pan operations work seamlessly
- **Preserved functionality**: All analysis features work exactly as before

## Installation

1. Extract this package to your desired location
2. Run the GUI application:
   \`\`\`bash
   ./truss-analysis-gui.sh
   \`\`\`
3. Or use the command-line interface:
   \`\`\`bash
   ./bin/TrussAnalysisCLI
   \`\`\`

## System Requirements

- **OS**: Linux (any modern distribution)
- **Display**: X11 or Wayland
- **RAM**: 512MB minimum, 2GB recommended
- **Qt**: Qt6 (usually installed by default on modern systems)

## Key Features

-  **Interactive Design**: Click and drag to create truss structures
-  **Advanced Analysis**: Professional finite element analysis engine
-  **Rich Visualization**: Deformed shapes, force diagrams, stress analysis
-  **Project Management**: Save, load, and export your designs
-  **Results Export**: CSV, JSON, XML, LaTeX, HTML formats
-  **Modern Interface**: Responsive, intuitive user interface

## Quick Start

1. Launch the application: \`./truss-analysis-gui.sh\`
2. Use **F11** for fullscreen mode
3. Create nodes by clicking "Add Node" and clicking on the canvas
4. Add members by selecting "Add Member" and dragging between nodes
5. Set supports by clicking "Set Support" and clicking nodes
6. Apply loads with "Add Load" tool
7. Click "Analyze Structure" to run the analysis
8. View results in the tabs below the drawing area

## Keyboard Shortcuts

- **F11**: Toggle fullscreen
- **F5**: Run analysis
- **Ctrl+N**: New project
- **Ctrl+O**: Open project
- **Ctrl+S**: Save project
- **Del**: Delete selected elements
- **Esc**: Cancel current operation

## Support

For issues or questions, please check the documentation or contact support.

---
**Built with ❤️ for structural engineers everywhere**
EOF

# Create a simple example project file
echo " Creating example project..."
cat > "$DEPLOY_DIR/examples/simple_truss.truss" << 'EOF'
{
  "metadata": {
    "version": "2.2.1",
    "created": "2024-06-30",
    "description": "Simple triangular truss example"
  },
  "nodes": [
    {"id": 1, "x": 0.0, "y": 0.0, "support": "Pinned"},
    {"id": 2, "x": 4.0, "y": 0.0, "support": "RollerY"},
    {"id": 3, "x": 2.0, "y": 3.0, "support": "Free", "force_x": 0.0, "force_y": -10000.0}
  ],
  "members": [
    {"id": 1, "start_node": 1, "end_node": 2, "area": 0.002, "young_modulus": 200e9},
    {"id": 2, "start_node": 1, "end_node": 3, "area": 0.002, "young_modulus": 200e9},
    {"id": 3, "start_node": 2, "end_node": 3, "area": 0.002, "young_modulus": 200e9}
  ]
}
EOF

# Create desktop entry for system integration (optional)
echo "🖥️ Creating desktop entry..."
cat > "$DEPLOY_DIR/truss-analysis.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=2D Truss Analysis
Comment=Professional structural analysis for 2D truss structures
Exec=$(pwd)/$DEPLOY_DIR/truss-analysis-gui.sh
Icon=$(pwd)/$DEPLOY_DIR/truss-analysis-icon.png
Terminal=false
Categories=Engineering;Science;Education;
Keywords=structural;analysis;engineering;truss;finite;element;
EOF

# Test the application quickly
echo " Testing deployed application..."
if [ -x "$DEPLOY_DIR/bin/TrussAnalysisGUI" ]; then
    echo "    GUI executable is ready"
else
    echo "   ❌ GUI executable not found or not executable"
fi

if [ -x "$DEPLOY_DIR/bin/TrussAnalysisCLI" ]; then
    echo "    CLI executable is ready"
else
    echo "   ❌ CLI executable not found or not executable"
fi

# Calculate package size
PACKAGE_SIZE=$(du -sh "$DEPLOY_DIR" | cut -f1)
echo "   📏 Package size: $PACKAGE_SIZE"

echo
echo " Deployment complete!"
echo "📂 Package location: $DEPLOY_DIR"
echo " Run the GUI: ./$DEPLOY_DIR/truss-analysis-gui.sh"
echo " Run the CLI: ./$DEPLOY_DIR/bin/TrussAnalysisCLI"
echo
echo "✨ Linux display issues have been resolved:"
echo "   • Proper window sizing and positioning"
echo "   • Fullscreen support (F11)"
echo "   • High-DPI compatibility"  
echo "   • Responsive resizing"
echo "   • Window state persistence"
echo
echo "Ready for distribution! "
EOF
