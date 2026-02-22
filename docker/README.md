# Docker Container Guide

This directory contains Docker configurations for the 2D Truss Analysis application.

## Quick Start

### Build and Run Production Container

```bash
# Build the production image
make docker-build

# Run analysis
make docker-run

# Or with manual docker command
docker run --rm -it \
  -v $(pwd)/tests/fixtures/sample_data:/data/input:ro \
  -v $(pwd)/output:/data/output:rw \
  truss-analysis:latest \
  --input /data/input/model.json \
  --output /data/output/results.csv
```

### Development Container

```bash
# Start development environment
make docker-dev

# Or manually with docker compose
docker compose run --rm truss-dev

# Inside the container, configure and build:
# Option 1: CLI-only (faster, no Qt6)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=OFF
cmake --build build -j$(nproc)
./build/TrussAnalysisCLI --help

# Option 2: Full build with GUI
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=ON
cmake --build build -j$(nproc)
./build/TrussAnalysisCLI example

# Option 3: Use Make wrapper (after configuring)
cmake -B build -DBUILD_GUI=OFF  # Configure first
make build                       # Then use Make targets
make test
```

## Container Images

### Production Image: `Dockerfile`

**Purpose:** Minimal runtime environment for the CLI application

**Features:**
- Multi-stage build (builder + runtime)
- Optimized for size (target < 500MB)
- Non-root user execution
- CLI-only (no Qt6 GUI dependencies)
- Health checks included

**Usage:**
```bash
docker run --rm truss-analysis:latest --help
```

### Development Image: `Dockerfile.dev`

**Purpose:** Full development environment with all tools

**Base Image:** Ubuntu 24.04.1 LTS (Noble Numbat)

**Features:**
- Complete build toolchain (gcc, cmake, ninja)
- All dependencies (Eigen3, Qt6 with Charts, nlohmann_json, tinyxml2)
- Debugging tools (gdb, valgrind)
- Static analysis (clang-tidy, cppcheck)
- Code coverage tools (lcov, gcovr)
- Interactive development environment
- Automated setup script (`setup` command)

**Usage:**
```bash
docker compose run --rm truss-dev
```

## Makefile Targets

| Target | Description |
|--------|-------------|
| `make docker-build` | Build production image |
| `make docker-build-dev` | Build development image |
| `make docker-run` | Run analysis in container |
| `make docker-dev` | Start development container |
| `make docker-shell` | Open shell in container |
| `make docker-test` | Test container functionality |
| `make docker-push` | Push to container registry |
| `make docker-clean` | Remove images and clean up |
| `make docker-size` | Show image sizes |

## Container Configuration

### Environment Variables

**Production Container:**
- `TRUSS_LOG_LEVEL` - Logging level (default: info)

**Development Container:**
- `CMAKE_BUILD_TYPE` - Build type (Debug/Release)
- `CC` - C compiler (default: gcc)
- `CXX` - C++ compiler (default: g++)

### Volume Mounts

**Input Data:**
```bash
-v /path/to/input:/data/input:ro  # Read-only input files
```

**Output Data:**
```bash
-v /path/to/output:/data/output:rw  # Read-write output directory
```

**Development Workspace:**
```bash
-v $(pwd):/workspace:rw  # Full workspace access
```

## Testing Containers

### Manual Tests

```bash
# Build test image
docker build -t truss-analysis:test -f docker/Dockerfile .

# Test help command
docker run --rm truss-analysis:test --help

# Test with sample data
docker run --rm \
  -v $(pwd)/tests/fixtures/sample_data:/data/input:ro \
  truss-analysis:test \
  --input /data/input/simple_truss.json
```

### Automated Tests

The project includes container structure tests:

```bash
# Install container-structure-test
curl -LO https://storage.googleapis.com/container-structure-test/latest/container-structure-test-linux-amd64
chmod +x container-structure-test-linux-amd64
sudo mv container-structure-test-linux-amd64 /usr/local/bin/container-structure-test

# Run tests
container-structure-test test \
  --image truss-analysis:latest \
  --config tests/container-test.yaml
```

## Security

### Best Practices Implemented

- ✅ Non-root user (`truss`, UID 1000)
- ✅ Minimal base image (Ubuntu 22.04)
- ✅ Multi-stage builds (separate builder and runtime)
- ✅ No unnecessary packages in runtime
- ✅ Read-only input volumes
- ✅ Health checks
- ✅ Container labels (OCI standard)
- ✅ No secrets in image

