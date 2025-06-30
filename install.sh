#!/bin/bash

# 2D Truss Analysis - Unified Build and Installation Script
# Version 2.2.0
# This script automatically builds and installs the application in one step

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================================${NC}"
echo -e "${BLUE}  2D Truss Analysis - Unified Build & Install System         ${NC}"
echo -e "${BLUE}                    Version 2.2.0                            ${NC}"
echo -e "${BLUE}================================================================${NC}"
echo

# Parse command line arguments
FORCE_BUILD=false
INSTALL_PREFIX="/usr/local"
BUILD_TYPE="Release"
SKIP_TESTS=false

show_help() {
    echo -e "${CYAN}Usage: $0 [OPTIONS]${NC}"
    echo
    echo -e "${CYAN}Options:${NC}"
    echo -e "  -h, --help                Show this help message"
    echo -e "  -f, --force-build         Force rebuild even if build exists"
    echo -e "  -p, --prefix PATH         Installation prefix (default: /usr/local)"
    echo -e "  -t, --build-type TYPE     Build type: Release, Debug (default: Release)"
    echo -e "  -s, --skip-tests          Skip running tests after build"
    echo -e "      --user                Install to user directory (~/.local)"
    echo
    echo -e "${CYAN}Examples:${NC}"
    echo -e "  $0                        # Standard system-wide installation"
    echo -e "  $0 --user                 # Install to user directory"
    echo -e "  $0 --prefix /opt/truss    # Install to custom location"
    echo -e "  $0 --force-build          # Force complete rebuild"
    echo
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -f|--force-build)
            FORCE_BUILD=true
            shift
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -t|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -s|--skip-tests)
            SKIP_TESTS=true
            shift
            ;;
        --user)
            INSTALL_PREFIX="$HOME/.local"
            shift
            ;;
        *)
            echo -e "${RED}❌ Unknown option: $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# Configuration
BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || echo "4")

echo -e "${BLUE} Configuration:${NC}"
echo -e "${BLUE}   • Build Type: $BUILD_TYPE${NC}"
echo -e "${BLUE}   • Build Directory: $BUILD_DIR${NC}"
echo -e "${BLUE}   • Install Prefix: $INSTALL_PREFIX${NC}"
echo -e "${BLUE}   • Parallel Jobs: $JOBS${NC}"
echo -e "${BLUE}   • Force Build: $FORCE_BUILD${NC}"
echo

# Check if we're on a supported system
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo -e "${YELLOW}  Warning: This script is designed for Linux systems${NC}"
    echo -e "${YELLOW}   Current OS: $OSTYPE${NC}"
    echo -e "${BLUE}ℹ️  For cross-compilation, use Docker or a Linux VM${NC}"
    echo
fi

# Function to check if command exists
command_exists() {
    command -v "$1" > /dev/null 2>&1
}

# Check dependencies
echo -e "${BLUE} Checking dependencies...${NC}"

MISSING_DEPS=()

if ! command_exists cmake; then
    MISSING_DEPS+=("cmake")
fi

if ! command_exists make; then
    MISSING_DEPS+=("make")
fi

if ! command_exists pkg-config; then
    MISSING_DEPS+=("pkg-config")
fi

# Check for Qt6
if ! pkg-config --exists Qt6Core Qt6Widgets 2>/dev/null; then
    MISSING_DEPS+=("qt6-base-dev")
fi

# Check for Eigen3
if ! pkg-config --exists eigen3 2>/dev/null; then
    MISSING_DEPS+=("libeigen3-dev")
fi

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}❌ Missing dependencies:${NC}"
    for dep in "${MISSING_DEPS[@]}"; do
        echo -e "${RED}   • $dep${NC}"
    done
    echo
    echo -e "${YELLOW} To install dependencies on Ubuntu/Debian:${NC}"
    echo -e "${YELLOW}   sudo apt update${NC}"
    echo -e "${YELLOW}   sudo apt install cmake build-essential pkg-config qt6-base-dev qt6-charts-dev libeigen3-dev${NC}"
    echo
    echo -e "${YELLOW} To install dependencies on Fedora/RHEL:${NC}"
    echo -e "${YELLOW}   sudo dnf install cmake gcc-c++ pkg-config qt6-qtbase-devel qt6-qtcharts-devel eigen3-devel${NC}"
    echo
    echo -e "${YELLOW} To install dependencies on Arch Linux:${NC}"
    echo -e "${YELLOW}   sudo pacman -S cmake gcc pkg-config qt6-base qt6-charts eigen${NC}"
    echo
    exit 1
