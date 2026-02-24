# GitHub Actions Workflows

Production-grade CI/CD infrastructure for the 2D Truss Analysis project.

## Active Workflows

### 🏗️ Build and Test ([build-and-test.yml](build-and-test.yml))

**Purpose:** Comprehensive build verification across Debug and Release configurations

**Triggers:**

- Push to `main`, `develop`
- Pull requests to `main`, `develop`
- Manual dispatch

**Features:**

- Matrix builds (Debug/Release)
- vcpkg dependency caching
- BUILD_TESTING validation (prevents false positives)
- Parallel test execution with xvfb for GUI tests
- Test result artifacts (7-day retention)
- Release binary uploads

**Matrix Configuration:**

- Debug: Development builds with symbols
- Release: Optimized production builds

**Validation Steps:**

1. ✅ System dependencies installation
2. ✅ CMake configuration with explicit BUILD_TESTING=ON
3. ✅ Executable verification (CLI, GUI, test binaries)
4. ✅ Test discovery validation (ctest -N check)
5. ✅ Test execution (unit, integration, GUI, GTest)
6. ✅ Test summary generation

---

### 🔍 Static Analysis ([static-analysis.yml](static-analysis.yml))

**Purpose:** Enforce coding standards and detect potential issues

**Triggers:**

- Push to `main`, `develop`
- Pull requests to `main`, `develop`
- Manual dispatch

**Tools:**

1. **clang-format-14** - Code formatting verification
   - Checks all `.cpp` and `.hpp` files
   - Fails on formatting violations
   - Provides diff for fixes

2. **clang-tidy-14** - Static analysis
   - Uses `.clang-tidy` configuration (348 lines)
   - Analyzes all source files (excludes tests, generated code)
   - Reports errors and warnings
   - 30-day artifact retention

3. **cppcheck** - Additional static analysis
   - Comprehensive checks (error, warning, style, performance)
   - Qt macro support (Q_OBJECT, Q_SIGNALS, Q_SLOTS)
   - Excludes corrupted files
   - XML and text output

**Quality Gates:**

- ❌ Fail on formatting violations (hard requirement)
- ⚠️ Report static analysis issues (informational)

---

### 📊 Code Coverage ([coverage.yml](coverage.yml))

**Purpose:** Generate comprehensive code coverage reports

**Triggers:**

- Push to `main`, `develop`
- Pull requests to `main`, `develop`
- Manual dispatch
- Weekly schedule (Monday 00:00 UTC)

**Features:**

- Debug build with `ENABLE_COVERAGE=ON`
- Full test suite execution with xvfb
- lcov/genhtml report generation
- Codecov integration with token support
- Coverage trend tracking on `main` branch
- PR coverage comments

**Coverage Tools:**

- **lcov** - Line and branch coverage capture
- **genhtml** - HTML report generation
- **gcovr** - Additional coverage metrics

**Quality Thresholds:**

- Target: 70% line coverage (warning if below)
- Branch coverage tracking enabled
- Per-directory coverage breakdown

**Artifacts:**

- HTML coverage report (30-day retention)
- Filtered coverage data (.info file)
- Coverage summary text

**Filters:**

- Excludes: `/usr/*`, `build/*`, `tests/*`, `external/*`, `vcpkg_installed/*`
- Excludes: Qt generated files (`moc_*`, `qrc_*`, `*_autogen/*`)

---

### 🎉 Release Automation ([release.yml](release.yml))

**Purpose:** Automated release creation with multi-format packaging

**Triggers:**

- Push tags matching `v*` (e.g., `v3.0.0`)
- Manual dispatch with version input

**Release Pipeline:**

1. **Build Release Binaries** (CLI and GUI)
   - Release configuration (optimized)
   - Strip symbols for smaller binaries
   - Create tarballs with LICENSE and README

2. **Create Debian Package**
   - Uses `scripts/package/create-deb.sh` if available
   - Proper versioning and dependencies
   - Ready for apt repositories

3. **Create GitHub Release**
   - Automatic release notes generation
   - Download instructions in release body
   - Upload all artifacts (tarballs, .deb)
   - Support for draft/pre-release flags