### Vulnerability Scanning

```bash
# Using Docker scan
docker scan truss-analysis:latest

# Using Trivy
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
  aquasec/trivy image truss-analysis:latest
```

## Size Optimization

### Current Image Sizes

| Image | Size | Description |
|-------|------|-------------|
| Production | ~450MB | CLI runtime only |
| Development | ~2.5GB | Full dev environment |

### Optimization Techniques

1. **Multi-stage builds** - Separate build and runtime stages
2. **Minimal dependencies** - Only runtime libs in final image
3. **Layer caching** - Ordered from least to most frequently changing
4. **Binary stripping** - Remove debug symbols in release builds
5. **Cleanup** - Remove package lists and cache after install

## CI/CD Integration

### GitHub Actions

The project includes automated Docker builds:

**Workflow:** `.github/workflows/docker-build.yml`

**Triggers:**
- Push to main/develop branches
- Version tags (`v*`)
- Pull requests

**Actions:**
- Build production and dev images
- Run container tests
- Security scanning with Trivy
- Push to GitHub Container Registry

### Manual Registry Push

```bash
# Login to GitHub Container Registry
echo $GITHUB_TOKEN | docker login ghcr.io -u USERNAME --password-stdin

# Tag and push
docker tag truss-analysis:latest ghcr.io/blackbird410/truss-analysis:latest
docker push ghcr.io/blackbird410/truss-analysis:latest

# Or use Makefile
make docker-push
```

## Troubleshooting

### Container Fails to Build

**Issue:** Missing dependencies or build failures

**Solution:**
```bash
# Clean and rebuild
make docker-clean
make docker-build

# Check build logs
docker build --no-cache -f docker/Dockerfile .
```

### Permission Denied

**Issue:** Cannot write to output directory

**Solution:**
```bash
# Ensure output directory exists and has correct permissions
mkdir -p output
chmod 777 output

# Or run as current user
docker run --rm --user $(id -u):$(id -g) \
  -v $(pwd)/output:/data/output:rw \
  truss-analysis:latest
```

### Image Too Large

**Issue:** Production image exceeds target size

**Solution:**
```bash
# Check image layers
docker history truss-analysis:latest

# Analyze with dive
docker run --rm -it \
  -v /var/run/docker.sock:/var/run/docker.sock \
  wagoodman/dive:latest truss-analysis:latest
```

## Development Workflow

### Typical Development Session

```bash
# 1. Start development container
make docker-dev

# 2. Inside container - configure build (IMPORTANT: do this first!)
# For CLI-only development (faster):
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=OFF

# OR for full development with GUI:
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=ON

# 3. Build the project
cmake --build build -j$(nproc)

# 4. Run tests
cd build && ctest --output-on-failure
# Or use the Makefile wrapper:
cd .. && make test

# 5. Run application
./build/TrussAnalysisCLI --help
./build/TrussAnalysisCLI example

# 6. Make changes (in host editor, mounted volume reflects changes)

# 7. Rebuild incrementally
cmake --build build -j$(nproc)
# Or: make build

# 8. Exit container
exit
```

**Quick Start Script:**

Create a file `dev-setup.sh` in the container:

```bash
#!/bin/bash
# Quick setup script for development container
echo "Configuring build..."
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=OFF \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
echo "Building..."
cmake --build build -j$(nproc)
echo "Build complete! Run './build/TrussAnalysisCLI --help' to test."
```

### Build Cache Optimization

Docker Compose uses a named volume for build cache:

```bash
# View build cache volume
docker volume inspect 2d-truss-analysis-cpp_build-cache

# Clean build cache
docker volume rm 2d-truss-analysis-cpp_build-cache
```

## References

- [Dockerfile Reference](https://docs.docker.com/engine/reference/builder/)
- [Docker Compose Specification](https://docs.docker.com/compose/compose-file/)
- [Container Structure Tests](https://github.com/GoogleContainerTools/container-structure-test)
- [Trivy Security Scanner](https://github.com/aquasecurity/trivy)
- [OCI Image Spec](https://github.com/opencontainers/image-spec/blob/main/annotations.md)

---

**Maintained by:** Neil Taison Rigaud  
**Last Updated:** February 2026
