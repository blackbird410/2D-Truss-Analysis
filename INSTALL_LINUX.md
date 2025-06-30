# 2D Truss Analysis - Linux Installation Guide

This guide provides comprehensive instructions for building, installing, and running 2D Truss Analysis on Linux systems.

## Quick Start

```bash
# One-command build and install
./build_linux.sh && ./install_linux.sh
```

## System Requirements

- **OS**: Linux (Ubuntu 20.04+, Fedora 34+, Arch Linux, or equivalent)
- **Compiler**: GCC 7+ or Clang 5+ with C++20 support
- **Memory**: 2GB RAM minimum, 4GB recommended
- **Disk Space**: 500MB for build, 100MB for installation

## Dependencies

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake build-essential pkg-config qt6-base-dev qt6-charts-dev libeigen3-dev
```

### Fedora/RHEL/CentOS
```bash
sudo dnf install cmake gcc-c++ pkg-config qt6-qtbase-devel qt6-qtcharts-devel eigen3-devel
```

### Arch Linux
```bash
sudo pacman -S cmake gcc pkg-config qt6-base qt6-charts eigen
```

### openSUSE
```bash
sudo zypper install cmake gcc-c++ pkg-config qt6-base-devel qt6-charts-devel eigen3-devel
```

## Building from Source

### Method 1: Automated Build Script (Recommended)

```bash
# Clone repository
git clone https://github.com/blackbird410/2D-Truss-Analysis-cpp.git
cd 2D-Truss-Analysis-cpp

# Build application
./build_linux.sh

# Optional: Specify build type
./build_linux.sh Debug    # or Release (default)
```

### Method 2: Manual Build

```bash
# Create build directory
mkdir build_linux && cd build_linux

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      ..

# Build with make
make -j$(nproc)

# Go back to project root
cd ..
```

## Installation

### Method 1: Automated Installation (Recommended)

```bash
# Install to /usr/local (requires sudo for system-wide)
./install_linux.sh

# Install to custom directory (no sudo required)
./install_linux.sh $HOME/.local
```

### Method 2: Manual Installation

```bash
cd build_linux

# System-wide installation
sudo make install

# User-specific installation
make install DESTDIR=$HOME/.local
```

## What Gets Installed

### Executables
- **TrussAnalysisGUI**: Graphical user interface
- **TrussAnalysisCLI**: Command-line interface

### Desktop Integration
- **Desktop Entry**: `~/.local/share/applications/trussanalysis.desktop`
- **Application Icon**: `~/.local/share/icons/trussanalysis.svg`
- **Man Pages**: `man TrussAnalysisGUI`, `man TrussAnalysisCLI`

### File Associations
- **MIME Type**: `application/x-truss-project`
- **Keywords**: truss, structural, analysis, engineering, civil

## Running the Application

### GUI Application
```bash
# From terminal
TrussAnalysisGUI

# From desktop environment
# Search for "2D Truss Analysis" in application menu

# Open with file
TrussAnalysisGUI project.truss
```

### CLI Application
```bash
# Show help
TrussAnalysisCLI --help

# Run example analysis
TrussAnalysisCLI --example

# Analyze project file
TrussAnalysisCLI project.truss
```

## Build Options

### Debug Build
```bash
./build_linux.sh Debug
```
- Includes debug symbols
- No optimizations
- Larger binary size
- Better for development

### Release Build (Default)
```bash
./build_linux.sh Release
```
- Optimized for performance
- Debug symbols stripped
- Smaller binary size
- Best for production use

### Custom CMake Options
```bash
mkdir build_custom && cd build_custom

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/trussanalysis \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..

make -j$(nproc)
```

## Troubleshooting

### Common Issues

#### Qt6 Not Found
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-charts-dev

# Fedora
sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel

# Arch
sudo pacman -S qt6-base qt6-charts
```

#### Eigen3 Not Found
```bash
# Ubuntu/Debian
sudo apt install libeigen3-dev

# Fedora
sudo dnf install eigen3-devel

# Arch
sudo pacman -S eigen
```