4. **Verify Release Assets**
   - Download and extract CLI tarball
   - Basic smoke test (--help command)
   - Ensures release integrity

**Manual Release Options:**

- `version`: Release version string (e.g., "3.0.0")
- `prerelease`: Mark as pre-release (boolean)
- `draft`: Create as draft (boolean)

**Release Assets:**

- `truss-cli-{version}-linux-x86_64.tar.gz`
- `truss-gui-{version}-linux-x86_64.tar.gz`
- `2d-truss-analysis_{version}_amd64.deb` (if packaging script exists)

---

### 📚 Documentation ([documentation.yml](documentation.yml))

**Purpose:** Automated Doxygen documentation generation and deployment

**Triggers:**

- Push to `main`, `develop`
- Pull requests to `main`, `develop`
- Manual dispatch with optional Pages deployment

**Features:**

- CMake Doxyfile.in template generation
- Quality validation (undocumented APIs check)
- HTML output verification (file count, size)
- Optional link checking
- GitHub Pages deployment (main branch only)
- 90-day artifact retention

**Quality Checks:**

- Undocumented API detection
- Missing @brief tag detection
- Documentation warnings logged
- HTML index.html verification

**Deployment:**

- Automatic Pages deployment on `main` push
- PR documentation preview via artifacts
- Deployment URL comment on PRs

---

### 🐳 Docker ([docker-build.yml](docker-build.yml))

**Purpose:** Container image builds and security scanning

**Triggers:**

- Push tags matching `v*`
- Push to `main`, `develop`
- Pull requests to `main`
- Manual dispatch

**Features:**

- Multi-platform builds (linux/amd64, linux/arm64)
- GitHub Container Registry (ghcr.io) push
- Development image build (Dockerfile.dev)
- Container structure testing
- Trivy security scanning

**Jobs:**

1. **build-and-push**
   - Multi-arch builds via buildx
   - Docker layer caching (GHA cache)
   - Semantic versioning tags
   - SHA-based tags for traceability

2. **test-container**
   - Help command test
   - User/working directory verification
   - container-structure-test validation

3. **security-scan**
   - Trivy vulnerability scanner
   - SARIF output to GitHub Security
   - Critical/High severity focus

---

## Workflow Architecture

### Modular Design

Each workflow has a single responsibility:

- **build-and-test.yml** - Build verification ✅
- **static-analysis.yml** - Code quality ✅
- **coverage.yml** - Coverage tracking ✅
- **release.yml** - Release automation ✅
- **documentation.yml** - Docs generation ✅
- **docker-build.yml** - Container builds ✅

**Benefits:**

- Independent execution (faster feedback)
- Easier maintenance (targeted changes)
- Clear separation of concerns
- Parallel execution where possible

### Dependency Management

**vcpkg Binary Caching:**

- GitHub Actions cache for vcpkg artifacts
- Cache keys include OS, build type, and manifest hash
- Significant build time reduction (5-10 minutes saved)

**Restore Keys Strategy:**

```yaml
restore-keys: |
  vcpkg-${{ runner.os }}-${{ matrix.build_type }}-
  vcpkg-${{ runner.os }}-
```

### Integration with Local Development

All CI workflows leverage existing local infrastructure:

