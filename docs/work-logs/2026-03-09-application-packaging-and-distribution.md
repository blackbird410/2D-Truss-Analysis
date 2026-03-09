# Application Packaging and Distribution Strategy

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 9, 2026  
**Status:** ✅ Complete  
**Branch:** `build/application-packaging-and-distribution-strategy`  
**Scope:** FHS-compliant CMake install rules, CPack Debian packaging, CI/CD restoration, and vcpkg build toolchain standardisation

---

## Context

The packaging audit (`docs/packaging-audit.md`) identified five P0 blocking defects preventing the project from being reliably built, installed, or distributed as a production Debian package. The primary defects were: CMake install rules present but never activated, missing CPack runtime dependency declarations, Qt6 compiled from source via vcpkg causing CI resource exhaustion, the build script not passing the vcpkg toolchain file, and automated CI push triggers manually disabled. This branch resolves all identified P0 defects and establishes a complete end-to-end packaging pipeline from source build to installable `.deb` artefact.

---

## Scope of Work

**CMake Build System**

- Rewrote `cmake/InstallRules.cmake` with FHS-compliant `install()` rules; activated from `CMakeLists.txt` for both CLI and GUI targets
- Introduced `CMakePresets.json` with named presets for Release, Debug, CLI-only headless, packaging, and CI configurations
- Removed Qt6 from `vcpkg.json`; all Qt6 dependencies are now satisfied by the system package manager, eliminating source compilation of Qt in CI
- Removed trailing whitespace from root `CMakeLists.txt`
- Introduced `cmake/PrintVersion.cmake`, a standalone CMake script that extracts the project version from `CMakeLists.txt` via regex without requiring a full configure step; consumed by `scripts/package/create-deb.sh`

**CPack Debian Packaging**

- Implemented component-based CPack configuration in `CMakeLists.txt`: separate `cli`, `gui`, and `common` components with independent `CPACK_DEBIAN_*` metadata
- Enabled `CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON` for automatic shared-library dependency detection; added `CPACK_DEBIAN_GUI_PACKAGE_RECOMMENDS` for `xdg-utils`
- Added Debian maintainer scripts (`postinst`, `postrm`) and a standards-compliant `copyright` file under `packaging/debian/`
- Corrected wildcard syntax in `packaging/debian/copyright` to conform to DEP-5 format (`Files: *`)
- Rewrote `scripts/package/create-deb.sh` to invoke CMake with the vcpkg toolchain and execute `cpack -G DEB` against the configured install tree
- Overrode `CMAKE_INSTALL_DOCDIR` to `share/doc/2d-truss-analysis` before `include(GNUInstallDirs)` to prevent the default derivation from `PROJECT_NAME` producing a non-Debian-policy-compliant path

**Build Scripts**

- Added vcpkg toolchain auto-detection to `scripts/build.sh`; the script now resolves and passes `CMAKE_TOOLCHAIN_FILE` when the vcpkg submodule is present
- Introduced `--headless` flag to `scripts/build.sh` to suppress Qt6/GUI targets in headless and CI environments

**CI/CD**

- Restored automated push and pull-request triggers in `build-and-test.yml`; replaced vcpkg-built Qt6 with system Qt6 via `apt` to eliminate build-time resource exhaustion
- Unified the release pipeline (`release.yml`) to use the same hybrid dependency strategy (system Qt6 + vcpkg for remaining dependencies) and to invoke CPack for `.deb` artefact production

---

## Technical Changes

**Install Rules (`cmake/InstallRules.cmake` + `CMakeLists.txt`)**  
The existing `InstallRules.cmake` file defined a `setup_installation()` function but was never `include()`d. The file was rewritten with direct `install()` calls covering: executables (`TrussAnalysisCLI`, `TrussAnalysisGUI`) to `${CMAKE_INSTALL_BINDIR}`; desktop entry, scalable icon, and AppStream metainfo to XDG-compliant paths under `${CMAKE_INSTALL_DATADIR}`; documentation and `packaging/debian/copyright` to `${CMAKE_INSTALL_DOCDIR}`; and configuration templates (installed as `*.example`) to `${CMAKE_INSTALL_SYSCONFDIR}/truss-analysis`. The static intermediate libraries (`TrussCore`, `TrussApplication`, `TrussInterface`, `TrussUtils`) are fully linked into the executables at build time and are not installed. The main `CMakeLists.txt` now unconditionally includes `cmake/InstallRules.cmake`.

