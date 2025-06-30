#!/bin/bash

# 2D Truss Analysis - Linux Installation Script
# Version 2.1.3

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  2D Truss Analysis - Linux Installer         ${NC}"
echo -e "${BLUE}              Version 2.1.3                   ${NC}"
echo -e "${BLUE}================================================${NC}"
echo

# Configuration
BUILD_DIR="build_linux"
INSTALL_PREFIX="${1:-/usr/local}"
DESKTOP_FILE_DIR="$HOME/.local/share/applications"
ICON_DIR="$HOME/.local/share/icons"

echo -e "${BLUE}🔧 Installation Configuration:${NC}"
echo -e "${BLUE}   • Build Directory: $BUILD_DIR${NC}"
echo -e "${BLUE}   • Install Prefix: $INSTALL_PREFIX${NC}"
echo -e "${BLUE}   • Desktop Files: $DESKTOP_FILE_DIR${NC}"
echo

# Check if we need sudo for system-wide installation
NEED_SUDO=false
if [[ "$INSTALL_PREFIX" == "/usr"* ]]; then
    NEED_SUDO=true
    echo -e "${YELLOW}⚠️  System-wide installation requires sudo privileges${NC}"
fi

# Check if build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}❌ Build directory not found: $BUILD_DIR${NC}"
    echo -e "${YELLOW}💡 Please run ./build_linux.sh first${NC}"
    exit 1
fi

cd "$BUILD_DIR"

# Check if executables exist
if [ ! -f "TrussAnalysisGUI" ] || [ ! -f "TrussAnalysisCLI" ]; then
    echo -e "${RED}❌ Built executables not found${NC}"
    echo -e "${YELLOW}💡 Please run ./build_linux.sh first${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Found built executables${NC}"

# Function to run command with or without sudo
run_cmd() {
    if [ "$NEED_SUDO" = true ]; then
        sudo "$@"
    else
        "$@"
    fi
}

# Install executables
echo -e "${BLUE}📋 Installing executables...${NC}"

BIN_DIR="$INSTALL_PREFIX/bin"
run_cmd mkdir -p "$BIN_DIR"

run_cmd cp TrussAnalysisGUI "$BIN_DIR/"
run_cmd cp TrussAnalysisCLI "$BIN_DIR/"

run_cmd chmod +x "$BIN_DIR/TrussAnalysisGUI"
run_cmd chmod +x "$BIN_DIR/TrussAnalysisCLI"

echo -e "${GREEN}✅ Executables installed to $BIN_DIR${NC}"

# Create desktop file for GUI application
echo -e "${BLUE}🖥️  Creating desktop entry...${NC}"

mkdir -p "$DESKTOP_FILE_DIR"

cat > "$DESKTOP_FILE_DIR/trussanalysis.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=2D Truss Analysis
Comment=Professional 2D truss structural analysis software
Exec=$BIN_DIR/TrussAnalysisGUI
Icon=trussanalysis
Terminal=false
Categories=Engineering;Education;Science;
StartupNotify=true
MimeType=application/x-truss-project;
Keywords=truss;structural;analysis;engineering;civil;
EOF

chmod +x "$DESKTOP_FILE_DIR/trussanalysis.desktop"

echo -e "${GREEN}✅ Desktop entry created${NC}"

# Create simple icon (text-based for now)
echo -e "${BLUE}🎨 Creating application icon...${NC}"

mkdir -p "$ICON_DIR"

# Create a simple SVG icon
cat > "$ICON_DIR/trussanalysis.svg" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<svg width="64" height="64" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg">
  <rect width="64" height="64" fill="#2E3440" rx="8"/>
  <g stroke="#88C0D0" stroke-width="2" fill="none">
    <!-- Truss structure -->
    <line x1="8" y1="48" x2="56" y2="48"/>
    <line x1="8" y1="48" x2="32" y2="16"/>
    <line x1="56" y1="48" x2="32" y2="16"/>
    <line x1="20" y1="40" x2="32" y2="16"/>
    <line x1="44" y1="40" x2="32" y2="16"/>
    <line x1="20" y1="40" x2="44" y2="40"/>
    <!-- Support symbols -->
    <rect x="6" y="48" width="4" height="8" fill="#BF616A"/>
    <rect x="54" y="48" width="4" height="8" fill="#BF616A"/>
  </g>
  <text x="32" y="58" text-anchor="middle" fill="#ECEFF4" font-family="monospace" font-size="8">TRUSS</text>
</svg>
EOF

echo -e "${GREEN}✅ Application icon created${NC}"