- **Makefile** - High-level targets (`make test`, `make coverage`)
- **scripts/** - Build, test, and analysis scripts
- **CMake** - Consistent configuration
- **.clang-format** - Formatting rules
- **.clang-tidy** - Analysis configuration

**Local-CI Parity:**

```bash
# What CI runs locally:
make clean
make build          # Same as build-and-test.yml
make test           # Same test execution
make lint           # Same as static-analysis.yml
make coverage       # Same as coverage.yml
make format         # Fix issues before CI
```

## Artifact Strategy

### Retention Policies

| Artifact Type    | Retention | Workflow                |
| ---------------- | --------- | ----------------------- |
| Test results     | 7 days    | build-and-test          |
| Release binaries | 7 days    | build-and-test, release |
| Static analysis  | 30 days   | static-analysis         |
| Coverage reports | 30 days   | coverage                |
| Documentation    | 90 days   | documentation           |
| Docker images    | 1 day     | docker-build            |

### Artifact Naming

- `test-results-{Debug|Release}` - Test execution logs
- `binaries-Release` - Release executables
- `clang-tidy-results` - Static analysis output
- `cppcheck-results` - cppcheck XML/text
- `coverage-report-html` - HTML coverage viewer
- `coverage-data` - Coverage .info files
- `doxygen-documentation` - Generated API docs
- `docker-test-image` - Container test artifact

## Quality Gates

### Build and Test

- ✅ All builds must succeed (Debug and Release)
- ✅ All tests must pass (765 tests)
- ✅ Executables must be generated and executable
- ✅ Test discovery must find non-zero tests

### Static Analysis

- ❌ **HARD FAIL:** Code formatting violations
- ⚠️ **WARNING:** clang-tidy errors/warnings
- ⚠️ **WARNING:** cppcheck errors

### Coverage

- ⚠️ **WARNING:** Coverage below 70%
- ✅ Coverage reports generated successfully
- ✅ Codecov upload succeeds

### Release

- ✅ All artifacts must build successfully
- ✅ Binaries must execute (smoke test)
- ✅ GitHub release creation succeeds

## GitHub Branch Protection

Recommended settings for `main` and `develop`:

```yaml
Required status checks:
  - Build & Test (Debug)
  - Build & Test (Release)
  - Format Verification
  - clang-tidy Analysis
  - cppcheck Analysis

Optional checks (informational):
  - Generate Coverage Report
  - Build documentation
```

## Secrets Configuration

Required GitHub Secrets:

- `CODECOV_TOKEN` - Codecov integration (optional but recommended)
- `GITHUB_TOKEN` - Automatic, provided by GitHub

## Continuous Improvement

### Monitoring

- GitHub Actions dashboard for workflow health
- Codecov dashboard for coverage trends
- GitHub Security for Trivy findings

### Future Enhancements

- [ ] Multi-OS builds (macOS, Windows via cross-compilation)
- [ ] Performance regression testing in CI
- [ ] Automated dependency updates (Dependabot/Renovate)
- [ ] Container image signing (Sigstore/Cosign)
- [ ] Release changelog auto-generation (conventional commits)

## Workflow Maintenance

### Updating Workflows

1. Edit workflow YAML files in `.github/workflows/`
2. Test changes on feature branch
3. Verify in PR (workflows run automatically)
4. Merge to `main` after validation

### Local Workflow Testing

Use [act](https://github.com/nektos/act) to run workflows locally:

```bash
# Install act
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Run specific workflow
act -j build-and-test

# Run with secrets
act -s CODECOV_TOKEN=xyz
```

### Debugging Workflow Failures

1. Check workflow run logs in Actions tab
2. Review step-by-step output
3. Download artifacts for detailed analysis
4. Re-run with debug logging:
   ```yaml
   env:
     RUNNER_DEBUG: 1
   ```

## Documentation

- Workflow files are self-documenting with extensive comments
- Each job has descriptive names and step descriptions
- Summary outputs (GitHub Step Summary) provide quick insights
- Artifact names clearly indicate content

---

## Archived Workflows

### ci.yml.disabled (Archived 2026-02-24)

Original monolithic CI workflow (202 lines, fully commented out).
Replaced by modular workflows:

- `build-and-test.yml` - Build and test jobs
- `static-analysis.yml` - Static analysis job
- `coverage.yml` - Coverage reporting (new)
- `release.yml` - Release automation (new)

**Reason for Replacement:**

- Monolithic design difficult to maintain
- Long execution time (sequential jobs)
- Unclear separation of concerns
- Modular design enables:
  - Faster feedback (parallel execution)
  - Easier debugging (isolated failures)
  - Independent updates (change one workflow)
  - Better resource utilization

---

**Last Updated:** February 24, 2026  
**Infrastructure Version:** 3.0.0