#### CMake Version Too Old
```bash
# Install newer CMake from Kitware APT repository (Ubuntu/Debian)
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install cmake
```

#### Build Fails with C++20 Errors
```bash
# Ensure GCC 7+ or Clang 5+
gcc --version
clang --version

# Update compiler if necessary
sudo apt install gcc-9 g++-9  # Ubuntu/Debian
sudo dnf install gcc-c++      # Fedora
```

### Build Performance

#### Parallel Compilation
```bash
# Use all CPU cores
make -j$(nproc)

# Use specific number of cores
make -j4
```

#### Memory Issues
```bash
# If build runs out of memory, reduce parallel jobs
make -j2

# Or build sequentially
make -j1
```

### Runtime Issues

#### Application Won't Start
```bash
# Check library dependencies
ldd build_linux/TrussAnalysisGUI
ldd build_linux/TrussAnalysisCLI

# Check if in PATH
which TrussAnalysisGUI
```

#### Qt Platform Plugin Issues
```bash
# Install Qt platform plugins
sudo apt install qt6-qpa-plugins  # Ubuntu/Debian

# Set Qt platform explicitly
export QT_QPA_PLATFORM=xcb
TrussAnalysisGUI
```

## Uninstallation

### Automated Uninstall Script
```bash
# Create uninstall script
cat > uninstall_linux.sh << 'EOF'
#!/bin/bash
sudo rm -f /usr/local/bin/TrussAnalysisGUI
sudo rm -f /usr/local/bin/TrussAnalysisCLI
sudo rm -f /usr/local/share/man/man1/trussanalysisgui.1
sudo rm -f /usr/local/share/man/man1/trussanalysiscli.1
rm -f ~/.local/share/applications/trussanalysis.desktop
rm -f ~/.local/share/icons/trussanalysis.svg
echo "Uninstallation complete"
EOF

chmod +x uninstall_linux.sh
./uninstall_linux.sh
```

### Manual Uninstall
```bash
# Remove executables
sudo rm /usr/local/bin/TrussAnalysis{GUI,CLI}

# Remove man pages
sudo rm /usr/local/share/man/man1/trussanalysis{gui,cli}.1

# Remove desktop files
rm ~/.local/share/applications/trussanalysis.desktop
rm ~/.local/share/icons/trussanalysis.svg
```

## Package Distribution

### Creating DEB Package
```bash
# Install packaging tools
sudo apt install devscripts debhelper

# Create package
cd build_linux
cpack -G DEB
```

### Creating RPM Package
```bash
# Install packaging tools
sudo dnf install rpm-build rpmlint

# Create package
cd build_linux
cpack -G RPM
```

### Creating Flatpak (Advanced)
```bash
# Requires flatpak-builder and runtime setup
# See: https://docs.flatpak.org/en/latest/building.html
```

## Development Setup

### IDE Configuration

#### VSCode
```bash
# Install C++ extension
code --install-extension ms-vscode.cpptools

# Generate compile_commands.json
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

#### CLion
- Open CMakeLists.txt as project
- Configure toolchain to use system GCC/Clang
- Set CMake options as needed

### Code Formatting
```bash
# Install clang-format
sudo apt install clang-format  # Ubuntu/Debian
sudo dnf install clang-tools-extra  # Fedora

# Format code
find src -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

## Performance Optimization

### Build Optimizations
```bash
# Maximum optimization
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..

# Link-time optimization
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
```

### Runtime Optimizations
```bash
# Use faster memory allocator
export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2
TrussAnalysisGUI
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines and contribution instructions.

## Support

- **Issues**: [GitHub Issues](https://github.com/blackbird410/2D-Truss-Analysis-cpp/issues)
- **Discussions**: [GitHub Discussions](https://github.com/blackbird410/2D-Truss-Analysis-cpp/discussions)
- **Documentation**: [Project Wiki](https://github.com/blackbird410/2D-Truss-Analysis-cpp/wiki)

---

**Made with ❤️ for the Linux and civil engineering communities**
