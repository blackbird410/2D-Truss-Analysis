# 2D Truss Analysis — Linux Packaging Audit & Implementation Strategy

**Audit Date:** 2026-03-08  
**Project Version:** 3.0.0  
**Auditor:** Senior C++ / Build-Release Engineer

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Packaging Readiness Analysis](#2-packaging-readiness-analysis)
3. [Critical Issues Preventing Packaging](#3-critical-issues-preventing-packaging)
4. [vcpkg Dependency Management Architecture](#4-vcpkg-dependency-management-architecture)
5. [CMake Build System Standardisation](#5-cmake-build-system-standardisation)
6. [Linux Installation Layout (FHS)](#6-linux-installation-layout-fhs)
7. [Debian Package Strategy](#7-debian-package-strategy)
8. [Distribution Strategy](#8-distribution-strategy)
9. [CI/CD Packaging Pipeline Design](#9-cicd-packaging-pipeline-design)
10. [Phased Implementation Roadmap](#10-phased-implementation-roadmap)

---

## 1. Executive Summary

The project is a mature, well-structured C++20/Qt6 application with clean layered architecture,
comprehensive test coverage (765 passing tests), and existing skeleton infrastructure for packaging.
However, **five blocking defects** prevent it from being reliably built, installed, or distributed
as a production-grade Linux application package.

The core of the problem is not a structural issue — the code quality is high — it is an
**incomplete translation from "developer build" to "distributable artefact"**:

- The installer rules file exists but is never activated.
- The Debian package metadata lacks runtime dependency declarations.
- The compiler flags bake in CPU-specific optimisations that make the binary non-portable.
- vcpkg's use of Qt6 causes CI resource exhaustion and has forced the CI pipeline offline.
- The desktop integration assets produced for Flatpak were never wired into the native package.

This document describes every finding, explains the root cause, and defines the complete
implementation needed to achieve `sudo apt install 2d-truss-analysis`.

---

## 2. Packaging Readiness Analysis

The table below scores each packaging dimension against the requirements for a production `.deb`.

| Dimension             | Current State                                                            | Score      | Finding                                              |
| --------------------- | ------------------------------------------------------------------------ | ---------- | ---------------------------------------------------- |
| Version definition    | `project(TrussAnalysis VERSION 3.0.0)` — correct                         | ✅ Pass    | —                                                    |
| C++ standard          | C++20, enforced via `CXX_STANDARD_REQUIRED ON`                           | ✅ Pass    | —                                                    |
| License file          | `LICENSE` (MIT) present at repository root                               | ✅ Pass    | —                                                    |
| Qt resource embedding | `.qrc` with themes + icons compiled into binary                          | ✅ Pass    | No runtime QSS files to install separately           |
| Target separation     | CLI and GUI are independent executables                                  | ✅ Pass    | —                                                    |
| vcpkg manifest        | `vcpkg.json` with valid `builtin-baseline`                               | ⚠️ Partial | Missing `qtsvg`; Qt6 should not be built from source |
| CMake install rules   | `cmake/InstallRules.cmake` exists but is **never included or called**    | ❌ Fail    | P0-1                                                 |
| Binary portability    | `-march=native` bakes in host CPU instructions                           | ❌ Fail    | P0-3                                                 |
| DEB runtime deps      | `CPACK_DEBIAN_PACKAGE_DEPENDS` is not set                                | ❌ Fail    | P0-2                                                 |
| Desktop entry         | Exists only under `docs/archive/flatpak/`, not installed                 | ❌ Fail    | P1-1                                                 |
| Application icon      | Same — flatpak archive only, not installed                               | ❌ Fail    | P1-2                                                 |
| AppStream metainfo    | Same — flatpak archive only, not installed                               | ❌ Fail    | P1-3                                                 |
| Binary naming         | Executables named `TrussAnalysisCLI` / `TrussAnalysisGUI` (non-standard) | ⚠️ Partial | Debian Policy §10.1 prefers lowercase                |
| RPATH strategy        | `$ORIGIN/../lib` set, but no libraries are installed there               | ⚠️ Partial | Will be cleaned up; system install needs no RPATH    |
| CI stability          | `build-and-test.yml` push triggers **manually disabled**                 | ❌ Fail    | P0-5                                                 |
| Packaging script      | `scripts/package/create-deb.sh` only wraps `cpack -G DEB`                | ⚠️ Partial | Functional once CPack config is fixed                |
| Build script          | `scripts/build.sh` does not pass `CMAKE_TOOLCHAIN_FILE`                  | ❌ Fail    | P0-4                                                 |

---

## 3. Critical Issues Preventing Packaging

Issues are ranked P0 (build-blocking), P1 (package-broken), P2 (should-fix).

### P0-1 — `cmake/InstallRules.cmake` is never loaded

**File:** `cmake/InstallRules.cmake`  
**Root cause:** The file defines a CMake function `setup_installation()` but the main
`CMakeLists.txt` never `include()`s the file and never calls the function.  
The only `install()` calls that execute are two minimal targets at the bottom of
`CMakeLists.txt` that install binaries to a bare `bin/` path without using
`GNUInstallDirs`, and which miss every other asset (desktop entry, icon, config, docs).

**Fix:** Rewrite `cmake/InstallRules.cmake` without the function wrapper so that rules
activate on `include()`. Add `include(GNUInstallDirs)` and
`include(cmake/InstallRules.cmake)` to `CMakeLists.txt` after all targets are defined.
Remove the duplicate minimal `install()` block.

---

### P0-2 — `CPACK_DEBIAN_PACKAGE_DEPENDS` is not set

**File:** `CMakeLists.txt` (bottom CPack section)  
**Root cause:** The CPack configuration declares the generator as `"TGZ;DEB"` but sets
zero Debian runtime dependency metadata. A `.deb` built this way installs cleanly on the
build machine (where Qt6 is already present) but fails silently or crashes on a clean
machine where Qt6 is not installed.

**Fix:** Enable `CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON` (auto-detects `.so` dependencies via
`dpkg-shlibdeps`) and add per-component `CPACK_DEBIAN_<COMP>_PACKAGE_DEPENDS` strings for
packages not traceable by shlibdeps (e.g. `xdg-utils`). Use component-based packaging to
produce separate `2d-truss-analysis` (GUI) and `2d-truss-analysis-cli` (CLI) packages.

---

### P0-3 — `-march=native` produces non-portable binaries

**File:** `CMakeLists.txt`, line 21  
**Root cause:** `-march=native` instructs the compiler to emit instructions for the exact
CPU model of the **build machine**. A binary produced on a CI runner with AVX-512 will
trigger `SIGILL` (Illegal Instruction) on a user machine with an older CPU.  
This makes every packaged binary host-specific by construction.

**Fix:** Remove `-march=native`. Use `-O2` for Release builds via generator expression
(`$<$<CONFIG:Release>:-O2>`). `cpython`, Linux kernel, and every major Debian C++ package
uses `-O2` as the standard Release optimisation level. Leave the `-O3` door open as an
opt-in CMake option for users who build from source for their own machine.

---

### P0-4 — `scripts/build.sh` ignores vcpkg toolchain file

**File:** `scripts/build.sh`  
**Root cause:** The script invokes `cmake` without `-DCMAKE_TOOLCHAIN_FILE`. This means
every local developer build discovers dependencies through the system's package-manager
paths, not through vcpkg. The build only works if all dependencies are installed as
system packages. When a fresh clone is done on a machine without system Eigen3 or
tinyxml2, the configure step fails with no clear error.

**Fix:** Detect the vcpkg toolchain file from `$PROJECT_DIR/vcpkg/` (submodule path) or
`$VCPKG_ROOT`, and pass it to CMake when found. Fall back gracefully to system packages
when vcpkg is not present, with a clear informational message.

---

### P0-5 — CI build-and-test pipeline is manually disabled

**File:** `.github/workflows/build-and-test.yml`  
**Root cause:** The pipeline was disabled on 2026-02-26 due to "vcpkg dependency
resolution issues causing runner instability". The root cause is that `qtbase` in
`vcpkg.json` causes vcpkg to build Qt6 from source, which requires ~20 GB of disk space
and ~60+ minutes of build time on a standard GitHub Actions runner (7 GB RAM, 14 GB
available disk after OS). This exhausts disk space mid-build.

**Fix:** Remove `qtbase` and `qtsvg` from `vcpkg.json`. Install Qt6 from the Ubuntu
system package repository (`apt install qt6-base-dev qt6-svg-dev`) in the CI workflow.
This reduces vcpkg's work to only Eigen3, nlohmann-json, and tinyxml2 — all lightweight —
and brings total CI build time from 60+ minutes down to approximately 12–15 minutes.
Re-enable push and pull_request triggers.

---

### P1-1 — Desktop entry is not installed

The file `docs/archive/flatpak/com.civilengsoft.TrussAnalysis.desktop` was created for
Flatpak distribution and archived. No `.desktop` file is installed to
`/usr/share/applications/` by any CMake rule. Without this, the application does not
appear in GNOME, KDE, or any XDG-compliant application launcher.

**Fix:** Create `packaging/linux/truss-analysis.desktop` using the system binary names
(`truss-analysis-gui`, `truss-analysis` icon). Install it via CMake.

---

### P1-2 — Application icon is not installed

The SVG icon `docs/archive/flatpak/com.civilengsoft.TrussAnalysis.svg` exists but is
never installed by any CMake rule. Without an icon registered in the hicolor theme,
the application appears with a blank/fallback icon in the application launcher.

**Fix:** Copy the icon to `packaging/linux/icons/truss-analysis.svg` and install it to
`/usr/share/icons/hicolor/scalable/apps/truss-analysis.svg`.

---

### P1-3 — AppStream metainfo is not installed

The AppStream XML `docs/archive/flatpak/com.civilengsoft.TrussAnalysis.metainfo.xml`
would enable the application to appear in GNOME Software, KDE Discover, and other
graphical software centres. It is not installed by any CMake rule.

**Fix:** Update metainfo (new app-id, correct binary names, add v3.0.0 release entry)
and install to `/usr/share/metainfo/`.

---

### P2-1 — Static libraries are incorrectly installed to `/usr/lib`

`cmake/InstallRules.cmake`'s `setup_installation()` function (which is never called, but
still wrong) would install `TrussCore` and `TrussApplication` to `/usr/lib`. These are
**build-time intermediate artefacts** with no ABI contract. They should never be
installed to the system; they are statically linked into the final executables.

**Fix:** The rewritten `InstallRules.cmake` only installs executable targets.

---

### P2-2 — Binary names violate Linux naming conventions

`TrussAnalysisCLI` and `TrussAnalysisGUI` use PascalCase, which is unusual for Unix
command-line tools (Debian Policy Manual §10.1). Standard practice is `truss-analysis`
and `truss-analysis-gui`.

**Fix:** Set `OUTPUT_NAME "truss-analysis"` and `OUTPUT_NAME "truss-analysis-gui"` on
the respective targets. The internal CMake target names remain unchanged.

---

### P2-3 — Log file default path will break after installation

`config/logging.json` sets `"path": "data/app.log"`. When the application runs from
`/usr/bin`, this resolves to `/usr/bin/data/app.log` which is unwritable. File logging
is disabled by default (`"enabled": false`) so this does not cause a crash, but enabling
it would produce a confusing error.

**Note:** Fixing this properly requires modifying the C++ logger factory to resolve
`$XDG_DATA_HOME/truss-analysis/app.log` at runtime. The config file default is updated
to a clearly documented placeholder; the behaviour fix is tracked as a follow-up task.

---

## 4. vcpkg Dependency Management Architecture

### 4.1 Current State

```
vcpkg.json
├── qtbase[widgets]   ← builds Qt6 from source (~60 min, ~20 GB) — REMOVE
├── eigen3            ← lightweight, keep
├── nlohmann-json     ← lightweight, keep
└── tinyxml2          ← lightweight, keep
```

`VCPKG_ROOT` is not a Git submodule; CI uses `lukka/run-vcpkg@v11` to clone it.
`build.sh` never passes `CMAKE_TOOLCHAIN_FILE`.

### 4.2 Target Architecture

```
Dependency Source          Package              Reason
─────────────────────────────────────────────────────────────
System apt               Qt6 (runtime + dev)  CI disk budget; DEB Depends alignment
System apt               GTest / GMock         Build-only; well-distributed on Ubuntu
vcpkg (submodule)        eigen3               Header-only; not always in system repos
vcpkg (submodule)        nlohmann-json        Header-only; not always in system repos
vcpkg (submodule)        tinyxml2             Small; consistent version via baseline
```

Removing Qt6 from vcpkg resolves the CI resource problem, the binary caching problem,
and naturally produces a DEB whose `Depends:` field aligns with Ubuntu/Debian official
Qt6 package names (as discovered by `dpkg-shlibdeps`).

### 4.3 Reproducible Builds via vcpkg Baseline

The `"builtin-baseline"` commit pin in `vcpkg.json` ensures that `eigen3`, `nlohmann-json`,
and `tinyxml2` are resolved to exact, locked versions. This baseline must be updated
intentionally (never automatically). The commit `01f602195983451bc83e72f4214af2cbc495aa94`
remains pinned.

### 4.4 vcpkg as Git Submodule

```bash
git submodule add https://github.com/microsoft/vcpkg.git vcpkg
git -C vcpkg checkout 01f602195983451bc83e72f4214af2cbc495aa94
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
```

This ensures every developer and CI job bootstraps from the exact same vcpkg revision
without network-cloning overhead per run.

### 4.5 CMakePresets.json

A `CMakePresets.json` file at the repository root standardises all build configurations,
eliminating per-developer inconsistency and replacing ad-hoc script arguments:

| Preset    | Use Case                  | Key Settings                                      |
| --------- | ------------------------- | ------------------------------------------------- |
| `default` | Development release build | Release, vcpkg toolchain                          |
| `debug`   | Development debug build   | Debug, testing ON, vcpkg toolchain                |
| `package` | CI/CD artefact production | Release, `CMAKE_INSTALL_PREFIX=/usr`, testing OFF |

---

## 5. CMake Build System Standardisation

### 5.1 Compiler Flag Changes

| Flag          | Old                               | New                         | Reason                                            |
| ------------- | --------------------------------- | --------------------------- | ------------------------------------------------- |
| Optimisation  | `-O3 -march=native`               | `$<$<CONFIG:Release>:-O2>`  | Portable binary; `-O3` is implicit for dev opt-in |
| Debug         | _(absent)_                        | `$<$<CONFIG:Debug>:-O0;-g>` | Explicit debug info for sanitiser/debugger use    |
| Warning flags | `add_compile_options(...)` global | Keep as-is                  | Correct                                           |

### 5.2 Install Rules Design

All install rules use `GNUInstallDirs`-defined variables:

| Variable                   | Typical Value                 | Content                       |
| -------------------------- | ----------------------------- | ----------------------------- |
| `CMAKE_INSTALL_BINDIR`     | `bin`                         | Executable files              |
| `CMAKE_INSTALL_DATADIR`    | `share`                       | Architecture-independent data |
| `CMAKE_INSTALL_SYSCONFDIR` | `etc`                         | Configuration templates       |
| `CMAKE_INSTALL_DOCDIR`     | `share/doc/2d-truss-analysis` | Docs, LICENSE                 |

With `CMAKE_INSTALL_PREFIX=/usr`, final paths are:
`/usr/bin/truss-analysis`, `/usr/share/applications/truss-analysis.desktop`, etc.

### 5.3 Component-Based Installation

CPack components allow producing separate CLI and GUI packages from a single build:

```
Component: cli     → 2d-truss-analysis-cli_3.0.0_amd64.deb
Component: gui     → 2d-truss-analysis_3.0.0_amd64.deb
Component: common  → 2d-truss-analysis-common_3.0.0_amd64.deb
```

The GUI package declares the common and cli packages as dependencies.

### 5.4 Standard Build Commands

After the fixes, the three canonical build commands work correctly:

```bash
cmake -B build --preset package
cmake --build build
cmake --install build --prefix /usr    # or let CPack handle it
cd build && cpack -G DEB
```

---

## 6. Linux Installation Layout (FHS)

```
/usr/
├── bin/
│   ├── truss-analysis             ← CLI (component: cli)
│   └── truss-analysis-gui         ← GUI (component: gui)
└── share/
    ├── applications/
    │   └── truss-analysis.desktop             ← component: gui
    ├── doc/
    │   └── 2d-truss-analysis/
    │       ├── README.md                      ← component: common
    │       └── copyright                      ← component: common (Debian required)
    ├── icons/
    │   └── hicolor/
    │       └── scalable/
    │           └── apps/
    │               └── truss-analysis.svg     ← component: gui
    ├── metainfo/
    │   └── io.github.blackbird410.TrussAnalysis.metainfo.xml  ← component: gui
    └── truss-analysis/
        └── examples/                          ← component: common
            ├── bridge-truss/
            ├── simple-triangular-truss/
            └── tower-structure/

/etc/
└── truss-analysis/
    ├── default.json.example                   ← component: common
    └── logging.json.example                   ← component: common
```

---

## 7. Debian Package Strategy

### 7.1 Toolchain Decision: CPack DEB (primary) + debhelper (future PPA)

CPack DEB generates a valid installable `.deb` with minimal overhead, is CMake-native,
and is sufficient for GitHub Release distribution. For a Launchpad PPA (which requires
source packages), a `debian/` directory using `debhelper` is needed — this is a future
milestone and does not block the initial release.

### 7.2 Runtime Dependency Strategy

**`CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON`** invokes `dpkg-shlibdeps` on the installed
binaries at package-creation time. This automatically detects:

- `libqt6core6`, `libqt6widgets6`, `libqt6svg6` and their exact minimum versions
- `libc6`, `libstdc++6`
- `libgcc-s1` and other GCC runtime libraries

This approach is version-agnostic (works on Ubuntu 22.04, 24.04, and derivatives without
hardcoding version strings) and is the standard used by every official Debian package.

### 7.3 Package Files

| File                                                                | Purpose                                                                  |
| ------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| `packaging/debian/postinst`                                         | Runs `update-desktop-database` and `gtk-update-icon-cache` after install |
| `packaging/debian/postrm`                                           | Runs `update-desktop-database` after removal                             |
| `packaging/linux/truss-analysis.desktop`                            | XDG application entry                                                    |
| `packaging/linux/icons/truss-analysis.svg`                          | Scalable application icon                                                |
| `packaging/linux/io.github.blackbird410.TrussAnalysis.metainfo.xml` | AppStream metadata                                                       |

### 7.4 Building the Package

```bash
# Configure with package preset (sets CMAKE_INSTALL_PREFIX=/usr)
cmake -B build --preset package \
      -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --parallel $(nproc)

# Create DEB (runs dpkg-shlibdeps, packages assets, applies postinst)
cd build && cpack -G DEB

# Output:
#   2d-truss-analysis-cli_3.0.0_amd64.deb
#   2d-truss-analysis_3.0.0_amd64.deb
#   2d-truss-analysis-common_3.0.0_amd64.deb
```

---

## 8. Distribution Strategy

### 8.1 Recommended: Dual-track

**Track 1 — GitHub Release binary packages (immediate)**

Trigger: push of a `v*` tag  
Artefacts: `.deb` per component + `.tar.gz` tarball  
Installation: `sudo dpkg -i *.deb && sudo apt-get install -f`  
Maintenance: zero — fully automated via GitHub Actions

**Track 2 — Launchpad PPA (3–6 months)**

Enables `sudo add-apt-repository ppa:blackbird410/2d-truss-analysis`.  
Requires: Launchpad account, GPG signing key, `debian/` directory.  
The `packaging/debian/` directory created in this audit provides the foundation.

**Track 3 — Flathub (optional, 6+ months)**

The `docs/archive/flatpak/` manifest provides a solid starting point.  
Update the manifest tag to `v3.0.0`, update binary names, submit to Flathub.  
Provides full sandboxing and universal Linux distribution.

### 8.2 Not Recommended

Building Qt6 via vcpkg and bundling it in a `.deb` would produce a conflict-prone
package that ships its own Qt alongside the system's Qt, violating Debian Policy §8.

---

## 9. CI/CD Packaging Pipeline Design

### 9.1 Revised Workflow Architecture

```
Push to main/develop  →  ci.yml
                           ├── matrix: [Debug, Release]
                           ├── apt: Qt6, GTest (fast, ~2 min)
                           ├── vcpkg: eigen3, nlohmann-json, tinyxml2 (cached, ~1 min)
                           ├── cmake --build
                           └── xvfb-run ctest

Push v* tag          →  release.yml
                           ├── build-packages job (ubuntu-22.04)
                           │    ├── same apt/vcpkg setup as ci.yml
                           │    ├── cmake --preset package
                           │    └── cpack -G DEB;TGZ
                           └── create-release job
                                ├── download artefacts
                                └── softprops/action-gh-release
```

### 9.2 Key Changes from Current State

| Aspect                  | Before                       | After                                  |
| ----------------------- | ---------------------------- | -------------------------------------- |
| Qt6 source              | vcpkg (builds from source)   | `apt install qt6-base-dev qt6-svg-dev` |
| CI build time           | 60–90 min (OOM/disk failure) | ~12–15 min                             |
| Push trigger            | Disabled                     | Enabled                                |
| DEB packaging           | Script stub (no-op)          | Full CPack DEB pipeline                |
| Binary names in release | `TrussAnalysisCLI`           | `truss-analysis`                       |

### 9.3 vcpkg Binary Caching in CI

```yaml
- name: Cache vcpkg installed packages
  uses: actions/cache@v4
  with:
    path: vcpkg/installed
    key: vcpkg-${{ hashFiles('vcpkg.json') }}-${{ runner.os }}
    restore-keys: vcpkg-
```

Since eigen3, nlohmann-json, and tinyxml2 are header-only or small, they build quickly
even on cache miss. The cache hit rate will be high because `vcpkg.json` rarely changes.

---

## 10. Phased Implementation Roadmap

### Phase 1 — Project Readiness (Immediate)

| #   | Task                                                          | File(s) Changed                | Expected Result                     |
| --- | ------------------------------------------------------------- | ------------------------------ | ----------------------------------- |
| 1.1 | Remove `-march=native`; use generator-expression optimisation | `CMakeLists.txt`               | Portable Release binary             |
| 1.2 | Rename binary outputs to lowercase                            | `CMakeLists.txt` (OUTPUT_NAME) | Unix-compliant executable names     |
| 1.3 | Remove `qtbase`/`qtsvg` from vcpkg.json                       | `vcpkg.json`                   | Qt6 sourced from system apt         |
| 1.4 | Create `packaging/` directory with all assets                 | New files                      | Desktop entry, icon, metainfo ready |
| 1.5 | Fix `config/logging.json` file path comment                   | `config/logging.json`          | Document XDG expectation            |

### Phase 2 — vcpkg Integration (Day 1–2)

| #   | Task                                               | File(s) Changed         | Expected Result             |
| --- | -------------------------------------------------- | ----------------------- | --------------------------- |
| 2.1 | Add vcpkg as Git submodule                         | `.gitmodules`, `vcpkg/` | Reproducible local builds   |
| 2.2 | Create `CMakePresets.json`                         | New file                | Standard build entry points |
| 2.3 | Update `scripts/build.sh` with toolchain detection | `scripts/build.sh`      | Local vcpkg builds work     |
| 2.4 | Update build documentation in README               | `README.md`             | Clear developer on-boarding |

### Phase 3 — CMake Install Configuration (Day 2–3)

| #   | Task                                                                              | File(s) Changed            | Expected Result                 |
| --- | --------------------------------------------------------------------------------- | -------------------------- | ------------------------------- |
| 3.1 | Rewrite `cmake/InstallRules.cmake` (no function, GNUInstallDirs)                  | `cmake/InstallRules.cmake` | Proper FHS layout               |
| 3.2 | Add `include(GNUInstallDirs)` + `include(cmake/InstallRules.cmake)` to main CMake | `CMakeLists.txt`           | Install rules actually activate |
| 3.3 | Remove broken RPATH and duplicate install() calls                                 | `CMakeLists.txt`           | Clean system install            |
| 3.4 | Validate: `cmake --install build --prefix /tmp/test-install`                      | _(local verification)_     | Correct FHS tree output         |

### Phase 4 — Packaging Configuration (Day 3–4)

| #   | Task                                                                 | File(s) Changed  | Expected Result                  |
| --- | -------------------------------------------------------------------- | ---------------- | -------------------------------- |
| 4.1 | Full CPack DEB config: SHLIBDEPS, components, control files          | `CMakeLists.txt` | Valid `.deb` with real Depends   |
| 4.2 | Create `packaging/debian/postinst` and `postrm`                      | New files        | Desktop DB updated after install |
| 4.3 | Test: `cd build && cpack -G DEB`                                     | _(local)_        | `.deb` created without errors    |
| 4.4 | Test: `dpkg -i *.deb && apt-get install -f` on clean Ubuntu 22.04 VM | _(local)_        | Clean installation, app launches |

### Phase 5 — CI/CD Pipeline (Day 4–5)

| #   | Task                                                              | File(s) Changed                        | Expected Result                                |
| --- | ----------------------------------------------------------------- | -------------------------------------- | ---------------------------------------------- |
| 5.1 | Rewrite `build-and-test.yml`: system Qt6, re-enable push triggers | `.github/workflows/build-and-test.yml` | CI passes in <15 min on every PR               |
| 5.2 | Rewrite `release.yml`: end-to-end DEB + TGZ packaging             | `.github/workflows/release.yml`        | Tag push → GitHub Release with `.deb` attached |
| 5.3 | Validate with test tag push                                       | _(CI validation)_                      | End-to-end packaging confirmed                 |

### Phase 6 — Release & Distribution (Ongoing)

| #   | Task                                                                   | Timeline                  |
| --- | ---------------------------------------------------------------------- | ------------------------- |
| 6.1 | Tag and publish v3.0.0 GitHub Release with `.deb` artefacts            | Immediately after Phase 5 |
| 6.2 | Update README installation section with `dpkg -i` instructions         | Immediately               |
| 6.3 | Create `debian/` source package layout for Launchpad                   | 3 months                  |
| 6.4 | Register Launchpad PPA and upload source package                       | 3–4 months                |
| 6.5 | Update Flatpak manifest (v3.0.0 tag, new binary names), submit Flathub | 6 months                  |

---

## Appendix A — Files Created or Modified by This Implementation

| Action       | Path                                                                |
| ------------ | ------------------------------------------------------------------- |
| **CREATED**  | `docs/packaging-audit.md`                                           |
| **CREATED**  | `CMakePresets.json`                                                 |
| **CREATED**  | `packaging/linux/truss-analysis.desktop`                            |
| **CREATED**  | `packaging/linux/icons/truss-analysis.svg`                          |
| **CREATED**  | `packaging/linux/io.github.blackbird410.TrussAnalysis.metainfo.xml` |
| **CREATED**  | `packaging/debian/postinst`                                         |
| **CREATED**  | `packaging/debian/postrm`                                           |
| **CREATED**  | `packaging/debian/copyright`                                        |
| **MODIFIED** | `CMakeLists.txt`                                                    |
| **MODIFIED** | `cmake/InstallRules.cmake`                                          |
| **MODIFIED** | `vcpkg.json`                                                        |
| **MODIFIED** | `scripts/build.sh`                                                  |
| **MODIFIED** | `scripts/package/create-deb.sh`                                     |
| **MODIFIED** | `.github/workflows/build-and-test.yml`                              |
| **MODIFIED** | `.github/workflows/release.yml`                                     |

---

_This document is the authoritative audit record for the 2D Truss Analysis packaging
initiative. All implementation decisions reference findings in this document._
