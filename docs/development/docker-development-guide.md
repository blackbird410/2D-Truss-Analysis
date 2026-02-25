# Docker Development Environment - Quick Reference

## Building the Development Image

```bash
make docker-build-dev
```

This will:

1. Build the Docker image with all development tools
2. Automatically run the setup script to configure and build the project
3. Create `truss-analysis:dev` image ready for use

## Using the Development Container

### Interactive Development Session

```bash
docker run --rm -it -v $(pwd):/workspace truss-analysis:dev
```

Then inside the container:

```bash
# Run tests
cd build && ctest

# Run clang-tidy on a file
clang-tidy -p build src/core/model/node.cpp

# Rebuild after code changes
cmake --build build -j$(nproc)

# Exit
exit
```

### One-off Commands

```bash
# Run tests
docker run --rm -v $(pwd):/workspace -w /workspace/build truss-analysis:dev ctest

# Run clang-tidy on entire project
docker run --rm -v $(pwd):/workspace truss-analysis:dev \
  bash -c "cd /workspace && make lint"

# Run setup script
docker run --rm -v $(pwd):/workspace truss-analysis:dev setup --install-tools
```

## Environment Details

### Tools Available in Development Image

- **Compilers:** gcc 13.3, clang 18
- **Build:** CMake 3.28, Ninja 1.11
- **Analysis:** clang-tidy 18, clang-format 18, cppcheck 2.13
- **Debugging:** gdb 15, valgrind 3.22
- **Testing:** GoogleTest 1.14, GoogleMock (built from source)
- **Coverage:** lcov 2.0, gcovr 7.0
- **Libraries:** Qt6 6.4.2, Eigen3 3.4.0, TinyXML2 10.0, nlohmann-json 3.11

### Build Types

The setup script offers two build configurations:

1. **CLI-only (Default, Option 1)**
   - Faster compilation
   - No Qt6 dependencies
   - Minimal binary size
   - Best for headless/server use
   - `BUILD_GUI=OFF`

2. **Full GUI Build (Option 2)**
   - Includes Qt6 GUI application
   - Longer compilation time
   - Full feature set
   - `BUILD_GUI=ON`

### Automatic Setup

The `setup` command (or `make docker-build-dev`) automatically:

1. Updates package lists
2. Installs all development tools
3. Removes stale build directory (prevents path conflicts)
4. Prompts for build configuration (CLI vs GUI)
5. Configures CMake with appropriate flags
6. Builds the project
7. Shows build completion summary

### Manual Configuration

To rebuild manually with specific options:

```bash
docker run --rm -v $(pwd):/workspace -w /workspace truss-analysis:dev bash -c "
  rm -rf build
  cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_GUI=OFF \
    -DBUILD_TESTING=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build build -j\$(nproc)
"
```

## Clang-Tidy Analysis Workflow

### Quick Check on One File

```bash
docker run --rm -v $(pwd):/workspace -w /workspace truss-analysis:dev \
  clang-tidy -p build src/core/model/node.cpp
```

### Full Project Analysis

```bash
docker run --rm -v $(pwd):/workspace -w /workspace truss-analysis:dev \
  make lint 2>&1 | tee analysis-results.txt
```

### Interactive Exploration

```bash
docker run --rm -it -v $(pwd):/workspace truss-analysis:dev

# Inside container:
cd /workspace
make lint | head -100    # See first 100 issues
make lint | grep "warning: uninitialized" | wc -l  # Count specific issues
```

## Development Workflow

### Edit → Compile → Test Cycle

**From Host Machine:**

```bash
# Edit code in your IDE/editor (e.g., VS Code)
# Then in terminal:

docker run --rm -v $(pwd):/workspace -w /workspace/build \
  truss-analysis:dev cmake --build . -j$(nproc)
```

**Or Interactive Session:**

```bash
docker run --rm -it -v $(pwd):/workspace truss-analysis:dev

# Inside container, do multiple edits and tests without rebuilding image
cmake --build build -j$(nproc)  # after each code change
ctest                           # run tests
```

## Troubleshooting

### "Cannot write to /workspace"

- Ensure Docker daemon has sufficient permissions
- Try: `docker run --rm -v $(pwd):/workspace --user root truss-analysis:dev`

### Build fails with "CMakeCache.txt" error

- The setup script automatically cleans stale builds
- If manual build fails: `docker run --rm -v $(pwd):/workspace truss-analysis:dev rm -rf /workspace/build`

### clang-tidy reports "header not found"

- This is expected - the Docker LLVM 18 might have different include paths than your local compiler
- The analysis results are still valid for code issues
- Filter to relevant headers: `clang-tidy -p build --header-filter='src/.*' file.cpp`

### Image build takes too long

- First build downloads packages and compiles GoogleTest (~2-3 minutes)
- Subsequent builds use Docker cache and are much faster
- Add `--no-cache` flag to rebuild from scratch: `docker build --no-cache -f docker/Dockerfile.dev -t truss-analysis:dev .`

## See Also

- [Development Docker Container Documentation](../../docker/README.md)
- [Build System Overview](../../README.md#build)
- [CI/CD Workflow with Docker Support](../development/ci-cd-workflow.md)