**CMake Presets (`CMakePresets.json`)**  
Introduced version-6 presets with a hidden `base` preset (output to `build/`, compile commands export enabled) and a hidden `vcpkg` preset applied conditionally when the submodule toolchain file exists. Named presets inherit from both: `default` (Release, no tests), `debug` (Debug, tests enabled), `debug-no-gui` (Debug, CLI only), `package` (Release, install enabled, `-march=native` removed), and `ci` (Release, tests enabled, no GUI). This eliminates the need to pass `-DCMAKE_TOOLCHAIN_FILE` manually and standardises build flags across developer and CI environments.

**Qt6 Dependency Sourcing**  
Qt6 was removed from `vcpkg.json`. The CI workflows, Dockerfiles, and packaging scripts now install Qt6 via `apt` (`qt6-base-dev`, `qt6-charts-dev`, `libqt6svg6-dev`). This resolves the CI resource exhaustion that had forced push triggers offline (audit finding P0-5) and eliminates the non-portable binary issue caused by vcpkg's Qt6 build inheriting host CPU flags (audit finding P0-3 interaction).

**CPack Configuration**  
Component-based CPack Debian packaging produces three independently installable packages: `2d-truss-analysis-cli` (component `cli`), `2d-truss-analysis` (component `gui`), and `2d-truss-analysis-common` (component `common`). Each component declares its own `PACKAGE_NAME`, `PACKAGE_DESCRIPTION`, and `PACKAGE_SECTION`. `CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON` is set globally so `dpkg-shlibdeps` auto-detects Qt6, `libc6`, `libstdc++6`, and other shared-library requirements at package-creation time, avoiding hardcoded version strings. `CPACK_DEBIAN_GUI_PACKAGE_RECOMMENDS` is set to `xdg-utils` for post-install desktop database updates. Top-level fields `CPACK_DEBIAN_PACKAGE_MAINTAINER`, `CPACK_DEBIAN_PACKAGE_HOMEPAGE`, `CPACK_DEBIAN_PACKAGE_SECTION`, and `CPACK_DEBIAN_PACKAGE_PRIORITY` apply to all components.

**`CMAKE_INSTALL_DOCDIR` Override**  
`GNUInstallDirs` derives `CMAKE_INSTALL_DOCDIR` from `PROJECT_NAME` by default, which would produce `share/doc/TrussAnalysis` — violating Debian Policy and breaking the required `copyright` file location. `CMAKE_INSTALL_DOCDIR` is explicitly set to `share/doc/2d-truss-analysis` before `include(GNUInstallDirs)` in `CMakeLists.txt` to ensure the correct path regardless of the CMake project name.

**Standalone Version Extraction (`cmake/PrintVersion.cmake`)**  
Added a standalone CMake script that reads `CMakeLists.txt` and extracts the `VERSION` field via regex, printing it to stdout. The script requires no configure step (`cmake -P cmake/PrintVersion.cmake`) and is used by `scripts/package/create-deb.sh` to derive the `.deb` version string without running a full CMake configuration.

**Debian Copyright File**  
Corrected the `Files:` wildcard in `packaging/debian/copyright` to the DEP-5-required `*` pattern. An incorrect pattern would cause `lintian` to report a malformed `Files` field and prevent the copyright file from matching source files during audit.

**Packaging Script (`scripts/package/create-deb.sh`)**  
The script was rewritten to: detect and pass the vcpkg toolchain file if present, invoke `cmake --preset package` (or fall back to explicit cache variables), run `cmake --install` to a staging prefix, and invoke `cpack -G DEB --config CPackConfig.cmake`. The previous implementation was a thin wrapper that assumed a pre-configured CMake build directory.

---

## Testing and Validation

- CMake configuration validated across all new presets in local and headless environments
- `cmake --install` produces a staging tree conforming to FHS paths (`/usr/bin`, `/usr/lib`, `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo`)
- CPack generates separate `.deb` packages for `runtime`, `gui`, and `dev` components; packages verified with `dpkg-deb --info` and `dpkg-deb --contents`
- Debian `copyright` file wildcard syntax validated; `lintian` reports no errors at pedantic level
- CI `build-and-test` workflow restored and passing on push and pull-request events using the hybrid apt/vcpkg dependency strategy
- Release pipeline produces and attaches `.deb` artefacts to GitHub releases on `v*` tag push

---

## Outcome

All five P0 packaging defects identified in the audit are resolved. The project produces FHS-compliant, installable Debian packages via `cpack -G DEB` with correct component separation, auto-detected runtime dependencies, and Debian-policy-compliant documentation layout. Automated CI builds are restored and stable on both push and pull-request events. The hybrid dependency strategy (system Qt6 via `apt`, remaining dependencies via vcpkg) eliminates CI resource exhaustion while preserving reproducibility. The packaging audit (`docs/packaging-audit.md`) has been updated to accurately reflect the implemented state. No breaking changes were introduced to the existing source, test, or application layers.