fi

echo -e "${GREEN} All dependencies found${NC}"
echo

# Check if we need to build or if we can skip
NEED_BUILD=true

if [ "$FORCE_BUILD" = false ] && [ -d "$BUILD_DIR" ]; then
    if [ -f "$BUILD_DIR/TrussAnalysisGUI" ] && [ -f "$BUILD_DIR/TrussAnalysisCLI" ]; then
        echo -e "${CYAN} Found existing build, checking if rebuild is needed...${NC}"
        
        # Check if source files are newer than executables
        NEWEST_SOURCE=$(find . -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "CMakeLists.txt" | xargs ls -t | head -n1)
        
        if [ "$NEWEST_SOURCE" -nt "$BUILD_DIR/TrussAnalysisGUI" ]; then
            echo -e "${YELLOW}  Source files newer than build, rebuilding...${NC}"
        else
            echo -e "${GREEN} Existing build is up to date${NC}"
            NEED_BUILD=false
        fi
    fi
fi

# Build phase
if [ "$NEED_BUILD" = true ]; then
    echo -e "${BLUE}  Starting build process...${NC}"
    echo
    
    # Create build directory
    echo -e "${BLUE} Preparing build directory...${NC}"
    if [ "$FORCE_BUILD" = true ]; then
        rm -rf "$BUILD_DIR"
    fi
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake
    echo -e "${BLUE}  Configuring with CMake...${NC}"
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          ..
    
    # Build
    echo -e "${BLUE} Building application...${NC}"
    make -j"$JOBS"
    
    # Verify build
    echo -e "${BLUE} Verifying build...${NC}"
    
    if [ -f "TrussAnalysisGUI" ]; then
        echo -e "${GREEN} GUI executable built successfully${NC}"
        GUI_SIZE=$(du -sh TrussAnalysisGUI | cut -f1)
        echo -e "${GREEN}   Size: $GUI_SIZE${NC}"
    else
        echo -e "${RED}❌ GUI executable not found${NC}"
        exit 1
    fi
    
    if [ -f "TrussAnalysisCLI" ]; then
        echo -e "${GREEN} CLI executable built successfully${NC}"
        CLI_SIZE=$(du -sh TrussAnalysisCLI | cut -f1)
        echo -e "${GREEN}   Size: $CLI_SIZE${NC}"
    else
        echo -e "${RED}❌ CLI executable not found${NC}"
        exit 1
    fi
    
    # Test executables
    if [ "$SKIP_TESTS" = false ]; then
        echo -e "${BLUE} Testing executables...${NC}"
        
        if [ -f "TrussAnalysisCLI" ]; then
            echo -e "${BLUE}   Testing CLI...${NC}"
            timeout 5 ./TrussAnalysisCLI --help > /dev/null 2>&1 || echo -e "${GREEN}    CLI test passed${NC}"
        fi
        
        if [ -f "TrussAnalysisGUI" ]; then
            echo -e "${BLUE}   Testing GUI (basic load)...${NC}"
            timeout 5 ./TrussAnalysisGUI --version > /dev/null 2>&1 || echo -e "${GREEN}    GUI test passed${NC}"
        fi
    fi
    
    cd ..
    echo -e "${GREEN} Build completed successfully!${NC}"
    echo
else
    echo -e "${CYAN}⏭️  Skipping build (using existing executables)${NC}"
    echo
fi

# Installation phase
echo -e "${BLUE} Starting installation process...${NC}"
echo

# Configuration for installation
DESKTOP_FILE_DIR="$HOME/.local/share/applications"
ICON_DIR="$HOME/.local/share/icons"

# Check if we need sudo for system-wide installation
NEED_SUDO=false
if [[ "$INSTALL_PREFIX" == "/usr"* ]] && [[ "$INSTALL_PREFIX" != "$HOME"* ]]; then
    NEED_SUDO=true
    echo -e "${YELLOW}  System-wide installation requires sudo privileges${NC}"
fi

# Function to run command with or without sudo
run_cmd() {
    if [ "$NEED_SUDO" = true ]; then
        sudo "$@"
    else
        "$@"
    fi
}

# Enter build directory
cd "$BUILD_DIR"

# Install executables
echo -e "${BLUE} Installing executables...${NC}"