# Update desktop database
if command -v update-desktop-database >/dev/null 2>&1; then
    echo -e "${BLUE}🔄 Updating desktop database...${NC}"
    update-desktop-database "$DESKTOP_FILE_DIR" 2>/dev/null || true
    echo -e "${GREEN}✅ Desktop database updated${NC}"
fi

# Create man pages
echo -e "${BLUE}📖 Installing man pages...${NC}"

MAN_DIR="$INSTALL_PREFIX/share/man/man1"
run_cmd mkdir -p "$MAN_DIR"

# TrussAnalysisGUI man page
cat > /tmp/trussanalysisgui.1 << 'EOF'
.TH TRUSSANALYSISGUI 1 "2025-06-30" "2.1.3" "2D Truss Analysis"
.SH NAME
TrussAnalysisGUI \- Professional 2D truss structural analysis software
.SH SYNOPSIS
.B TrussAnalysisGUI
[\fIFILE\fP]
.SH DESCRIPTION
TrussAnalysisGUI is a professional-grade 2D truss structural analysis application built with modern C++20 and Qt6. It features an intuitive interactive drawing interface and robust computational engine for analyzing truss structures.
.SH OPTIONS
.TP
.B FILE
Optional project file to open at startup
.SH FEATURES
.IP \(bu 2
Interactive drawing canvas with mouse-based node and member placement
.IP \(bu 2
Direct stiffness method implementation
.IP \(bu 2
Support for various load types and boundary conditions
.IP \(bu 2
Professional results display with comprehensive tables
.IP \(bu 2
Visual deformation plots and force diagrams
.SH SEE ALSO
.BR TrussAnalysisCLI (1)
.SH AUTHOR
Civil Engineering Software Solutions
EOF

run_cmd cp /tmp/trussanalysisgui.1 "$MAN_DIR/"
rm /tmp/trussanalysisgui.1

# TrussAnalysisCLI man page
cat > /tmp/trussanalysiscli.1 << 'EOF'
.TH TRUSSANALYSISCLI 1 "2025-06-30" "2.1.3" "2D Truss Analysis"
.SH NAME
TrussAnalysisCLI \- Command-line 2D truss structural analysis tool
.SH SYNOPSIS
.B TrussAnalysisCLI
[\fIOPTIONS\fP] [\fIFILE\fP]
.SH DESCRIPTION
TrussAnalysisCLI is the command-line interface for 2D truss structural analysis. It provides batch processing capabilities and can be used in automated workflows.
.SH OPTIONS
.TP
.B \-\-help
Show help message and exit
.TP
.B \-\-version
Show version information and exit
.TP
.B \-\-example
Run example analysis
.SH SEE ALSO
.BR TrussAnalysisGUI (1)
.SH AUTHOR
Civil Engineering Software Solutions
EOF

run_cmd cp /tmp/trussanalysiscli.1 "$MAN_DIR/"
rm /tmp/trussanalysiscli.1

echo -e "${GREEN}✅ Man pages installed${NC}"

# Test installation
echo -e "${BLUE}🧪 Testing installation...${NC}"

if command -v TrussAnalysisCLI >/dev/null 2>&1; then
    echo -e "${GREEN}✅ TrussAnalysisCLI available in PATH${NC}"
else
    echo -e "${YELLOW}⚠️  TrussAnalysisCLI not in PATH - may need to restart shell${NC}"
fi

if command -v TrussAnalysisGUI >/dev/null 2>&1; then
    echo -e "${GREEN}✅ TrussAnalysisGUI available in PATH${NC}"
else
    echo -e "${YELLOW}⚠️  TrussAnalysisGUI not in PATH - may need to restart shell${NC}"
fi

cd ..

echo
echo -e "${GREEN}🎉 Linux installation completed successfully!${NC}"
echo
echo -e "${BLUE}🚀 How to use:${NC}"
echo -e "${BLUE}   • GUI Application: TrussAnalysisGUI${NC}"
echo -e "${BLUE}   • Command Line: TrussAnalysisCLI --help${NC}"
echo -e "${BLUE}   • Desktop: Search for '2D Truss Analysis' in applications${NC}"
echo
echo -e "${BLUE}📖 Documentation:${NC}"
echo -e "${BLUE}   • man TrussAnalysisGUI${NC}"
echo -e "${BLUE}   • man TrussAnalysisCLI${NC}"
echo
echo -e "${BLUE}📍 Installation Details:${NC}"
echo -e "${BLUE}   • Executables: $BIN_DIR${NC}"
echo -e "${BLUE}   • Desktop Entry: $DESKTOP_FILE_DIR/trussanalysis.desktop${NC}"
echo -e "${BLUE}   • Icon: $ICON_DIR/trussanalysis.svg${NC}"
echo -e "${BLUE}   • Man Pages: $MAN_DIR${NC}"
echo
