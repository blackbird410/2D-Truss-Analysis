# Packaging and Distribution Architecture

**Version:** 3.0.0  
**Scope:** Linux packaging and distribution surfaces for the current implementation

---

## 1. Overview

This document defines the current packaging and distribution architecture for 2D Truss Analysis.

It covers:

- Build-time package metadata
- Installation layout
- Desktop integration assets
- Package generation scripts
- Runtime dependency surfaces

---

## 2. Build and Packaging Surfaces

| Surface                  | Location          | Purpose                                                                            |
| ------------------------ | ----------------- | ---------------------------------------------------------------------------------- |
| CMake project root       | CMakeLists.txt    | Defines project metadata, install/export behavior, and package generation settings |
| CMake helper modules     | cmake/            | Compiler, install, sanitizer, static-analysis helper configuration                 |
| Debian metadata          | packaging/debian/ | Debian package control hooks and legal metadata                                    |
| Linux desktop metadata   | packaging/linux/  | Desktop entry, AppStream metainfo, icon assets                                     |
| Distribution scripts     | scripts/package/  | Package creation entry points for DEB, RPM, and AppImage                           |
| Container packaging docs | docker/README.md  | Docker development and runtime packaging notes                                     |

---

## 3. Install Layout

The project installs application binaries and Linux desktop metadata under standard Linux filesystem locations.

| Asset Type                 | Installed Location                                                    |
| -------------------------- | --------------------------------------------------------------------- |
| GUI executable             | /usr/bin/truss-analysis                                               |
| CLI executable             | /usr/bin/truss-analysis-cli                                           |
| Desktop entry              | /usr/share/applications/truss-analysis.desktop                        |
| App icon                   | /usr/share/icons/hicolor/scalable/apps/truss-analysis.svg             |
| AppStream metadata         | /usr/share/metainfo/io.github.blackbird410.TrussAnalysis.metainfo.xml |
| README / LICENSE           | /usr/share/doc/2d-truss-analysis/                                     |
| Machine-readable copyright | /usr/share/doc/2d-truss-analysis/copyright                            |
| Debian changelog           | /usr/share/doc/2d-truss-analysis/changelog.Debian.gz                  |
| Example project files      | /usr/share/2d-truss-analysis/examples/                                |
| Config templates           | /etc/2d-truss-analysis/default.json.example                           |
|                            | /etc/2d-truss-analysis/logging.json.example                           |

---

## 4. Debian Packaging Metadata

The Debian packaging surface includes:

| File                       | Role                                                               |
| -------------------------- | ------------------------------------------------------------------ |
| packaging/debian/changelog | Debian changelog (compressed at build time to changelog.Debian.gz) |
| packaging/debian/copyright | Machine-readable license and copyright metadata (DEP-5 format)     |
| packaging/debian/postinst  | Post-install integration hook                                      |
| packaging/debian/postrm    | Post-remove cleanup hook                                           |

Package generation is performed through CPack and helper scripts.

---

## 5. Linux Desktop Integration Assets

| File                                                              | Role                        |
| ----------------------------------------------------------------- | --------------------------- |
| packaging/linux/truss-analysis.desktop                            | Desktop launcher definition |
| packaging/linux/icons/truss-analysis.svg                          | Application icon            |
| packaging/linux/io.github.blackbird410.TrussAnalysis.metainfo.xml | Software center metadata    |

These assets are part of the production distribution path and are included in packaging outputs.

---

## 6. Package Creation Commands

Primary script entry points:

```bash
scripts/package/create-deb.sh
scripts/package/create-rpm.sh
scripts/package/create-appimage.sh
```

Typical workflow:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cd build
cpack -G DEB
```

---

## 7. Runtime Dependency Surfaces

Runtime dependencies are resolved through the build and package toolchain.

Major dependency families:

- Qt6 runtime libraries
- Standard C++ runtime libraries
- System graphics/input stack required by Qt6 widgets

Build dependencies are declared in project manifests and install/setup scripts:

- vcpkg.json
- scripts/install-deps-ubuntu.sh
- scripts/install-deps-fedora.sh
- scripts/dev-setup.sh

---

## 8. Distribution Channels

Supported distribution outputs:

| Channel                    | Artifact                     |
| -------------------------- | ---------------------------- |
| Debian-based distributions | .deb package                 |
| RPM-based distributions    | .rpm package                 |
| Portable Linux runtime     | AppImage                     |
| Source distribution        | Git repository + CMake build |

---

## 9. Validation Checklist

Pre-release packaging validation checklist:

- Binaries build in Release mode
- GUI and CLI executables run from installed paths
- Desktop launcher appears in application menu
- App icon resolves in desktop shell
- AppStream metadata validates and is discoverable
- Package installs and uninstalls cleanly
- Post-install and post-remove hooks execute without errors

---

## 10. Related Documentation

- README.md
- docs/development/development-guide.md
- docker/README.md
- scripts/README.md