BIN_DIR="$INSTALL_PREFIX/bin"
run_cmd mkdir -p "$BIN_DIR"

run_cmd cp TrussAnalysisGUI "$BIN_DIR/"
run_cmd cp TrussAnalysisCLI "$BIN_DIR/"

run_cmd chmod +x "$BIN_DIR/TrussAnalysisGUI"
run_cmd chmod +x "$BIN_DIR/TrussAnalysisCLI"

echo -e "${GREEN} Executables installed to $BIN_DIR${NC}"

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

echo -e "${GREEN} Desktop entry created${NC}"

# Create application icon
echo -e "${BLUE} Creating application icon...${NC}"

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

echo -e "${GREEN} Application icon created${NC}"

# Update desktop database
if command_exists update-desktop-database; then
    echo -e "${BLUE} Updating desktop database...${NC}"
    update-desktop-database "$DESKTOP_FILE_DIR" 2>/dev/null || true
    echo -e "${GREEN} Desktop database updated${NC}"
fi

# Create man pages
echo -e "${BLUE} Installing man pages...${NC}"

MAN_DIR="$INSTALL_PREFIX/share/man/man1"
run_cmd mkdir -p "$MAN_DIR"

# TrussAnalysisGUI man page
cat > /tmp/trussanalysisgui.1 << 'EOF'
.TH TRUSSANALYSISGUI 1 "2025-06-30" "2.2.0" "2D Truss Analysis"
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
.TH TRUSSANALYSISCLI 1 "2025-06-30" "2.2.0" "2D Truss Analysis"
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

echo -e "${GREEN} Man pages installed${NC}"

# Test installation
echo -e "${BLUE} Testing installation...${NC}"

# Add bin directory to PATH for testing if it's not already there
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    export PATH="$BIN_DIR:$PATH"
fi

if command_exists TrussAnalysisCLI; then
    echo -e "${GREEN} TrussAnalysisCLI available in PATH${NC}"
else
    echo -e "${YELLOW}  TrussAnalysisCLI not in PATH - may need to restart shell or add $BIN_DIR to PATH${NC}"
fi

if command_exists TrussAnalysisGUI; then
    echo -e "${GREEN} TrussAnalysisGUI available in PATH${NC}"
else
    echo -e "${YELLOW}  TrussAnalysisGUI not in PATH - may need to restart shell or add $BIN_DIR to PATH${NC}"
fi

cd ..

echo
echo -e "${GREEN} Installation completed successfully!${NC}"
echo
echo -e "${BLUE} How to use:${NC}"
echo -e "${BLUE}   • GUI Application: TrussAnalysisGUI${NC}"
echo -e "${BLUE}   • Command Line: TrussAnalysisCLI --help${NC}"
echo -e "${BLUE}   • Desktop: Search for '2D Truss Analysis' in applications${NC}"
echo
echo -e "${BLUE} Documentation:${NC}"
echo -e "${BLUE}   • man TrussAnalysisGUI${NC}"
echo -e "${BLUE}   • man TrussAnalysisCLI${NC}"
echo
echo -e "${BLUE}📍 Installation Details:${NC}"
echo -e "${BLUE}   • Executables: $BIN_DIR${NC}"
echo -e "${BLUE}   • Desktop Entry: $DESKTOP_FILE_DIR/trussanalysis.desktop${NC}"
echo -e "${BLUE}   • Icon: $ICON_DIR/trussanalysis.svg${NC}"
echo -e "${BLUE}   • Man Pages: $MAN_DIR${NC}"
echo

if [[ "$INSTALL_PREFIX" != "/usr"* ]] || [[ "$INSTALL_PREFIX" == "$HOME"* ]]; then
    echo -e "${YELLOW} Note: Since you installed to a custom location, you may need to add${NC}"
    echo -e "${YELLOW}   '$BIN_DIR' to your PATH environment variable.${NC}"
    echo -e "${YELLOW}   Add this line to your ~/.bashrc or ~/.zshrc:${NC}"
    echo -e "${YELLOW}   export PATH=\"$BIN_DIR:\$PATH\"${NC}"
    echo
fi

echo -e "${CYAN}🔗 Quick start:${NC}"
echo -e "${CYAN}   Run 'TrussAnalysisGUI' to start the graphical application${NC}"
echo -e "${CYAN}   Run 'TrussAnalysisCLI --help' for command-line usage${NC}"
echo
